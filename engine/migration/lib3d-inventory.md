# lib3d call inventory and backend-route recommendation (T006)

Date: 2026-10-05 · Scope: `bob-flight-sim/lib3d/` @ 5e70a2d · Method: COM-call
census in `lib3d.cpp` (~18K lines) + review of migalley's 3D port structure.

## What lib3d uses (D3D7/DD7 immediate mode only — no retained mode)

Render/state: `SetTextureStageState`×193, `SetRenderState`×130,
`Transform`×41, `SetMaterial`×9, `LightEnable`×6, `SetTransform`×8,
`SetViewport`×7, `GetViewport`×11, `Clear`×15, `BeginScene`/`EndScene`,
`DrawPrimitiveVB`×9, `DrawIndexedPrimitiveVB`×4.

Resources: `Lock`/`Unlock`×61, `CreateSurface`×21, `GetSurfaceDesc`×21,
`GetAttachedSurface`×13, `AddAttachedSurface`×5, `Blt`×12, `SetTexture`×12,
`CreateVertexBuffer`×4, `CopyToVertexBuffer`/`CopyToIndexdedVB`×10,
`CreateStateBlock`×12, `ApplyStateBlock`×16, `EvictManagedTextures`×4,
`SetPalette`×3, `SetRenderTarget`×4.

Setup/misc: `DirectDrawCreateEx`, `QueryInterface(IID_IDirect3D7)`,
`SetCooperativeLevel`×5, `GetAvailableVidMem`×6, `GetCaps`×3,
`IDirectDrawGammaControl`, `ChangeDisplaySettings`×3, `EnumZBufferCallback`,
TnL-vs-HAL device selection (`IID_IDirect3DTnLHalDevice`).

## Routes scored

- **A. MiG API-shim reuse — REJECTED as-is.** migalley's port happened at the
  *win3d.cpp* level (its internal 3D interface), not at the D3D COM level.
  `H/d3d_stub.h` exposes only ~13 methods (LightElements/TransformVertices-era
  surface); `direct_3d.cpp` contains no D3D7-device entry points
  (SetRenderState/SetTextureStageState/DrawPrimitiveVB all absent). The shim
  APIs are not compatible with lib3d's call pattern.
- **B. Surviving software path — NOT FOUND.** `flagsw.cpp` is license
  boilerplate; the only "software" remnant in lib3d.cpp is a
  `//DeadCode JON 24Jul00 software fog only` block. D3D is the only renderer.
- **C. Fresh port of lib3d internals behind `lib3d.h`, reusing migalley's
  Vulkan backend machinery — RECOMMENDED.** `Hardware/direct_3d.cpp` (13.7K
  lines) is a working D3D-concept→Vulkan implementation: surfaces, locks,
  blts, textures, state mapping, frame management — the same concepts lib3d
  needs at D3D7 granularity. The port mirrors what MiG did one level lower:
  keep the `CLib3D`/`Lib3DCreate` API, replace each D3D7 call site with the
  equivalent backend call, and build the needed state/buffer vocabulary on
  top of the proven backend rather than re-deriving it.

## Consequences

- M3's renderer work is a real port, not an adapter: the D3D7 state model
  (193 stage-state + 130 render-state sites, state blocks, vertex buffers)
  must be reproduced — but the seam is clean, all calls are inside one file,
  and the backend concepts already exist in `direct_3d.cpp`.
- Order inside M3: map lib3d.h's used API subset → minimal backend surface →
  port `lib3d.cpp` site by site. Estimate sits inside M3's 20–35 days shared
  with the frontend port; the risk moved from "unknown" to "bounded."
