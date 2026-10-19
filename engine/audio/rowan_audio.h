/* rowan_audio — singletons shared across every TU that includes
   mssw_compat.h (static inline headers give each TU its own copy of
   file-scope globals, so the ma_engine/FluidSynth state lives here). */
#pragma once

#include <miniaudio.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Engine singleton — created lazily by rowan_audio_init(). NULL before
   init or after shutdown. */
ma_engine* rowan_audio_engine(void);
int        rowan_audio_init(int sampleRate);   /* idempotent */
void       rowan_audio_shutdown(void);
void       rowan_audio_set_master_volume(int vol127); /* Miles 0..127 */

/* Render a MIDI sequence to 32-bit float stereo PCM at 44100 Hz via
   FluidSynth. `data` may be SMF (MThd) or Miles XMIDI (FORM/XDIR);
   `track` selects the sequence inside an XMIDI CAT.
   Returns frame count (>0) with *outPcm malloc'd, or 0 on failure. */
uint64_t rowan_midi_render(const void* data, size_t len, int track,
                           float** outPcm);

/* ---- testable seams (pure logic, dependencies injected) -------------
   These keep the policy separate from the environment so unit tests
   drive fakes instead of touching the real filesystem/audio device. */

/* Master-volume clamp: Miles volume is 0..127; out-of-range callers
   (negative, >127) saturate. */
static inline int rowan_clamp_volume(int vol127) {
    return vol127 < 0 ? 0 : (vol127 > 127 ? 127 : vol127);
}

/* Soundfont selection: env override wins when non-empty, else the first
   candidate that `exists` reports present. Returns NULL when neither.
   `exists` is injected — production passes an fopen probe, tests a fake. */
static inline const char* rowan_pick_soundfont(
        const char* envOverride,
        const char* const* candidates, size_t candidateCount,
        int (*exists)(const char* path)) {
    if (envOverride && envOverride[0]) return envOverride;
    for (size_t i = 0; i < candidateCount; ++i)
        if (candidates[i] && exists(candidates[i])) return candidates[i];
    return NULL;
}

#ifdef __cplusplus
}
#endif
