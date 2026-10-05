// T106: canonical runner — one source, two builds.
//
//   bob_runner  (rowan-engine):   -DROWAN_GAME_BOB, links bob_math
//   mig_runner  (migalley/tests): -DROWAN_GAME_MIG, links Math
//
// Usage:
//   <runner> <fixture.rfx>                 emit canonical stream on stdout
//   <runner> --gen <file> <family> <n> [seed]   write a fixture
//
// Families (ids are stable, see tests/fixtures/manifest.tsv):
//   1 sincos  in:{i16 ang}          out: sin,cos            (exhaustive at n=65536)
//   2 atan2   in:{i32 dx,i32 dy}    out: angle
//   3 dist2   in:{i32 dx,i32 dy}    out: dist
//   4 dist3   in:{i32 dx,i32 dy,i32 dz}  out: dist
//   5 rndseq  in:{u32 count}        out: rnd() sequence      (stateful PRNG check)
//
// Determinism: output depends only on fixture bytes + the routine under test.
// No clocks, no environment, no locale-sensitive formatting.
#include "canonical.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#if defined(ROWAN_GAME_MIG)
#  include "DOSDEFS.H"
#  include "MYANGLES.H"
#  include "MYMATH.H"
   static const char* GAME = "mig";
#elif defined(ROWAN_GAME_BOB)
#  include "DOSDefs.h"
#  include "myangles.h"
#  include "mymath.h"
   static const char* GAME = "bob";
#else
#  error "define ROWAN_GAME_BOB or ROWAN_GAME_MIG"
#endif

// ---------------------------------------------------------------------------
// input records (wire format — fixed-size little-endian ints only)
// ---------------------------------------------------------------------------
#pragma pack(push, 1)
struct InSincos { int16_t ang; };
struct InVec2   { int32_t dx, dy; };
struct InVec3   { int32_t dx, dy, dz; };
struct InRnd    { uint32_t count; };
#pragma pack(pop)

enum Family { F_SINCOS=1, F_ATAN2, F_DIST2, F_DIST3, F_RNDSEQ };

struct FamilyEnt { const char* name; Family id; size_t recsz; };
static const FamilyEnt FAMILIES[] = {
    {"sincos", F_SINCOS, sizeof(InSincos)},
    {"atan2",  F_ATAN2,  sizeof(InVec2)},
    {"dist2",  F_DIST2,  sizeof(InVec2)},
    {"dist3",  F_DIST3,  sizeof(InVec3)},
    {"rndseq", F_RNDSEQ, sizeof(InRnd)},
    {nullptr,  (Family)0, 0},
};

static const FamilyEnt* find_family(const char* name) {
    for (auto* f = FAMILIES; f->name; ++f)
        if (!std::strcmp(f->name, name)) return f;
    return nullptr;
}

// ---------------------------------------------------------------------------
// generation: deterministic per family; edge records first, then rng fill.
// sincos at n>=65536 becomes an exhaustive domain sweep (sequence, not rng).
// ---------------------------------------------------------------------------
static std::vector<uint8_t> gen_payload(Family fam, uint32_t n, uint32_t seed) {
    canon::XorShift r(seed);
    std::vector<uint8_t> buf;
    buf.reserve(n * 8);

    auto put_i16 = [&](int16_t a){ InSincos v{a};
        auto p=(uint8_t*)&v; buf.insert(buf.end(), p, p+sizeof v); };
    auto put_v2  = [&](int32_t x, int32_t y){ InVec2 v{x,y};
        auto p=(uint8_t*)&v; buf.insert(buf.end(), p, p+sizeof v); };
    auto put_v3  = [&](int32_t x, int32_t y, int32_t z){ InVec3 v{x,y,z};
        auto p=(uint8_t*)&v; buf.insert(buf.end(), p, p+sizeof v); };
    auto put_u32 = [&](uint32_t c){ InRnd v{c};
        auto p=(uint8_t*)&v; buf.insert(buf.end(), p, p+sizeof v); };

    uint32_t i = 0;
    switch (fam) {
    case F_SINCOS:
        if (n >= 65536) {                              // exhaustive sweep
            for (; i < 65536; ++i) put_i16((int16_t)i);
        }
        put_i16(0); put_i16(1); put_i16(-1);           // edge angles
        put_i16(16384); put_i16(-32768); put_i16(32767);
        for (i += 6; i < n; ++i) put_i16((int16_t)r.next());
        break;
    case F_ATAN2:
        put_v2(0,0); put_v2(0,1); put_v2(1,0);         // degenerate + axes
        put_v2(0,-1); put_v2(-1,0);
        put_v2(1,1); put_v2(-1,-1); put_v2(1,-1); put_v2(-1,1);
        put_v2(0x7FFFFFFF,0x7FFFFFFF); put_v2(-0x7FFFFFFF,-0x7FFFFFFF);
        for (i += 11; i < n; ++i)
            put_v2(r.range(-4000000, 4000000), r.range(-4000000, 4000000));
        break;
    case F_DIST2:
        put_v2(0,0); put_v2(3,4); put_v2(0x7FFFFFFF,0);
        for (i += 3; i < n; ++i)
            put_v2(r.range(-2000000, 2000000), r.range(-2000000, 2000000));
        break;
    case F_DIST3:
        put_v3(0,0,0); put_v3(3,4,12); put_v3(0x7FFFFFFF,0,0);
        for (i += 3; i < n; ++i)
            put_v3(r.range(-2000000,2000000), r.range(-2000000,2000000),
                   r.range(-2000000,2000000));
        break;
    case F_RNDSEQ:
        put_u32(4096);                                  // one long sequence
        for (i += 1; i < n; ++i) put_u32(r.range(1, 256));
        break;
    default: break;
    }
    return buf;
}

