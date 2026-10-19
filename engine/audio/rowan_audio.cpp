/* rowan_audio — miniaudio engine singleton + FluidSynth MIDI renderer.
   All state lives here so every TU that includes mssw_compat.h shares
   one device and one FluidSynth path. */

#include "rowan_audio.h"

#if ROWAN_HAVE_FLUIDSYNTH
#include <fluidsynth.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mutex>

extern "C" int xmi_to_smf(const void* in, size_t inLen, int seq,
                          uint8_t** out, size_t* outLen);

static ma_engine g_engine;
static bool      g_engineInit = false;
static int       g_masterVol  = 127;
static std::mutex g_mx;

ma_engine* rowan_audio_engine(void) {
    return g_engineInit ? &g_engine : nullptr;
}

int rowan_audio_init(int sampleRate) {
    std::lock_guard<std::mutex> lk(g_mx);
    if (g_engineInit) return 1;
    ma_engine_config cfg = ma_engine_config_init();
    cfg.sampleRate = sampleRate > 0 ? (ma_uint32)sampleRate : 44100;
    if (ma_engine_init(&cfg, &g_engine) != MA_SUCCESS) return 0;
    g_engineInit = true;
    ma_engine_set_volume(&g_engine, g_masterVol / 127.0f);
    return 1;
}

void rowan_audio_shutdown(void) {
    std::lock_guard<std::mutex> lk(g_mx);
    if (!g_engineInit) return;
    ma_engine_uninit(&g_engine);
    g_engineInit = false;
}

void rowan_audio_set_master_volume(int vol127) {
    std::lock_guard<std::mutex> lk(g_mx);
    if (vol127 < 0) vol127 = 0;
    if (vol127 > 127) vol127 = 127;
    g_masterVol = vol127;
    if (g_engineInit)
        ma_engine_set_volume(&g_engine, vol127 / 127.0f);
}

/* ---- MIDI via FluidSynth --------------------------------------------
   Offline render: player + synth (no audio driver); pulling
   fluid_synth_write_float advances the player through the sequence.
   Output: f32 stereo 44100. */

static const char* rowan_soundfont(void) {
    static const char* chosen = nullptr;
    if (chosen) return chosen;
    if (const char* e = getenv("ROWAN_SOUNDFONT")) {
        if (e[0]) return chosen = e;
    }
    static const char* cands[] = {
        "/usr/share/sounds/sf2/FluidR3_GM.sf2",
        "/usr/share/soundfonts/FluidR3_GM.sf2",
        "/usr/share/sounds/sf2/default-GM.sf2",
        "/usr/share/soundfonts/default.sf2",
        "/usr/share/sounds/sf2/TimGM6mb.sf2",
        nullptr
    };
    for (int i = 0; cands[i]; ++i) {
        FILE* f = fopen(cands[i], "rb");
        if (f) { fclose(f); return chosen = cands[i]; }
    }
    return nullptr;
}

uint64_t rowan_midi_render(const void* data, size_t len, int track,
                           float** outPcm) {
    *outPcm = nullptr;
#if !ROWAN_HAVE_FLUIDSYNTH
    (void)data; (void)len; (void)track;
    return 0;          /* no synth — music degrades to silence */
#else
    if (!data || len < 8) return 0;

    /* XMIDI -> SMF; SMF passes through */
    const uint8_t* smf = (const uint8_t*)data;
    size_t smfLen = len;
    uint8_t* conv = nullptr;
    if (!memcmp(data, "FORM", 4)) {
        if (xmi_to_smf(data, len, track, &conv, &smfLen)) return 0;
        smf = conv;
    } else if (memcmp(data, "MThd", 4)) {
        return 0;   /* not a MIDI container */
    }

    const char* sf = rowan_soundfont();
    if (!sf) { free(conv); return 0; }

    fluid_settings_t* st = new_fluid_settings();
    fluid_settings_setnum(st, "synth.sample-rate", 44100.0);
    fluid_synth_t* sy = new_fluid_synth(st);
    if (!sy || fluid_synth_sfload(sy, sf, 1) < 0) {
        if (sy) delete_fluid_synth(sy);
        delete_fluid_settings(st);
        free(conv);
        return 0;
    }
    fluid_player_t* pl = new_fluid_player(sy);
    if (!pl || fluid_player_add_mem(pl, smf, smfLen) != FLUID_OK) {
        if (pl) delete_fluid_player(pl);
        delete_fluid_synth(sy); delete_fluid_settings(st);
        free(conv);
        return 0;
    }
    free(conv);
    fluid_player_play(pl);

    /* pull-render until the player is done; cap at ~10 min of audio */
    const int N = 4096;
    size_t cap = 0, frames = 0;
    float* pcm = nullptr;
    float l[N], r[N];
    while (fluid_player_get_status(pl) == FLUID_PLAYER_PLAYING &&
           frames < 44100u * 600) {
        if (frames + N > cap) {
            size_t nc = (cap ? cap * 2 : 44100u * 60);
            float* np = (float*)realloc(pcm, nc * 2 * sizeof(float));
            if (!np) break;
            pcm = np; cap = nc;
        }
        fluid_synth_write_float(sy, N, l, 0, 1, r, 0, 1);
        for (int i = 0; i < N; ++i) {
            pcm[2 * (frames + i)]     = l[i];
            pcm[2 * (frames + i) + 1] = r[i];
        }
        frames += N;
    }
    delete_fluid_player(pl);
    delete_fluid_synth(sy);
    delete_fluid_settings(st);
    if (!frames) { free(pcm); return 0; }
    *outPcm = pcm;
    return frames;
#endif
}
