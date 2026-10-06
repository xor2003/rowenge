// D3D7 type-level shim for BoB native build.
//
// The DirectDraw primitive types the SDK's d3dtypes.h expects from <ddraw.h>
// are declared inline below (BoB's tree has no ddraw.h; the winelib build
// used Wine's).  The DX7 enum/vertex universe then comes from BoB's own
// vendored Microsoft SDK header (h/vs2005/d3dtypes.h) reached through the
// bob_overlay link <d3d7_sdk.h>.  Everything the SDK header lacks but
// lib3d.h/lib3d.cpp needs is appended afterwards.
#ifndef ROWAN_COMPAT_D3DTYPES_H
#define ROWAN_COMPAT_D3DTYPES_H
#include <stdint.h>
#include "WIN32_COMPAT.H" // DWORD, WORD, BYTE, BOOL, HRESULT, GUID

// --- DirectDraw types (SDK <ddraw.h> fragment d3dtypes.h depends on) ---------
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
typedef DDSCAPS2* LPDDSCAPS2;

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
typedef DDSURFACEDESC2* LPDDSURFACEDESC2;
typedef DDSURFACEDESC2  DDSURFACEDESC;      // DX7 unify: SDK code aliases these
typedef DDSURFACEDESC2* LPDDSURFACEDESC;

// --- Real DX7 type universe (vendored SDK header via bob_overlay link) --------
// The vendored header does `#include "ddraw.h"`, which resolves to our
// emulated compat/ddraw.h — that header's interfaces need D3DDEVICEDESC7 &
// friends, defined only AFTER this include.  Pre-setting the interface
// guards makes the SDK's inner include a no-op (the DD types it actually
// uses are already defined above); the real ddraw.h/d3d.h are still parsed
// normally when the TU includes them.
#ifndef ROWAN_COMPAT_DDRAW_H
#define ROWAN_COMPAT_DDRAW_H
#define ROWAN_DDRAW_GUARD_UNDO
#endif
#ifndef ROWAN_COMPAT_D3D_H
#define ROWAN_COMPAT_D3D_H
#define ROWAN_D3D_GUARD_UNDO
#endif
#include <d3d7_sdk.h>
#ifdef ROWAN_DDRAW_GUARD_UNDO
#undef ROWAN_COMPAT_DDRAW_H
#undef ROWAN_DDRAW_GUARD_UNDO
#endif
#ifdef ROWAN_D3D_GUARD_UNDO
#undef ROWAN_COMPAT_D3D_H
#undef ROWAN_D3D_GUARD_UNDO
#endif

// --- Pieces the SDK d3dtypes.h lacks ------------------------------------------

// DX7-era device description (d3dtypes.h stops at the DX3/6 forms).
// Faithful DX7 layout: D3DPRIMCAPS line/tri caps replace the old
// D3DCOLORMODEL/shade/blend/texture/transform/clipping fields.
typedef struct _D3DPRIMCAPS {
    DWORD dwSize;
    DWORD dwMiscCaps;
    DWORD dwRasterCaps;
    DWORD dwZCmpCaps;
    DWORD dwSrcBlendCaps;
    DWORD dwDestBlendCaps;
    DWORD dwAlphaCmpCaps;
    DWORD dwShadeCaps;
    DWORD dwTextureCaps;
    DWORD dwTextureFilterCaps;
    DWORD dwTextureBlendCaps;
    DWORD dwTextureAddressCaps;
    DWORD dwStippleWidth;
    DWORD dwStippleHeight;
} D3DPRIMCAPS;
typedef D3DPRIMCAPS* LPD3DPRIMCAPS;

typedef struct _D3DDEVICEDESC7 {
    DWORD       dwDevCaps;
    D3DPRIMCAPS dpcLineCaps;
    D3DPRIMCAPS dpcTriCaps;
    DWORD       dwDeviceRenderBitDepth;
    DWORD       dwDeviceZBufferBitDepth;
    DWORD       dwMinTextureWidth, dwMinTextureHeight;
    DWORD       dwMaxTextureWidth, dwMaxTextureHeight;
    DWORD       dwMinTextureRepeat, dwMaxTextureRepeat;
    DWORD       dwMaxTextureAspectRatio;
    DWORD       dwMaxAnisotropy;
    D3DVALUE    dvGuardBandLeft, dvGuardBandTop, dvGuardBandRight, dvGuardBandBottom;
    D3DVALUE    dvExtentsAdjust;
    DWORD       dwStencilCaps;
    DWORD       dwFVFCaps;
    DWORD       dwTextureOpCaps;
    WORD        wMaxTextureBlendStages;
    WORD        wMaxSimultaneousTextures;
    DWORD       dwMaxActiveLights;
    D3DVALUE    dvMaxVertexW;
    GUID        deviceGUID;
    WORD        wMaxUserClipPlanes;
    WORD        wMaxVertexBlendMatrices;
    DWORD       dwVertexProcessingCaps;
    DWORD       dwReserved1, dwReserved2, dwReserved3, dwReserved4;
} D3DDEVICEDESC7;
typedef D3DDEVICEDESC7* LPD3DDEVICEDESC7;

