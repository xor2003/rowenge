# Capability ledger (T010) — subsystem presence and lineage per game

Date: 2026-10-05. Sources: census (484 shared paths), task surveys, tree inspection.
Donor direction per owner: migalley = older lineage + fixes; BoB = newer
lineage + open defects.

| Subsystem | MiG (migalley/) | BoB (bob-flight-sim/) | Notes |
|---|---|---|---|
| Files/case-resolver | `wincompat/fileman.cpp` `fopen_ci` | `fileman` — case/lookup differs | Shared resolver candidate; test REQ-TEST |
| Fixed-point math | `Math/` | `Math/` | Same API family; MiG has bug-fix tests to transfer |
| 3D engine | `3d/` + `win3d` internals→Vulkan | `3d/` + `lib3d` (D3D7 DLL) | Different port levels; lib3d must port behind own API (T006) |
| Battlefields/terrain | `Bfields/` | `Bfields/` | Byte-compatible format? verify with format tests |
| Shapes loader | `Shapes/` | `Shapes/` | Shared instruction-layout tests planned |
| Model | `model/` | `model/` | BoB version newer (defects per owner) |
| Movecode | `Movecode/` | `Movecode/` | Zero Windows deps in both |
| Frontend | Sheets → MFC_stub | `mfc/` ~79K lines + 15 `r*` OCX dirs | BoB's real port cost; stub needs Win32-surface extension (T005) |
| Audio | AIL→SDL_mixer adapter (`winmiles` etc.) | Miles AIL (`hardware/*.cpp`) | Same API — adapter reuse route (T007) |
| Input | Win32 msg + XInput→SDL2 | DirectInput-era + rjoycfg OCX | Map BoB's DI usage to SDL2/XInput layer |
| Net/comms | `COMMS` | `comms/` | Migrate+link per REQ-PORT-02; functional acceptance deferred |
| Video | Smack (`smack_stub`) | Smacker (`smkplayer`) | Same-era Smacker; shared stub candidate |
| Installer/registry | `Detect` + `roots.dir` | registry path + `Detect` | REQ-DETECT-01/02 — signature-file detection (T008) |
| Persistence | savegame/USERPROF formats | `savegame/` + USERPROF | Binary compat per game; parsers tested |

## License/provenance notes

- Both trees carry the Rowan/Empire "Source Code … Agreement" header (seen
  in `direct_3d.cpp`, `flagsw.cpp`): source released for non-profit
  derivative works with prominent change notices. Same notice family on both
  lineages — derivative notices required on ported files.
- Redistribution of game data/assets in fixtures: private suites only
  (constitution P10 + spec REQ-TEST-01).
