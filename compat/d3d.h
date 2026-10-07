// <d3d.h> shim — DX7's umbrella header pulled types + interfaces.
// Types come from d3dtypes.h (real SDK enums via <d3d7_sdk.h>); the COM
// device surface is a native emulation in the mig-stub style: plain C++
// structs the game calls directly. Rendering calls record state and
// succeed; enumeration/caps report a real synthetic TnL-HAL device so
// lib3d's driver selection runs the same code paths as on hardware.
#ifndef ROWAN_COMPAT_D3D_H
#define ROWAN_COMPAT_D3D_H

// BoB TUs sit under a global pack(1) (dosdefs.h); the port-layer
// COM classes/DD structs must keep natural alignment so every TU
// agrees on member offsets (ODR) — T123 BoBFrag OOB via CREdtBt.
#pragma pack(push,8)
#include "d3dtypes.h"

#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <alloca.h>
#include <map>

// Missing-name aliases used by lib3d.h
typedef D3DDEVICEDESC7* LPD3DDEVICEDESC7;

// --- HRESULT space ------------------------------------------------------------
#ifndef D3D_OK
#define D3D_OK                  0 /* S_OK */
#endif
#define D3DERR_BADMAJORVERSION  ((HRESULT)0x88760083L)
#define D3DERR_BADMINORVERSION  ((HRESULT)0x88760084L)
#define D3DERR_INVALID_DEVICE   ((HRESULT)0x88760325L)
#define D3DERR_INITFAILED       ((HRESULT)0x88760326L)
#define D3DERR_DEVICEAGGREGATED ((HRESULT)0x88760327L)
#define D3DERR_EXECUTE_CREATE_FAILED ((HRESULT)0x88760328L)
#define D3DERR_EXECUTE_DESTROY_FAILED ((HRESULT)0x88760329L)
#define D3DERR_EXECUTE_LOCK_FAILED ((HRESULT)0x8876032aL)
#define D3DERR_EXECUTE_LOCKED   ((HRESULT)0x8876032dL)
#define D3DERR_EXECUTE_NOT_LOCKED ((HRESULT)0x8876032eL)
#define D3DERR_EXECUTE_FAILED   ((HRESULT)0x8876032fL)
#define D3DERR_EXECUTE_CLIPPED_FAILED ((HRESULT)0x88760330L)
#define D3DERR_TEXTURE_NO_SUPPORT ((HRESULT)0x88760336L)
#define D3DERR_TEXTURE_CREATE_FAILED ((HRESULT)0x88760337L)
#define D3DERR_TEXTURE_DESTROY_FAILED ((HRESULT)0x88760338L)
#define D3DERR_TEXTURE_LOCK_FAILED ((HRESULT)0x88760339L)
#define D3DERR_TEXTURE_LOCKED   ((HRESULT)0x8876033cL)
#define D3DERR_TEXTURE_NOT_LOCKED ((HRESULT)0x8876033dL)
#define D3DERR_TEXTURE_GETSURF_FAILED ((HRESULT)0x8876033eL)
#define D3DERR_MATRIX_CREATE_FAILED ((HRESULT)0x88760340L)
#define D3DERR_MATRIX_DESTROY_FAILED ((HRESULT)0x88760341L)
#define D3DERR_MATRIX_SETDATA_FAILED ((HRESULT)0x88760342L)
#define D3DERR_MATRIX_GETDATA_FAILED ((HRESULT)0x88760343L)
#define D3DERR_SETVIEWPORTDATA_FAILED ((HRESULT)0x88760344L)
#define D3DERR_INVALIDCURRENTVIEWPORT ((HRESULT)0x8876034aL)
#define D3DERR_INVALIDPRIMITIVETYPE ((HRESULT)0x8876034bL)
#define D3DERR_INVALIDVERTEXTYPE ((HRESULT)0x8876034cL)
#define D3DERR_TEXTURE_BADSIZE  ((HRESULT)0x8876034dL)
#define D3DERR_INVALIDRAMPTEXTURE ((HRESULT)0x8876034eL)
#define D3DERR_MATERIAL_CREATE_FAILED ((HRESULT)0x8876034fL)
#define D3DERR_MATERIAL_DESTROY_FAILED ((HRESULT)0x88760350L)
#define D3DERR_MATERIAL_SETDATA_FAILED ((HRESULT)0x88760351L)
#define D3DERR_MATERIAL_GETDATA_FAILED ((HRESULT)0x88760352L)
#define D3DERR_INVALIDPALETTE   ((HRESULT)0x88760353L)
#define D3DERR_ZBUFF_NEEDS_SYSTEMMEMORY ((HRESULT)0x88760354L)
#define D3DERR_ZBUFF_NEEDS_VIDEOMEMORY ((HRESULT)0x88760355L)
#define D3DERR_SURFACENOTINVIDMEM ((HRESULT)0x88760356L)
#define D3DERR_LIGHT_SET_FAILED ((HRESULT)0x88760358L)
#define D3DERR_LIGHTHASVIEWPORT ((HRESULT)0x88760359L)
#define D3DERR_LIGHTNOTINTHISVIEWPORT ((HRESULT)0x8876035aL)
#define D3DERR_SCENE_IN_SCENE   ((HRESULT)0x8876035bL)
#define D3DERR_SCENE_NOT_IN_SCENE ((HRESULT)0x8876035cL)
#define D3DERR_SCENE_BEGIN_FAILED ((HRESULT)0x8876035dL)
#define D3DERR_SCENE_END_FAILED ((HRESULT)0x8876035eL)
#define D3DERR_INBEGIN          ((HRESULT)0x8876035fL)
#define D3DERR_NOTINBEGIN       ((HRESULT)0x88760360L)
#define D3DERR_NOVIEWPORTS      ((HRESULT)0x88760361L)
#define D3DERR_VIEWPORTDATANOTSET ((HRESULT)0x88760362L)
#define D3DERR_VIEWPORTHASNODEVICE ((HRESULT)0x88760363L)
#define D3DERR_NOCURRENTVIEWPORT ((HRESULT)0x88760364L)
#define D3DERR_INVALIDVERTEXFORMAT ((HRESULT)0x88760365L)
#define D3DERR_COLORKEYATTACHED ((HRESULT)0x88760366L)
#define D3DERR_VERTEXBUFFEROPTIMIZED ((HRESULT)0x88760367L)
#define D3DERR_VBUF_CREATE_FAILED ((HRESULT)0x88760368L)
#define D3DERR_VERTEXBUFFERLOCKED ((HRESULT)0x88760369L)
#define D3DERR_VERTEXBUFFERUNLOCKFAILED ((HRESULT)0x8876036aL)
#define D3DERR_ZBUFFER_NOTPRESENT ((HRESULT)0x8876036bL)
#define D3DERR_STENCILBUFFER_NOTPRESENT ((HRESULT)0x8876036cL)
#define D3DERR_WRONGTEXTUREFORMAT ((HRESULT)0x88760372L)
#define D3DERR_UNSUPPORTEDCOLOROPERATION ((HRESULT)0x88760373L)
#define D3DERR_UNSUPPORTEDCOLORARG ((HRESULT)0x88760374L)
#define D3DERR_UNSUPPORTEDALPHAOPERATION ((HRESULT)0x88760375L)
#define D3DERR_UNSUPPORTEDALPHAARG ((HRESULT)0x88760376L)
#define D3DERR_TOOMANYOPERATIONS ((HRESULT)0x88760377L)
#define D3DERR_CONFLICTINGTEXTUREFILTER ((HRESULT)0x88760378L)
#define D3DERR_UNSUPPORTEDFACTORVALUE ((HRESULT)0x88760379L)
#define D3DERR_CONFLICTINGRENDERSTATE ((HRESULT)0x88760381L)
#define D3DERR_UNSUPPORTEDTEXTUREFILTER ((HRESULT)0x88760382L)
#define D3DERR_TOOMANYPRIMITIVES ((HRESULT)0x88760383L)
#define D3DERR_INVALIDMATRIX    ((HRESULT)0x88760384L)
#define D3DERR_TOOMANYVERTICES  ((HRESULT)0x88760385L)
#define D3DERR_CONFLICTINGTEXTUREPALETTE ((HRESULT)0x88760386L)
#define D3DERR_INVALIDSTATEBLOCK ((HRESULT)0x88760387L)
#define D3DERR_INBEGINSTATEBLOCK ((HRESULT)0x88760388L)
#define D3DERR_NOTINBEGINSTATEBLOCK ((HRESULT)0x88760389L)
#define D3DERR_TEXTURE_LOAD_FAILED  ((HRESULT)0x8876038aL)
#define D3DERR_TEXTURE_SWAP_FAILED  ((HRESULT)0x8876038bL)
#define D3DERR_TEXTURE_UNLOCK_FAILED ((HRESULT)0x8876038cL)

