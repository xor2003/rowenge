// T106: canonical output + fixture format shared by the BoB and MiG runners.
//
// Contract (REQ-REPLAY-01):
//   * a runner fed fixture F emits a canonical text stream: one
//     "REC <tick> <field>=<value>" line per output field per input record,
//     then a final "CANON <fnv64hex>" line.
//   * three runs of the same binary+fixture MUST byte-match; the runner that
//     diverges reports the first differing REC line (tick + field are on it).
//   * CANON is FNV-1a/64 over every emitted byte, so streams compare by hash
//     first and by line-diff for diagnosis.
//
// Values serialize as fixed-width lowercase hex — never %g/%f (locale, float
// formatting differences across CRTs), never raw size_t (width-dependent).
#pragma once
#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <cstring>

namespace canon {

constexpr uint32_t RFX_MAGIC = 0x31465852u;   // "RFX1" little-endian

struct Header {
    uint32_t magic;      // RFX_MAGIC
    uint32_t family;     // family id (runner-owned enum, manifest keeps names)
    uint32_t nrecords;   // input records in payload
    uint32_t reserved;   // 0; future flags (endianness check word etc.)
};

// Fixture payload is a flat array of the family's input record, little-endian
// native layout. All families use PODs of fixed-size int fields only — no
// floats in inputs (determinism is checked at record granularity anyway).

struct Emitter {
    uint64_t h = 14695981039346656037ull;   // FNV-1a 64 offset basis
    uint32_t tick = 0;
    FILE*   out = stdout;

    void byte(uint8_t b) { h ^= b; h *= 1099511628211ull; }
    void raw(const void* p, size_t n) {
        const uint8_t* b = (const uint8_t*)p;
        for (size_t i = 0; i < n; i++) byte(b[i]);
    }
    void rec() { ++tick; }
    void field(const char* name, uint64_t v, int width) {
        char buf[64];
        int n = std::snprintf(buf, sizeof buf, "REC %u %s=0x%0*llx\n",
                              tick, name, width, (unsigned long long)v);
        std::fputs(buf, out);
        raw(buf, (size_t)n);
    }
    void field_i32(const char* n, int32_t v)  { field(n, (uint32_t)v, 8); }
    void field_u32(const char* n, uint32_t v) { field(n, v, 8); }
    void field_i16(const char* n, int16_t v)  { field(n, (uint16_t)v, 4); }
    void field_u16(const char* n, uint16_t v) { field(n, v, 4); }
    void field_f32(const char* n, float v) {          // bit-exact, not %g
        uint32_t u; std::memcpy(&u, &v, 4); field(n, u, 8);
    }
    void field_f64(const char* n, double v) {
        uint64_t u; std::memcpy(&u, &v, 8); field(n, u, 16);
    }
    void field_str(const char* n, const char* s) {    // hashed but unquoted
        field(n, (uint64_t)std::strlen(s), 8);        //  — text hashed via raw()
        std::fprintf(out, "REC %u %s_str=\"%s\"\n", tick, n, s);
        raw(n, std::strlen(n)); raw(s, std::strlen(s));
    }
    void done() {
        char buf[32];
        int n = std::snprintf(buf, sizeof buf, "CANON %016llx\n",
                              (unsigned long long)h);
        std::fputs(buf, out); raw(buf, (size_t)n);
    }
};

// Deterministic fixture PRNG — xorshift32, explicit seed per fixture (stored
// in the manifest), never environment-dependent.
struct XorShift {
    uint32_t s;
    explicit XorShift(uint32_t seed) : s(seed ? seed : 0x5EEDu) {}
    uint32_t next() { s ^= s << 13; s ^= s >> 17; s ^= s << 5; return s; }
    int32_t  i32() { return (int32_t)next(); }
    int32_t  range(int32_t lo, int32_t hi) {         // inclusive
        return lo + (int32_t)(next() % (uint32_t)(hi - lo + 1));
    }
};

inline bool write_fixture(FILE* f, const Header& h, const void* payload,
                          size_t recsz) {
    Header hh = h; hh.magic = RFX_MAGIC;
    return std::fwrite(&hh, sizeof hh, 1, f) == 1 &&
           (h.nrecords == 0 ||
            std::fwrite(payload, recsz, h.nrecords, f) == h.nrecords);
}

// Reads header + payload; payload buffer must fit nrecords*recsz.
inline bool read_fixture(FILE* f, Header& h, void* payload, size_t recsz) {
    if (std::fread(&h, sizeof h, 1, f) != 1 || h.magic != RFX_MAGIC) {
        std::fprintf(stderr, "runner: bad fixture header\n");
        return false;
    }
    if (h.nrecords &&
        std::fread(payload, recsz, h.nrecords, f) != h.nrecords) {
        std::fprintf(stderr, "runner: short fixture payload (%u records)\n",
                     h.nrecords);
        return false;
    }
    return true;
}

} // namespace canon
