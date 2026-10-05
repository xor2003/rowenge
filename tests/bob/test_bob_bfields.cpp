// Tests for BoB bfields/ leaf code (T104).
//
// Persons2::SetLoc/GetLoc read and write named globals referenced by
// BFieldGlobalTable[] (generated bfrefs.g). The makebf generator emits only
// {(void*) &GR_x} initializers — fieldsize/fieldshift/ptrindx are always 0 in
// the shipped table, so the mask formula degenerates:
//   imask = (((1<<0)>>1)-1) = 0xFFFFFFFF  (unsigned underflow, full word)
// making SetLoc a full-ULong write and GetLoc a full-ULong read through the
// table. The table ends with a {NULL} terminator; GetLoc returns BAD_RV
// (0x80000000) for null references, which is also how upstream scans detect
// the end (persons2.cpp uses the same rule to enumerate globals).

#include "harness.h"

#include "dosdefs.h"
#include "makebf.h"
#include "persons2.h"
#include "package.h"

extern GlobalRef BFieldGlobalTable[];
extern int debugmaxcount;

static const int BAD_RV = (int)0x80000000;

REGISTER_TEST(bfields_globref_table_integrity)
{
    // The generated table must be non-empty and must terminate: scanning with
    // the upstream BAD_RV rule has to finish inside a sane bound.
    int n = 0, live_refs = 0;
    for (; n < 4096; n++)
    {
        if (!BFieldGlobalTable[n].reference)
            break;
        live_refs++;
    }
    CHECK(n > 150);                 // table has real content (~206 entries)
    CHECK(n < 4096);                // terminator found
    CHECK_EQ(live_refs, n);         // every scanned entry is live
    CHECK_EQ(Persons2::GetLoc(n), BAD_RV);  // terminator behaves as upstream expects
}

REGISTER_TEST(bfields_globref_setloc_roundtrip)
{
    // Full-word semantics: SetLoc(i,v) then GetLoc(i) == v for every live
    // entry. Also verifies GetLoc's BAD_RV path only triggers on null refs.
    int n = 0;
    for (int i = 0; i < 4096; i++)
    {
        if (!BFieldGlobalTable[i].reference)
            break;
        n++;
        int probe = (int)rng();
        Persons2::SetLoc(i, probe);
        CHECK_EQ(Persons2::GetLoc(i), probe);
    }
    CHECK(n > 150);
}

REGISTER_TEST(bfields_globref_setloc_writes_whole_word)
{
    // fieldsize==0 → imask underflows to all-ones → the whole ULong at
    // *reference is replaced, not a subfield. Verify against the raw word.
    int i;
    for (i = 0; i < 4096 && BFieldGlobalTable[i].reference; i++)
        ;
    CHECK(i > 150);

    GlobalRef& g = BFieldGlobalTable[0];
    unsigned long* host = (unsigned long*)g.reference;
    CHECK(host != NULL);
    assert(host != NULL);   // analyzer hint: CHECK() doesn't narrow the pointer
    unsigned long saved = *host;
    *host = 0xDEADBEEF;
    Persons2::SetLoc(0, 0x1234);
    CHECK_EQ(*host, 0x1234UL);
    Persons2::SetLoc(0, -1);
    CHECK_EQ(*host, 0xFFFFFFFFUL);
    *host = saved;
}

REGISTER_TEST(bfields_packagelist_reorder)
{
    // PackageList::Reorder/ ReorderZero/ ReorderNoCheck remap UIDs in
    // [SagBAND,RealAcBAND) through ReorderLookup; UIDs outside pass through.
    // Fill the table with a known permutation and a debugmaxcount watermark.
    int saved_max = debugmaxcount;
    for (int i = 0; i < PackageList::MAX_REORDERLOOKUP; i++)
        PackageList::ReorderLookup[i] = UniqueID(0x4000 + i);
    debugmaxcount = 0x8000;

    // in-band: returns the lookup. The band is 256 wide (0x1200..0x12FF)
    // though MAX_REORDERLOOKUP is 512 — indices >=256 are out-of-band.
    CHECK_EQ((int)PackageList::Reorder(UniqueID(SagBAND)), 0x4000);
    CHECK_EQ((int)PackageList::Reorder(UniqueID(SagBAND + 255)), 0x4000 + 255);
    // out-of-band: identity
    CHECK_EQ((int)PackageList::Reorder(UniqueID(RealAcBAND)), (int)RealAcBAND);
    CHECK_EQ((int)PackageList::Reorder(UniqueID(SagBAND - 1)), (int)(SagBAND - 1));
    CHECK_EQ((int)PackageList::Reorder(UID_NULL), (int)UID_NULL);
    // ReorderNoCheck: same in-band mapping without the assert
    CHECK_EQ((int)PackageList::ReorderNoCheck(UniqueID(SagBAND + 7)), 0x4000 + 7);

    // ReorderZero: in-band entries >= debugmaxcount collapse to UID_NULL
    PackageList::ReorderLookup[3] = UniqueID(0x9000);   // >= debugmaxcount
    CHECK_EQ((int)PackageList::ReorderZero(UniqueID(SagBAND + 3)), (int)UID_NULL);
    CHECK_EQ((int)PackageList::ReorderZero(UniqueID(SagBAND + 4)), 0x4000 + 4);
    // out-of-band: identity
    CHECK_EQ((int)PackageList::ReorderZero(UniqueID(RealAcBAND + 9)),
             (int)(RealAcBAND + 9));

    debugmaxcount = saved_max;
}
