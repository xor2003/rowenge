// <d3d.h> shim — DX7's umbrella header pulled types + interfaces.
// Native build exposes only the type layer (d3dtypes.h); the COM device
// surface arrives with the M3 backend port.
#ifndef ROWAN_COMPAT_D3D_H
#define ROWAN_COMPAT_D3D_H
#include "d3dtypes.h"

// Missing-name aliases used by lib3d.h
typedef D3DDEVICEDESC7* LPD3DDEVICEDESC7;
typedef DDSURFACEDESC2* LPDDSURFACEDESC2;

// DX7 COM interface shells — pointer carriers for game headers; the real
// vtables arrive with the M3 backend port.
struct IDirectDraw7;
struct IDirectDrawSurface7;
struct IDirect3D7;
struct IDirect3DDevice7;
struct IDirect3DVertexBuffer7;
typedef IDirectDraw7*           LPDIRECTDRAW7;
typedef IDirectDrawSurface7*    LPDIRECTDRAWSURFACE7;
typedef IDirect3D7*             LPDIRECT3D7;
typedef IDirect3DDevice7*       LPDIRECT3DDEVICE7;
typedef IDirect3DVertexBuffer7* LPDIRECT3DVERTEXBUFFER7;
#endif