// DX7 device enumeration callback (the SDK header only carries the older
// LPD3DENUMDEVICESCALLBACK/...PIXELFORMATS forms)
typedef HRESULT (CALLBACK* LPD3DENUMDEVICESCALLBACK7)(LPSTR lpDeviceDescription,
    LPSTR lpDeviceName, LPD3DDEVICEDESC7 lpD3DDeviceDesc7, LPVOID lpContext);

// BoB extensions of the TL vertex (multi-uv variants used by R3DVERTEX2/3)
typedef struct _D3DTLVERTEX2 {
    D3DVALUE sx,sy,sz, rhw;
    D3DCOLOR color, specular;
    D3DVALUE tu0,tv0, tu1,tv1;
} D3DTLVERTEX2;
typedef struct _D3DTLVERTEX3 {
    D3DVALUE sx,sy,sz, rhw;
    D3DCOLOR color, specular;
    D3DVALUE tu0,tv0, tu1,tv1, tu2,tv2;
} D3DTLVERTEX3;

// Display-mode enumeration callback (DD7, takes DDSURFACEDESC2 — the older
// LPDDENUMMODESCALLBACK uses the DX1 desc)
typedef HRESULT (WINAPI *LPDDENUMMODESCALLBACK2)(LPDDSURFACEDESC2, LPVOID);

// ============================================================================
// DirectDraw 7 constant/struct universe — the SDK <ddraw.h> layer.
// Kept in the type header so compat/d3d.h's device emulation can use the
// constants without a circular include on compat/ddraw.h's interfaces.
// ============================================================================

// --- misc Win32 bits lib3d touches ----------------------------------------------
#ifndef S_FALSE
#define S_FALSE ((HRESULT)1L)
#endif
#ifndef NOERROR
#define NOERROR 0
#endif
#ifndef APIENTRY
#define APIENTRY WINAPI
#endif
typedef POINT*   LPPOINT;
typedef BOOL*    LPBOOL;
#ifndef IID_IUnknown_DEFINED
#define IID_IUnknown_DEFINED
DEFINE_GUID(IID_IUnknown, 0x00000000,0x0000,0x0000,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46);
#endif

