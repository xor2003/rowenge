// T111 — shared-shell detection tests (REQ-DETECT-01/02/03).
// Fixture installs are generated under TMPDIR with real files of the
// manifest sizes (truncate) — detection must never see archive paths.
#include "../harness.h"
#include "../../shell/detect.h"

#include <cstdio>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

using rowan_shell::Verdict;
using rowan_shell::detect_install;

static std::string tmp_root(const char* name)
{
    std::string d = std::string("/tmp/rowan_detect_") + name;
    mkdir(d.c_str(), 0755);
    return d;
}

// create file of exact size (sparse — truncate)
static void mkfile(const std::string& dir, const char* rel, int64_t size)
{
    std::string p = dir + "/" + rel;
    std::string parent = p.substr(0, p.rfind('/'));
    mkdir(parent.c_str(), 0755);
    FILE* f = fopen(p.c_str(), "wb");
    CHECK(f != nullptr);
    if (!f) return;
    if (size > 0) CHECK(ftruncate(fileno(f), size) == 0);
    fclose(f);
}

static void make_mig(const std::string& d)
{
    mkfile(d, "roots.dir", 197);   mkfile(d, "Battle.dir", 331);
    mkfile(d, "Common.dir", 405);  mkfile(d, "Dir.dir", 128);
    mkfile(d, "Buttons.bmp", 30838); mkfile(d, "INTEL.TTF", 52864);
    mkfile(d, "SMACKER/kimpo.smk", 4727148); mkfile(d, "SMACKER/Migs.smk", 7785548);
    mkfile(d, "INSTALL.LOG", 184598); mkfile(d, "Mig.exe", 2315264);
}

static void make_bob_bdg(const std::string& d)
{
    mkfile(d, "BOBLICENCE.txt", 10990); mkfile(d, "BATTLE.DIR", 323);
    mkfile(d, "COMMON.DIR", 526);  mkfile(d, "DIR.DIR", 128);
    // filelist.txt is game-generated (fileman truncates it each launch) —
    // a 0-byte leftover must not affect detection.
    mkfile(d, "filelist.txt", 0);      mkfile(d, "cardbase.rc", 7720);
    mkfile(d, "bdg.txt", 2862);    mkfile(d, "mfc120.dll", 4424344);
    mkfile(d, "bob.exe", 830464);
}

REGISTER_TEST(detect_mig_retail_ok)
{
    std::string d = tmp_root("mig_ok");
    make_mig(d);
    auto r = detect_install(d.c_str());
    CHECK_EQ((int)r.verdict, (int)Verdict::OK);
    CHECK(r.game == "mig");
    CHECK(r.edition == "retail-compat");
    // per-game filter reaches same result (REQ-LAUNCH-02 embedding model)
    auto r2 = detect_install(d.c_str(), "mig");
    CHECK_EQ((int)r2.verdict, (int)Verdict::OK);
}

REGISTER_TEST(detect_mig_case_insensitive_paths)
{
    std::string d = tmp_root("mig_case");
    // DOS-era installs are case-messy: lowercase names must still resolve
    mkfile(d, "roots.dir", 197);   mkfile(d, "battle.dir", 331);
    mkfile(d, "common.dir", 405);  mkfile(d, "dir.dir", 128);
    mkfile(d, "buttons.bmp", 30838); mkfile(d, "intel.ttf", 52864);
    mkfile(d, "smacker/KIMPO.SMK", 4727148); mkfile(d, "smacker/migs.smk", 7785548);
    mkfile(d, "install.log", 184598); mkfile(d, "mig.exe", 999);
    auto r = detect_install(d.c_str());
    CHECK_EQ((int)r.verdict, (int)Verdict::OK);
    CHECK(r.game == "mig");
}

REGISTER_TEST(detect_bob_bdg_ok)
{
    std::string d = tmp_root("bob_bdg");
    make_bob_bdg(d);
    auto r = detect_install(d.c_str());
    CHECK_EQ((int)r.verdict, (int)Verdict::OK);
    CHECK(r.game == "bob");
    CHECK(r.edition == "BDG-community");
}