#ifndef DDENUMRET_OK
#define DDENUMRET_OK     1
#define DDENUMRET_CANCEL 0
#endif
#ifndef D3DENUMRET_OK
#define D3DENUMRET_OK     DDENUMRET_OK
#define D3DENUMRET_CANCEL DDENUMRET_CANCEL
#endif

// --- Device/caps constants (SDK <d3dcaps.h>/<d3d.h> content) --------------------
#define D3DDEVCAPS_FLOATTLVERTEX        0x00000001L
#define D3DDEVCAPS_SORTINCREASINGZ      0x00000002L
#define D3DDEVCAPS_SORTDECREASINGZ      0x00000004L
#define D3DDEVCAPS_SORTEXACT            0x00000008L
#define D3DDEVCAPS_EXECUTESYSTEMMEMORY  0x00000010L
#define D3DDEVCAPS_EXECUTEVIDEOMEMORY   0x00000020L
#define D3DDEVCAPS_TLVERTEXSYSTEMMEMORY 0x00000040L
#define D3DDEVCAPS_TLVERTEXVIDEOMEMORY  0x00000080L
#define D3DDEVCAPS_TEXTURESYSTEMMEMORY  0x00000100L
#define D3DDEVCAPS_TEXTUREVIDEOMEMORY   0x00000200L
#define D3DDEVCAPS_DRAWPRIMTLVERTEX     0x00000400L
#define D3DDEVCAPS_CANRENDERAFTERFLIP   0x00000800L
#define D3DDEVCAPS_TEXTURENONLOCALVIDMEM 0x00001000L
#define D3DDEVCAPS_DRAWPRIMITIVES2      0x00002000L
#define D3DDEVCAPS_SEPARATETEXTUREMEMORIES 0x00004000L
#define D3DDEVCAPS_DRAWPRIMITIVES2EX    0x00008000L
#define D3DDEVCAPS_HWTRANSFORMANDLIGHT  0x00010000L
#define D3DDEVCAPS_CANBLTSYSTONONLOCAL  0x00020000L
#define D3DDEVCAPS_HWRASTERIZATION      0x00080000L
#define D3DDEVCAPS_PUREDEVICE           0x00100000L

#define D3DPCMISCCAPS_MASKPLANES            0x00000001L
#define D3DPCMISCCAPS_MASKZ                 0x00000002L
#define D3DPCMISCCAPS_LINEPATTERNREP        0x00000004L
#define D3DPCMISCCAPS_CONFORMANT            0x00000008L
#define D3DPCMISCCAPS_CULLNONE              0x00000010L
#define D3DPCMISCCAPS_CULLCW                0x00000020L
#define D3DPCMISCCAPS_CULLCCW               0x00000040L