// --- HRESULT space --------------------------------------------------------------
#ifndef DD_OK
#define DD_OK                        0 /* S_OK */
#endif
#define DDERR_ALREADYINITIALIZED     ((HRESULT)0x88760005L)
#define DDERR_BLTFASTCANTCLIP        ((HRESULT)0x88760012L)
#define DDERR_CANNOTATTACHSURFACE    ((HRESULT)0x88760036L)
#define DDERR_CANNOTDETACHSURFACE    ((HRESULT)0x88760037L)
#define DDERR_CANTCREATEDC           ((HRESULT)0x8876003cL)
#define DDERR_CANTDUPLICATE          ((HRESULT)0x8876003dL)
#define DDERR_CANTLOCKSURFACE        ((HRESULT)0x8876003fL)
#define DDERR_CANTPAGELOCK           ((HRESULT)0x88760040L)
#define DDERR_CANTPAGEUNLOCK         ((HRESULT)0x88760044L)
#define DDERR_CLIPPERISUSINGHWND     ((HRESULT)0x88760046L)
#define DDERR_COLORKEYNOTSET         ((HRESULT)0x8876005aL)
#define DDERR_CURRENTLYNOTAVAIL      ((HRESULT)0x88760064L)
#define DDERR_DDSCAPSCOMPLEXREQUIRED ((HRESULT)0x88760066L)
#define DDERR_DCALREADYCREATED       ((HRESULT)0x88760069L)
#define DDERR_DEVICEDOESNTOWNSURFACE ((HRESULT)0x88760070L)
#define DDERR_DIRECTDRAWALREADYCREATED ((HRESULT)0x88760072L)
#define DDERR_EXCEPTION              ((HRESULT)0x88760078L)
#define DDERR_EXCLUSIVEMODEALREADYSET ((HRESULT)0x88760079L)
#define DDERR_EXPIRED                ((HRESULT)0x8876007bL)
#define DDERR_GENERIC                ((HRESULT)0x80004005L)
#define DDERR_HEIGHTALIGN            ((HRESULT)0x88760082L)
#define DDERR_HWNDALREADYSET         ((HRESULT)0x88760087L)
#define DDERR_HWNDSUBCLASSED         ((HRESULT)0x88760088L)
#define DDERR_IMPLICITLYCREATED      ((HRESULT)0x88760089L)
#define DDERR_INCOMPATIBLEPRIMARY    ((HRESULT)0x8876008bL)
#define DDERR_INVALIDCAPS            ((HRESULT)0x8876008cL)
#define DDERR_INVALIDCLIPLIST        ((HRESULT)0x8876008dL)
#define DDERR_INVALIDDIRECTDRAWGUID  ((HRESULT)0x8876008eL)
#define DDERR_INVALIDMODE            ((HRESULT)0x8876008fL)
#define DDERR_INVALIDOBJECT          ((HRESULT)0x88760090L)
#define DDERR_INVALIDPARAMS          ((HRESULT)0x80070057L)
#define DDERR_INVALIDPIXELFORMAT     ((HRESULT)0x88760091L)
#define DDERR_INVALIDPOSITION        ((HRESULT)0x88760092L)
#define DDERR_INVALIDRECT            ((HRESULT)0x88760096L)
#define DDERR_INVALIDSTREAM          ((HRESULT)0x88760099L)
#define DDERR_INVALIDSURFACETYPE     ((HRESULT)0x8876009aL)
#define DDERR_LOCKEDSURFACES         ((HRESULT)0x887600a0L)
#define DDERR_MOREDATA               ((HRESULT)0x887600a1L)
#define DDERR_NO3D                   ((HRESULT)0x887600a2L)
#define DDERR_NOALPHAHW              ((HRESULT)0x887600a3L)
#define DDERR_NOBLTHW                ((HRESULT)0x887600a5L)
#define DDERR_NOCLIPLIST             ((HRESULT)0x887600a9L)
#define DDERR_NOCLIPPERATTACHED      ((HRESULT)0x887600aaL)
#define DDERR_NOCOLORCONVHW          ((HRESULT)0x887600abL)
#define DDERR_NOCOLORKEY             ((HRESULT)0x887600adL)
#define DDERR_NOCOLORKEYHW           ((HRESULT)0x887600aeL)
#define DDERR_NOCOOPERATIVELEVELSET  ((HRESULT)0x887600b0L)
#define DDERR_NODC                   ((HRESULT)0x887600b1L)
#define DDERR_NODDROPSHW             ((HRESULT)0x887600b3L)
#define DDERR_NODIRECTDRAWHW         ((HRESULT)0x887600b5L)
#define DDERR_NODIRECTDRAWSUPPORT    ((HRESULT)0x887600b7L)
#define DDERR_NOEMULATION            ((HRESULT)0x887600b8L)
#define DDERR_NOEXCLUSIVEMODE        ((HRESULT)0x887600b9L)
#define DDERR_NOFLIPHW               ((HRESULT)0x887600baL)
#define DDERR_NOFOCUSWINDOW          ((HRESULT)0x887600bcL)
#define DDERR_NOGDI                  ((HRESULT)0x887600beL)
#define DDERR_NOHWND                 ((HRESULT)0x887600bfL)
#define DDERR_NOMIPMAPHW             ((HRESULT)0x887600c3L)
#define DDERR_NOMIRRORHW             ((HRESULT)0x887600c5L)
#define DDERR_NONONLOCALVIDMEM       ((HRESULT)0x887600c6L)
#define DDERR_NOOPTIMIZEHW           ((HRESULT)0x887600c8L)
#define DDERR_NOOVERLAYDEST          ((HRESULT)0x887600caL)
#define DDERR_NOOVERLAYHW            ((HRESULT)0x887600cbL)
#define DDERR_NOPALETTEATTACHED      ((HRESULT)0x887600ccL)
#define DDERR_NOPALETTEHW            ((HRESULT)0x887600cdL)
#define DDERR_NORASTEROPHW           ((HRESULT)0x887600d2L)
#define DDERR_NOROTATIONHW           ((HRESULT)0x887600d3L)
#define DDERR_NOSTRETCHHW            ((HRESULT)0x887600d5L)
#define DDERR_NOT4BITCOLOR           ((HRESULT)0x887600d6L)
#define DDERR_NOT4BITCOLORINDEX      ((HRESULT)0x887600d7L)
#define DDERR_NOT8BITCOLOR           ((HRESULT)0x887600d8L)
#define DDERR_NOTAOVERLAYSURFACE     ((HRESULT)0x88760219L)
#define DDERR_NOTEXTUREHW            ((HRESULT)0x887600d9L)
#define DDERR_NOTFLIPPABLE           ((HRESULT)0x887600daL)
#define DDERR_NOTFOUND               ((HRESULT)0x887600dbL)
#define DDERR_NOTINITIALIZED         ((HRESULT)0x8876018AL)
#define DDERR_NOTLOCKED              ((HRESULT)0x887600dcL)
#define DDERR_NOTPAGELOCKED          ((HRESULT)0x887600deL)
#define DDERR_NOTPALETTIZED          ((HRESULT)0x887600dfL)
#define DDERR_NOVSYNCHW              ((HRESULT)0x887600e1L)
#define DDERR_NOZBUFFERHW            ((HRESULT)0x887600e2L)
#define DDERR_NOZOVERLAYHW           ((HRESULT)0x887600e3L)
#define DDERR_OUTOFCAPS              ((HRESULT)0x887600e4L)
#define DDERR_OUTOFMEMORY            ((HRESULT)0x8007000EL)
#define DDERR_OUTOFVIDEOMEMORY       ((HRESULT)0x8876017CL)
#define DDERR_OVERLAYCANTCLIP        ((HRESULT)0x887600e6L)
#define DDERR_OVERLAYCOLORKEYONLYONEACTIVE ((HRESULT)0x887600e7L)
#define DDERR_OVERLAYNOTVISIBLE      ((HRESULT)0x887600e9L)
#define DDERR_PALETTEBUSY            ((HRESULT)0x887600eaL)
#define DDERR_PRIMARYSURFACEALREADYEXISTS ((HRESULT)0x887600ecL)
#define DDERR_REGIONTOOSMALL         ((HRESULT)0x887600eeL)
#define DDERR_REGIONTOOBIG           ((HRESULT)0x887600efL)
#define DDERR_SURFACEBUSY            ((HRESULT)0x88760176L)
#define DDERR_SURFACEALREADYATTACHED ((HRESULT)0x887600f4L)
#define DDERR_SURFACEALREADYDEPENDENT ((HRESULT)0x887600f5L)
#define DDERR_SURFACENOTATTACHED     ((HRESULT)0x887600f6L)
#define DDERR_SURFACEISOBSCURED      ((HRESULT)0x887600feL)
#define DDERR_SURFACELOST            ((HRESULT)0x887601C2L)
#define DDERR_TESTFINISHED           ((HRESULT)0x8876010aL)
#define DDERR_TOOBIGHEIGHT           ((HRESULT)0x8876010eL)
#define DDERR_TOOBIGSIZE             ((HRESULT)0x8876010fL)
#define DDERR_TOOBIGWIDTH            ((HRESULT)0x88760110L)
#define DDERR_UNSUPPORTED            ((HRESULT)0x80004001L)
#define DDERR_UNSUPPORTEDFORMAT      ((HRESULT)0x8876011dL)
#define DDERR_UNSUPPORTEDMASK        ((HRESULT)0x8876011eL)
#define DDERR_UNSUPPORTEDMODE        ((HRESULT)0x8876011fL)
#define DDERR_VERTICALBLANKINPROGRESS ((HRESULT)0x88760121L)
#define DDERR_VIDEONOTACTIVE         ((HRESULT)0x88760123L)
#define DDERR_WASSTILLDRAWING        ((HRESULT)0x8876018cL)
#define DDERR_WRONGMODE              ((HRESULT)0x8876012eL)
#define DDERR_XALIGN                 ((HRESULT)0x88760130L)
#define DDERR_NODRIVERSUPPORT        ((HRESULT)0x88760048L)
#define DDERR_NEWMODE                ((HRESULT)0x887600c0L)
#define DDERR_NOMONITORINFORMATION   ((HRESULT)0x887601D5L)
#define DDERR_NOSTEREOHARDWARE       ((HRESULT)0x887600d4L)
#define DDERR_NOSURFACELEFT          ((HRESULT)0x887601E7L)
#define DDERR_NOTLOADED              ((HRESULT)0x887600ECL)
#define DDERR_OVERLAPPINGRECTS       ((HRESULT)0x887600E8L)

