# Pathological architectural / code-organizational problems

Audit of `bob/` + `migalley/` (clang-check sweep 2026-10 + manual review).
Crash-class items found by clang-check were fixed in the source; this file
records the *systemic* problems that remain, ordered by how much they will
bite future work. Items marked **[OPEN]** are unresolved cleanup tasks;
items marked **[DONE]** were fixed in the crash-hardening pass.

## UB / correctness landmines

1. **`*(T*)NULL` null-reference idiom — ~126 sites audited.** Two distinct
   sub-patterns: (a) **114 default-arg sites** — `f(CRect r=*(CRect*)NULL)`
   optional-param idiom concentrated in `RDIALOG.H`/`rdialog.h`/`TOPPLATE.H`
   (MFC dialog API contract; callers probe `if(&r)`); (b) **12 deref/sentinel
   sites** — `return *(Target*)NULL`, `const DialBox& ND=*(DialBox*)NULL`.
   Kept alive only by `-fno-delete-null-pointer-checks` (enforced for all
   targets in migalley/CMakeLists.txt + engine/CMakeLists.txt — verified in
   compile_commands.json). Every new toolchain/flag is one optimization away
   from mass UB.
   **[DONE — commit 54d105b. DialBox/IdEntry/DialList gained `Null()` +
   `AsPtr()` sentinels (callers pass temporaries, so pointers couldn't
   work); scalar out-params became `T* =NULL` with `&var` callers;
   `AddMessage` binds a real `TargetIndexes()` temp; `Target::Nil()` and
   `ItemBase::Nil()` sentinels with nil-aware conversion ops +
   `IsNil()`; `ItemBase::operator delete` skips the sentinel;
   `MakeInteractive`/`MakePassive`/`MakeResize`/`AddChildren` take
   `const CRect*`; dead `NULLREF` macro removed. `&ConvertPtrUID` callers
   fixed to use the conversion directly. `-fno-delete-null-pointer-checks`
   retained as belt-and-braces but no longer load-bearing]**

2. **Release-no-op "asserts".** `INT3`, `bobassert`, `NODEFAULT`,
   `default_neverreferenced` compile to nothing in release — code then
   derefs the NULL it just "checked".
   **[DONE at flag sites — pattern still lurks; add real guards when
   touching a switch/deref]**

3. **`breakif`/`loop` macros hide control flow.** Half the found bug sites
   were `if(cond) stmt; stmt2;` where `stmt2` ran unconditionally.
   **[DONE — verified `breakif(cond)` expands to a braced `{if(cond)break;}`
   (self-contained statement); no `loop`/`exitif` macros exist. Real residual
   was misleading indentation: `-Wmisleading-indentation` added to common
   flags in migalley/CMakeLists.txt and ALL flagged sites in both trees
   audited+fixed (dedent/merge/brace — semantics preserved; verified: the
   unguarded statements were intended unconditional in every audited case,
   e.g. `pForceX=NULL` after conditional `Release()`, `Save()` after
   conditional tag write). Warning count now 0; flag stays on as gate]**

4. **Unchecked `ConvertPtrUID` / `Find*ForItem` / `ConvertPtrUIDSkip`
   results.** Pointer derefs straight off lookups that legitimately fail;
   same for `waypoint->`, `playernode->`, `EngineList->`, `af->`,
   `transit->Target`, `transit->launcher`.
   **[DONE at flag sites — pervasive pattern, grep `->applyflight`,
   `->World`, `->supplyline` before assuming safe]**

5. **Uninitialized locals fall through partial `switch` coverage.**
   `tlx/tly`, `fDamage`, `retval`, `range`, `nextViewRec`, `wp0/wp1`,
   `ThrustVol`, `theValue` — pattern is `declare → switch(partial) → use`.
   **[DONE at flag sites — any `switch` without `default:` feeding a use
   is suspect]**

## Structural / architectural

