// xmi2smf unit tests (engine/audio/xmi2smf.c): Miles XMIDI -> SMF
// conversion.  Fixtures are built in-memory — FORM/XMID chunking, EVNT
// delay-byte runs, TIMB patch maps — and the emitted SMF bytes are
// checked exactly (tempo, deltas, materialized note-offs, EOT).

#include "../harness.h"
#include <vector>
#include <cstring>
#include <cstdint>
#include <cstdlib>

extern "C" int xmi_to_smf(const void* in, size_t inLen, int seq,
                          uint8_t** out, size_t* outLen);

namespace {

using V = std::vector<uint8_t>;

void be32out(V& v, uint32_t x) {
    v.push_back((uint8_t)(x >> 24)); v.push_back((uint8_t)(x >> 16));
    v.push_back((uint8_t)(x >> 8));  v.push_back((uint8_t)x);
}
void id4(V& v, const char id[4]) { v.insert(v.end(), id, id + 4); }

// id + be32(len) + data + pad-byte when len is odd (IFF 2-alignment)
V chunk(const char id[4], const V& data) {
    V v; id4(v, id); be32out(v, (uint32_t)data.size());
    v.insert(v.end(), data.begin(), data.end());
    if (data.size() & 1) v.push_back(0);
    return v;
}
// FORM child chunk: FORM <len> <type> <inner-chunks>
V form(const char type[4], const V& inner) {
    V body; id4(body, type); body.insert(body.end(), inner.begin(), inner.end());
    return chunk("FORM", body);
}

// Top-level file: FORM <len> "XDIR" <children>
V xmi_file(const V& children) {
    V v; id4(v, "FORM");
    V body; id4(body, "XDIR"); body.insert(body.end(), children.begin(), children.end());
    be32out(v, (uint32_t)body.size());
    v.insert(v.end(), body.begin(), body.end());
    return v;
}

V xmid(const V& inner) { return form("XMID", inner); }
V evnt(const V& events) { return chunk("EVNT", events); }
V cat(V a, const V& b) { a.insert(a.end(), b.begin(), b.end()); return a; }
V timb(std::initializer_list<uint16_t> pairs) {  // prog,patch u16le pairs
    V d;
    for (auto it = pairs.begin(); it != pairs.end(); it += 2) {
        uint16_t prog = it[0], patch = it[1];
        d.push_back((uint8_t)prog);        d.push_back((uint8_t)(prog >> 8));
        d.push_back((uint8_t)patch);       d.push_back((uint8_t)(patch >> 8));
    }
    return chunk("TIMB", d);
}

struct Smf {
    V bytes;
    bool Parse(const uint8_t* p, size_t n) {
        if (!p || n < 22) return false;         // 14 hdr + 8 trk hdr
        if (std::memcmp(p, "MThd", 4) || std::memcmp(p + 14, "MTrk", 4))
            return false;
        bytes.assign(p, p + n);
        return true;
    }
    // Track-body helpers return "not found" when no SMF was parsed —
    // CHECK failures must report, never segv on an empty buffer.
    const uint8_t* track() const {
        return bytes.size() >= 22 ? bytes.data() + 22 : nullptr;
    }
    size_t trackLen() const { return bytes.size() >= 22 ? bytes.size() - 22 : 0; }
    // Does the track body contain the byte subsequence?
    bool contains(std::initializer_list<int> seq) const {
        V want; for (int b : seq) want.push_back((uint8_t)b);
        const uint8_t* t = track(); size_t tn = trackLen();
        if (!t) return false;
        for (size_t i = 0; i + want.size() <= tn; ++i)
            if (!std::memcmp(t + i, want.data(), want.size())) return true;
        return false;
    }
    size_t count(std::initializer_list<int> seq) const {
        V want; for (int b : seq) want.push_back((uint8_t)b);
        const uint8_t* t = track(); size_t tn = trackLen();
        if (!t) return 0;
        size_t c = 0;
        for (size_t i = 0; i + want.size() <= tn; ++i)
            if (!std::memcmp(t + i, want.data(), want.size())) ++c;
        return c;
    }
    // File-level prefix match (for the MThd header itself).
    bool headerIs(std::initializer_list<int> seq) const {
        V want; for (int b : seq) want.push_back((uint8_t)b);
        return bytes.size() >= want.size() &&
               !std::memcmp(bytes.data(), want.data(), want.size());
    }
};

int Run(const V& xmi, int seq, Smf& smf) {
    uint8_t* out = nullptr; size_t outLen = 0;
    int rc = xmi_to_smf(xmi.data(), xmi.size(), seq, &out, &outLen);
    if (rc == 0) { CHECK(smf.Parse(out, outLen)); free(out); }
    else CHECK(out == nullptr);
    return rc;
}

} // namespace

// Header: MThd fmt0 / 1 track / division 120 (1 SMF tick == 1 XMI tick).
REGISTER_TEST(xmi_header_and_tempo)
{
    V xmi = xmi_file(xmid(evnt({0x90, 0x40, 0x64, 0x10})));
    Smf smf; CHECK_EQ(Run(xmi, 0, smf), 0);
    CHECK(smf.headerIs({'M','T','h','d',0,0,0,6,0,0,0,1,0,120}));
    // fixed 1,000,000 us/qn tempo at delta 0, track start
    CHECK(smf.contains({0x00,0xFF,0x51,0x03,0x0F,0x42,0x40}));
    CHECK(smf.contains({0x00,0xFF,0x2F,0x00}));           // EOT
}