// DllMain reason codes (native build compiles lib3d's DllMain as dead code —
// it's a shared object seam the native exe doesn't call)
#ifndef DLL_PROCESS_DETACH
#define DLL_PROCESS_DETACH   0
#define DLL_PROCESS_ATTACH   1
#define DLL_THREAD_ATTACH    2
#define DLL_THREAD_DETACH    3
#endif

// --- Enumeration -----------------------------------------------------------------
#define DDENUM_ATTACHEDSECONDARYDEVICES  0x00000001L
#define DDENUM_DETACHEDSECONDARYDEVICES  0x00000002L
#define DDENUM_NONDISPLAYDEVICES         0x00000004L
#ifndef DDENUMRET_OK
#define DDENUMRET_OK     1
#define DDENUMRET_CANCEL 0
#endif

// --- Cooperative level ------------------------------------------------------------
#define DDSCL_FULLSCREEN        0x00000001L
#define DDSCL_ALLOWREBOOT       0x00000002L
#define DDSCL_NOWINDOWCHANGES   0x00000004L
#define DDSCL_NORMAL            0x00000008L
#define DDSCL_EXCLUSIVE         0x00000010L
#define DDSCL_ALLOWMODEX        0x00000040L
#define DDSCL_SETFOCUSWINDOW    0x00000080L
#define DDSCL_SETDEVICEWINDOW   0x00000100L
#define DDSCL_CREATEDEVICEWINDOW 0x00000200L
#define DDSCL_MULTITHREADED     0x00000400L
#define DDSCL_FPUSETUP          0x00000800L