6. **Intrusive self-detaching linked lists.** `while (head) delete head`
   where `~T` calls `List.Detach(this)` — valid but invisible to tools,
   review, and anyone refactoring. Ownership = "whoever holds the head".
   **[OPEN — ELMT.H, CURVES.CPP, MODEL.CPP, EngineList/MainPlaneList]**

7. **Global singletons as the object graph.** `_Miles`, `_DPlay`,
   `OverLay`, `Trans_Obj`, `View_Point`, `Persons2/3/4`, `SupplyTree` —
   every subsystem reaches sideways through globals; no DI, no testability
   boundary.
   **[OPEN — no cheap fix; document which globals each subsystem touches]**

8. **God objects.** `mobileitem`/`AirStruc`/`Persons3`/`SAGairgrp` fuse
   AI + physics + comms + animation + world-position. `TANK.CPP`'s
   `nextbftype` switch and `TRANSITE.CPP`'s ~90 copy-pasted `*SpriteLaunch`
   functions are the same shape.
   **[OPEN — refactor candidates, but only with tests covering behavior]**

9. **Duplicated, diverged trees.** `bob/` and `migalley/` share ~415
   same-named files; only ~2 are code-identical after normalizing
   EOL/comments/case. ~77 differ by ≤20 normalized tokens (mostly include
   case, whitespace, per-tree fix wording); the rest differ semantically
   (BoB is a different game on the same engine DNA). Every fix needs
   applying twice and they WILL drift — the `pFound` NULL bug existed in
   only one tree until this session.
   **[OPEN — see "Dedup" section below]**

10. **No RAII / manual memory.** `new[]`/`delete` mismatches
    (`REPLAY.CPP`), leaks on early return (`COMMS.CPP`, `WINMOVE.CPP`,
    `mssw_compat.h`), `delete` on small-int handle values (`CloseHandle`
    0x10000 path). Ownership is by convention, not type.
    **[DONE at flag sites — systemic]**

11. **`pack(1)` ABI fragility.** `dosdefs.h` pushes `pshpack1.h` unguarded
    at file end so every TU re-asserts pack(1) — save formats + wire
    formats depend on it, and the PCH freezes it. The BFSAVE bug came from
    a TU that lost the pack.
    **[OPEN — considered load-bearing; do NOT "fix" by relaxing]**

## Porting-scar organization

12. **`..\h\X.h` backslash includes + case-mismatched filenames.** Only
    worked via MSVC path normalization; needed ~5.3k alias symlinks just
    to get clang/Linux to see them (`ai`+`AI`, `comms`+`COMMS`,
    `files`+`FILES`, `general`+`GENERAL`, `input`+`Input` dir pairs in
    `bob/`).
    **[DONE at dir level — verified 2026-10: each `bob/` case-pair is one
    canonical lowercase dir + a dir-level symlink (`AI->ai`, `COMMS->comms`,
    `FILES->files`, `GENERAL->general`, `Input->input`); no duplicate content
    exists. Remaining residue: per-file case symlinks inside dirs (e.g.
    `Comms.cpp`->`comms.cpp`) — harmless, they exist for `..\COMMS\X.cpp`
    Windows-style include paths. True backslash-include cleanup means
    normalizing `#include "..\\X\\y.h"` spellings — separate sweep]**

13. **`#if 0` / `DEADCODE`/`TEMP`/`RERUN` debris.** Thousands of lines
    kept "just in case" — confuses analyzer and humans (the stubbed
    `Aggregator::ReceiveMessage` returning `DP_OK` on garbage hid in a
    `#if 0` wrapper).
    **[PARTIAL — all 4,682 `//Old_Code` whole-line comment blocks removed
    across 62 files (both trees). `#if 0`/`#ifdef DEADCODE` regions and
    `//TempCode` lines remain — TempCode marks live "temporary" code in
    places, so it needs per-site judgement, not a sweep]**

