#include "detect.h"

// Edition registry — sizes measured from the reference installs recorded in
// migration/editions.tsv (mig retail CD installed via Wine SETUP.EXE; bob
// BDG tree in runtime/wineprefix). Patch-volatile files are never identity:
// our executable replaces the retail binary, so we fingerprint the DATA the
// game actually consumes. exe entries use -1 (must exist, any size) purely
// as an install-root sanity check.
//
// Mig rows: all retail-compatible installs share identical CD data —
// official/BDG patches only touch binaries we don't run — so one row with
// edition "retail-compat" is the honest granularity until a patch-specific
// data marker is found.
//
// Bob rows: retail vs BDG are distinguishable — BDG adds bdg.txt +
// mfc120.dll and ships a different bob.exe. Subset/superset resolved by
// most-signatures-wins in detect.cpp.

namespace rowan_shell {

static const FileSig MIG_SIGS[] = {
    { "roots.dir",    197 },
    { "Battle.dir",   331 },
    { "Common.dir",   405 },
    { "Dir.dir",      128 },
    { "Buttons.bmp",  30838 },
    { "INTEL.TTF",    52864 },
    { "SMACKER/kimpo.smk", 4727148 },
    { "SMACKER/Migs.smk",    7785548 },
    { "INSTALL.LOG",  184598 },
    { "Mig.exe",      -1 },
};

// filelist.txt is deliberately absent from both bob rows: the ported
// fileman truncates it to a 0-byte "used files" dump on every startup —
// it is generated state, not install identity.
static const FileSig BOB_RETAIL_SIGS[] = {
    { "BOBLICENCE.txt", 10990 },
    { "BATTLE.DIR",     323 },
    { "COMMON.DIR",     526 },
    { "DIR.DIR",        128 },
    { "cardbase.rc",    7720 },
    { "bob.exe",        -1 },
};

static const FileSig BOB_BDG_SIGS[] = {
    { "BOBLICENCE.txt", 10990 },
    { "BATTLE.DIR",     323 },
    { "COMMON.DIR",     526 },
    { "DIR.DIR",        128 },
    { "cardbase.rc",    7720 },
    { "bdg.txt",        2862 },
    { "mfc120.dll",     4424344 },
    { "bob.exe",        830464 },
};

static const EditionManifest REGISTRY[] = {
    { "mig", "retail-compat", MIG_SIGS,        (int)(sizeof(MIG_SIGS)/sizeof(MIG_SIGS[0])) },
    { "bob", "retail-CD",     BOB_RETAIL_SIGS, (int)(sizeof(BOB_RETAIL_SIGS)/sizeof(BOB_RETAIL_SIGS[0])) },
    { "bob", "BDG-community", BOB_BDG_SIGS,    (int)(sizeof(BOB_BDG_SIGS)/sizeof(BOB_BDG_SIGS[0])) },
};

const EditionManifest* registry(int* n)
{
    *n = (int)(sizeof(REGISTRY) / sizeof(REGISTRY[0]));
    return REGISTRY;
}

} // namespace rowan_shell