// --- Surface description flags ------------------------------------------------------
#define DDSD_CAPS               0x00000001L
#define DDSD_HEIGHT             0x00000002L
#define DDSD_WIDTH              0x00000004L
#define DDSD_PITCH              0x00000008L
#define DDSD_BACKBUFFERCOUNT    0x00000020L
#define DDSD_ZBUFFERBITDEPTH    0x00000040L
#define DDSD_ALPHABITDEPTH      0x00000080L
#define DDSD_LPSURFACE          0x00000800L
#define DDSD_PIXELFORMAT        0x00001000L
#define DDSD_CKDESTOVERLAY      0x00002000L
#define DDSD_CKDESTBLT          0x00004000L
#define DDSD_CKSRCOVERLAY       0x00008000L
#define DDSD_CKSRCBLT           0x00010000L
#define DDSD_MIPMAPCOUNT        0x00020000L
#define DDSD_REFRESHRATE        0x00040000L
#define DDSD_LINEARSIZE         0x00080000L
#define DDSD_TEXTURESTAGE       0x00100000L
#define DDSD_FVF                0x00200000L
#define DDSD_SRCVBHANDLE        0x00400000L
#define DDSD_DEPTH              0x00800000L
#define DDSD_ALL                0x00fff9eeL

// --- Surface caps ----------------------------------------------------------------------
#define DDSCAPS_RESERVED1               0x00000001L
#define DDSCAPS_ALPHA                   0x00000002L
#define DDSCAPS_BACKBUFFER              0x00000004L
#define DDSCAPS_COMPLEX                 0x00000008L
#define DDSCAPS_FLIP                    0x00000010L
#define DDSCAPS_FRONTBUFFER             0x00000020L
#define DDSCAPS_OFFSCREENPLAIN          0x00000040L
#define DDSCAPS_OVERLAY                 0x00000080L
#define DDSCAPS_PALETTE                 0x00000100L
#define DDSCAPS_PRIMARYSURFACE          0x00000200L
#define DDSCAPS_RESERVED3               0x00000400L
#define DDSCAPS_PRIMARYSURFACELEFT      0x00000000L
#define DDSCAPS_SYSTEMMEMORY            0x00000800L
#define DDSCAPS_TEXTURE                 0x00001000L
#define DDSCAPS_3DDEVICE                0x00002000L
#define DDSCAPS_VIDEOMEMORY             0x00004000L
#define DDSCAPS_VISIBLE                 0x00008000L
#define DDSCAPS_WRITEONLY               0x00010000L
#define DDSCAPS_ZBUFFER                 0x00020000L
#define DDSCAPS_OWNDC                   0x00040000L
#define DDSCAPS_LIVEVIDEO               0x00080000L
#define DDSCAPS_HWCODEC                 0x00100000L
#define DDSCAPS_MODEX                   0x00200000L
#define DDSCAPS_MIPMAP                  0x00400000L
#define DDSCAPS_RESERVED2               0x00800000L
#define DDSCAPS_ALLOCONLOAD             0x04000000L
#define DDSCAPS_VIDEOPORT               0x08000000L
#define DDSCAPS_LOCALVIDMEM             0x10000000L
#define DDSCAPS_NONLOCALVIDMEM          0x20000000L
#define DDSCAPS_STANDARDVGAMODE         0x40000000L
#define DDSCAPS_OPTIMIZED               0x80000000L

#define DDSCAPS2_HARDWAREDEINTERLACE    0x00000002L
#define DDSCAPS2_HINTDYNAMIC            0x00000004L
#define DDSCAPS2_HINTSTATIC             0x00000008L
#define DDSCAPS2_TEXTUREMANAGE          0x00000010L
#define DDSCAPS2_RESERVED1              0x00000020L
#define DDSCAPS2_RESERVED2              0x00000040L
#define DDSCAPS2_OPAQUE                 0x00000080L
#define DDSCAPS2_HINTANTIALIASING       0x00000100L
#define DDSCAPS2_CUBEMAP                0x00000200L
#define DDSCAPS2_CUBEMAP_POSITIVEX      0x00000400L
#define DDSCAPS2_CUBEMAP_NEGATIVEX      0x00000800L
#define DDSCAPS2_CUBEMAP_POSITIVEY      0x00001000L
#define DDSCAPS2_CUBEMAP_NEGATIVEY      0x00002000L
#define DDSCAPS2_CUBEMAP_POSITIVEZ      0x00004000L
#define DDSCAPS2_CUBEMAP_NEGATIVEZ      0x00008000L
#define DDSCAPS2_MIPMAPSUBLEVEL         0x00010000L
#define DDSCAPS2_D3DTEXTUREMANAGE       0x00020000L
#define DDSCAPS2_DONOTPERSIST           0x00040000L
#define DDSCAPS2_STEREOSURFACELEFT      0x00080000L
#define DDCAPS2_NOPAGELOCKREQUIRED      0x00800000L  // driver-caps bit (SDK name)
#define DDSCAPS2_NOPAGELOCKREQUIRED     0x00800000L
#define DDSCAPS2_VOLUMETEXTURE          0x20000000L
#define DDSCAPS2_NOTUSERLOCKABLE        0x00400000L
#define DDSCAPS2_POINTS                 0x00800000L
#define DDSCAPS2_RTPATCHES              0x01000000L
#define DDSCAPS2_NPATCHES               0x02000000L
#define DDSCAPS2_DISCARDBACKBUFFER      0x10000000L
#define DDSCAPS2_ENABLEALPHACHANNEL     0x40000000L
#define DDSCAPS2_EXTENDEDFORMATPRIMARY  0x80000000L
#define DDSCAPS2_ADDITIONALPRIMARY      0x80000000L