static int do_gen(const char* file, const char* famname, uint32_t n,
                  uint32_t seed) {
    auto* f = find_family(famname);
    if (!f) { std::fprintf(stderr, "unknown family '%s'\n", famname); return 2; }
    std::vector<uint8_t> payload = gen_payload(f->id, n, seed);
    size_t recsz = f->recsz;
    uint32_t nrec = (uint32_t)(payload.size() / recsz);
    FILE* out = std::fopen(file, "wb");
    if (!out) { std::perror(file); return 2; }
    canon::Header h{0, (uint32_t)f->id, nrec, 0};
    bool ok = canon::write_fixture(out, h, payload.data(), recsz);
    std::fclose(out);
    if (!ok) { std::fprintf(stderr, "write failed\n"); return 2; }
    std::fprintf(stderr, "%s: family=%s records=%u seed=0x%x\n",
                 file, famname, nrec, seed);
    return 0;
}

// ---------------------------------------------------------------------------
// run: dispatch on family, emit one canon record per input record.
// ---------------------------------------------------------------------------
static int do_run(const char* file) {
    FILE* in = std::fopen(file, "rb");
    if (!in) { std::perror(file); return 2; }
    canon::Header h;
    std::vector<uint8_t> payload(8 * 1024 * 1024);
    // first pass: header only to size the buffer
    if (std::fread(&h, sizeof h, 1, in) != 1 || h.magic != canon::RFX_MAGIC) {
        std::fprintf(stderr, "%s: bad fixture header\n", file);
        std::fclose(in); return 2;
    }
    std::rewind(in);
    size_t recsz = 0;
    for (auto* f = FAMILIES; f->name; ++f)
        if ((uint32_t)f->id == h.family) { recsz = f->recsz; break; }
    if (!recsz) {
        std::fprintf(stderr, "%s: unknown family %u\n", file, h.family);
        std::fclose(in); return 2;
    }
    payload.resize((size_t)h.nrecords * recsz);
    if (!canon::read_fixture(in, h, payload.data(), recsz)) {
        std::fclose(in); return 2;
    }
    std::fclose(in);

    std::fprintf(stderr, "%s %s: family=%u records=%u\n",
                 GAME, file, h.family, h.nrecords);

    MathLib ml;                                        // local instance
    canon::Emitter e;
    std::fprintf(e.out, "GAME %s FIX %s FAMILY %u N %u\n",
                 GAME, file, h.family, h.nrecords);

    const uint8_t* p = payload.data();
    for (uint32_t i = 0; i < h.nrecords; ++i, p += recsz) {
        e.rec();
        switch (h.family) {
        case F_SINCOS: {
            auto* v = (const InSincos*)p;
            ANGLES a; a = (Angles)v->ang;
            SWord s, c;
            // sin_cos is private in BoB's MathLib but public in mig's;
            // high_sin_cos is public in both and calls the same table path.
            ml.high_sin_cos(a, s, c);
            e.field_i16("ang", v->ang);
            e.field_i16("sin", s);
            e.field_i16("cos", c);
            break;
        }
        case F_ATAN2: {
            auto* v = (const InVec2*)p;
            ANGLES a = ml.arctan(v->dx, v->dy);
            e.field_i32("dx", v->dx); e.field_i32("dy", v->dy);
            e.field_i16("ang", (int16_t)+a);
            break;
        }
        case F_DIST2: {
            auto* v = (const InVec2*)p;
            ULong d = ml.Distance2d(v->dx, v->dy);
            e.field_i32("dx", v->dx); e.field_i32("dy", v->dy);
            e.field_u32("dist", (uint32_t)d);
            break;
        }
        case F_DIST3: {
            auto* v = (const InVec3*)p;
            ULong d = ml.distance3d(v->dx, v->dy, v->dz);
            e.field_i32("dx", v->dx); e.field_i32("dy", v->dy);
            e.field_i32("dz", v->dz);
            e.field_u32("dist", (uint32_t)d);
            break;
        }
        case F_RNDSEQ: {
            auto* v = (const InRnd*)p;
            for (uint32_t j = 0; j < v->count; ++j) {
                // rnd() family differs in shape across trees; call the
                // free function where present.
                e.field_u32("r", (uint32_t)ml.rnd());
            }
            break;
        }
        default: break;
        }
    }
    e.done();
    return 0;
}

int main(int argc, char** argv) {
    if (argc >= 4 && !std::strcmp(argv[1], "--gen")) {
        if (argc < 5) { std::fprintf(stderr, "--gen <file> <fam> <n> [seed]\n");
            return 2; }
        uint32_t n = (uint32_t)std::strtoul(argv[4], nullptr, 0);
        uint32_t seed = argc > 5 ? (uint32_t)std::strtoul(argv[5], nullptr, 0)
                                 : 0x5EEDu;
        return do_gen(argv[2], argv[3], n, seed);
    }
    if (argc == 2) return do_run(argv[1]);
    std::fprintf(stderr, "usage: %s <fixture.rfx> | --gen <f> <fam> <n> [s]\n",
                 argv[0]);
    return 2;
}
