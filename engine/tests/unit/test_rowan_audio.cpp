// rowan_audio seam tests: the pure helpers in rowan_audio.h
// (volume clamp, soundfont selection) with dependencies injected —
// no audio device, no real filesystem.

#include "../harness.h"
#include <cstring>
#include <vector>
#include <string>

#include "rowan_audio.h"

namespace {

// Fake filesystem: a set of "existing" paths.
std::vector<std::string> g_present;
int probe_calls = 0;

int fake_exists(const char* path) {
    ++probe_calls;
    for (auto& p : g_present) if (p == path) return 1;
    return 0;
}

} // namespace

// Miles volume clamps into 0..127 at both ends.
REGISTER_TEST(audio_clamp_volume)
{
    CHECK_EQ(rowan_clamp_volume(-1), 0);
    CHECK_EQ(rowan_clamp_volume(-999), 0);
    CHECK_EQ(rowan_clamp_volume(0), 0);
    CHECK_EQ(rowan_clamp_volume(64), 64);
    CHECK_EQ(rowan_clamp_volume(127), 127);
    CHECK_EQ(rowan_clamp_volume(128), 127);
    CHECK_EQ(rowan_clamp_volume(100000), 127);
}

// A non-empty env override wins over every candidate — no probing at all.
REGISTER_TEST(audio_sf_env_override_wins)
{
    g_present = {"/a.sf2", "/b.sf2"};
    probe_calls = 0;
    const char* cands[] = {"/a.sf2", "/b.sf2"};
    const char* r = rowan_pick_soundfont("/env/custom.sf2", cands, 2, fake_exists);
    CHECK_STREQ(r, "/env/custom.sf2");
    CHECK_EQ(probe_calls, 0);              // candidates never consulted
}

// NULL/empty env falls through to the first existing candidate.
REGISTER_TEST(audio_sf_first_existing_candidate)
{
    g_present = {"/b.sf2"};
    const char* cands[] = {"/a.sf2", "/b.sf2", "/c.sf2"};
    CHECK_STREQ(rowan_pick_soundfont(nullptr, cands, 3, fake_exists), "/b.sf2");
    CHECK_STREQ(rowan_pick_soundfont("", cands, 3, fake_exists), "/b.sf2");
}

// No override + nothing present -> NULL (degrades to silence upstream).
// NULL entries in the list are skipped safely.
REGISTER_TEST(audio_sf_nothing_found)
{
    g_present.clear();
    const char* cands[] = {"/a.sf2", nullptr, "/c.sf2"};
    CHECK(rowan_pick_soundfont(nullptr, cands, 3, fake_exists) == nullptr);
    CHECK(rowan_pick_soundfont(nullptr, cands, 0, fake_exists) == nullptr);
    CHECK(rowan_pick_soundfont(nullptr, nullptr, 0, fake_exists) == nullptr);
}