REGISTER_TEST(detect_bob_retail_ok)
{
    std::string d = tmp_root("bob_ret");
    mkfile(d, "BOBLICENCE.txt", 10990); mkfile(d, "BATTLE.DIR", 323);
    mkfile(d, "COMMON.DIR", 526);  mkfile(d, "DIR.DIR", 128);
    mkfile(d, "filelist.txt", 0);      mkfile(d, "cardbase.rc", 7720);
    mkfile(d, "bob.exe", 2785792);  // retail binary — any size accepted
    auto r = detect_install(d.c_str());
    CHECK_EQ((int)r.verdict, (int)Verdict::OK);
    CHECK(r.edition == "retail-CD");
}

REGISTER_TEST(detect_incomplete_lists_missing)
{
    std::string d = tmp_root("mig_inc");
    make_mig(d);
    unlink((d + "/SMACKER/kimpo.smk").c_str());
    unlink((d + "/SMACKER/Migs.smk").c_str());
    auto r = detect_install(d.c_str(), "mig");
    CHECK_EQ((int)r.verdict, (int)Verdict::INCOMPLETE);
    CHECK(r.missing.size() == 2);
    CHECK(r.detail.find("SMACKER/kimpo.smk") != std::string::npos);
}

REGISTER_TEST(detect_wrong_size_diagnostic)
{
    std::string d = tmp_root("mig_ws");
    make_mig(d);
    mkfile(d, "roots.dir", 999);   // corrupt size
    auto r = detect_install(d.c_str(), "mig");
    CHECK_EQ((int)r.verdict, (int)Verdict::INCOMPLETE);
    CHECK(r.wrongsize.size() == 1);
    CHECK(r.wrongsize[0].find("exp 197 got 999") != std::string::npos);
}

REGISTER_TEST(detect_not_a_game_dir)
{
    std::string d = tmp_root("junk");
    mkfile(d, "readme.txt", 100);
    auto r = detect_install(d.c_str());
    CHECK_EQ((int)r.verdict, (int)Verdict::NOT_A_GAME);
}

REGISTER_TEST(detect_ambiguous_tie_editions)
{
    // Two same-size edition rows both fully matching = genuinely mixed
    // install -> AMBIGUOUS. Injected table exercises the tie rule directly.
    static const rowan_shell::FileSig SIGS_A[] = {
        { "marker.a", 10 }, { "shared.dat", 55 },
    };
    static const rowan_shell::FileSig SIGS_B[] = {
        { "marker.b", 20 }, { "shared.dat", 55 },
    };
    static const rowan_shell::EditionManifest TBL[] = {
        { "mig", "ed-a", SIGS_A, 2 },
        { "mig", "ed-b", SIGS_B, 2 },
    };
    std::string d = tmp_root("tie");
    mkfile(d, "marker.a", 10); mkfile(d, "marker.b", 20);
    mkfile(d, "shared.dat", 55);
    auto r = rowan_shell::detect_install_with(d.c_str(), nullptr, TBL, 2);
    CHECK_EQ((int)r.verdict, (int)Verdict::AMBIGUOUS);
    CHECK(r.matched.size() == 2);
}

REGISTER_TEST(detect_mixed_dir_resolves_by_game)
{
    // A dir holding one game's files + foreign files is not ambiguous for an
    // embedded exe: only_game restricts candidacy (REQ-LAUNCH-02/03).
    // BATTLE.DIR collides case-insensitively across games (mig 331 / bob 323)
    // so a "merge" on a case-insensitive fs is impossible — mig sees the
    // foreign size as corruption -> INCOMPLETE with diagnostic.
    std::string d = tmp_root("mixed");
    make_bob_bdg(d);
    auto r = detect_install(d.c_str());
    CHECK_EQ((int)r.verdict, (int)Verdict::OK);
    CHECK(r.edition == "BDG-community");
    auto r2 = detect_install(d.c_str(), "mig");
    CHECK_EQ((int)r2.verdict, (int)Verdict::INCOMPLETE);
    CHECK(!r2.detail.empty());
}

REGISTER_TEST(detect_empty_and_null_dirs)
{
    auto r1 = detect_install("");
    CHECK_EQ((int)r1.verdict, (int)Verdict::NOT_A_GAME);
    auto r2 = detect_install("/tmp/rowan_detect_nonexistent_xyz");
    CHECK_EQ((int)r2.verdict, (int)Verdict::NOT_A_GAME);
}
