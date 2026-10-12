// T107 family 6: save-byte parsing — Profile::DecodePackage consumes the
// text+binary hybrid package record: two sscanf lines, then `flights` "Sq: "
// blocks and `numraids` "Rd: " blocks where each 4 bytes of the struct is
// one 5-char base-90 group (char = digit + 34, little-endian digit order).
// Production invariant: pack[] lives in a zero-initialised global, so
// squadlist.pointer starts NULL — tests memset Profile to match.
#include "harness.h"
#include "DOSDefs.h"
#include "myangles.h"
#include "mymath.h"
#include "worldinc.h"
#include "package.h"
#include <cstring>
#include <string>

// Persons2::pItem is a protected static ItemBase** (persons2.cpp:209).
// DecodePackage -> TargetDesc::SetSGTBandOffFromTarget -> ConvertPtrUID reads
// pItem[uid][0]; a NULL entry yields the deliberate null-ref sentinel, so an
// all-NULL table is the faithful "no world loaded" state the shipped code
// also produces for any UID slot with no item. Seeded here by asm label so
// the real symbol is written without punching through the protected access.
extern ItemBasePtr* pitem_under_test __asm__("_ZN8Persons25pItemE");
static ItemBasePtr g_pitem[0x4000];   // PITEMTABLESIZE = IllegalBAND + 1
static void seed_empty_world()
{
    memset(g_pitem, 0, sizeof g_pitem);
    pitem_under_test = g_pitem;
}

// ---- byte-level encoder mirroring the on-disk record ---------------------
static void b90(std::string& s, ULong v)
{
    for (int i = 0; i < 5; i++) { s += char(34 + v % 90); v /= 90; }
}
template <typename T>
static void emit_rec(std::string& s, const char* tag, const T& rec)
{
    s += tag;
    const ULong* u = reinterpret_cast<const ULong*>(&rec);
    for (size_t i = 0; i < sizeof(T) / 4; i++) b90(s, u[i]);
    s += '\n';
}

static std::string line1(int duty_a, int duty_b, int squads, int raids, int wps,
                         unsigned toff, unsigned targ, unsigned dog,
                         unsigned t1, unsigned t2)
{
    char b[256];
    snprintf(b, sizeof b,
        "Package battlefield: Duty %d %d, Squads %d, Raids: %d, WPs %d, "
        "Takeoff,Target,Dogleg %05x,%05x,%05x, Times %09x,%09x\n",
        duty_a, duty_b, squads, raids, wps, toff, targ, dog, t1, t2);
    return b;
}
static std::string line2(const unsigned* t /*9 pairs*/)
{
    std::string s = "Secondaries:";
    for (int i = 0; i < 9; i++)
    {
        char b[32];
        snprintf(b, sizeof b, " %05x+%05x", t[i * 2], t[i * 2 + 1]);
        s += b;
    }
    return s + "\n";
}

// ---- valid record --------------------------------------------------------
REGISTER_TEST(test_bob_decodepackage_valid)
{
    Profile p;
    memset(&p, 0, sizeof p);
    seed_empty_world();

    Profile::Squad sq;
    memset(&sq, 0, sizeof sq);
    sq.instance = UniqueID(0x4242);
    sq.squadnum = SquadNum(7);
    sq.method   = Profile::AM_DIVEBOMB;

    Profile::RaidNumEntry re;
    memset(&re, 0, sizeof re);
    re.raidnum        = 0x777;
    re.squadliststart = 3;
    re.detector       = 1;
    re.alertlevel     = 0x11223344;

    unsigned t9[18];
    for (int i = 0; i < 9; i++) { t9[i * 2] = 0x111 + i * 0x100; t9[i * 2 + 1] = 0; }

    std::string rec =
        line1(12, 4, /*squads*/1, /*raids*/1, /*wps*/5,
              0xabc, 0xdef, 0x123, 100, 250)
        + line2(t9);
    emit_rec(rec, "Sq: ", sq);
    rec += "Lolly 000001f5\n";
    emit_rec(rec, "Rd: ", re);

    int flights = p.DecodePackage(const_cast<char*>(rec.c_str()), true);

    // return value = flight count
    CHECK_EQ(flights, 1);

    // line1 scalar fields (note the sscanf arg order: Duty b -> status)
    CHECK_EQ((int)p.attackmethod, (int)Profile::AM_LEVELBOMB);
    CHECK_EQ((int)p.packagestatus, (int)Profile::PS_AT_5);
    CHECK_EQ(p.flags, 5);
    CHECK_EQ((int)(UniqueID)p.packagetakeoff, 0xabc);
    CHECK_EQ((int)(UniqueID)p.packagetarget[0].SGT, 0xdef);
    CHECK_EQ((int)(UniqueID)p.doglegloc, 0x123);
    CHECK_EQ(p.primarytargetETA, 100);
    CHECK_EQ(p.playerETAdelta, 250);

    // line2 secondary targets (indices 1..9)
    CHECK_EQ((int)(UniqueID)p.packagetarget[1].SGT, 0x111);
    CHECK_EQ((int)(UniqueID)p.packagetarget[9].SGT, 0x911);

    // squad block round-tripped through base-90
    CHECK_EQ(p.squadlist.Max(), 1);
    CHECK_EQ((int)(UniqueID)p.squadlist[0].instance, 0x4242);
    CHECK_EQ((int)(SquadNum)p.squadlist[0].squadnum, 7);
    CHECK_EQ((int)(Profile::AttackMethod)p.squadlist[0].method,
             (int)Profile::AM_DIVEBOMB);

    // flights==1 -> Lolly line set the first-squad ac bit pattern
    CHECK_EQ(p.squadlist.AcBitsFirstSquad(), 0x1f5);

    // raid block + sentinel record
    CHECK_EQ(p.raidnumentries[0].raidnum, 0x777);
    CHECK_EQ(p.raidnumentries[0].detector, 1);
    CHECK_EQ((int)p.raidnumentries[0].squadliststart, 3);
    CHECK_EQ(p.raidnumentries[1].squadliststart,
             Profile::RNEC_LISTVALISLASTREC);   // sentinel record at [numraids]
}

