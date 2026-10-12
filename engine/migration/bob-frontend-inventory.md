# BoB frontend inventory (T116) — wave-2/3 port surface

Surveyed `battle-of-britain/source-ported/bob-flight-sim` against the migalley
compat layer (`migalley/MFC/`, `RowanUI/`, `WIN32_COMPAT`, SDL2 backend).

## Size surface

| Area | Files | LOC | Notes |
|---|---|---|---|
| mfc/ frontend | 177 cpp | ~79.5k | app+dialogs+doc/view+campaign UI; entry `mfc/mig.cpp` `CMIGApp::InitInstance` |
| r* OCX projects | 18 dirs | ~20k | unported ActiveX sources (.dsp/.ODL/.def, Ctrl+PropPage classes) |
| comms/ | 5 cpp | ~22.7k | 640 DPlay refs + IDirectPlay/CoCreate — REQ-PORT-02 compile-only |
| meditor/ | 34 cpp | ~19.9k | separate mission-editor exe — NOT part of game journey, defer |
| placenam/itemgrid | 4 cpp | ~0.6k | separate tool exes — defer |
| lib3d/ | dll-shaped | — | lib3d.cpp/.spec exports; winelib-ported already; backend route TBD (T120) |

## Win32/MFC API surface (mfc/ + r*)

Top hits are the standard MFC GDI/window family — same API class mig's stub
already implements: ShowWindow 279, SelectObject 254, SendMessage 221,
GetParent 196, LineTo 119, GetClientRect 100, GetDlgItem 82, MoveWindow 77,
GetDC 61, BitBlt 25, CreateCompatibleDC/Bitmap 10/10, timers, capture,
MessageBox 12, registry (RegOpenKeyEx in InitInstance — stub like mig).

Mig compat already provides: CWnd CDialog CDC CBitmap CBrush CPen CFont
CPalette CRect CPoint CSize CString CWinApp CMainFrame RDialog + Win32
message pump, SDL2 backend, io_compat, dinput_stub, smacker, SDL_mixer audio.

## r* widget coverage vs mig's ported set (RowanAFX)

Mig ported: RBUTTON RCOMBO(+RCOMBOX) REDIT REDTBT RJOYCFG RLISTBOX RRADIO
RSCRLBAR RSPINBUT RSPLTBAR RSTATIC RTABS RTITLE RTOOLBAR + RDialog/RDEmpty.

| BoB widget | mig ported equivalent | verdict |
|---|---|---|
| rbutton, rcombo, redit, redtbt, rjoycfg, rlistbox, rradio, rscrlbar, rspinbut, rspltbar, rstatic, rtabs, rtitle | yes (same family) | reuse port; verify API/behavior parity per widget |
| rtickbox (714) | NO RTICKBOX in mig | port |
| rtree (745) | NO RTREE | port |
| ranibut (166) | NO RANIBUT | port |
| rtoolbar (mfc/) | mig has RTOOLBAR | compare/port |

## Dialog / resource system

`english/bob.rc` (+5 language variants) holds **217** standard .rc DIALOG
templates — same class as mig's MIG_RC extraction (port path: rc→json dump
into the shared RCParser/MFC_stub loader, T119). OCX proppage .rc files under
each r* dir are widget-side resources.

## Frontend internals

- `mfc/rdialog.cpp` (2972) — BoB's unported Win32 RDialog, same lineage as
  mig's ported `RDialog.cpp` (2677, 22 RERUN markers). Port the same way;
  dragstate/dialog-sheet machinery must match BoB semantics.
- `mfc/mainfrm.cpp` (1298) doc/view frame; `migview.cpp`/`migdoc.cpp` —
  mig replaced doc/view with the RDialog-sheet host; same route for BoB.
- `mfc/mig.cpp` InitInstance: registry reads (stub), TESTS path (dead),
  RCommandLineInfo flags (BORDER/USE16/NOMEM/WINDOWED/HOST/GUEST/DIAGS).
- `comms/` — DirectPlay API surface; compile per REQ-PORT-02 behind stubs,
  no functional acceptance this release.

## Wave-2 target set (first compileable slice)

1. shell gate + `main()`/`CMIGApp` skeleton onto WIN32_COMPAT (T117)
2. RDialog + sheet host port (BoB rdialog.cpp semantics)
3. widget reuse from RowanAFX + 3 missing ports (tickbox/tree/anibut)
4. bob.rc → dialog-template extraction (shared RCParser path)
5. comms compile-only behind DPlay stubs

Wave-3 (T120): lib3d backend route + journey/save.
