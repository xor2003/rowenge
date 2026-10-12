// BoB files/ leaf tests — exercises real fileman.o / winfile.o objects.
// T103: native file resolution. The dirlist(path,wild) scan routes through
// migalley's io_compat _findfirst (FNM_CASEFOLD) — case-insensitive matching
// proven on a real fixture dir, no symlink aliases.
#include "harness.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <sys/stat.h>

#include "dosdefs.h"
#include "mymath.h"
#include "fileman.h"

// replace_extention lives at fileman.cpp:946, extern'd here (fileman.h has no decl)
void replace_extention(string &path, string from, string to);
// fopen_ci: component-wise case-insensitive path resolve (rowan_io_compat)
extern "C" FILE* fopen_ci(const char* path, const char* mode);

static char testdir[256];
static int made_dir = 0;

static void make_fixture_dir()
{
    if (made_dir) return;
    strcpy(testdir, "/tmp/bobfiles_XXXXXX");
    CHECK(mkdtemp(testdir) != nullptr);
    // Mixed-case game-data style filenames
    FILE* f = fopen((std::string(testdir) + "/PILOT1.X8").c_str(), "wb");
    CHECK(f != nullptr); fputs("x8data", f); fclose(f);
    f = fopen((std::string(testdir) + "/b26eng.x8").c_str(), "wb");
    CHECK(f != nullptr); fputs("lower", f); fclose(f);
    f = fopen((std::string(testdir) + "/readme.txt").c_str(), "wb");
    CHECK(f != nullptr); fputs("txt", f); fclose(f);
    made_dir = 1;
}

// files.g inlines: dirnum(FileNum) -> dirindex (>>8), dirnum(dirindex) -> FileNum (<<8)
REGISTER_TEST(dirnum_roundtrip)
{
    // dirindex = high byte of FileNum (DIRNUMSH=8, FILENUMMASK=0x00FF)
    CHECK_EQ((int)dirnum((FileNum)0x12ab), 0x12);
    CHECK_EQ((int)dirnum((dirindex)0x34), 0x3400);
    // round-trip preserves the dir bits, zeroes the file index
    CHECK_EQ((int)dirnum(dirnum((FileNum)0x77cd)), 0x7700);
}

REGISTER_TEST(dirlist_scans_real_dir)
{
    make_fixture_dir();
    char path[300], wild[32];
    strcpy(path, testdir);
    strcpy(wild, "*.X8");            // uppercase pattern must match lowercase names
    dirlist dl(path, wild);
    int n = (int)dl;                 // operator int() -> numfiles
    // Faithful BoB semantics: slot [0] holds the wildcard itself
    // (makedirlist seeds it before the scan); real files start at [1].
    CHECK_EQ(n, 3);                  // "*.X8" + PILOT1.X8 + b26eng.x8
    CHECK_STREQ(dl[0], "*.X8");
    int found = 0;
    for (int i = 1; i < n; i++)
    {
        char* name = dl[i];
        if (strstr(name, "PILOT1.X8") || strstr(name, "b26eng.x8")) found++;
    }
    CHECK_EQ(found, 2);              // case-insensitive: .x8 AND .X8 matched
}

REGISTER_TEST(dirlist_empty_dir_gives_zero)
{
    char path[300] = "/tmp/bobfiles_empty_XXXXXX", wild[8] = "*.*";
    CHECK(mkdtemp(path) != nullptr);
    dirlist dl(path, wild);
    CHECK_EQ((int)dl, 1);            // wildcard slot only
    CHECK_STREQ(dl[0], "*.*");
}

REGISTER_TEST(dirlist_missing_dir_gives_zero)
{
    char path[300] = "/nonexistent_dir_xyz", wild[8] = "*.*";
    dirlist dl(path, wild);
    CHECK_EQ((int)dl, 1);            // wildcard slot only
}

