// D3D7 type-level shim for BoB native build — types/constants only, no COM
// interfaces. Grows into the real backend-facing header in M3 (lib3d port).
// Layouts follow the DX7 SDK. Base types mirror migalley/H/d3d_stub.h where
// the names coincide (D3DVALUE/VECTOR/MATRIX, D3DCOLOR, DDPIXELFORMAT...).
#ifndef ROWAN_COMPAT_D3DTYPES_H
#define ROWAN_COMPAT_D3DTYPES_H
#include <stdint.h>
#include "WIN32_COMPAT.H" // DWORD, WORD, BYTE, BOOL, HRESULT, GUID

typedef float           D3DVALUE;
typedef D3DVALUE*       LPD3DVALUE;
typedef DWORD           D3DCOLOR;
typedef D3DCOLOR*       LPD3DCOLOR;

// Faithful DX7 layout: x and dvX alias via union (BoB writes dv*, MiG-era
// code writes x/y/z — both must compile).
typedef struct _D3DVECTOR {
    union { D3DVALUE x;  D3DVALUE dvX; };
    union { D3DVALUE y;  D3DVALUE dvY; };
    union { D3DVALUE z;  D3DVALUE dvZ; };
} D3DVECTOR;
typedef D3DVECTOR* LPD3DVECTOR;

typedef struct _D3DMATRIX {
    D3DVALUE _11,_12,_13,_14, _21,_22,_23,_24, _31,_32,_33,_34, _41,_42,_43,_44;
} D3DMATRIX;
typedef D3DMATRIX* LPD3DMATRIX;

typedef struct _D3DCOLORVALUE { D3DVALUE r,g,b,a; } D3DCOLORVALUE;

// --- Vertices ---------------------------------------------------------------
typedef struct _D3DVERTEX {
    D3DVALUE x,y,z, nx,ny,nz, tu,tv;
} D3DVERTEX;
typedef D3DVERTEX* LPD3DVERTEX;

typedef struct _D3DTLVERTEX {           // transformed+lit, 1 uv
    D3DVALUE sx,sy,sz, rhw;
    D3DCOLOR color, specular;
    D3DVALUE tu,tv;
} D3DTLVERTEX;
typedef D3DTLVERTEX* LPD3DTLVERTEX;

typedef struct _D3DTLVERTEX2 {          // 2 uv sets (BoB extension of D3DLVERTEX2)
    D3DVALUE sx,sy,sz, rhw;
    D3DCOLOR color, specular;
    D3DVALUE tu0,tv0, tu1,tv1;
} D3DTLVERTEX2;
typedef struct _D3DTLVERTEX3 {          // 3 uv sets
    D3DVALUE sx,sy,sz, rhw;
    D3DCOLOR color, specular;
    D3DVALUE tu0,tv0, tu1,tv1, tu2,tv2;
} D3DTLVERTEX3;

// --- FVF codes (DX7) ----------------------------------------------------------
#define D3DFVF_XYZ             0x002
#define D3DFVF_XYZRHW          0x004
#define D3DFVF_NORMAL          0x010
#define D3DFVF_DIFFUSE         0x040
#define D3DFVF_SPECULAR        0x080
#define D3DFVF_TEX1            0x100
#define D3DFVF_TEX2            0x200
#define D3DFVF_TEX3            0x300
#define D3DFVF_TEXCOORDSIZE2(i) (0x10000 << ((i)*2))
#define D3DFVF_VERTEX          (D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_TEX1)
#define D3DFVF_D3DTLVERTEX     (D3DFVF_XYZRHW|D3DFVF_DIFFUSE|D3DFVF_SPECULAR|D3DFVF_TEX1)

// --- Device/light/material (DX7) ----------------------------------------------
typedef enum _D3DZBUFFERTYPE {
    D3DZB_FALSE=0, D3DZB_TRUE=1, D3DZB_USEW=2, D3DZB_FORCE_DWORD=0x7fffffff
} D3DZBUFFERTYPE;