#define D3DPRASTERCAPS_DITHER               0x00000001L
#define D3DPRASTERCAPS_ROP2                 0x00000002L
#define D3DPRASTERCAPS_XOR                  0x00000004L
#define D3DPRASTERCAPS_PAT                  0x00000008L
#define D3DPRASTERCAPS_ZTEST                0x00000010L
#define D3DPRASTERCAPS_SUBPIXEL             0x00000020L
#define D3DPRASTERCAPS_SUBPIXELX            0x00000040L
#define D3DPRASTERCAPS_FOGVERTEX            0x00000080L
#define D3DPRASTERCAPS_FOGTABLE             0x00000100L
#define D3DPRASTERCAPS_STIPPLE              0x00000200L
#define D3DPRASTERCAPS_ANTIALIASSORTDEPENDENT 0x00000400L
#define D3DPRASTERCAPS_ANTIALIASSORTINDEPENDENT 0x00000800L
#define D3DPRASTERCAPS_ANTIALIASEDGES       0x00001000L
#define D3DPRASTERCAPS_MIPMAPLODBIAS        0x00002000L
#define D3DPRASTERCAPS_ZBIAS                0x00004000L
#define D3DPRASTERCAPS_ZBUFFERLESSHSR       0x00008000L
#define D3DPRASTERCAPS_FOGRANGE             0x00010000L
#define D3DPRASTERCAPS_ANISOTROPY           0x00020000L
#define D3DPRASTERCAPS_WBUFFER              0x00040000L
#define D3DPRASTERCAPS_TRANSLUCENTSORTINDEPENDENT 0x00080000L
#define D3DPRASTERCAPS_WFOG                 0x00100000L
#define D3DPRASTERCAPS_ZFOG                 0x00200000L
#define D3DPRASTERCAPS_COLORPERSPECTIVE     0x00400000L
#define D3DPRASTERCAPS_STRETCHBLTMULTISAMPLE 0x00800000L

#define D3DPTEXTURECAPS_PERSPECTIVE         0x00000001L
#define D3DPTEXTURECAPS_POW2                0x00000002L
#define D3DPTEXTURECAPS_ALPHA               0x00000004L
#define D3DPTEXTURECAPS_TRANSPARENCY        0x00000008L
#define D3DPTEXTURECAPS_BORDER              0x00000010L
#define D3DPTEXTURECAPS_SQUAREONLY          0x00000020L
#define D3DPTEXTURECAPS_TEXREPEATNOTSCALEDBYSIZE 0x00000040L
#define D3DPTEXTURECAPS_ALPHAPALETTE        0x00000080L
#define D3DPTEXTURECAPS_NONPOW2CONDITIONAL  0x00000100L
#define D3DPTEXTURECAPS_PROJECTED           0x00000400L
#define D3DPTEXTURECAPS_CUBEMAP             0x00000800L
#define D3DPTEXTURECAPS_COLORKEYBLEND       0x00001000L

#define D3DPTFILTERCAPS_NEAREST             0x00000001L
#define D3DPTFILTERCAPS_LINEAR              0x00000002L
#define D3DPTFILTERCAPS_MIPNEAREST          0x00000004L
#define D3DPTFILTERCAPS_MIPLINEAR           0x00000008L
#define D3DPTFILTERCAPS_LINEARMIPNEAREST    0x00000010L
#define D3DPTFILTERCAPS_LINEARMIPLINEAR     0x00000020L
#define D3DPTFILTERCAPS_MINFPOINT           0x00000100L
#define D3DPTFILTERCAPS_MINFLINEAR          0x00000200L
#define D3DPTFILTERCAPS_MINFANISOTROPIC     0x00000400L
#define D3DPTFILTERCAPS_MIPFPOINT           0x00001000L
#define D3DPTFILTERCAPS_MIPFLINEAR          0x00002000L
#define D3DPTFILTERCAPS_MAGFPOINT           0x00010000L
#define D3DPTFILTERCAPS_MAGFLINEAR          0x00020000L
#define D3DPTFILTERCAPS_MAGFANISOTROPIC     0x00040000L
#define D3DPTFILTERCAPS_MAGFAFLATCUBIC      0x00100000L
#define D3DPTFILTERCAPS_MAGFANISOTROPIC2    0x00200000L

#define D3DPTBLENDCAPS_DECAL                0x00000001L
#define D3DPTBLENDCAPS_MODULATE             0x00000002L
#define D3DPTBLENDCAPS_DECALALPHA           0x00000004L
#define D3DPTBLENDCAPS_MODULATEALPHA        0x00000008L
#define D3DPTBLENDCAPS_DECALMASK            0x00000010L
#define D3DPTBLENDCAPS_MODULATEMASK         0x00000020L
#define D3DPTBLENDCAPS_COPY                 0x00000040L
#define D3DPTBLENDCAPS_ADD                  0x00000080L

#define D3DPTADDRESSCAPS_WRAP               0x00000001L
#define D3DPTADDRESSCAPS_MIRROR             0x00000002L
#define D3DPTADDRESSCAPS_CLAMP              0x00000004L
#define D3DPTADDRESSCAPS_BORDER             0x00000008L
#define D3DPTADDRESSCAPS_INDEPENDENTUV      0x00000010L

#define D3DPBLENDCAPS_ZERO                  0x00000001L
#define D3DPBLENDCAPS_ONE                   0x00000002L
#define D3DPBLENDCAPS_SRCCOLOR              0x00000004L
#define D3DPBLENDCAPS_INVSRCCOLOR           0x00000008L
#define D3DPBLENDCAPS_SRCALPHA              0x00000010L
#define D3DPBLENDCAPS_INVSRCALPHA           0x00000020L
#define D3DPBLENDCAPS_DESTALPHA             0x00000040L
#define D3DPBLENDCAPS_INVDESTALPHA          0x00000080L
#define D3DPBLENDCAPS_DESTCOLOR             0x00000100L
#define D3DPBLENDCAPS_INVDESTCOLOR          0x00000200L
#define D3DPBLENDCAPS_SRCALPHASAT           0x00000400L
#define D3DPBLENDCAPS_BOTHSRCALPHA          0x00000800L
#define D3DPBLENDCAPS_BOTHINVSRCALPHA       0x00001000L

#define D3DPCMPCAPS_NEVER               0x00000001L
#define D3DPCMPCAPS_LESS                0x00000002L
#define D3DPCMPCAPS_EQUAL               0x00000004L
#define D3DPCMPCAPS_LESSEQUAL           0x00000008L
#define D3DPCMPCAPS_GREATER             0x00000010L
#define D3DPCMPCAPS_NOTEQUAL            0x00000020L
#define D3DPCMPCAPS_GREATEREQUAL        0x00000040L
#define D3DPCMPCAPS_ALWAYS              0x00000080L

