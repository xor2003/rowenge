// Rowan shared shell — installation detection (REQ-DETECT-01/02/03).
//
// Pure C++ module: no SDL, no game deps. Each game executable embeds it and
// runs detection on the selected install dir BEFORE entering game code —
// there is intentionally no API to skip validation (REQ-DETECT-03).
//
// Detection = multi-file fingerprinting (REQ-DETECT-01): each edition
// manifest lists distinguishing files with expected sizes. Sizes come from
// the reference installs recorded in migration/editions.tsv; patch-volatile
// files (exes) are never used alone — CD data files carry identity.

#ifndef ROWAN_SHELL_DETECT_H
#define ROWAN_SHELL_DETECT_H

#include <cstdint>
#include <vector>
#include <string>

namespace rowan_shell {

// A distinguishing file: path relative to install root + exact size.
// Paths resolve case-insensitively (DOS-era installs are case-messy).
struct FileSig {
    const char* relpath;   // '/'-separated, case-insensitive match
    int64_t     size;      // expected byte size; -1 = exist, any size
};

struct EditionManifest {
    const char* game;       // "mig" | "bob"
    const char* edition;    // "retail-CD" | "v1.23" | "BDG-0.85F" | ...
    const FileSig* sigs;
    int          nsigs;
};

enum class Verdict {
    OK,          // exactly one manifest fully matched
    NOT_A_GAME,  // no manifest came close (nothing Rowan here)
    INCOMPLETE,  // nearest manifest partially matched — missing files listed
    AMBIGUOUS,   // >1 manifest fully matched (merged/mixed install)
};

struct Report {
    Verdict                verdict = Verdict::NOT_A_GAME;
    std::string            game;         // set when verdict==OK
    std::string            edition;      // set when verdict==OK
    std::vector<std::string> matched;    // editions that fully matched
    std::vector<std::string> missing;    // files absent in nearest manifest
    std::vector<std::string> wrongsize;  // present but wrong size ("path exp/got")
    std::string            detail;       // human diagnostic for REQ-DETECT-02
};

// Scan `dir`. If `only_game` is non-NULL, restrict candidacy to manifests of
// that game (the embedding executable knows its own game — REQ-LAUNCH-02 —
// but validation still must fully pass; a flag cannot bypass REQ-DETECT-03).
Report detect_install(const char* dir, const char* only_game = nullptr);

// Registry of supported editions — lives in manifests.cpp, one row per
// supported install. Read-only; tests may inject their own table.
const EditionManifest* registry(int* n);
Report detect_install_with(const char* dir, const char* only_game,
                           const EditionManifest* table, int ntable);

} // namespace rowan_shell
#endif