// --- Lock/blt/flip flags -----------------------------------------------------------------
#define DDLOCK_SURFACEMEMORYPTR  0x00000000L
#define DDLOCK_WAIT              0x00000001L
#define DDLOCK_EVENT             0x00000002L
#define DDLOCK_READONLY          0x00000010L
#define DDLOCK_WRITEONLY         0x00000020L
#define DDLOCK_NOSYSLOCK         0x00000800L
#define DDLOCK_NOOVERWRITE       0x00001000L
#define DDLOCK_DISCARDCONTENTS   0x00002000L
#define DDLOCK_NODIRTYUPDATE     0x00008000L

#define DDBLT_ALPHADEST                   0x00000001L
#define DDBLT_ALPHADESTCONSTOVERRIDE      0x00000002L
#define DDBLT_ALPHADESTNEGATE             0x00000004L
#define DDBLT_ALPHADESTSURFACEOVERRIDE    0x00000008L
#define DDBLT_ALPHAEDGEBLEND              0x00000010L
#define DDBLT_ALPHASRC                    0x00000020L
#define DDBLT_ALPHASRCCONSTOVERRIDE       0x00000040L
#define DDBLT_ALPHASRCNEGATE              0x00000080L
#define DDBLT_ALPHASRCSURFACEOVERRIDE     0x00000100L
#define DDBLT_ROTATIONANGLE               0x00000200L
#define DDBLT_COLORFILL                   0x00000400L
#define DDBLT_DDFX                        0x00080000L
#define DDBLT_DDROPS                      0x00100000L
#define DDBLT_KEYDEST                     0x00001000L
#define DDBLT_KEYDESTOVERRIDE             0x00002000L
#define DDBLT_KEYSRC                      0x00004000L
#define DDBLT_KEYSRCOVERRIDE              0x00008000L
#define DDBLT_ROP                         0x00010000L
#define DDBLT_WAIT                        0x01000000L
#define DDBLT_DEPTHFILL                   0x02000000L
#define DDBLT_DONOTWAIT                   0x08000000L
#define DDBLT_PRESENTATION                0x10000000L
#define DDBLT_LAST_PRESENTATION           0x20000000L
#define DDBLT_LAST_DIRTY                  0x80000000L

#define DDBLTFAST_NOCOLORKEY      0x00000000L
#define DDBLTFAST_SRCCOLORKEY     0x00000001L
#define DDBLTFAST_DESTCOLORKEY    0x00000002L
#define DDBLTFAST_WAIT            0x00000010L
#define DDBLTFAST_DONOTWAIT       0x00000020L

#define DDFLIP_WAIT       0x00000001L
#define DDFLIP_EVEN       0x00000002L
#define DDFLIP_ODD        0x00000004L
#define DDFLIP_NOVSYNC    0x00000008L
#define DDFLIP_STEREO     0x00000010L
#define DDFLIP_DONOTWAIT  0x00000020L
#define DDFLIP_INTERVAL2  0x02000000L
#define DDFLIP_INTERVAL3  0x03000000L
#define DDFLIP_INTERVAL4  0x04000000L

// --- Palette/pixel-format flags -------------------------------------------------------------
#define DDPCAPS_4BIT            0x00000001L
#define DDPCAPS_8BITENTRIES     0x00000002L
#define DDPCAPS_8BIT            0x00000004L
#define DDPCAPS_INITIALIZE      0x00000008L
#define DDPCAPS_PRIMARYSURFACE  0x00000010L
#define DDPCAPS_PRIMARYSURFACELEFT 0x00000020L
#define DDPCAPS_ALLOW256        0x00000040L
#define DDPCAPS_VSYNC           0x00000080L
#define DDPCAPS_1BIT            0x00000100L
#define DDPCAPS_2BIT            0x00000200L