#define D3DPSHADECAPS_COLORFLATMONO         0x00000001L
#define D3DPSHADECAPS_COLORFLATRGB          0x00000002L
#define D3DPSHADECAPS_COLORGOURAUDMONO      0x00000004L
#define D3DPSHADECAPS_COLORGOURAUDRGB       0x00000008L
#define D3DPSHADECAPS_COLORPHONGMONO        0x00000010L
#define D3DPSHADECAPS_COLORPHONGRGB         0x00000020L
#define D3DPSHADECAPS_SPECULARFLATMONO      0x00000040L
#define D3DPSHADECAPS_SPECULARFLATRGB       0x00000080L
#define D3DPSHADECAPS_SPECULARGOURAUDMONO   0x00000100L
#define D3DPSHADECAPS_SPECULARGOURAUDRGB    0x00000200L
#define D3DPSHADECAPS_ALPHAFLATBLEND        0x00000400L
#define D3DPSHADECAPS_ALPHAFLATSTIPPLED     0x00000800L
#define D3DPSHADECAPS_ALPHAGOURAUDBLEND     0x00001000L
#define D3DPSHADECAPS_ALPHAGOURAUDSTIPPLED  0x00002000L
#define D3DPSHADECAPS_ALPHAPHONGBLEND       0x00004000L
#define D3DPSHADECAPS_ALPHAPHONGSTIPPLED    0x00008000L
#define D3DPSHADECAPS_FOGFLAT               0x00010000L
#define D3DPSHADECAPS_FOGGOURAUD            0x00020000L
#define D3DPSHADECAPS_FOGPHONG              0x00040000L

// Render-bit-depth bits (SDK ddraw.h): tested by EnumModesCallback2
#define DDBD_1          0x00004000l
#define DDBD_2          0x00002000l
#define DDBD_4          0x00001000l
#define DDBD_8          0x00000800l
#define DDBD_16         0x00000400l
#define DDBD_24         0x00000200l
#define DDBD_32         0x00000100l

// FVF caps bits (D3DFVFCAPS_*)
#define D3DFVFCAPS_TEXCOORDCOUNTMAX    0x0000ffffL
#define D3DFVFCAPS_DONOTSTRIPELEMENTS  0x00080000L
#define D3DFVFCAPS_PSIZE               0x00100000L

// Vertex-processing caps (dwVertexProcessingCaps)
#define D3DVTXPCAPS_TEXGEN              0x00000001L
#define D3DVTXPCAPS_MATERIALSOURCE7     0x00000002L
#define D3DVTXPCAPS_VERTEXFOG           0x00000004L
#define D3DVTXPCAPS_DIRECTIONALLIGHTS   0x00000008L
#define D3DVTXPCAPS_POSITIONALLIGHTS    0x00000010L
#define D3DVTXPCAPS_LOCALVIEWER         0x00000020L

// State-block capture bits come from d3dtypes.h's D3DSTATEBLOCKTYPE enum
// (D3DSBT_ALL/_PIXELSTATE/_VERTEXSTATE).

// DX7 device IIDs (real SDK values; DEFINE_GUID emits static const per TU,
// and lib3d compares device GUIDs by value)
#ifndef IID_IDirect3D7_DEFINED
#define IID_IDirect3D7_DEFINED
DEFINE_GUID(IID_IDirect3D7,            0xf5049e77,0x4861,0x11d2,0xa4,0x07,0x00,0xa0,0xc9,0x06,0x29,0xa8);
DEFINE_GUID(IID_IDirect3DTnLHalDevice, 0xf5049e78,0x4861,0x11d2,0xa4,0x07,0x00,0xa0,0xc9,0x06,0x29,0xa8);
DEFINE_GUID(IID_IDirect3DHALDevice,    0x84e63de0,0x46aa,0x11cf,0x81,0x6f,0x00,0x00,0xc0,0x20,0x15,0x6e);
DEFINE_GUID(IID_IDirect3DRGBDevice,    0xa4665c60,0xa267,0x11cf,0x81,0x0f,0x00,0x00,0xc0,0x20,0x15,0x6e);
DEFINE_GUID(IID_IDirect3DMMXDevice,    0xc5d93f60,0x46aa,0x11cf,0x81,0x6f,0x00,0x00,0xc0,0x20,0x15,0x6e);
DEFINE_GUID(IID_IDirect3DRampDevice,   0xf2086b20,0x259f,0x11cf,0xa3,0x1a,0x00,0xaa,0x00,0xb9,0x33,0x56);
DEFINE_GUID(IID_IDirect3DRefDevice,    0x50936643,0x13e9,0x11d1,0x89,0xaa,0x00,0xa0,0xc9,0x05,0x41,0x29);
DEFINE_GUID(IID_IDirect3DNullDevice,   0x8767df22,0xb306,0x11d1,0x9b,0x0d,0x00,0xa0,0xc9,0x06,0x29,0xa8);
DEFINE_GUID(IID_IDirectDraw7,          0x15e65ec0,0x3b9c,0x11d2,0xb9,0x2f,0x00,0x60,0x97,0x97,0xea,0x5b);
#endif

// --- Emulated interfaces --------------------------------------------------------
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

// Vertex buffers are real malloc'd stores — Lock/Unlock hand the game
// working memory so VB upload paths execute for real.
struct IDirect3DVertexBuffer7 {
    ULONG  refCnt;
    BYTE*  data;
    DWORD  size;
    DWORD  fvf;
    DWORD  caps;
    bool   locked;

    IDirect3DVertexBuffer7() : refCnt(1),data(0),size(0),fvf(0),caps(0),locked(false) {}
    ~IDirect3DVertexBuffer7() { free(data); }

    // First three virtuals = IUnknown layout (see ddraw.h header note).
    virtual HRESULT QueryInterface(REFIID, LPVOID* ppv)
        { if(!ppv)return DDERR_INVALIDPARAMS; *ppv=this; AddRef(); return D3D_OK; }
    virtual ULONG AddRef() { return ++refCnt; }
    virtual ULONG Release() { if(--refCnt==0){delete this;return 0;} return refCnt; }