// ---- two flights, no raids -----------------------------------------------
REGISTER_TEST(test_bob_decodepackage_two_squads)
{
    Profile p;
    memset(&p, 0, sizeof p);
    seed_empty_world();

    Profile::Squad sq0, sq1;
    memset(&sq0, 0, sizeof sq0);
    memset(&sq1, 0, sizeof sq1);
    sq0.instance = UniqueID(0x1000);
    sq1.instance = UniqueID(0x2000);
    sq1.method   = Profile::AM_LEVELBOMB;

    unsigned zeros[18] = {0};
    std::string rec =
        line1(0, 2, /*squads*/2, /*raids*/0, 0, 0, 0, 0, 0, 0)
        + line2(zeros);
    emit_rec(rec, "Sq: ", sq0);
    emit_rec(rec, "Sq: ", sq1);
    // flights != 1 -> no Lolly line expected

    int flights = p.DecodePackage(const_cast<char*>(rec.c_str()), true);
    CHECK_EQ(flights, 2);
    CHECK_EQ(p.squadlist.Max(), 2);
    CHECK_EQ((int)(UniqueID)p.squadlist[0].instance, 0x1000);
    CHECK_EQ((int)(UniqueID)p.squadlist[1].instance, 0x2000);
    CHECK_EQ((int)(Profile::AttackMethod)p.squadlist[1].method,
             (int)Profile::AM_LEVELBOMB);
    // no raids -> raidnumentries stays NULL
    CHECK(p.raidnumentries == nullptr);
}

// ---- decodeprimary=false leaves the primary target alone ------------------
REGISTER_TEST(test_bob_decodepackage_notprimary)
{
    Profile p;
    memset(&p, 0, sizeof p);
    seed_empty_world();
    p.packagetarget[0].SGT = UniqueID(0x555);

    unsigned zeros[18] = {0};
    std::string rec =
        line1(12, 4, 0, 0, 5, 0xabc, 0xdef, 0x123, 100, 250) + line2(zeros);

    int flights = p.DecodePackage(const_cast<char*>(rec.c_str()), false);
    CHECK_EQ(flights, 0);
    CHECK_EQ((int)(UniqueID)p.packagetarget[0].SGT, 0x555);  // untouched
}

// ---- malformed / truncated inputs ----------------------------------------
REGISTER_TEST(test_bob_decodepackage_malformed)
{
    {   // empty -> sscanf gets nothing
        Profile p; memset(&p, 0, sizeof p);
    seed_empty_world();
        std::string rec = "";
        CHECK_EQ(p.DecodePackage(const_cast<char*>(rec.c_str()), true), 0);
    }
    {   // truncated line1 (only 4 fields of 10) -> argsused<10 bail-out
        Profile p; memset(&p, 0, sizeof p);
    seed_empty_world();
        std::string rec = "Package battlefield: Duty 12 4, Squads 1\n";
        CHECK_EQ(p.DecodePackage(const_cast<char*>(rec.c_str()), true), 0);
    }
    {   // total garbage -> 0
        Profile p; memset(&p, 0, sizeof p);
    seed_empty_world();
        std::string rec = "not a package at all\n";
        CHECK_EQ(p.DecodePackage(const_cast<char*>(rec.c_str()), true), 0);
    }
    {   // valid line1, flights=0/raids=0, but line2 missing entirely ->
        // line2 sscanf fails (its return is unchecked — shipped behaviour),
        // function still returns cleanly with scalar fields decoded.
        Profile p; memset(&p, 0, sizeof p);
    seed_empty_world();
        std::string rec = line1(8, 1, 0, 0, 3, 0, 0, 0, 7, 9);
        CHECK_EQ(p.DecodePackage(const_cast<char*>(rec.c_str()), true), 0);
        CHECK_EQ((int)p.attackmethod, 8);
        CHECK_EQ(p.primarytargetETA, 7);
    }
}