typedef struct _D3DDEVICEDESC7 {
    DWORD  dwDevCaps, dwDeviceLineCaps, dwDeviceRenderBitDepth, dwDeviceZBufferBitDepth;
    DWORD  dwMinTextureWidth, dwMinTextureHeight, dwMaxTextureWidth, dwMaxTextureHeight;
    DWORD  dwMinTextureRepeat, dwMaxTextureRepeat, dwMaxTextureAspectRatio;
    DWORD  dwMaxAnisotropy, dwGuardBandLeft, dwGuardBandTop, dwGuardBandRight, dwGuardBandBottom;
    D3DVALUE dvExtentsAdjust, dvMaxTextureRepeat;
    DWORD  dwStencilCaps, dwFVFCaps, dwTextureOpCaps;
    WORD   wMaxTextureBlendStages, wMaxSimultaneousTextures, dwMaxActiveLights;
    D3DVALUE dvMaxVertexW;
    GUID   deviceGUID;
    WORD   wMaxUserClipPlanes, wMaxVertexBlendMatrices;
    DWORD  dwVertexProcessingCaps, dwReserved1, dwReserved2, dwReserved3, dwReserved4;
} D3DDEVICEDESC7;
typedef D3DDEVICEDESC7* LPD3DDEVICEDESC;

typedef struct _D3DLIGHT7 {
    DWORD        dltType;           // D3DLIGHTTYPE
    D3DCOLORVALUE dcvDiffuse, dcvSpecular, dcvAmbient;
    D3DVECTOR    dvPosition, dvDirection;
    D3DVALUE     dvRange, dvFalloff;
    D3DVALUE     dvAttenuation0, dvAttenuation1, dvAttenuation2;
    D3DVALUE     dvTheta, dvPhi;
} D3DLIGHT7;

typedef struct _D3DMATERIAL7 {
    D3DCOLORVALUE dcvDiffuse, dcvAmbient, dcvSpecular, dcvEmissive;
    D3DVALUE      dvPower;
} D3DMATERIAL7;
typedef D3DMATERIAL7* LPD3DMATERIAL7;

// --- DirectDraw types needed by lib3d.h ----------------------------------------
typedef struct _DDCOLORKEY { DWORD dwColorSpaceLowValue, dwColorSpaceHighValue; } DDCOLORKEY;

typedef struct _DDPIXELFORMAT {
    DWORD dwSize, dwFlags, dwFourCC;
    union { DWORD dwRGBBitCount, dwYUVBitCount, dwZBufferBitDepth,
            dwAlphaBitDepth, dwLuminanceBitCount, dwBumpBitCount; };
    union { DWORD dwRBitMask, dwYBitMask, dwStencilBitDepth, dwLuminanceBitMask, dwBumpDuBitMask; };
    union { DWORD dwGBitMask, dwUBitMask, dwZBitMask, dwBumpDvBitMask; };
    union { DWORD dwBBitMask, dwVBitMask, dwStencilBitMask, dwBumpLuminanceBitMask; };
    union { DWORD dwRGBAlphaBitMask, dwYUVAlphaBitMask, dwLuminanceAlphaBitMask, dwRGBZBitMask; };
} DDPIXELFORMAT;
typedef DDPIXELFORMAT* LPDDPIXELFORMAT;

typedef struct _DDSCAPS2 {
    DWORD dwCaps, dwCaps2, dwCaps3, dwCaps4;
} DDSCAPS2;

typedef struct _DDSURFACEDESC2 {
    DWORD dwSize, dwFlags, dwHeight, dwWidth;
    union { LONG lPitch, dwLinearSize; };
    DWORD dwBackBufferCount;
    union { DWORD dwMipMapCount, dwZBufferBitDepth, dwRefreshRate; };
    DWORD dwAlphaBitDepth, dwReserved;
    void* lpSurface;
    DDCOLORKEY ddckCKDestOverlay, ddckCKDestBlt, ddckCKSrcOverlay, ddckCKSrcBlt;
    DDPIXELFORMAT ddpfPixelFormat;
    DDSCAPS2 ddsCaps;
    DWORD dwTextureStage;
} DDSURFACEDESC2;
typedef DDSURFACEDESC2* LPDDSURFACEDESC;

typedef BOOL (WINAPI *DDEnumCallbackEx)(GUID*, char*, char*, void*, void*);
#endif // ROWAN_COMPAT_D3DTYPES_H