    HRESULT Init(LPD3DVERTEXBUFFERDESC d) {
        if(!d) return DDERR_INVALIDPARAMS;
        fvf=d->dwFVF; caps=d->dwCaps;
        static const DWORD szTL=sizeof(D3DTLVERTEX), szV=sizeof(D3DVERTEX);
        DWORD vsz = (fvf & D3DFVF_XYZRHW) ? szTL : szV;
        DWORD nt  = d->dwNumVertices ? d->dwNumVertices : 1;
        size = vsz*nt;
        data=(BYTE*)calloc(1,size);
        return data?D3D_OK:DDERR_OUTOFMEMORY;
    }
    HRESULT Lock(DWORD, LPVOID* ppv, LPDWORD pdwSize) {
        if(!ppv) return DDERR_INVALIDPARAMS;
        locked=true; *ppv=data;
        if(pdwSize) *pdwSize=size;
        return D3D_OK;
    }
    HRESULT Unlock() { locked=false; return D3D_OK; }
    HRESULT GetVertexBufferDesc(LPD3DVERTEXBUFFERDESC d) {
        if(!d) return DDERR_INVALIDPARAMS;
        d->dwSize=sizeof(*d); d->dwCaps=caps; d->dwFVF=fvf;
        d->dwNumVertices=size/((fvf&D3DFVF_XYZRHW)?sizeof(D3DTLVERTEX):sizeof(D3DVERTEX));
        return D3D_OK;
    }
    HRESULT Optimize(LPDIRECT3DDEVICE7,DWORD) { return D3D_OK; }
    HRESULT ProcessVertices(DWORD,DWORD,DWORD,LPDIRECT3DVERTEXBUFFER7,DWORD,LPDIRECT3DDEVICE7,DWORD){return D3D_OK;}
};

// Rasterizer state lives in d3dsoft.h (needs the complete surface type).
namespace Rast7 { struct State; }

// Recorded device state for Begin/End and Create state blocks (d3dsoft.h).
struct RowanSBlock {
    DWORD rs[256]; DWORD ss[8][64]; LPDIRECTDRAWSURFACE7 tex[8];
    BYTE rsM[32], ssM[64]; BYTE texM;   // capture masks (bit per state)
};


// The device records render/texture-stage state and hands back real values
// for everything the game reads; draw calls rasterize for real through the
// Rast7 software pipeline defined in d3dsoft.h.
struct IDirect3DDevice7 {
    ULONG        refCnt;
    DWORD        renderStates[256];
    DWORD        stageStates[8][64];
    LPDIRECTDRAWSURFACE7 textures[8];
    LPDIRECTDRAWSURFACE7 renderTarget;
    D3DMATRIX    transforms[16];
    D3DVIEWPORT7 viewport;
    D3DMATERIAL7 material;
    D3DLIGHT7    lights[8];
    BOOL         lightEnabled[8];
    bool         inScene;
    DWORD        stateBlockCount;
    std::map<DWORD,RowanSBlock*> sblocks;
    bool         sbRec;
    BYTE         recRS[32], recSS[64], recTex;
    Rast7::State* rast;
    // Rast7::State is incomplete here; embed its storage so no destructor is
    // needed (State is POD — d3dsoft.h static_asserts the fit).
    BYTE         rastMem[4096];

    IDirect3DDevice7() : refCnt(1),renderTarget(0),inScene(false),
                         stateBlockCount(0),sbRec(false),rast(0) {
        memset(renderStates,0,sizeof(renderStates));
        memset(stageStates,0,sizeof(stageStates));
        memset(textures,0,sizeof(textures));
        memset(transforms,0,sizeof(transforms));
        memset(&viewport,0,sizeof(viewport));
        memset(&material,0,sizeof(material));
        memset(lights,0,sizeof(lights));
        memset(lightEnabled,0,sizeof(lightEnabled));
        memset(recRS,0,sizeof(recRS));
        memset(recSS,0,sizeof(recSS));
        recTex=0;
        // DX7 render-state defaults (the ones the pipeline consults).
        renderStates[D3DRENDERSTATE_ZFUNC]=D3DCMP_LESSEQUAL;
        renderStates[D3DRENDERSTATE_ZWRITEENABLE]=TRUE;
        renderStates[D3DRENDERSTATE_CULLMODE]=D3DCULL_CCW;
        renderStates[D3DRENDERSTATE_SRCBLEND]=D3DBLEND_ONE;
        renderStates[D3DRENDERSTATE_DESTBLEND]=D3DBLEND_ZERO;
        renderStates[D3DRENDERSTATE_TEXTUREPERSPECTIVE]=TRUE;
        renderStates[D3DRENDERSTATE_SHADEMODE]=D3DSHADE_GOURAUD;
        renderStates[D3DRENDERSTATE_ALPHAFUNC]=D3DCMP_ALWAYS;
        renderStates[D3DRENDERSTATE_TEXTUREFACTOR]=0xffffffff;
        renderStates[D3DRENDERSTATE_FILLMODE]=D3DFILL_SOLID;
        renderStates[D3DRENDERSTATE_DITHERENABLE]=TRUE;
        renderStates[D3DRENDERSTATE_CLIPPING]=TRUE;
        renderStates[D3DRENDERSTATE_LIGHTING]=TRUE;
        // Stage-state defaults: stage0 modulates, others disabled; args and
        // addressing match the DX7 spec.
        for(int i=0;i<8;i++){
            stageStates[i][D3DTSS_COLOROP]= i==0?D3DTOP_MODULATE:D3DTOP_DISABLE;
            stageStates[i][D3DTSS_ALPHAOP]= i==0?D3DTOP_SELECTARG1:D3DTOP_DISABLE;
            stageStates[i][D3DTSS_COLORARG1]=D3DTA_TEXTURE;
            stageStates[i][D3DTSS_COLORARG2]=D3DTA_CURRENT;
            stageStates[i][D3DTSS_ALPHAARG1]=D3DTA_TEXTURE;
            stageStates[i][D3DTSS_ALPHAARG2]=D3DTA_CURRENT;
            stageStates[i][D3DTSS_TEXCOORDINDEX]=i;
            stageStates[i][D3DTSS_ADDRESS]=D3DTADDRESS_WRAP;
            stageStates[i][D3DTSS_ADDRESSU]=D3DTADDRESS_WRAP;
            stageStates[i][D3DTSS_ADDRESSV]=D3DTADDRESS_WRAP;
            stageStates[i][D3DTSS_MAGFILTER]=D3DTFG_POINT;
            stageStates[i][D3DTSS_MINFILTER]=D3DTFG_POINT;
            stageStates[i][D3DTSS_MIPFILTER]=D3DTFP_NONE;
            stageStates[i][D3DTSS_TEXTURETRANSFORMFLAGS]=D3DTTFF_DISABLE;
        }
    }
    // Defined out-of-line in d3dsoft.h (needs the complete
    // IDirectDrawSurface7 from ddraw.h).
    void SyncRast();
   // First three virtuals = IUnknown layout (see ddraw.h header note).
    virtual HRESULT QueryInterface(REFIID, LPVOID* ppv) {
        if(!ppv)return DDERR_INVALIDPARAMS; *ppv=this; AddRef(); return D3D_OK;
    }
    virtual ULONG AddRef() { return ++refCnt; }
    virtual ULONG Release() { if(--refCnt==0){delete this;return 0;} return refCnt; }

