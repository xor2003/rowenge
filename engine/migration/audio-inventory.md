# BoB audio path inventory (T007)

Date: 2026-10-05 · Scope: `bob-flight-sim` audio code.

## Finding: Miles Sound System (AIL) — same API migalley already ported

BoB's audio lives in `hardware/`: `winmiles.cpp`, `digdrvr.cpp`,
`sample.cpp`, `music.cpp`, `sound.cpp`. It calls the **AIL (Miles Sound
System) API** — the same family migalley reimplemented on SDL_mixer.

Call census (BoB tree): `AIL_shutdown`×12, `AIL_set_sequence_volume`×8,
`AIL_last_error`×7, `AIL_set_sample_loop_count`×5,
`AIL_allocate_sequence_handle`/`AIL_start_sequence`×4 each,
`AIL_start_sample`/`AIL_end_sample`×3 each, `AIL_set_sample_volume`/
`_rate`/`_pan`/`_status`/`_position`, `AIL_register_trigger_callback`,
`AIL_set_preference`. Additional AIL usage in `model/acmman.cpp`,
`model/sky.cpp`, `3d/`, `missman/formatn.cpp`, headers `radiocht.h`,
`sdetail.h`, `weather.h`.

- "Sequences" = MIDI/XMI music via Miles; "samples" = PCM effects.
- **No DirectMusic/DirectSound COM usage found** — the audio dependency is
  the AIL surface only (plus `mmsystem` include once).

## Consequence

The migalley SDL_mixer-backed AIL adapter is the direct donor for BoB's
audio: same API, already proven on Linux. Work = map BoB's used AIL subset
against the existing adapter, extend where BoB uses calls MiG didn't
(sequence volume/trigger callbacks are the likely gaps), and port
`hardware/` glue. This removes the previously feared DirectMusic problem —
no DirectMusic assets or API in the tree.