14. **`LBMCPP.H` textual-template inclusion.** A `.H` file `#include`d
    *inside function bodies* with macro-defined inputs (`LBM_END_PTR`,
    `DECTYPE`) — four separate instances per TU, each re-deriving bounds.
    **[DONE — all live instantiations used the identical UByte
    configuration, so `migalley/H/LBMCPP.H` is now a real
    `static inline UByte* LBMUnpackNRow(c,cend,dst,bpl,w,x,y,minx,maxx)`
    (same bounds/saturation semantics; `cend=NULL` selects the historic
    unbounded mode). Callers: `IMAGEMAP.CPP` ×2 (ALFA+BODY),
    `bob/3d/imagemap.cpp` ×2, `LBM.CPP::Graphic::UnpackRow` wrapper.
    `bob/h/lbmcpp.h` is now a symlink to the canonical header]**

15. **`ON_COMMAND` message maps need `-fms-extensions`.** MSVC-specific
    syntax baked into the ABI of every dialog.
    **[OPEN — fixed cost; only matters if the compiler changes]**

16. **Fixed-size magic arrays.** `AIRSTRUCSIZE`, `pack[packnum][wave]
    [group]`, `teamlist[MAXTEAMS]`, `route[256]` — bounds by convention;
    `pack[packnum][-1]` was a real underflow.
    **[AUDITED — no `[-1]` underflows remain in either tree (verified grep
    2026-10). Bounds are still convention-based: any `x[lookup]` write where
    `lookup` comes from a Find*/Convert* call can go negative/out-of-range —
    grep `\[[a-z_]+\]` writes near lookup calls before trusting them]**

17. **Bit-packed `UniqueID`/enum arithmetic.** `(nextbftype&63)-1` as a
    table index, `FILENUMMASK` extracting file indices — pack/unpack by
    masking, UB if layout shifts.
    **[AUDITED — `FILENUMMASK=0x00FF` is applied as `&FILENUMMASK` before the
    `<<4` shift in all 10 sites (bob+MiG identical) → 12-bit index, safe in
    the enum's int storage. The mask is a value-level convention, not a
    struct-layout dependency, so no UB from pack/alignment; risk is only a
    future widening of the FileNum space — re-audit if masks change]**

18. **Stack addresses as tokens.** `RowTasks` stores `&count` as an
    ordering key — works, ASLR-hostile, freaks every analyzer.
    **[DONE — opaque barrier added; token still stack-derived by design]**

19. **`EmitSysErr`/`SayAndQuit` abort contract lost in porting.**
    Watcom's `#pragma aux ... aborts` didn't survive; analyzer correctly
    flagged continuing-past-abort until `noreturn` was restored.
    **[DONE]**

20. **`strcpy` into fixed `PLAYERNAMELEN` buffers.** Network/name paths
    trust the wire length.
    **[DONE at wire/name sites — bounded copies applied: sessiondesc
    heap alloc was PLAYERNAMELEN(21) vs SESSIONNAMELEN src (heap overflow,
    now SESSIONNAMELEN + strncpy); `H2H_Player[].name`, `INITINFO.Name`,
    `PASSWORDPACK.name`, `ChatBuffer` name/chat, `Messages`/`p.messages`
    radio packets, `temptext` visitor strcat, `ServiceName`, LOCKER.CPP
    unguarded m_hWnd paths, filename paths in REPLAY.CPP/replay.cpp
    (`buffer2[150]`, `buff` min-caller 149), Ready.cpp `score[10]`
    off-by-one `"UN "+num`, `MMC.PlayerName`/`CommsPlayerName`/
    `CommsPassword` same-size copies. Literal-source and
    `new char[strlen+1]` sites left as-is]**

21. **Pointer fields inside baked-layout animdata structs.** `AnimPtr`
    blocks are indexed by shape instructions (`DONTPOINTS::flag` etc.)
    via raw byte offsets authored against the shipped 32-bit layout —
    so animdata classes can NEVER grow/widen a member without breaking
    every baked offset (a +4B `WeapAnimData` change moved
    `AircraftAnimData::acrpm` by +20 → props frozen). Pointer-carrying
    fields (`currLink`, `lastmissile`, `thelist`, `originitm`) must stay
    `ULong`; the full pointer round-trips through `AnimLink::Store/Load`
    (engine/compat/animlink.h) keyed by field address — the field keeps
    the low32 for truthiness/save fidelity. Fixes the 64-bit
    `TransObj::LaunchWeapon` SIGSEGV (`0x5555_b102f3a0` → truncated
    `0xb102f3a0` deref) and the `corrupted size vs. prev_size` variant
    (stray `isOwned=1` write into a heap chunk header when the truncated
    addr was mapped). **Do NOT widen any field inside `*AnimData`
    classes — append new fields at the tail only, and even then flag
    offsets must be rechecked.**

