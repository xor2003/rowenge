// mssw_compat.h, a miles stub — miniaudio + FluidSynth backend
#ifndef MSSW_COMPAT_H
#define MSSW_COMPAT_H

#include <SDL2/SDL.h>

// Force natural layout - straddles DOSDEFS.H's leaked pack(1) across TUs (ODR).
#pragma pack(push)
#pragma pack()

///// RERUN Miles to SDL Start
#pragma once

#include <miniaudio.h>
#include "rowan_audio.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#pragma pack()	// re-assert: an included header may have leaked pack(1) on its first parse

#ifdef __cplusplus
extern "C" {
#endif

// ----- MSS 4.0e type aliases -----
typedef struct _SEQUENCE* HSEQUENCE;
typedef MDI_DRIVER* HMDIDRIVER;

typedef SLong (*MIDIEventCallback)(HMDIDRIVER mdi,
                                   HSEQUENCE seq,
                                   SLong status,
                                   SLong data1,
                                   SLong data2);

typedef struct _MDI_DRIVER {
    int opened;
    int driver_id;
    int master_volume; // 0–127
    MIDIEventCallback callback;
} MDI_DRIVER;

typedef struct _DIG_DRIVER* HDIGDRIVER;
typedef struct _MDI_DRIVER* HMDIDRIVER;
typedef struct _SAMPLE* HSAMPLE;

// Basic types
typedef void* HSTREAM;
typedef void* H3DSAMPLE;
typedef void* HPROVIDER;

typedef void* HDLSFILEID;
typedef void* HWAVESYNTH;
typedef void* HDLSDEVICE;

#define HMIDI MDI_DRIVER*

// ------------------------------------------------------------------
// Sample format constants
// ------------------------------------------------------------------
#define DIG_F_MONO_8   0x0001  // 8-bit mono format
#define DIG_F_STEREO_8 0x0002  // (optional) 8-bit stereo
#define DIG_F_MONO_16  0x0004  // (optional) 16-bit mono
#define DIG_F_STEREO_16 0x0008 // (optional) 16-bit stereo

// ----- Status flags -----
#define SMP_FREE     0x0001
#define SMP_DONE     0x0002
#define SMP_PLAYING  0x0004
#define SMP_STOPPED  0x0008

#define SEQ_FREE     0x0001
#define SEQ_DONE     0x0002
#define SEQ_PLAYING  0x0004
#define SEQ_STOPPED  0x0008

#ifndef MIDI_NULL_DRIVER
#define MIDI_NULL_DRIVER 0xFFFF
#endif

#define WAVE_FORMAT_PCM 1

// ------------------------------------------------------------------
// File type constants
// ------------------------------------------------------------------
#define AILFILETYPE_XMIDI      1   // Standard XMIDI file
#define AILFILETYPE_XMIDI_DLS  2   // XMIDI with DLS instrument set
#define AILFILETYPE_XMIDI_MLS  3   // XMIDI with MLS instrument set

// ------------------------------------------------------------------
// Constant used by AIL_DLS_close
// ------------------------------------------------------------------
#define RETURN_TO_GM_ONLY_STATE 1

// ------------------------------------------------------------------
// Shim structures — miniaudio objects embedded; the engine itself is
// the rowan_audio singleton so all TUs share one device.
// ------------------------------------------------------------------
typedef struct _SAMPLE {
    float* pcm;               /* decoded f32 PCM at pcmRate Hz */
    uint64_t pcmFrames;
    int pcmChans;
    int pcmRate;              /* Hz of pcm data (0 = unknown) */
    ma_audio_buffer abuf;
    int abufInit;
    ma_sound snd;
    int sndInit;
    int loopsLeft;            /* counted-loop remainder */

    int status;
    int volume;        // 0..127
    int pan;           // 0..127
    int loop_count;    // Miles loops semantics (-1 = infinite)
    void (*EOS)(HSAMPLE);
    long playback_rate;

    long format;       // MSS-side format code (optional)
    long flags;        // DIG_F_* hints
    const void* address;
    long length;

    int orig_rate;             // parsed from WAV if available
    int channels;              // 1 or 2
} SAMPLE;

typedef struct _SEQUENCE {
    float* pcm;               /* fluidsynth-rendered f32 stereo 44100 */
    uint64_t pcmFrames;
    ma_audio_buffer abuf;
    int abufInit;
    ma_sound snd;
    int sndInit;
    int status;
    int volume;   // 0..127
    int track;
    int initialized;
} SEQUENCE;

typedef SEQUENCE* HSEQUENCE;

/* end-of-sample: re-arm counted loops, fire EOS on the audio thread —
   same threading contract SDL_mixer's channel-finished callback had. */
static void mssw_sample_end(void* user, ma_sound* snd) {
    SAMPLE* s = (SAMPLE*)user;
    if (s->loopsLeft > 0) {
        --s->loopsLeft;
        ma_sound_seek_to_pcm_frame(snd, 0);
        ma_sound_start(snd);
        return;
    }
    s->status = SMP_DONE;
    if (s->EOS) s->EOS((HSAMPLE)s);
}

// ----- Initialization -----
static inline int AIL_startup(void) {
    /* probe a soundfont once so the fluidsynth path has one */
    if (!getenv("ROWAN_SOUNDFONT")) {
        static const char* fonts[] = {
            "/usr/share/sounds/sf2/FluidR3_GM.sf2",
            "/usr/share/soundfonts/FluidR3_GM.sf2",
            "/usr/share/sounds/sf2/default-GM.sf2",
            "/usr/share/soundfonts/default.sf2",
            "/usr/share/sounds/sf2/TimGM6mb.sf2",
            NULL
        };
        for (int i = 0; fonts[i]; i++) {
            FILE* f = fopen(fonts[i], "rb");
            if (f) { fclose(f); break; }
        }
    }
    return rowan_audio_init(44100);
}

static inline void AIL_shutdown(void) {
    rowan_audio_shutdown();
}

static inline void AIL_set_DirectSound_HWND(HDIGDRIVER dig, void* hwnd) {
    (void)dig;
    (void)hwnd;
    // No-op: DirectSound HWND binding not required
}

// ----- Sample API -----
static inline HSAMPLE AIL_allocate_sample_handle(HDIGDRIVER d) {
    (void)d;
    SAMPLE* s = (SAMPLE*)calloc(1, sizeof(SAMPLE));
    if (!s) return NULL;
    s->status = SMP_FREE;
    s->volume = 127;
    s->pan = 64;
    return (HSAMPLE)s;
}

static void mssw_sample_stop_voice(SAMPLE* s) {
    if (s->sndInit) {
        ma_sound_stop(&s->snd);
        ma_sound_uninit(&s->snd);
        s->sndInit = 0;
    }
}
static void mssw_sample_free_pcm(SAMPLE* s) {
    mssw_sample_stop_voice(s);
    if (s->abufInit) { ma_audio_buffer_uninit(&s->abuf); s->abufInit = 0; }
    free(s->pcm); s->pcm = NULL; s->pcmFrames = 0; s->pcmRate = 0;
}

static inline void AIL_release_sample_handle(HSAMPLE h) {
    SAMPLE* s = (SAMPLE*)h;
    if (!s) return;
    mssw_sample_free_pcm(s);
    free(s);
}

// Minimal RIFF parser to compute WAV length
static long detect_wav_length(const void* address) {
    const unsigned char* buf = (const unsigned char*)address;
    if (!buf) return 0;

    // Must start with "RIFF"
    if (memcmp(buf, "RIFF", 4) != 0) return 0;
    // Must have "WAVE" at offset 8
    if (memcmp(buf + 8, "WAVE", 4) != 0) return 0;

    // Read little-endian 32-bit size at offset 4
    uint32_t riffSize = buf[4] | (buf[5] << 8) | (buf[6] << 16) | (buf[7] << 24);

    // Total length = riffSize + 8
    return (long)riffSize + 8;
}


static int parse_wav_fmt(const unsigned char* buf, int len,
                         int* out_rate, int* out_channels)
{
    if (!buf || len < 44) return 0;
    const unsigned char* p = buf + 12;
    const unsigned char* end = buf + len;
    while (p + 8 <= end) {
        uint32_t ckid = p[0] | (p[1]<<8) | (p[2]<<16) | (p[3]<<24);
        uint32_t cksz = p[4] | (p[5]<<8) | (p[6]<<16) | (p[7]<<24);
        if (p + 8 + cksz > end) break;
        if (ckid == 0x20746D66) { // "fmt "
            uint16_t audioFormat   = p[8] | (p[9] << 8);
            uint16_t numChannels   = p[10] | (p[11] << 8);
            uint32_t sampleRate    = p[12] | (p[13] << 8) | (p[14] << 16) | (p[15] << 24);
            if (audioFormat == WAVE_FORMAT_PCM) {
                *out_rate     = (int)sampleRate;
                *out_channels = (int)numChannels;
                return 1;
            }
            break;
        }
        p += 8 + cksz;
    }
    return 0;
}

/* decode WAV-in-memory to engine-uniform f32 44100 (native channels) */
static int mssw_decode_wav(SAMPLE* s, const void* address, long length) {
    ma_decoder_config cfg =
        ma_decoder_config_init(ma_format_f32, 0, 44100);
    ma_uint64 frames = 0;
    void* pcm = NULL;
    if (ma_decode_memory(address, (size_t)length, &cfg, &frames, &pcm)
            != MA_SUCCESS || !frames)
        return 0;
    s->pcm = (float*)pcm;
    s->pcmFrames = frames;
    s->pcmRate = 44100;
    return 1;
}

static int mssw_sample_setup_abuf(SAMPLE* s) {
    if (s->abufInit) return 1;
    if (!s->pcm || !s->pcmFrames) return 0;
    ma_audio_buffer_config cfg = ma_audio_buffer_config_init(
        ma_format_f32, (ma_uint32)(s->pcmChans ? s->pcmChans : s->channels),
        s->pcmFrames, s->pcm, NULL);
    cfg.sampleRate = s->pcmRate ? (ma_uint32)s->pcmRate : 44100;
    if (ma_audio_buffer_init(&cfg, &s->abuf) != MA_SUCCESS) return 0;
    s->abufInit = 1;
    return 1;
}

static inline int AIL_set_sample_file(HSAMPLE h, const void* address, long length) {
    SAMPLE* s = (SAMPLE*)h;
    if (!s || !address) return 0;

    mssw_sample_free_pcm(s);

    if (length == -1) {
        length = detect_wav_length(address);
        if (length <= 0) return 0;
    }

    int rate = 0, chans = 1;
    parse_wav_fmt((const unsigned char*)address, (int)length, &rate, &chans);
    s->orig_rate   = rate ? rate : 44100;
    s->channels    = chans;
    s->pcmChans    = chans;

    if (!mssw_decode_wav(s, address, length)) return 0;

    s->address = address;
    s->length  = length;
    s->status  = SMP_DONE;
    return 1;
}


static void mssw_apply_voice(SAMPLE* s) {
    if (!s->sndInit) return;
    ma_sound_set_volume(&s->snd, s->volume / 127.0f);
    ma_sound_set_pan(&s->snd, (s->pan - 64) / 64.0f);
    if (s->playback_rate && s->orig_rate)
        ma_sound_set_pitch(&s->snd, (float)s->playback_rate / (float)s->orig_rate);
}

static inline void AIL_set_sample_volume(HSAMPLE h, int vol) {
    SAMPLE* s = (SAMPLE*)h;
    if (!s) return;
    if (vol < 0) vol = 0;
    if (vol > 127) vol = 127;
    s->volume = vol;
    if (s->sndInit) ma_sound_set_volume(&s->snd, vol / 127.0f);
}

static inline void AIL_set_sample_pan(HSAMPLE h, int pan) {
    SAMPLE* s = (SAMPLE*)h;
    if (!s) return;
    if (pan < 0) pan = 0;
    if (pan > 127) pan = 127;
    s->pan = pan;
    if (s->sndInit) ma_sound_set_pan(&s->snd, (pan - 64) / 64.0f);
}

// Initialize a sample to default state
static inline void AIL_init_sample(HSAMPLE h) {
    SAMPLE* s = (SAMPLE*)h;
    if (!s) return;
    s->status     = SMP_DONE;
    s->volume     = 127;
    s->pan        = 64;
    s->loop_count = 0;
    s->EOS        = NULL;
    // PCM buffer left intact; caller sets via AIL_set_sample_file
}

// Stop playback immediately
static inline void AIL_stop_sample(HSAMPLE h) {
    SAMPLE* s = (SAMPLE*)h;
    if (!s) return;
    mssw_sample_stop_voice(s);
    s->status = SMP_STOPPED;
}

// Resume playback if paused/stopped
static inline void AIL_resume_sample(HSAMPLE h) {
    SAMPLE* s = (SAMPLE*)h;
    if (!s) return;
    if (s->sndInit && !ma_sound_is_playing(&s->snd)) {
        ma_sound_start(&s->snd);
        s->status = SMP_PLAYING;
    }
}

// Set playback rate (pitch) in Hz — miniaudio resamples natively,
// so this is a live pitch factor, not a stored placeholder.
static inline void AIL_set_sample_playback_rate(HSAMPLE h, long rate) {
    SAMPLE* s = (SAMPLE*)h;
    if (!s) return;
    s->playback_rate = rate;
    if (s->sndInit && s->orig_rate)
        ma_sound_set_pitch(&s->snd, (float)rate / (float)s->orig_rate);
}


static inline int AIL_start_sample(HSAMPLE h) {
    SAMPLE* s = (SAMPLE*)h;
    if (!s || !s->abufInit && !mssw_sample_setup_abuf(s)) return 0;
    ma_engine* eng = rowan_audio_engine();
    if (!eng) return 0;

    mssw_sample_stop_voice(s);
    if (ma_sound_init_from_data_source(eng, &s->abuf, 0, NULL, &s->snd)
            != MA_SUCCESS)
        return 0;
    s->sndInit = 1;

    s->loopsLeft = s->loop_count > 0 ? s->loop_count : 0;
    if (s->loop_count < 0) {
        ma_data_source_set_looping(&s->abuf, MA_TRUE);
        ma_sound_set_looping(&s->snd, MA_TRUE);
    } else {
        ma_data_source_set_looping(&s->abuf, MA_FALSE);
        ma_sound_set_looping(&s->snd, MA_FALSE);
        ma_sound_set_end_callback(&s->snd, mssw_sample_end, s);
    }
    mssw_apply_voice(s);
    if (ma_sound_start(&s->snd) != MA_SUCCESS) {
        mssw_sample_stop_voice(s);
        return 0;
    }
    s->status = SMP_PLAYING;
    return 1;
}

static inline void AIL_end_sample(HSAMPLE h) {
    AIL_stop_sample(h);
}

static inline int AIL_sample_status(HSAMPLE h) {
    SAMPLE* s = (SAMPLE*)h;
    if (!s) return SMP_FREE;
    if (s->sndInit && ma_sound_is_playing(&s->snd)) return SMP_PLAYING;
    return s->status == SMP_STOPPED ? SMP_STOPPED : SMP_DONE;
}

// ----- Sequence (music) API — fluidsynth renders XMIDI/SMF to PCM -----
static inline HSEQUENCE AIL_allocate_sequence_handle(HMDIDRIVER d) {
    (void)d;
    SEQUENCE* m = (SEQUENCE*)calloc(1, sizeof(SEQUENCE));
    if (!m) return NULL;
    m->status = SEQ_FREE;
    m->volume = 127;
    return (HSEQUENCE)m;
}

static void mssw_seq_stop_voice(SEQUENCE* m) {
    if (m->sndInit) {
        ma_sound_stop(&m->snd);
        ma_sound_uninit(&m->snd);
        m->sndInit = 0;
    }
}
static void mssw_seq_free_pcm(SEQUENCE* m) {
    mssw_seq_stop_voice(m);
    if (m->abufInit) { ma_audio_buffer_uninit(&m->abuf); m->abufInit = 0; }
    free(m->pcm); m->pcm = NULL; m->pcmFrames = 0;
}

static inline void AIL_release_sequence_handle(HSEQUENCE h) {
    SEQUENCE* m = (SEQUENCE*)h;
    if (!m) return;
    mssw_seq_free_pcm(m);
    free(m);
}

static inline int AIL_set_sequence_file(HSEQUENCE h, const char* path) {
    SEQUENCE* m = (SEQUENCE*)h;
    if (!m || !path) return 0;
    FILE* f = fopen(path, "rb");
    if (!f) return 0;
    fseek(f, 0, SEEK_END);
    long n = ftell(f);
    fseek(f, 0, SEEK_SET);
    void* buf = malloc(n > 0 ? n : 1);
    if (buf && fread(buf, 1, (size_t)n, f) != (size_t)n) { free(buf); buf = NULL; }
    fclose(f);
    if (!buf) return 0;
    mssw_seq_free_pcm(m);
    m->pcmFrames = rowan_midi_render(buf, (size_t)n, m->track, &m->pcm);
    free(buf);
    if (!m->pcmFrames) return 0;
    m->initialized = 1;
    return 1;
}

static inline int AIL_start_sequence(HSEQUENCE h) {
    SEQUENCE* m = (SEQUENCE*)h;
    if (!m || !m->pcm || !m->pcmFrames) return 0;
    ma_engine* eng = rowan_audio_engine();
    if (!eng) return 0;

    if (!m->abufInit) {
        ma_audio_buffer_config cfg = ma_audio_buffer_config_init(
            ma_format_f32, 2, m->pcmFrames, m->pcm, NULL);
        cfg.sampleRate = 44100;
        if (ma_audio_buffer_init(&cfg, &m->abuf) != MA_SUCCESS) return 0;
        m->abufInit = 1;
    }
    mssw_seq_stop_voice(m);
    if (ma_sound_init_from_data_source(eng, &m->abuf, 0, NULL, &m->snd)
            != MA_SUCCESS)
        return 0;
    m->sndInit = 1;
    ma_sound_set_volume(&m->snd, m->volume / 127.0f);
    if (ma_sound_start(&m->snd) != MA_SUCCESS) {
        mssw_seq_stop_voice(m);
        return 0;
    }
    m->status = SEQ_PLAYING;
    return 1;
}


static inline void AIL_stop_sequence(HSEQUENCE h) {
    SEQUENCE* m = (SEQUENCE*)h;
    if (!m) return;
    mssw_seq_stop_voice(m);
    m->status = SEQ_STOPPED;
}

static inline int AIL_sequence_status(HSEQUENCE h) {
    SEQUENCE* m = (SEQUENCE*)h;
    if (!m) return SEQ_FREE;
    if (m->sndInit && ma_sound_is_playing(&m->snd)) return SEQ_PLAYING;
    return m->status == SEQ_PLAYING ? SEQ_DONE : m->status;
}

inline int AIL_midiOutOpen(HMDIDRIVER* mdi,
                           void* midiOutDev,
                           unsigned int uDriverID)
{
    (void)midiOutDev; // unused

    HMDIDRIVER d = (HMDIDRIVER)calloc(1, sizeof(*d));
    if (!d) return -1;

    d->opened = 1;
    d->driver_id = (int)uDriverID;

    if (mdi) *mdi = d;

    return 0; // success
}


typedef struct _DLS_SYNTH {
    int sample_rate;
    int bits;
    int channels;
    int opened;
} DLS_SYNTH;

inline void* AIL_DLS_open(void* dls,
                          HDIGDRIVER dig,
                          const char* DLS_dll_name,
                          unsigned int flags,
                          int sample_value,
                          int bits,
                          int channels)
{
    (void)dls;
    (void)dig;
    (void)DLS_dll_name;
    (void)flags;

    DLS_SYNTH* synth = (DLS_SYNTH*)calloc(1, sizeof(DLS_SYNTH));
    if (!synth) return NULL;

    synth->sample_rate = sample_value ? sample_value : 44100;
    synth->bits = bits;
    synth->channels = channels;
    synth->opened = 1;

    return synth;
}

typedef struct _WAVE_SYNTH {
    HDIGDRIVER dig;
    HMDIDRIVER mdi;
    void*      config;
    int        flags;
    int        opened;
} WAVE_SYNTH;

inline void* AIL_create_wave_synthesizer(HDIGDRIVER dig,
                                         HMDIDRIVER mdi,
                                         void* tmpptr,
                                         int flags)
{
    WAVE_SYNTH* synth = (WAVE_SYNTH*)calloc(1, sizeof(WAVE_SYNTH));
    if (!synth) return NULL;

    synth->dig    = dig;
    synth->mdi    = mdi;
    synth->config = tmpptr;
    synth->flags  = flags;
    synth->opened = 1;

    // MIDI playback is handled by fluidsynth via rowan_midi_render.
    return synth;
}


// ------------------------------------------------------------------
// Digital driver
// ------------------------------------------------------------------
inline HDIGDRIVER AIL_open_digital_driver(int rate, int bits, int channels) { return nullptr; }
inline void AIL_close_digital_driver(HDIGDRIVER driver) {}

// ------------------------------
// Helpers: flag decoding
// ------------------------------
static inline int sample_channels_from_flags(long flags) {
    if (flags & (DIG_F_STEREO_8 | DIG_F_STEREO_16)) return 2;
    if (flags & (DIG_F_MONO_8   | DIG_F_MONO_16))   return 1;
    return 1;
}

inline void AIL_set_sample_type(HSAMPLE sample, SLong format, SLong flags) {
    SAMPLE* s = (SAMPLE*)sample;
    if (!s) return;

    s->format = format;
    s->flags  = flags;

    if (s->orig_rate == 0) s->orig_rate = 44100;
    if (s->channels   == 0) s->channels = sample_channels_from_flags(flags);
}


// Convert 8-bit unsigned mono samples to f32 mono
static float* u8_to_f32(const uint8_t* b, long n) {
    float* out = (float*)malloc((size_t)n * sizeof(float));
    if (!out) return NULL;
    for (long i = 0; i < n; i++)
        out[i] = (b[i] - 128) / 128.0f;
    return out;
}

// Inside your AIL_set_sample_address implementation:
static inline void AIL_set_sample_address(HSAMPLE sample, const void* start, long len) {
    SAMPLE* s = (SAMPLE*)sample;
    if (!s) return;

    s->address = start;
    s->length  = len;

    mssw_sample_free_pcm(s);

    if (!start || len <= 0) return;

    const uint8_t* buf = (const uint8_t*)start;

    // --- WAV path — miniaudio decodes + resamples; no manual CVT ---
    if (len >= 12 && memcmp(buf, "RIFF", 4) == 0 && memcmp(buf + 8, "WAVE", 4) == 0) {
        int rate = 0, chans = 1;
        parse_wav_fmt(buf, (int)len, &rate, &chans);
        s->orig_rate = rate ? rate : 44100;
        s->channels  = chans;
        s->pcmChans  = chans;
        if (mssw_decode_wav(s, start, len))
            s->status = SMP_DONE;
        return;
    }

    // --- Radio messages path: raw u8 22050 Hz mono -> f32 ---
    if (s->format == DIG_F_MONO_8) {
        float* pcm = u8_to_f32(buf, len);
        if (pcm) {
            s->pcm        = pcm;
            s->pcmFrames  = (uint64_t)len;
            s->pcmChans   = 1;
            s->pcmRate    = 22050;
            s->orig_rate  = 22050;
            s->channels   = 1;
            s->status     = SMP_DONE;
        }
        return;
    }
}

// ------------------------------------------------------------------
// Stream handling
// ------------------------------------------------------------------
inline HSTREAM AIL_open_stream(HDIGDRIVER driver, const char* filename, int flags) { return nullptr; }
inline void AIL_close_stream(HSTREAM stream) {}
inline void AIL_start_stream(HSTREAM stream) {}
inline void AIL_pause_stream(HSTREAM stream, int pause) {}
inline int AIL_stream_volume(HSTREAM stream) { return 0; }
inline void AIL_set_stream_volume(HSTREAM stream, int volume) {}

// ------------------------------------------------------------------
// 3D sample handling
// ------------------------------------------------------------------
inline void AIL_start_3D_sample(H3DSAMPLE sample) {}
inline void AIL_stop_3D_sample(H3DSAMPLE sample) {}
inline void AIL_resume_3D_sample(H3DSAMPLE sample) {}
inline void AIL_end_3D_sample(H3DSAMPLE sample) {}
inline int AIL_3D_sample_volume(H3DSAMPLE sample) { return 0; }
inline void AIL_set_3D_sample_volume(H3DSAMPLE sample, float volume) {}
inline int AIL_3D_sample_playback_rate(H3DSAMPLE sample) { return 0; }
inline void AIL_set_3D_sample_playback_rate(H3DSAMPLE sample, int rate) {}

// ------------------------------------------------------------------
// Preferences / misc
// ------------------------------------------------------------------
inline int AIL_set_preference(unsigned int number, int value) { return 0; }

inline HDLSFILEID AIL_DLS_load_file(const char* filename) { return nullptr; }
inline void AIL_DLS_unload_file(HDLSFILEID file) {}
inline HWAVESYNTH AIL_open_wave_synthesizer() { return nullptr; }
inline void AIL_close_wave_synthesizer(HWAVESYNTH synth) {}
inline HDLSDEVICE AIL_DLS_open_device(HWAVESYNTH synth) { return nullptr; }
inline void AIL_DLS_close_device(HDLSDEVICE device) {}

// Return the current volume of a sequence.
typedef struct _SEQUENCE    * HSEQUENCE;       // Handle to sequence
inline int AIL_sequence_volume(HSEQUENCE sequence) {
    SEQUENCE* m = (SEQUENCE*)sequence;
    return m ? m->volume : 0;
}

// In Miles, this returns the current branch index of an XMIDI sequence.
inline int AIL_branch_index(HSEQUENCE sequence, long newcode) {
    (void)sequence;
    (void)newcode;
    return 0;
}

// In Miles, this sets the playback volume of a sequence (0–127).
inline void AIL_set_sequence_volume(HSEQUENCE seq, int chan, int vol) {
    SEQUENCE* m = (SEQUENCE*)seq;
    (void)chan;
    if (!m) return;
    if (vol < 0) vol = 0;
    if (vol > 127) vol = 127;
    m->volume = vol;
    if (m->sndInit) ma_sound_set_volume(&m->snd, vol / 127.0f);
}

// In Miles: opens a digital audio driver with given format.
typedef struct _DIG_DRIVER {
    int sample_rate;
    int channels;
    int format;
    int opened;
    int master_volume; // 0–127
} DIG_DRIVER;


static inline void AIL_set_digital_master_volume(HDIGDRIVER dig, int volume) {
    if (!dig) return;

    DIG_DRIVER* d = (DIG_DRIVER*)dig;

    // Clamp to 0–127
    if (volume < 0) volume = 0;
    if (volume > 127) volume = 127;

    d->master_volume = volume;
    rowan_audio_set_master_volume(volume);
}

inline int AIL_waveOutOpen(HDIGDRIVER* driver,
                           void* waveout,   // unused
                           int id,          // unused
                           void* format)    // pointer to WAVEFORMAT/PCMWAVEFORMAT
{
    (void)waveout;
    (void)id;

    DIG_DRIVER* d = (DIG_DRIVER*)calloc(1, sizeof(DIG_DRIVER));
    if (!d) return -1;

    // Default values
    int freq = 44100;
    int channels = 2;

    if (format) {
        PCMWAVEFORMAT* wf = (PCMWAVEFORMAT*)format;
        freq = wf->wf.nSamplesPerSec;
        channels = wf->wf.nChannels;
    }

    if (!rowan_audio_init(freq)) {
        free(d);
        return -1;
    }

    d->sample_rate = freq;
    d->channels = channels;
    d->opened = 1;

    if (driver) *driver = d;
    return 0; // success
}

// In Miles: closes the audio driver and releases resources.
inline void AIL_waveOutClose(HDIGDRIVER driver) {
    if (!driver) return;

    DIG_DRIVER* d = (DIG_DRIVER*)driver;
    d->opened = 0;   // engine is shared — game calls AIL_shutdown to close it

    free(d);
}


// In Miles: returns a string describing the last error.
inline const char* AIL_last_error() {
    return "No error";
}

static long detect_midi_length(const void* address) {
    const unsigned char* buf = (const unsigned char*)address;
    if (!buf) return 0;

    // XMIDI: "FORM" <size:4> <type:4> ...
    if (memcmp(buf, "FORM", 4) == 0) {
        uint32_t size = (buf[4] << 24) | (buf[5] << 16) | (buf[6] << 8) | buf[7];
        return (long)size + 8;
    }

    // Standard MIDI: "MThd" <size:4> ...
    if (memcmp(buf, "MThd", 4) == 0) {
        long pos = 0;
        uint32_t len = (buf[pos+4] << 24) | (buf[pos+5] << 16) | (buf[pos+6] << 8) | buf[pos+7];
        int ntrks = (buf[pos+10] << 8) | buf[pos+11];
        pos += 8 + len;

        for (int i = 0; i < ntrks; i++) {
            if (memcmp(buf + pos, "MTrk", 4) != 0) break;
            uint32_t tlen = (buf[pos+4] << 24) | (buf[pos+5] << 16) | (buf[pos+6] << 8) | buf[pos+7];
            pos += 8 + tlen;
        }
        return pos;
    }
    return 0;
}

inline int AIL_init_sequence(HSEQUENCE seq, void* data, int track) {
    if (!seq || !data) return -1;
    SEQUENCE* m = (SEQUENCE*)seq;

    // Free any existing render
    mssw_seq_free_pcm(m);

    long length = detect_midi_length(data);
    if (length <= 0) return -1;

    m->track = track;
    m->pcmFrames = rowan_midi_render(data, (size_t)length, track, &m->pcm);
    if (!m->pcmFrames) return -1;
    m->initialized = 1;
    return 0;
}


inline void AIL_end_sequence(HSEQUENCE seq) {
    AIL_stop_sequence(seq);
}


// Real Miles: registers a callback for XMIDI trigger events.
typedef void (*AILTRIGGERCB)(HSEQUENCE seq, SLong channel, SLong value);
inline void AIL_register_trigger_callback(HSEQUENCE seq, AILTRIGGERCB callback) {
    (void)seq;
    (void)callback;
}

inline void AIL_resume_sequence(HSEQUENCE seq) {
    SEQUENCE* m = (SEQUENCE*)seq;
    if (m && m->sndInit && !ma_sound_is_playing(&m->snd))
        ma_sound_start(&m->snd);
}

// Real Miles: locks and unlock internal data structures (thread safety).
inline void AIL_lock() {
    // no-op
}
inline void AIL_unlock() {
    // no-op
}

// ------------------------------------------------------------------
// File type detection
// ------------------------------------------------------------------
inline long AIL_file_type(void* headptr, long size) {
    (void)headptr;
    (void)size;
    return AILFILETYPE_XMIDI;
}

// ------------------------------------------------------------------
// Current sample position (in samples/bytes depending on API usage)
// ------------------------------------------------------------------
inline SLong AIL_sample_position(HSAMPLE sample) {
    SAMPLE* s = (SAMPLE*)sample;
    if (!s || !s->sndInit) return 0;
    ma_uint64 cur = 0;
    ma_sound_get_cursor_in_pcm_frames(&s->snd, &cur);
    return (SLong)cur;
}

inline void AIL_set_sample_loop_count(HSAMPLE sample, SLong count) {
    SAMPLE* s = (SAMPLE*)sample;
    if (!s) return;
    s->loop_count = (count <= 0) ? -1 : (int)(count - 1);
}


// ------------------------------------------------------------------
// Destroy wave synthesizer
// ------------------------------------------------------------------
inline void AIL_destroy_wave_synthesizer(HWAVESYNTH wavesynth) {
    free(wavesynth);
}

// ------------------------------------------------------------------
// Close DLS wave synthesizer
// ------------------------------------------------------------------
inline void AIL_DLS_close(HDLSDEVICE dlswavesynth, SLong state) {
    (void)state;
    free(dlswavesynth);
}

// ------------------------------------------------------------------
// Release timer handle
// ------------------------------------------------------------------
inline void AIL_release_timer_handle(SLong timer) {
    (void)timer;
    // no-op
}


inline void AIL_midiOutClose(HMIDI midi) {
    free(midi);
}

static inline void AIL_set_XMIDI_master_volume(HMDIDRIVER mdi, int volume) {
    if (!mdi) return;

    // Clamp to 0–127
    if (volume < 0) volume = 0;
    if (volume > 127) volume = 127;

    mdi->master_volume = volume;
    rowan_audio_set_master_volume(volume);
}

static inline SLong AIL_extract_DLS(void* ptr,
                                    SLong size,
                                    void** xmiPtr,
                                    SLong* xmiSize,
                                    void** dlsPtr,
                                    SLong* dlsSize)
{
    if (!ptr || size <= 0) return -1;

    // For now, assume the block is just XMIDI data
    if (xmiPtr) *xmiPtr = ptr;
    if (xmiSize) *xmiSize = size;

    // No DLS support in the shim
    if (dlsPtr) *dlsPtr = NULL;
    if (dlsSize) *dlsSize = 0;

    return 0; // success
}


static inline HDLSFILEID AIL_DLS_load_memory(void* wavesynth,
                                             void* dlsPtr,
                                             unsigned int flags)
{
    (void)wavesynth;
    (void)dlsPtr;
    (void)flags;

    if (!dlsPtr) return NULL;

    // Allocate a dummy handle to represent the loaded DLS bank
    void* fakeHandle = calloc(1, 1); // just a unique pointer
    return (HDLSFILEID)fakeHandle;
}

static inline SLong AIL_find_DLS(void* ptr,
                                 SLong size,
                                 void** xmiPtr,
                                 SLong* xmiSize,
                                 void** dlsPtr,
                                 SLong* dlsSize)
{
    if (!ptr || size <= 0) return -1;

    // For now, assume the block is just XMIDI data
    if (xmiPtr) *xmiPtr = ptr;
    if (xmiSize) *xmiSize = size;

    // No DLS support in the shim
    if (dlsPtr) *dlsPtr = NULL;
    if (dlsSize) *dlsSize = 0;

    return 0; // success
}


static inline void AIL_register_event_callback(HMDIDRIVER mdi,
                                               MIDIEventCallback cb)
{
    if (!mdi) return;
    mdi->callback = cb;
}

#ifdef __cplusplus
}
#endif

#pragma pack(pop)
#endif // MSSW_COMPAT_H