#define DDPF_ALPHAPIXELS        0x00000001L
#define DDPF_ALPHA              0x00000002L
#define DDPF_FOURCC             0x00000004L
#define DDPF_PALETTEINDEXED4    0x00000008L
#define DDPF_PALETTEINDEXEDTO8  0x00000010L
#define DDPF_PALETTEINDEXED8    0x00000020L
#define DDPF_RGB                0x00000040L
#define DDPF_COMPRESSED         0x00000080L
#define DDPF_RGBTOYUV           0x00000100L
#define DDPF_YUV                0x00000200L
#define DDPF_ZBUFFER            0x00000400L
#define DDPF_PALETTEINDEXED1    0x00000800L
#define DDPF_PALETTEINDEXED2    0x00001000L
#define DDPF_ZPIXELS            0x00002000L
#define DDPF_STENCILBUFFER      0x00004000L
#define DDPF_ALPHAPREMULT       0x00008000L
#define DDPF_LUMINANCE          0x00010000L
#define DDPF_BUMPLUMINANCE      0x00020000L
#define DDPF_BUMPDUDV           0x00040000L

// --- DDCAPS / DDDEVICEIDENTIFIER2 / DDBLTFX / DDGAMMARAMP -------------------------------------
typedef struct _DDCAPS {
    DWORD dwSize, dwCaps, dwCaps2, dwCaps3, dwCaps4;
    DWORD dwDSCAPS, dwCKeyCaps, dwFXCaps;
    DWORD dwFXAlphaCaps;
    DWORD dwPalCaps, dwSVCaps;
    DWORD dwAlphaBltConstBitDepths, dwAlphaBltPixelBitDepths;
    DWORD dwAlphaBltSurfaceBitDepths;
    DWORD dwAlphaOverlayConstBitDepths, dwAlphaOverlayPixelBitDepths;
    DWORD dwAlphaOverlaySurfaceBitDepths;
    DWORD dwZBufferBitDepths;
    DWORD dwVidMemTotal, dwVidMemFree, dwMaxVisibleOverlays, dwCurrVisibleOverlays;
    DWORD dwNumFourCCCodes;
    DWORD dwAlignBoundarySrc, dwAlignSizeSrc, dwAlignBoundaryDest, dwAlignSizeDest;
    DWORD dwAlignStrideAlign;
    DWORD dwRops[8];
    DDSCAPS2 ddsOldCaps;
    DWORD dwMinOverlayStretch, dwMaxOverlayStretch;
    DWORD dwMinLiveVideoStretch, dwMaxLiveVideoStretch;
    DWORD dwMinHwCodecStretch, dwMaxHwCodecStretch;
    DWORD dwReserved1, dwReserved2, dwReserved3;
    DWORD dwSVBCaps, dwSVBCKeyCaps, dwSVBFXCaps; DWORD dwSVBRops[8];
    DWORD dwVSBCaps, dwVSBCKeyCaps, dwVSBFXCaps; DWORD dwVSBRops[8];
    DWORD dwSSBCaps, dwSSBCKeyCaps, dwSSBCKeyCaps2, dwSSBFXCaps; DWORD dwSSBRops[8];
    DWORD dwMaxVideoPorts, dwCurrVideoPorts;
    DWORD dwSVBCaps2;
    DWORD dwNLVBCaps, dwNLVBCaps2, dwNLVBCKeyCaps, dwNLVBFXCaps; DWORD dwNLVBRops[8];
    DDSCAPS2 ddsCaps;
} DDCAPS;
typedef DDCAPS* LPDDCAPS;
#define DDCAPS_3D               0x00000001L
#define DDCAPS_ALIGNBOUNDARYDEST 0x00000002L
#define DDCAPS_ALIGNSIZEDEST    0x00000004L
#define DDCAPS_ALIGNBOUNDARYSRC 0x00000008L
#define DDCAPS_ALIGNSIZESRC     0x00000010L
#define DDCAPS_ALIGNSTRIDE      0x00000020L
#define DDCAPS_BLT              0x00000040L
#define DDCAPS_BLTQUEUE         0x00000080L
#define DDCAPS_BLTFOURCC        0x00000100L
#define DDCAPS_BLTSTRETCH       0x00000200L
#define DDCAPS_GDI              0x00000400L
#define DDCAPS_OVERLAY          0x00000800L
#define DDCAPS_ZBLTS            0x00001000L
#define DDCAPS_ZOVERLAYS        0x00002000L
#define DDCAPS_COLORKEY         0x00004000L
#define DDCAPS_ALPHA            0x00008000L
#define DDCAPS_COLORKEYHWASSIST 0x00010000L
#define DDCAPS_NOHARDWARE       0x00020000L
#define DDCAPS_BLTCOLORFILL     0x00040000L
#define DDCAPS_BANKSWITCHED     0x00080000L
#define DDCAPS_BLTDEPTHFILL     0x00100000L
#define DDCAPS_CANCLIP          0x00200000L
#define DDCAPS_CANCLIPSTRETCHED 0x00400000L
#define DDCAPS_READSCANLINE     0x00800000L
#define DDCAPS_CERTIFIED        0x02000000L
#define DDCAPS_NOBLT            0x04000000L