    HRESULT SetRenderState(D3DRENDERSTATETYPE s, DWORD v) {
        if((DWORD)s<256) {
            renderStates[s]=v;
            if(sbRec) recRS[s>>3]|=(BYTE)(1<<(s&7));
        }
        return D3D_OK;
    }
    HRESULT GetRenderState(D3DRENDERSTATETYPE s, LPDWORD v)
        { if(v)*v=(DWORD)s<256?renderStates[s]:0; return D3D_OK; }
    HRESULT SetTextureStageState(DWORD s, D3DTEXTURESTAGESTATETYPE t, DWORD v) {
        if(s<8&&(DWORD)t<64) {
            stageStates[s][t]=v;
            if(sbRec) recSS[(s*64+t)>>3]|=(BYTE)(1<<((s*64+t)&7));
        }
        return D3D_OK;
    }
    HRESULT GetTextureStageState(DWORD s, D3DTEXTURESTAGESTATETYPE t, LPDWORD v)
        { if(v)*v=(s<8&&(DWORD)t<64)?stageStates[s][t]:0; return D3D_OK; }
    HRESULT SetTexture(DWORD s, LPDIRECTDRAWSURFACE7 t);   // in d3dsoft.h
    HRESULT GetTexture(DWORD s, LPDIRECTDRAWSURFACE7* t);  // in d3dsoft.h

    HRESULT BeginScene() { inScene=true; SyncRast(); return D3D_OK; }
    HRESULT EndScene()   { inScene=false; return D3D_OK; }
    HRESULT Clear(DWORD nRect,LPD3DRECT rects,DWORD flags,D3DCOLOR col,D3DVALUE z,DWORD);

    HRESULT SetTransform(D3DTRANSFORMSTATETYPE s, LPD3DMATRIX m)
        { if((DWORD)s<16&&m)transforms[s]=*m; return D3D_OK; }
    HRESULT GetTransform(D3DTRANSFORMSTATETYPE s, LPD3DMATRIX m)
        { if(m&&(DWORD)s<16)*m=transforms[s]; return D3D_OK; }
    HRESULT MultiplyTransform(D3DTRANSFORMSTATETYPE,LPD3DMATRIX) { return D3D_OK; }

    HRESULT SetViewport(LPD3DVIEWPORT7 v) { if(v)viewport=*v; return D3D_OK; }
    HRESULT GetViewport(LPD3DVIEWPORT7 v) { if(v)*v=viewport; return D3D_OK; }
    HRESULT SetMaterial(LPD3DMATERIAL7 m) { if(m)material=*m; return D3D_OK; }
    HRESULT GetMaterial(LPD3DMATERIAL7 m) { if(m)*m=material; return D3D_OK; }
    HRESULT SetLight(DWORD i, LPD3DLIGHT7 l)
        { if(i<8&&l)lights[i]=*l; return D3D_OK; }
    HRESULT GetLight(DWORD i, LPD3DLIGHT7 l)
        { if(i<8&&l)*l=lights[i]; return D3D_OK; }
    HRESULT LightEnable(DWORD i, BOOL e)
        { if(i<8)lightEnabled[i]=e; return D3D_OK; }
    HRESULT GetLightEnable(DWORD i, BOOL* e)
        { if(i<8&&e)*e=lightEnabled[i]; return D3D_OK; }

    HRESULT SetRenderTarget(LPDIRECTDRAWSURFACE7 t, DWORD);  // in d3dsoft.h
    HRESULT GetRenderTarget(LPDIRECTDRAWSURFACE7* t);        // in d3dsoft.h

    // State blocks record/restore device state for real (d3dsoft.h).
    HRESULT CreateStateBlock(D3DSTATEBLOCKTYPE, LPDWORD tok);
    HRESULT ApplyStateBlock(DWORD tok);
    HRESULT DeleteStateBlock(DWORD tok);
    HRESULT CaptureStateBlock(DWORD tok);
    HRESULT BeginStateBlock();
    HRESULT EndStateBlock(LPDWORD t);

    HRESULT EnumTextureFormats(LPD3DENUMPIXELFORMATSCALLBACK cb, LPVOID ctx);
    HRESULT ValidateDevice(LPDWORD passes) { if(passes)*passes=1; return D3D_OK; }
    HRESULT GetCaps(LPD3DDEVICEDESC7 d);

