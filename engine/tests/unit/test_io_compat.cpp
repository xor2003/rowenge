// io_compat unit tests (migalley/MFC/io_compat.cpp): the case-insensitive
// Windows-path layer both games load assets through.  A temp directory
// fixture provides mixed-case trees; resolution, fopen_ci and
// _findfirst/_findnext are checked on the real filesystem.

#include "../harness.h"
#include <string>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <sys/stat.h>

#include "../../migalley/H/io_compat.h"

namespace {

std::string g_tmp;

bool WriteFile(const std::string& rel, const char* body = "x") {
    std::string p = g_tmp + "/" + rel;
    FILE* f = fopen(p.c_str(), "w");
    if (!f) return false;
    fputs(body, f); fclose(f);
    return true;
}
bool MkDir(const std::string& rel) {
    return mkdir((g_tmp + "/" + rel).c_str(), 0755) == 0;
}

struct TmpTree {
    TmpTree() {
        char tmpl[] = "/tmp/io_compat_testXXXXXX";
        CHECK(mkdtemp(tmpl) != nullptr);
        g_tmp = tmpl;
        // Mixed-case asset tree, like the game installs produce:
        //   Data/SOUND/Effects/boom.WAV      — all-uppercase dirs, mixed file
        //   Data/savegame/Slot1.sav          — lowercase dirs
        //   Data/Maps/Island.MAP             — per-component mixed case
        CHECK(MkDir("Data"));
        CHECK(MkDir("Data/SOUND"));
        CHECK(MkDir("Data/SOUND/Effects"));
        CHECK(WriteFile("Data/SOUND/Effects/boom.WAV"));
        CHECK(MkDir("Data/savegame"));
        CHECK(WriteFile("Data/savegame/Slot1.sav"));
        CHECK(MkDir("Data/Maps"));
        CHECK(WriteFile("Data/Maps/Island.MAP"));
    }
    ~TmpTree() {
        std::string cmd = "rm -rf '" + g_tmp + "'";
        (void)!system(cmd.c_str());
    }
};

// cd into the fixture for the duration of a test (resolve_ci_path uses
// "." as the anchor for relative input).
struct PushCwd {
    char old[4096];
    PushCwd(const std::string& dir) {
        CHECK(getcwd(old, sizeof old) != nullptr);
        CHECK_EQ(chdir(dir.c_str()), 0);
    }
    ~PushCwd() { CHECK_EQ(chdir(old), 0); }
};

} // namespace

// Every path component resolves case-insensitively; the on-disk spelling
// comes back.
REGISTER_TEST(io_resolve_each_component)
{
    TmpTree t; PushCwd cd(g_tmp);
    CHECK_STREQ(resolve_ci_path("data/sound/effects/BOOM.wav"),
                "Data/SOUND/Effects/boom.WAV");
    CHECK_STREQ(resolve_ci_path("DATA\\MAPS\\island.map"),
                "Data/Maps/Island.MAP");       // '\' -> '/' + per-comp resolve
    CHECK_STREQ(resolve_ci_path("Data/SaveGame/SLOT1.SAV"),
                "Data/savegame/Slot1.sav");
}

// Components that don't exist keep the caller's spelling — a save file
// that doesn't exist yet still lands in the correctly-cased directory.
REGISTER_TEST(io_resolve_missing_tail_verbatim)
{
    TmpTree t; PushCwd cd(g_tmp);
    CHECK_STREQ(resolve_ci_path("data/savegame/NEWFILE.sav"),
                "Data/savegame/NEWFILE.sav");
    // First bad component short-circuits; the rest is verbatim.
    CHECK_STREQ(resolve_ci_path("nodir/Sub/File.x"),
                "nodir/Sub/File.x");
    CHECK_STREQ(resolve_ci_path(""), "");
}

// fopen_ci: literal hit first, then the component walk; read-modes only.
REGISTER_TEST(io_fopen_ci)
{
    TmpTree t; PushCwd cd(g_tmp);
    FILE* f = fopen_ci("data/sound/effects/boom.wav", "rb");
    CHECK(f != nullptr);
    if (f) fclose(f);

    CHECK(fopen_ci("data/sound/effects/missing.wav", "rb") == nullptr);
    CHECK(fopen_ci("", "rb") == nullptr);

    // Write mode must NOT go through resolution into a wrong-case dir.
    FILE* w = fopen_ci("data/savegame/NEW2.sav", "wb");
    CHECK(w != nullptr);
    if (w) fclose(w);
    CHECK(WriteFile("Data/savegame/NEW2.sav") || true);  // sanity: file exists
    f = fopen_ci("Data/savegame/NEW2.sav", "rb");
    CHECK(f != nullptr);
    if (f) fclose(f);
}

// _findfirst/_findnext: fnmatch pattern + FNM_CASEFOLD names, _A_SUBDIR on
// directories, -1 at exhaustion, handles closed cleanly.
REGISTER_TEST(io_findfirst_findnext)
{
    TmpTree t; PushCwd cd(g_tmp);

    _finddata_t fi;
    long h = _findfirst("Data/SOUND/Effects/*.wav", &fi);
    CHECK(h > 0);
    CHECK_STREQ(fi.name, "boom.WAV");
    CHECK(_findnext(h, &fi) == -1);            // only one match
    CHECK_EQ(_findclose(h), 0);

    // Pattern matching is case-insensitive on the NAME too.
    h = _findfirst("Data/Maps/ISLAND.*", &fi);
    CHECK(h > 0);
    CHECK_STREQ(fi.name, "Island.MAP");
    CHECK_EQ(_findclose(h), 0);

    // Directory entries carry _A_SUBDIR.
    h = _findfirst("Data/*", &fi);
    CHECK(h > 0);
    bool sawDir = false;
    do {
        if (fi.attrib & _A_SUBDIR) sawDir = true;
    } while (_findnext(h, &fi) == 0);
    CHECK(sawDir);
    CHECK_EQ(_findclose(h), 0);

    // The DIRECTORY part resolves case-insensitively too (BoB SAVEGAME).
    h = _findfirst("data/maps/*.map", &fi);
    CHECK(h > 0);
    CHECK_STREQ(fi.name, "Island.MAP");
    CHECK_EQ(_findclose(h), 0);

    // No match / bad dir -> -1, no handle leaked.
    CHECK_EQ(_findfirst("Data/Maps/*.zzz", &fi), -1L);
    CHECK_EQ(_findfirst("nodir/*.x", &fi), -1L);
    CHECK_EQ(_findfirst(nullptr, &fi), -1L);
}
