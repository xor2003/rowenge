// MSVC <ddraw.h> shim — BoB's lib3d.h speaks DirectDraw 7 (DDSURFACEDESC2),
// so the DX7 type universe lives in d3dtypes.h/d3d.h (NOT migalley's
// ddraw_stub.h, whose DX1 _DDSURFACEDESC names conflict with lib3d).
#ifndef ROWAN_COMPAT_DDRAW_H
#define ROWAN_COMPAT_DDRAW_H
#include "d3dtypes.h"
#include "d3d.h"
#define ROWAN_DDRAW_STUB 1
#endif