## Dedup plan (task: common parts into rowan-engine)

Measured 2026-10: 399 same-name pairs; after EOL/comment/case
normalization → **2 identical**, **77 ≤20-token diffs**, **320 larger**.
The ≤20-token diffs are mostly include-case (`"DOSDEFS.H"` vs
`"dosdefs.h"`), blank lines, and divergent wording of the same fix —
mergeable with review. The 320 are genuine game divergence (BoB has
different aircraft/AI/campaign code).

Correct boundary: **platform/compat infra is already in `engine/`**
(graphics compat, fileio, shell, tests). What CAN still unify: pure
mechanical headers (`intable.h` family) and genuinely identical engine
utilities. What must NOT unify: anything game-domain (AI, campaign,
aircraft data, mission logic) or where the trees made independent port
fixes.

Strategy: merge-by-verification — for each tiny-diff pair, diff the
normalized tokens; if diff is cosmetic (case/comment/format), unify into
`engine/shared/` or symlink one tree's file to the other. For divergent
files, keep per-game copies and document the split here.

### Executed dedup (2026-10)

TU-level audit against `build64/compile_commands.json`: BoB compiles 216
`bob/` + 13 `migalley/` TUs; MiG compiles 261 `migalley/` TUs. **Zero
token-identical compiled TUs** — the game code genuinely diverged (BoB
has 119 TUs with no MiG counterpart: RAF/Luftwaffe campaign frontend).
The 5 already-shared TUs (`engine/shell/*`, `dplay_net`, `dinput_stub`,
`MigCString`) stay shared.

