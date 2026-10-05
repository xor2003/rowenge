# BoB frontend dependency-surface survey (T005)

Date: 2026-10-05 · Scope: `mfc/` frontend + `r*` custom controls in
`bob-flight-sim` @ 5e70a2d · Method: grep census of Win32 calls, MFC class
derivation, dispatch maps, and dialog-resource bindings.

## Shape of the problem

The frontend is NOT the migalley-shaped problem. BoB's dialogs use **custom
ActiveX (OCX) controls** — this is why the Windows build requires
`regsvr32` registration and real `mfc120.dll`:

- ~18 `r*` control projects (rbutton, rcombo, redit, redtbt, rjoycfg,
  rlistbox, rradio, ranibut, rscrlbar, rspinbut, rspltbar, rstatic, rtabs,
  rtickbox, rtitle, rtree, itemgrid): **75 .cpp, ~21.5K lines** of actual
  control logic (draw, hit-test, message handling).
- Each project is `CR*Ctrl : COleControl` + `CR*PropPage : COlePropertyPage`
  + `CR*App : COleControlModule` with `.ODL`/`.DEF`/`.LIC` registration glue.
- OLE surface: **127 `DISP_PROPERTY_EX`, 57 `DISP_FUNCTION`, 15 dispatch
  maps**, plus stock props (FORECOLOR/CAPTION/BACKCOLOR/ENABLED/FONT).
- `mfc/` frontend: **177 .cpp, ~79.5K lines**, conventional MFC dialogs:
  **~1,150 `DDX_Control` bindings**, message maps (176 BEGIN/END), the usual
  `ON_WM_*` set (TIMER 28, DESTROY 27, LBUTTON 36, PAINT 10, CREATE 10, ...).
- `h/r*.h` are **machine-generated IDispatch wrappers** (`CRButton : CWnd`,
  `CreateControl(CLSID)`): dialog code talks to these proxies, never to
  `COleControl` directly. Wrapper calls map 1:1 onto the dispatch entries.
- Dialog `.rc` resources embed OCX placeholders:
  `CONTROL "",IDC_X,"{78918646-...}"` — a CLSID lookup binds resource → class.

## Win32 API surface (r* + mfc, direct calls)

Bounded and conventional — largely overlaps what migalley's stub already
implements: ShowWindow(267), SelectObject(190), SendMessage(172),
GetParent(151), GetWindowRect(120), GetClientRect(99), GetDlgItem(78),
MoveWindow(73), GetDC/ReleaseDC(78), TextOut(33), SetTimer/KillTimer(70),
capture APIs(50), EnableWindow(23), SetWindowText/SetWindowPos(42),
BitBlt(21), SetCursor(19), MessageBox(16), InvalidateRect(13),
DefWindowProc(11), PostMessage(9), CreateCompatibleDC(9),
Begin/EndPaint(12), SetFocus(4), coordinate transforms(31), DrawText(4),
PeekMessage(2), GlobalAlloc family(3), clipboard/drag-drop(≤2).

MFC include surface is small: afxwin/afxtempl plus a handful of
afxctl/afxodlgs/afxpriv includes (all in the OCX projects).

## Recommended route: source-level de-OLE (not OLE emulation)

Emulating OLE control containers in the stub is a rabbit hole
(site/container protocols, dispatch, property bags, type info). Since we own
the control sources, strip the OLE wrapping instead:

1. **Pilot on `rbutton`** (smallest viable): rebase `CRButtonCtrl` logic onto
   the stub's `CWnd` control base; rewrite `h/rbutton.h` as a real CWnd class
   (properties → methods/members); CLSID table → class-factory map.
2. Loader/`DDX_Control` equivalent binds `CONTROL "",IDC,"{CLSID}"` resources
   to the registered classes — one code path, works for every control.
3. Repeat per control (~15). Pattern is mechanical after the pilot; the
   127-property + 57-method dispatch surface becomes direct calls.
4. `mfc/` dialogs then run on the same dialog/message machinery as migalley's
   `MFC_stub` (extended where the survey shows gaps).

## Sizing

- Pilot (rbutton end-to-end in one dialog): ~3–5 days incl. stub gaps.
- Remaining 14–17 controls: ~1–2 days each after the pattern exists.
- `mfc/` dialogs + resource/DDX binding layer: the other half of the work.
- **Frontend total: est. ~20–30 days inside M3** — consistent with M3's upper
  bound; the unknown is reduced from "shape unknown" to "bounded but real."

## Risks / things to check in M3

- Property pages (`CR*PropPage`) are design-time only — likely droppable with
  evidence, not ported.
- Any control behavior reachable only via OLE persistence/property bags must
  be preserved explicitly (recorded per REQ-PORT-03).
- `COleDispatchDriver` (1 use) and clipboard/drag-drop (≤2 uses) — small
  special cases to enumerate during the pilot.