REGISTER_TEST(fileman_getfilesize)
{
    FILE* f = tmpfile();
    CHECK(f != nullptr);
    fputs("12345678901234567890", f);   // 20 bytes
    CHECK_EQ((long)FILEMAN.getfilesize(f), 20L);
    // position preserved (function seeks to end then restores)
    fseek(f, 7, SEEK_SET);
    CHECK_EQ((long)FILEMAN.getfilesize(f), 20L);
    CHECK_EQ(ftell(f), 7L);
    fclose(f);
}

REGISTER_TEST(replace_extention_swaps_when_target_exists)
{
    make_fixture_dir();
    // path ending .pcx; PILOT1.X8 exists -> path becomes the .X8 file
    char pathbuf[300];
    snprintf(pathbuf, sizeof(pathbuf), "%s/PILOT1.pcx", testdir);
    char* path = pathbuf;
    replace_extention(path, (string)".pcx", (string)".x8");
    // fopen_ci resolves pilot1.x8 -> PILOT1.X8 on disk; note the function
    // also lowercases path in place (faithful side effect, Windows-safe).
    CHECK(strstr(pathbuf, ".x8") != nullptr);
}

// T107 family 4: fopen_ci resolves each path component case-insensitively
// against the real filesystem — the semantic every fileman open relies on.
REGISTER_TEST(fopen_ci_resolves_mixed_case)
{
    make_fixture_dir();
    // wrong case on file component: pilot1.X8 on disk is PILOT1.X8
    std::string p = std::string(testdir) + "/pilot1.X8";
    FILE* f = fopen_ci(p.c_str(), "rb");
    CHECK(f != nullptr);
    if (f) { char buf[8] = {0}; fread(buf, 1, 6, f); CHECK_STREQ(buf, "x8data"); fclose(f); }
    // wrong case on extension too
    p = std::string(testdir) + "/PILOT1.x8";
    f = fopen_ci(p.c_str(), "rb");
    CHECK(f != nullptr); if (f) fclose(f);
    // all-lower file opened via upper pattern
    p = std::string(testdir) + "/B26ENG.X8";
    f = fopen_ci(p.c_str(), "rb");
    CHECK(f != nullptr); if (f) fclose(f);
}

REGISTER_TEST(fopen_ci_missing_returns_null)
{
    make_fixture_dir();
    std::string p = std::string(testdir) + "/nosuchfile.x8";
    CHECK(fopen_ci(p.c_str(), "rb") == nullptr);
    CHECK(fopen_ci("/nonexistent_dir_xyz/file.x8", "rb") == nullptr);
    CHECK(fopen_ci("", "rb") == nullptr);
}

REGISTER_TEST(fopen_ci_nested_component_case)
{
    // two-level case mismatch: dir and file both differ from on-disk case
    char nd[256];
    strcpy(nd, "/tmp/bobfiles_nest_XXXXXX");
    CHECK(mkdtemp(nd) != nullptr);
    std::string realdir = std::string(nd) + "/MiXeDDir";
    CHECK(mkdir(realdir.c_str(), 0755) == 0);
    FILE* f = fopen((realdir + "/WeApOn.BF").c_str(), "wb");
    CHECK(f != nullptr); fputs("bf", f); fclose(f);
    std::string ask = std::string(nd) + "/mixeddir/WEAPON.bf";
    f = fopen_ci(ask.c_str(), "rb");
    CHECK(f != nullptr);
    if (f) { char buf[4] = {0}; fread(buf, 1, 2, f); CHECK_STREQ(buf, "bf"); fclose(f); }
}

REGISTER_TEST(replace_extention_keeps_path_when_missing)
{
    make_fixture_dir();
    char pathbuf[300];
    snprintf(pathbuf, sizeof(pathbuf), "%s/nosuch.pcx", testdir);
    char* path = pathbuf;
    replace_extention(path, (string)".pcx", (string)".zzz");
    // unchanged extension, but the path is lowercased unconditionally —
    // compare against the lowercase form (BoB relies on case-insensitive fs).
    std::string lower = std::string(testdir) + "/nosuch.pcx";
    for (auto& c : lower) c = tolower((unsigned char)c);
    CHECK_STREQ(pathbuf, lower.c_str());
}