Merged **28 headers**: `bob/h/{intable.h, 16 asm-era .G/.I/.INC/.ASH
tables, bfenum.h, ceditor.h, ctrl.h, diskio.h, h2h.h, link.h,
msn_ctrl.h, rowtasks.h, sqview.h, sviewer.h, vertex.h}` → symlinks to
`migalley/H` (token-identical or pragma-comment-only). Merge rule used:
symlink keeps BoB's filename casing so overlay resolution is unchanged;
canonical content lives in `migalley/H` (MiG's direct include dir).

**Deliberately NOT merged** (real per-game divergence):
- `IDDX_*`/`IDD_*` enum diffs — per-game dialog resource IDs
- `palette.h` etc. — `#include "dosdefs.h"` resolves to the *local*
  DOSDEFS, which is the game-identity header (BOB vs MIG type universe)
- `hardprim.h`/`GRAFPRIM.H` — token-identical forwarder text but
  resolves to different `display.h` per tree; a symlink would retarget
- `makebf.h` (T_applywave enum), `weather.h` (CRStatic members),
  `commsac.h` (signature), `migdoc.h` (protected/public), `contlist.h`
  (return type), `tga.h`, `filing.h`, `vwqlist.h`, `credits.h` —
  real API/data differences
- 320 larger-divergence files — BoB vs MiG game logic, separate by design

## Library migrations

- **DirectPlay transport → ENet (done).** `migalley/MFC/dplay_net.cpp`
  session channel is now vendored ENet 1.3.18 (`engine/enet/`, MIT,
  `rowan_enet` static lib). One `ENetHost` per side; a single service
  thread owns all `enet_*` calls — it drains a mutex-guarded outbound
  queue into `enet_peer_send` (channel 0, reliable ordered = the TCP
  guarantee) and dispatches CONNECT/RECEIVE/DISCONNECT into the same
  `WHdr` message switch the TCP loop ran. Application wire protocol
  unchanged; LAN discovery stays raw UDP broadcast (ENet has no
  broadcast). Gotcha preserved for posterity: `ENetAddress.host` is
  **network** byte order, `.port` is host order.
- **Audio → miniaudio + FluidSynth (done).** `migalley/H/mssw_compat.h`
  (the whole Miles-compat backend) now drives one shared `ma_engine`
  singleton (`engine/audio/rowan_audio.cpp` — state can't live in the
  header; it's included by 4 TUs). Samples decode via
  `ma_decode_memory` (WAV) or u8→f32 (raw radio speech); volume/pan/
  playback-rate→pitch on `ma_sound`; counted loops + EOS via the sound
  end-callback. `engine/miniaudio/miniaudio_impl.c` vendors miniaudio
  0.11.23. MIDI: `engine/audio/xmi2smf.c` converts Miles XMIDI
  (FORM/XDIR/CAT/EVNT, fixed 120 Hz, TIMB patch remap, **varlen note-on
  durations** materialized as real 8n offs via a min-heap) to SMF, then
  FluidSynth renders to f32 PCM played as a `ma_sound` — XMIDI finally
  *plays* (SDL2_mixer had no XMI decoder; music was silently dead).
  FluidSynth is optional (`ROWAN_HAVE_FLUIDSYNTH`) — absent → music
  degrades to silence, same as before. `ROWAN_SOUNDFONT` overrides the
  sf2 search path.
- **SMK video → vendored libsmacker (done).** `migalley/Graphics/
  WINSMACK.CPP` no longer pipes ffprobe/ffmpeg; libsmacker 1.2.0
  (`engine/smacker/`, single-file decoder) gives palette-indexed frames
  expanded to RGB24 at blit, plus per-frame decoded audio chunks queued
  to SDL audio at the track's native spec (A/V stays in step because
  audio advances per presented frame). No external tools at runtime.
- Pending: none of the library migrations.

## Verification state

- 2026-10 crash-hardening pass: ~50 real null/uninit/div0/leak sites
  fixed across both trees; per-file re-lint clean; `ctest` 6/6.
- ENet migration: `bob_dplay_net` (loopback host+guest: discovery, join,
  CPREQ/CPACK ids, direct/broadcast/group routing, destroy sysmsg,
  graceful close) + full `ctest` 6/6 on the ENet transport.
- Audio: XMIDI→SMF→FluidSynth→PCM verified on retail `.XMI` data
  (COMBA2 ~9.8min, PARACHUT 49s, OBJCOMPL 43s of rendered audio);
  miniaudio path builds/links clean; both games + `ctest` 6/6.
- Video: libsmacker decodes retail SMKs (intro.smk 640x332@14fps,
  16-bit audio chunks per frame); intro plays through to menu under
  Xvfb; `ctest` 6/6.
- clang-tidy: `.clang-tidy` (bug-finding checks only, no style/
  modernize) gates `error:` diagnostics in changed files in CI;
  `lint.sh` runs it in default mode + `-t` standalone.
- Memcheck gameplay soak (Quick Shots→Take-off→Fly→cockpit→debrief→
  quit, ~65min under `--track-origins=yes`): baseline 348 uninit-read
  contexts / 0 invalid accesses. Root causes fixed: `ViewPoint` ctor now
  `memset(this,0,...)` (~60 members incl. `jhead`/`straightback`/
  `viewnum` — no vptr in chain); `Lib3D` ctor zeroes `d3dl7Lights`,
  `sunTX/Y/Z`, `fog*`, `backProject*`, matrix-stack positions,
  `gLightingDesc`; `Material` ctors init `materialFlags`+pads;
  `SetPilotedAcAnim` zeroes `PolyPitAnimData` before base-copy;
  `LandScape::Init` value-inits `gridCone`/`horizCone`/`tileCoords`/
  `pDataBlocks`/`tileCacheGrid`; `SmkCloudDesc` ctor zeroes `pos`;
  `CloudAnimP` byte memset (shared `repair`/`IsBullied`/`IsInvisible`
  bitfields); `pball->blendType` set (shares bitfield byte);
  `Gear::Touched=FALSE`; `View3d` frametime fields zeroed;
  `BoBFrag::ResetPlayerEntry` clears unconditionally.
  Second wave (flight-model phase, soak3: 844 contexts/0 invalid —
  baseline "348" was truncated by Memcheck's 10M-event cap): `Model()`
  now `memset(this,0,sizeof(Model))` (hundreds of flight-model fields;
  `LINKEDLIST`/`ACMMODEL`/`INSTRUMENTS`/`Gust` members all zero-
  compatible, no vptr — fixes the `NewModel` cluster: Curve::GetValue,
  ProcessGears, CalcNettForceMoment, MainPlane::Process);
  `AirStruc()` now `memset(this,0,sizeof(AirStruc))` (`ai`/`weap`/`fly`
  PODs; `make_airgrp` `new AirStruc` cluster); `MovePilotEye` zeroes
  stack `EyePos` (`SetCockpitBuffet` copies `hdg/pitch/roll` which were
  never written → rendered ANGLES in InfiniteStrip/RenderLandscape).
  `alloc.c` arena blocks now `calloc` (every SPolygon/SVertex/SLine/
  SPoint + clip work buffers were carved uninit); soak7 interim
  (menu→briefing→ready→worldload, killed by 90min timeout before 3D):
  844 → 3 contexts, all the same `LoaderScreen`→`RenderTPolyList`→
  `DrawTri` channel-LUT indexed read (origin `new Lib3D` — a Lib3D
  member LUT, cosmetic-path only). Native build verifies the full path:
  menu→Quick Shots→ready room→cockpit 3D in ~70s.
  Definitive soak9 (fully-fixed binary, full loop: menu→Quick Shots→
  briefing→ready→worldload→3D flight→Combat Report→menu→quit):
  **4 contexts total** (844→4); the 3 DrawTri loader-LUT reads +
  `CCredits::OnTimer` conditional. 0 invalid accesses. Residual leaks
  (~58MB possibly-lost) are the legacy alloc/graph teardown idioms
  documented below — cosmetic, not crash-class.
- Survivors are the classified idioms above (self-detaching lists,
  placement-delete forwarders, `*smartptr` nullref conversions, MFC
  ownership transfers, `AfxBeginThread` auto-delete, `DEBUGREFCOUNT`
  UAF-artifact).

## 2026-10-19 — Max-detail hidden-screen test: IDirectSound3DBuffer double-free

Found via live max-settings run (settings.cfg patched to
detail_3d=0x03F6, filtering=3, textureQuality=4, contourDetail=4,
cockpit3Ddetail=0x07FFFC, HW_SFXQUALITY|HW_3DSOUND|weather/voices/clouds,
music on). Enabling **HW_3DSOUND** made `Sample::Init3d()` call
`QueryInterface(IID_IDirectSound3DBuffer)` on every sample buffer —
previously dormant path.

`IDirectSound3DBuffer::Release()` at refCnt 0 did `delete this` then
`owner->Release()`; if that dropped the owner to 0,
`~IDirectSoundBuffer` ran `delete buf3d` on the already-freed facade →
`free(): double free detected in tcache 2`, abort inside
`Sample::DSoundRelease` ← `Sound::PlayOnce(enrafinh.wav)` ←
`_instruments::SetInstruments` ← flight-model thread. Crashed during
worldload→3D entry every time with 3D sound on.

Fix (`migalley/H/dsound_stub.h`):
- `~IDirectSound3DBuffer` now clears `owner->buf3d` so the owner's dtor
  can't re-delete the facade.
- `~IDirectSoundBuffer` now orphans the facade (`buf3d->owner=0`) and
  only deletes it when unreferenced — covers the owner-dies-first
  direction without UAF/leak-amplification.

Verified: hidden-X run reached live 3D cockpit at max detail — engine
start, full throttle, rolling take-off to 30+ Kts, spinning prop,
gauges, canopy glass reflections (SKYIMAGES path), HUD — no abort.