    // --- rasterized draw entry points (defined in d3dsoft.h) -------------
    HRESULT DrawPrimitive(D3DPRIMITIVETYPE pt,DWORD fvf,LPVOID verts,DWORD n,DWORD);
    HRESULT DrawIndexedPrimitive(D3DPRIMITIVETYPE pt,DWORD fvf,LPVOID verts,DWORD nv,
                                 LPWORD idx,DWORD ni,DWORD);
    HRESULT DrawPrimitiveVB(D3DPRIMITIVETYPE pt,LPDIRECT3DVERTEXBUFFER7 vb,
                            DWORD start,DWORD n,DWORD);
    HRESULT DrawIndexedPrimitiveVB(D3DPRIMITIVETYPE pt,LPDIRECT3DVERTEXBUFFER7 vb,
                                   DWORD start,DWORD nv,LPWORD idx,DWORD ni,DWORD);
    HRESULT DrawPrimitiveStrided(D3DPRIMITIVETYPE,DWORD,void*,DWORD,DWORD){return D3D_OK;}
    HRESULT DrawIndexedPrimitiveStrided(D3DPRIMITIVETYPE,DWORD,void*,DWORD,LPWORD,DWORD,DWORD){return D3D_OK;}
    HRESULT ComputeSphereVisibility(LPD3DVECTOR,LPD3DVALUE,DWORD,DWORD,LPDWORD){return D3D_OK;}
    HRESULT PreLoad(LPDIRECTDRAWSURFACE7){return D3D_OK;}
    HRESULT Load(LPDIRECTDRAWSURFACE7,LPPOINT,LPDIRECTDRAWSURFACE7,LPRECT,DWORD){return D3D_OK;}
    HRESULT GetInfo(DWORD,LPVOID,DWORD){return D3D_OK;}
    HRESULT SetClipStatus(LPD3DCLIPSTATUS){return D3D_OK;}
    HRESULT GetClipStatus(LPD3DCLIPSTATUS c){if(c)memset(c,0,sizeof(*c));return D3D_OK;}
    HRESULT SetClipPlane(DWORD,D3DVALUE*){return D3D_OK;}
    HRESULT GetClipPlane(DWORD,D3DVALUE*){return D3D_OK;}
    HRESULT ApplyStateBlockEx(DWORD,DWORD){return D3D_OK;}
};

// Standard texture formats the native device reports (16bpp + 32bpp RGB).
inline void FillD3DFormat(DDPIXELFORMAT* f, DWORD bits, DWORD rM, DWORD gM, DWORD bM, DWORD aM) {
    memset(f,0,sizeof(*f));
    f->dwSize=sizeof(*f); f->dwFlags=DDPF_RGB|(aM?DDPF_ALPHAPIXELS:0);
    f->dwRGBBitCount=bits; f->dwRBitMask=rM; f->dwGBitMask=gM;
    f->dwBBitMask=bM; f->dwRGBAlphaBitMask=aM;
}
inline HRESULT IDirect3DDevice7::EnumTextureFormats(LPD3DENUMPIXELFORMATSCALLBACK cb, LPVOID ctx) {
    if(!cb) return DDERR_INVALIDPARAMS;
    DDPIXELFORMAT f;
    FillD3DFormat(&f,16,0xf800,0x07e0,0x001f,0x0000);      if(cb(&f,ctx)!=D3DENUMRET_OK)return D3D_OK;
    FillD3DFormat(&f,16,0x7c00,0x03e0,0x001f,0x8000);      if(cb(&f,ctx)!=D3DENUMRET_OK)return D3D_OK;
    FillD3DFormat(&f,16,0x0f00,0x00f0,0x000f,0xf000);      if(cb(&f,ctx)!=D3DENUMRET_OK)return D3D_OK;
    FillD3DFormat(&f,32,0x00ff0000,0x0000ff00,0x000000ff,0xff000000); cb(&f,ctx);
    return D3D_OK;
}

inline HRESULT IDirect3DDevice7::GetCaps(LPD3DDEVICEDESC7 d) {
    if(!d) return DDERR_INVALIDPARAMS;
    memset(d,0,sizeof(*d));
    d->dwDevCaps=D3DDEVCAPS_HWTRANSFORMANDLIGHT|D3DDEVCAPS_HWRASTERIZATION|
                 D3DDEVCAPS_TEXTUREVIDEOMEMORY|D3DDEVCAPS_DRAWPRIMITIVES2;
    D3DPRIMCAPS* pc=&d->dpcTriCaps;
    pc->dwSize=sizeof(*pc);
    pc->dwMiscCaps=D3DPCMISCCAPS_CULLNONE|D3DPCMISCCAPS_CULLCW|D3DPCMISCCAPS_CULLCCW;
    pc->dwRasterCaps=D3DPRASTERCAPS_DITHER|D3DPRASTERCAPS_ZTEST|
                     D3DPRASTERCAPS_FOGVERTEX|D3DPRASTERCAPS_FOGTABLE|
                     D3DPRASTERCAPS_ANTIALIASEDGES|D3DPRASTERCAPS_WBUFFER|
                     D3DPRASTERCAPS_WFOG|D3DPRASTERCAPS_ZFOG|
                     D3DPRASTERCAPS_COLORPERSPECTIVE|D3DPRASTERCAPS_MIPMAPLODBIAS;
    pc->dwZCmpCaps=0xff;
    pc->dwSrcBlendCaps=pc->dwDestBlendCaps=D3DPBLENDCAPS_ZERO|D3DPBLENDCAPS_ONE|
        D3DPBLENDCAPS_SRCCOLOR|D3DPBLENDCAPS_INVSRCCOLOR|D3DPBLENDCAPS_SRCALPHA|
        D3DPBLENDCAPS_INVSRCALPHA|D3DPBLENDCAPS_DESTALPHA|D3DPBLENDCAPS_INVDESTALPHA|
        D3DPBLENDCAPS_DESTCOLOR|D3DPBLENDCAPS_INVDESTCOLOR|D3DPBLENDCAPS_SRCALPHASAT;
    pc->dwAlphaCmpCaps=0xff;
    pc->dwShadeCaps=D3DPSHADECAPS_COLORGOURAUDRGB|D3DPSHADECAPS_SPECULARGOURAUDRGB|
                    D3DPSHADECAPS_ALPHAGOURAUDBLEND|D3DPSHADECAPS_FOGGOURAUD;
    pc->dwTextureCaps=D3DPTEXTURECAPS_PERSPECTIVE|D3DPTEXTURECAPS_ALPHA|
                     D3DPTEXTURECAPS_TRANSPARENCY|D3DPTEXTURECAPS_BORDER;
    pc->dwTextureFilterCaps=0x00003f3f;
    pc->dwTextureBlendCaps=D3DPTBLENDCAPS_DECAL|D3DPTBLENDCAPS_MODULATE|
                          D3DPTBLENDCAPS_DECALALPHA|D3DPTBLENDCAPS_MODULATEALPHA|
                          D3DPTBLENDCAPS_COPY|D3DPTBLENDCAPS_ADD;
    pc->dwTextureAddressCaps=D3DPTADDRESSCAPS_WRAP|D3DPTADDRESSCAPS_MIRROR|
                             D3DPTADDRESSCAPS_CLAMP|D3DPTADDRESSCAPS_BORDER;
    d->dpcLineCaps=d->dpcTriCaps;
    d->dwDeviceRenderBitDepth=DDBD_16|DDBD_24|DDBD_32;
    d->dwDeviceZBufferBitDepth=DDBD_16|DDBD_24|DDBD_32;
    d->dwMinTextureWidth=1; d->dwMinTextureHeight=1;
    d->dwMaxTextureWidth=4096; d->dwMaxTextureHeight=4096;
    d->dwMinTextureRepeat=1; d->dwMaxTextureRepeat=4096;
    d->dwMaxTextureAspectRatio=4096; d->dwMaxAnisotropy=16;
    d->dvGuardBandLeft=-1000; d->dvGuardBandTop=-1000;
    d->dvGuardBandRight=1000; d->dvGuardBandBottom=1000;
    d->dvExtentsAdjust=0;
    d->dwFVFCaps=D3DFVFCAPS_TEXCOORDCOUNTMAX|D3DFVFCAPS_PSIZE;
    d->dwTextureOpCaps=0xffffffff;
    d->wMaxTextureBlendStages=8; d->wMaxSimultaneousTextures=8;
    d->dwMaxActiveLights=8; d->dvMaxVertexW=1e9f;
    d->deviceGUID=IID_IDirect3DTnLHalDevice;
    d->wMaxUserClipPlanes=6; d->wMaxVertexBlendMatrices=4;
    d->dwVertexProcessingCaps=D3DVTXPCAPS_TEXGEN|D3DVTXPCAPS_MATERIALSOURCE7|
                              D3DVTXPCAPS_VERTEXFOG|D3DVTXPCAPS_DIRECTIONALLIGHTS|
                              D3DVTXPCAPS_POSITIONALLIGHTS|D3DVTXPCAPS_LOCALVIEWER;
    return D3D_OK;
}