// 0x9n + varlen duration -> real 0x8n note-off materialized at on+dur.
REGISTER_TEST(xmi_noteoff_materialized)
{
    V xmi = xmi_file(xmid(evnt({0x90, 0x3C, 0x64, 0x78})));  // dur=120
    Smf smf; CHECK_EQ(Run(xmi, 0, smf), 0);
    CHECK(smf.contains({0x00,0x90,0x3C,0x64}));            // on @0
    CHECK(smf.contains({0x78,0x80,0x3C,0x40}));            // off @120 vel64
}

// Overlapping notes: offs must sort by expiry, deltas stay relative.
REGISTER_TEST(xmi_overlapping_offs_sorted)
{
    V xmi = xmi_file(xmid(evnt({
        0x90, 0x40, 0x64, 0x7F,            // on n64 @0   dur127 -> off@127
        0x0A,                              // delay 10
        0x90, 0x41, 0x64, 0x7F             // on n65 @10  dur127 -> off@137
    })));
    Smf smf; CHECK_EQ(Run(xmi, 0, smf), 0);
    CHECK(smf.contains({0x00,0x90,0x40,0x64}));            // on n64 @0
    CHECK(smf.contains({0x0A,0x90,0x41,0x64}));            // on n65 @10
    CHECK(smf.contains({0x75,0x80,0x40,0x40}));            // off n64 @127 (d117)
    CHECK(smf.contains({0x0A,0x80,0x41,0x40}));            // off n65 @137 (d10)
}

// TIMB maps XMI program numbers onto GM patches for 0xCn events.
REGISTER_TEST(xmi_timb_remap)
{
    V xmi = xmi_file(xmid(cat(timb({10, 42}),
                             evnt({0xC0, 0x0A, 0xC0, 0x0B})))); // prog10->42, prog11 stays
    Smf smf; CHECK_EQ(Run(xmi, 0, smf), 0);
    CHECK(smf.contains({0x00,0xC0,0x2A}));               // remapped
    CHECK(smf.contains({0x00,0xC0,0x0B}));               // untouched
}

// CAT container: seq picks one of several FORM XMID sequences.
REGISTER_TEST(xmi_cat_seq_select)
{
    V catBody; id4(catBody, "XMID");
    catBody = cat(cat(catBody, xmid(evnt({0x90, 0x3C, 0x64, 0x00}))), // seq0 n60
                            xmid(evnt({0x90, 0x45, 0x64, 0x00})));  // seq1 n69
    V xmi = xmi_file(chunk("CAT ", catBody));

    Smf smf;
    CHECK_EQ(Run(xmi, 1, smf), 0);
    CHECK(smf.contains({0x90,0x45,0x64}));
    CHECK(!smf.contains({0x90,0x3C}));

    Smf smf0;
    CHECK_EQ(Run(xmi, 0, smf0), 0);
    CHECK(smf0.contains({0x90,0x3C}));

    uint8_t* out = nullptr; size_t outLen = 0;
    CHECK(xmi_to_smf(xmi.data(), xmi.size(), 2, &out, &outLen) != 0);
    CHECK(out == nullptr);
}

// Meta events pass through; embedded tempo/EOT are dropped (XMI's clock
// is fixed — only the converter's own tempo may appear).
REGISTER_TEST(xmi_meta_passthrough_tempo_dropped)
{
    V xmi = xmi_file(xmid(evnt({
        0xFF, 0x03, 0x01, 0x58,            // track name 'X' -> kept
        0xFF, 0x51, 0x03, 0x0A, 0x0B, 0x0C,// embedded tempo -> dropped
        0x90, 0x40, 0x00                   // note-on vel0 (no dur parse)
    })));
    Smf smf; CHECK_EQ(Run(xmi, 0, smf), 0);
    CHECK(smf.contains({0x00,0xFF,0x03,0x01,0x58}));
    CHECK_EQ(smf.count({0xFF,0x51}), 1);                   // ours only
    CHECK(smf.contains({0x00,0x90,0x40,0x00}));
}

// Delay bytes (< 0x80) SUM: 0x7F+0x7F = 254 ticks -> SMF varlen 0x81 0x7E.
REGISTER_TEST(xmi_delay_accumulation)
{
    V xmi = xmi_file(xmid(evnt({0x7F,0x7F, 0xB0,0x07,0x7F})));
    Smf smf; CHECK_EQ(Run(xmi, 0, smf), 0);
    CHECK(smf.contains({0x81,0x7E,0xB0,0x07,0x7F}));       // ctrl @254
}

// Malformed inputs: non-FORM, missing XMID child, truncated EVNT.
REGISTER_TEST(xmi_rejects_malformed)
{
    Smf smf;
    V garbage = {'n','o','t',' ','a',' ','x','m','i','f','i','l','e','!','!'};
    CHECK(Run(garbage, 0, smf) != 0);

    V noSeq = xmi_file(chunk("INFO", {0,0,1,0}));        // no XMID children
    CHECK(Run(noSeq, 0, smf) != 0);

    V xmi = xmi_file(xmid(evnt({0x90, 0x40})));          // trunc'd channel ev
    CHECK(Run(xmi, 0, smf) != 0);
}