typedef struct _DDDEVICEIDENTIFIER2 {
    char  szDriver[512];
    char  szDescription[512];
    LARGE_INTEGER liDriverVersion;
    DWORD dwVendorId, dwDeviceId, dwSubSysId, dwRevision;
    GUID  guidDeviceIdentifier;
    DWORD dwWHQLLevel;
} DDDEVICEIDENTIFIER2;
typedef DDDEVICEIDENTIFIER2* LPDDDEVICEIDENTIFIER2;
#define DDGDI_GETHOSTIDENTIFIER 0x00000001L

struct IDirectDrawSurface7;  // fwd for DDBLTFX unions
typedef struct _DDBLTFX {
    DWORD dwSize, dwDDFX;
    DWORD dwROP, dwDDROP, dwRotationAngle, dwZBufferOpCode, dwZBufferLow, dwZBufferHigh;
    DWORD dwZBufferBaseDest, dwZDestConstBitDepth;
    union { DWORD dwZDestConst; struct IDirectDrawSurface7* lpDDSZBufferDest; };
    DWORD dwZSrcConstBitDepth;
    union { DWORD dwZSrcConst; struct IDirectDrawSurface7* lpDDSZBufferSrc; };
    DWORD dwAlphaEdgeBlendBitDepth, dwAlphaEdgeBlend, dwReserved;
    DWORD dwAlphaDestConstBitDepth;
    union { DWORD dwAlphaDestConst; struct IDirectDrawSurface7* lpDDSAlphaDest; };
    DWORD dwAlphaSrcConstBitDepth;
    union { DWORD dwAlphaSrcConst; struct IDirectDrawSurface7* lpDDSAlphaSrc; };
    union { DWORD dwFillColor; DWORD dwFillDepth; DWORD dwFillPixel; struct IDirectDrawSurface7* lpDDSPattern; };
    DDCOLORKEY ddckDestColorkey, ddckSrcColorkey;
} DDBLTFX;
typedef DDBLTFX* LPDDBLTFX;

typedef struct _DDGAMMARAMP { WORD red[256], green[256], blue[256]; } DDGAMMARAMP;
typedef DDGAMMARAMP* LPDDGAMMARAMP;

#ifndef PALETTEENTRY_DEFINED
#define PALETTEENTRY_DEFINED
typedef struct tagPALETTEENTRY { BYTE peRed, peGreen, peBlue, peFlags; } PALETTEENTRY, *LPPALETTEENTRY;
#endif

// --- Emulated-interface typedefs shared by ddraw.h/d3d.h ------------------------
struct IDirectDraw7;
struct IDirectDrawSurface7;
struct IDirectDrawPalette;
struct IDirectDrawClipper;
struct IDirectDrawGammaControl;
struct IDirect3D7;
struct IDirect3DDevice7;
struct IDirect3DVertexBuffer7;
typedef IDirectDraw7*           LPDIRECTDRAW7;
typedef IDirectDrawSurface7*    LPDIRECTDRAWSURFACE7;
typedef IDirectDrawPalette*     LPDIRECTDRAWPALETTE;
typedef IDirectDrawClipper*     LPDIRECTDRAWCLIPPER;
typedef IDirectDrawGammaControl* LPDIRECTDRAWGAMMACONTROL;
typedef IDirect3D7*             LPDIRECT3D7;
typedef IDirect3DDevice7*       LPDIRECT3DDEVICE7;
typedef IDirect3DVertexBuffer7* LPDIRECT3DVERTEXBUFFER7;
typedef BOOL (WINAPI *LPDDENUMCALLBACKEXA)(GUID*, LPSTR, LPSTR, LPVOID, HMONITOR);
typedef LPDDENUMCALLBACKEXA LPDDENUMCALLBACKEX;
typedef BOOL (WINAPI *LPDDENUMCALLBACKA)(GUID*, LPSTR, LPSTR, LPVOID);
typedef HRESULT (WINAPI *LPDDENUMSURFACESCALLBACK7)(LPDIRECTDRAWSURFACE7, LPDDSURFACEDESC2, LPVOID);

// IID_IDirectDrawGammaControl — queried off surfaces by lib3d's gamma path
#ifndef IID_IDirectDrawGammaControl_DEFINED
#define IID_IDirectDrawGammaControl_DEFINED
DEFINE_GUID(IID_IDirectDrawGammaControl, 0x1860d94e,0x41ed,0x11d0,0x97,0xa8,0x00,0x00,0xc0,0x3f,0xd8,0x00);
#endif
#endif // ROWAN_COMPAT_D3DTYPES_H