struct IDirect3D7 {
    ULONG refCnt;
    IDirect3D7() : refCnt(1) {}
    // First three virtuals = IUnknown layout (see ddraw.h header note).
    virtual HRESULT QueryInterface(REFIID, LPVOID* ppv)
        { if(!ppv)return DDERR_INVALIDPARAMS; *ppv=this; AddRef(); return D3D_OK; }
    virtual ULONG AddRef() { return ++refCnt; }
    virtual ULONG Release() { if(--refCnt==0){delete this;return 0;} return refCnt; }

    // Yields HAL, then TnL-HAL, then RGB — real DX7 enumeration order.
    // BoB's lib3d is compiled with NO_HARD_TNL so it selects the plain
    // IID_IDirect3DHALDevice entry; the TnL offer stays for call sites
    // (and any future consumer) built without that define.
    HRESULT EnumDevices(LPD3DENUMDEVICESCALLBACK7 cb, LPVOID ctx) {
        if(!cb) return DDERR_INVALIDPARAMS;
        IDirect3DDevice7 probe; D3DDEVICEDESC7 desc;
        probe.GetCaps(&desc); desc.deviceGUID=IID_IDirect3DHALDevice;
        if(cb((LPSTR)"Direct3D HAL",(LPSTR)"Rowan Native HAL",&desc,ctx)!=D3DENUMRET_OK)
            return D3D_OK;
        desc.deviceGUID=IID_IDirect3DTnLHalDevice;
        if(cb((LPSTR)"Direct3D T&L HAL",(LPSTR)"Rowan Native TnL",&desc,ctx)!=D3DENUMRET_OK)
            return D3D_OK;
        desc.deviceGUID=IID_IDirect3DRGBDevice;
        cb((LPSTR)"RGB Emulation",(LPSTR)"Rowan Native RGB",&desc,ctx);
        return D3D_OK;
    }
    HRESULT EnumZBufferFormats(REFCLSID, LPD3DENUMPIXELFORMATSCALLBACK cb, LPVOID ctx) {
        if(!cb) return DDERR_INVALIDPARAMS;
        DDPIXELFORMAT f;
        memset(&f,0,sizeof(f));
        f.dwSize=sizeof(f); f.dwFlags=DDPF_ZBUFFER;
        f.dwZBufferBitDepth=16; if(cb(&f,ctx)!=D3DENUMRET_OK) return D3D_OK;
        f.dwZBufferBitDepth=24; if(cb(&f,ctx)!=D3DENUMRET_OK) return D3D_OK;
        f.dwZBufferBitDepth=32; cb(&f,ctx);
        return D3D_OK;
    }
    // DX7: the device renders to the surface handed to CreateDevice;
    // SetRenderTarget is only needed for later render-target switches.
    // (BoB fullscreen lib3d never calls SetRenderTarget — T123 gray frame.)
    HRESULT CreateDevice(REFCLSID, LPDIRECTDRAWSURFACE7 surf, LPDIRECT3DDEVICE7* out);
    HRESULT CreateVertexBuffer(LPD3DVERTEXBUFFERDESC d, LPDIRECT3DVERTEXBUFFER7* out, DWORD) {
        if(!d||!out) return DDERR_INVALIDPARAMS;
        IDirect3DVertexBuffer7* vb=new IDirect3DVertexBuffer7;
        HRESULT hr=vb->Init(d);
        if(hr!=D3D_OK){vb->Release();return hr;}
        *out=vb; return D3D_OK;
    }
    HRESULT EvictManagedTextures() { return D3D_OK; }
};

#pragma pack(pop)
#endif // ROWAN_COMPAT_D3D_H
