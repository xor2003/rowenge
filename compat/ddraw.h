// MSVC <ddraw.h> shim — BoB's lib3d.h speaks DirectDraw 7 (DDSURFACEDESC2),
// so the DX7 type/constant universe lives in d3dtypes.h (NOT migalley's
// ddraw_stub.h, whose DX1 _DDSURFACEDESC names conflict with lib3d).
//
// Interface objects are native emulations in the same style as migalley's
// stubs: plain C++ structs the game calls directly.  Surfaces are real
// malloc'd framebuffers (Lock/Unlock/Blt actually work — the frontend and
// texture-upload paths get functional memory surfaces); device/3D calls
// record state and report success so driver/mode enumeration is live.
#ifndef ROWAN_COMPAT_DDRAW_H
#define ROWAN_COMPAT_DDRAW_H
#include "d3dtypes.h"
#include "d3d.h"
#define ROWAN_DDRAW_STUB 1

#include <string.h>
#include <stdlib.h>

// --- Emulated interface objects --------------------------------------------------
// COM ABI: QueryInterface/AddRef/Release are the first three virtual
// members (in that order) in every interface below.  GCC's i386 vtable
// then lines up 1:1 with IUnknownVtbl, so lib3d's IUnknown* paths
// (getRefCount, DeRefAndNULL, IUnknownPtrRef) reach the real refcount
// code through the vptr — exactly like a true COM lpVtbl call.
struct IDirectDrawPalette {
    PALETTEENTRY entries[256];
    ULONG refCnt;
    IDirectDrawPalette() : refCnt(1) {}
    virtual HRESULT QueryInterface(REFIID, LPVOID* ppv)
        { if(!ppv)return DDERR_INVALIDPARAMS; *ppv=this; AddRef(); return DD_OK; }
    virtual ULONG AddRef() { return ++refCnt; }
    virtual ULONG Release() { if (--refCnt==0){delete this;return 0;} return refCnt; }
    HRESULT GetEntries(DWORD f,DWORD n,DWORD c,PALETTEENTRY* e)
        { if(!e) return DDERR_INVALIDPARAMS; memcpy(e,entries+f,c*sizeof(PALETTEENTRY)); return DD_OK; }
    HRESULT SetEntries(DWORD f,DWORD n,DWORD c,PALETTEENTRY* e)
        { if(!e) return DDERR_INVALIDPARAMS; memcpy(entries+f,e,c*sizeof(PALETTEENTRY)); return DD_OK; }
};

struct IDirectDrawClipper {
    ULONG refCnt;
    IDirectDrawClipper() : refCnt(1) {}
    virtual HRESULT QueryInterface(REFIID, LPVOID* ppv)
        { if(!ppv)return DDERR_INVALIDPARAMS; *ppv=this; AddRef(); return DD_OK; }
    virtual ULONG AddRef() { return ++refCnt; }
    virtual ULONG Release() { if (--refCnt==0){delete this;return 0;} return refCnt; }
    HRESULT SetHWnd(DWORD, HWND) { return DD_OK; }
    HRESULT SetClipList(void*,DWORD) { return DD_OK; }
};

struct IDirectDrawGammaControl {
    ULONG refCnt;
    DDGAMMARAMP ramp;
    IDirectDrawGammaControl() : refCnt(1) { memset(&ramp,0,sizeof(ramp)); }
    virtual HRESULT QueryInterface(REFIID, LPVOID* ppv)
        { if(!ppv)return DDERR_INVALIDPARAMS; *ppv=this; AddRef(); return DD_OK; }
    virtual ULONG AddRef() { return ++refCnt; }
    virtual ULONG Release() { if (--refCnt==0){delete this;return 0;} return refCnt; }
    HRESULT GetGammaRamp(DWORD, LPVOID r)
        { if(r)memcpy(r,&ramp,sizeof(ramp)); return DD_OK; }
    HRESULT SetGammaRamp(DWORD, LPVOID r)
        { if(r)memcpy(&ramp,r,sizeof(ramp)); return DD_OK; }
};

struct IDirectDrawSurface7 {
    DDSURFACEDESC2      desc;
    IDirectDrawPalette* pal;
    IDirectDrawClipper* clip;
    IDirectDrawSurface7* attached;
    IDirectDrawSurface7* attachedFrom;
    ULONG   refCnt;
    BYTE*   pixels;

    IDirectDrawSurface7() : pal(0),clip(0),attached(0),attachedFrom(0),refCnt(1),pixels(0)
        { memset(&desc,0,sizeof(desc)); desc.dwSize=sizeof(desc); }
    ~IDirectDrawSurface7() { free(pixels); if(pal) pal->Release(); if(clip) clip->Release();
                             if(attached){attached->attachedFrom=0;attached->Release();} }

    static DWORD PitchFor(DWORD w, DWORD bpp) { return ((w*bpp/8)+3)&~3; }

    HRESULT Alloc(DDSURFACEDESC2* d) {
        desc=*d; desc.dwSize=sizeof(desc);
        DWORD bpp = desc.ddpfPixelFormat.dwRGBBitCount;
        if (!bpp) bpp = desc.ddpfPixelFormat.dwZBufferBitDepth;
        if (!bpp) bpp = 16;
        if (!desc.lPitch) desc.lPitch = PitchFor(desc.dwWidth?desc.dwWidth:1,bpp);
        if (!desc.lpSurface) {
            pixels=(BYTE*)calloc(1, (size_t)desc.lPitch*(desc.dwHeight?desc.dwHeight:1)+16);
            if(!pixels) return DDERR_OUTOFMEMORY;
            desc.lpSurface = pixels;
        }
        return DD_OK;
    }

    virtual HRESULT QueryInterface(REFIID riid, LPVOID* ppv) {
        if(!ppv)return DDERR_INVALIDPARAMS;
        if(riid==IID_IDirectDrawGammaControl){*ppv=new IDirectDrawGammaControl;return DD_OK;}
        *ppv=this; AddRef(); return DD_OK;
    }
    virtual ULONG AddRef() { return ++refCnt; }
    virtual ULONG Release() { if(--refCnt==0){delete this;return 0;} return refCnt; }

    HRESULT Lock(LPRECT, LPDDSURFACEDESC2 lpDdsd, DWORD, HANDLE) {
        if(!lpDdsd) return DDERR_INVALIDPARAMS;
        *lpDdsd = desc; lpDdsd->dwSize = sizeof(*lpDdsd);
        return DD_OK;
    }
    HRESULT Unlock(LPRECT) { return DD_OK; }

    HRESULT GetSurfaceDesc(LPDDSURFACEDESC2 lpDdsd) {
        if(!lpDdsd) return DDERR_INVALIDPARAMS;
        *lpDdsd = desc; lpDdsd->dwSize = sizeof(*lpDdsd);
        return DD_OK;
    }

    HRESULT AddAttachedSurface(LPDIRECTDRAWSURFACE7 a) {
        if(!a) return DDERR_INVALIDPARAMS;
        if(attached) return DDERR_CANNOTATTACHSURFACE;
        a->AddRef(); attached=a; a->attachedFrom=this;
        return DD_OK;
    }
    HRESULT GetAttachedSurface(LPDDSCAPS2, LPDIRECTDRAWSURFACE7* out) {
        if(!out) return DDERR_INVALIDPARAMS;
        *out = attached;
        if(attached) attached->AddRef(); else return DDERR_GENERIC;
        return DD_OK;
    }
    HRESULT DeleteAttachedSurface(DWORD, LPDIRECTDRAWSURFACE7 a) {
        if(attached==a){attached->Release();attached=0;a->attachedFrom=0;}
        return DD_OK;
    }

    static void BlitRect(BYTE*dst,DWORD dpitch,const BYTE*src,DWORD spitch,DWORD w,DWORD h){
        for(DWORD y=0;y<h;y++) memcpy(dst+y*dpitch, src+y*spitch, w);
    }
    HRESULT Blt(LPRECT dst, LPDIRECTDRAWSURFACE7 src, LPRECT srcr, DWORD flags, LPDDBLTFX fx) {
        DWORD bpp=desc.ddpfPixelFormat.dwRGBBitCount/8; if(!bpp)bpp=2;
        if((flags & DDBLT_COLORFILL) && fx){
            BYTE*p=(BYTE*)desc.lpSurface;
            for(DWORD y=0;y<desc.dwHeight;y++)
                for(DWORD x=0;x<desc.dwWidth*bpp;x+=bpp)
                    memcpy(p+y*desc.lPitch+x,&fx->dwFillColor,bpp<4?bpp:4);
            return DD_OK;
        }
        if(!src) return DD_OK;
        DWORD w=src->desc.dwWidth*bpp, h=src->desc.dwHeight;
        if(w>desc.dwWidth*bpp) w=desc.dwWidth*bpp;
        if(h>desc.dwHeight) h=desc.dwHeight;
        LONG dx=dst?dst->left:0, dy=dst?dst->top:0;
        LONG sx=srcr?srcr->left:0, sy=srcr?srcr->top:0;
        BlitRect((BYTE*)desc.lpSurface+dy*desc.lPitch+dx*bpp,desc.lPitch,
                 (BYTE*)src->desc.lpSurface+sy*src->desc.lPitch+sx*bpp,src->desc.lPitch,w,h);
        return DD_OK;
    }
    HRESULT BltFast(DWORD x,DWORD y,LPDIRECTDRAWSURFACE7 src,LPRECT,DWORD){
        if(!src) return DD_OK;
        DWORD bpp=desc.ddpfPixelFormat.dwRGBBitCount/8; if(!bpp)bpp=2;
        DWORD w=src->desc.dwWidth*bpp,h=src->desc.dwHeight;
        BlitRect((BYTE*)desc.lpSurface+y*desc.lPitch+x*bpp,desc.lPitch,
                 (BYTE*)src->desc.lpSurface,src->desc.lPitch,w,h);
        return DD_OK;
    }
    HRESULT Flip(LPDIRECTDRAWSURFACE7, DWORD) { return DD_OK; }

    HRESULT SetPalette(LPDIRECTDRAWPALETTE p) {
        if(p) p->AddRef(); if(pal) pal->Release(); pal=p; return DD_OK;
    }
    HRESULT SetClipper(LPDIRECTDRAWCLIPPER c) {
        if(c) c->AddRef(); if(clip) clip->Release(); clip=c; return DD_OK;
    }
    HRESULT GetDC(HDC*) { return DDERR_UNSUPPORTED; }   // no GDI on 3D surfaces
    HRESULT ReleaseDC(HDC) { return DD_OK; }
    HRESULT PageLock(DWORD) { return DD_OK; }
    HRESULT PageUnlock(DWORD) { return DD_OK; }
    HRESULT IsLost() { return DD_OK; }
    HRESULT Restore() { return DD_OK; }
    HRESULT SetColorKey(DWORD, DDCOLORKEY*) { return DD_OK; }
};

// Definitions for the refcount helpers d3d.h's device declared while
// IDirectDrawSurface7 was still incomplete.
inline ULONG RowanSurf7AddRef(LPDIRECTDRAWSURFACE7 s) { return s?s->AddRef():0; }
inline ULONG RowanSurf7Release(LPDIRECTDRAWSURFACE7 s) { return s?s->Release():0; }

struct IDirectDraw7 {
    ULONG refCnt;
    IDirectDraw7() : refCnt(1) {}
    virtual HRESULT QueryInterface(REFIID riid, LPVOID* ppv) {
        if(!ppv) return DDERR_INVALIDPARAMS;
        if(riid==IID_IDirect3D7){ *ppv=new IDirect3D7; return DD_OK; }
        *ppv=this; AddRef(); return DD_OK;
    }
    virtual ULONG AddRef() { return ++refCnt; }
    virtual ULONG Release() { if(--refCnt==0){delete this;return 0;} return refCnt; }
    HRESULT SetCooperativeLevel(HWND,DWORD) { return DD_OK; }
    HRESULT SetDisplayMode(DWORD,DWORD,DWORD,DWORD,DWORD) { return DD_OK; }
    HRESULT RestoreDisplayMode() { return DD_OK; }
    // Static mode list — the resolutions the game was authored for.
    HRESULT EnumDisplayModes(DWORD, LPDDSURFACEDESC2, LPVOID ctx, LPDDENUMMODESCALLBACK2 cb) {
        if(!cb) return DDERR_INVALIDPARAMS;
        static const DWORD modes[][3] = {
            {640,480,16},{800,600,16},{1024,768,16},{1280,1024,16},{1600,1200,16},
            {640,480,32},{800,600,32},{1024,768,32},{1280,1024,32},{1600,1200,32},
        };
        for(unsigned i=0;i<sizeof(modes)/sizeof(modes[0]);i++){
            DDSURFACEDESC2 d;
            memset(&d,0,sizeof(d)); d.dwSize=sizeof(d);
            d.dwFlags=DDSD_WIDTH|DDSD_HEIGHT|DDSD_PIXELFORMAT;
            d.dwWidth=modes[i][0]; d.dwHeight=modes[i][1];
            d.ddpfPixelFormat.dwSize=sizeof(DDPIXELFORMAT);
            d.ddpfPixelFormat.dwFlags=DDPF_RGB;
            d.ddpfPixelFormat.dwRGBBitCount=modes[i][2];
            if(cb(&d,ctx)!=DDENUMRET_OK) break;
        }
        return DD_OK;
    }
    HRESULT CreateSurface(LPDDSURFACEDESC2 d, LPDIRECTDRAWSURFACE7* out, IUnknown*) {
        if(!d||!out) return DDERR_INVALIDPARAMS;
        IDirectDrawSurface7* s=new IDirectDrawSurface7;
        HRESULT hr=s->Alloc(d);
        if(hr!=DD_OK){s->Release();return hr;}
        *out=s;
        if(d->ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE) s->desc.ddsCaps.dwCaps|=DDSCAPS_FRONTBUFFER;
        // COMPLEX|FLIP with DDSD_BACKBUFFERCOUNT: DD attaches that many
        // back buffers atomically — lib3d then finds them via
        // GetAttachedSurface(DDSCAPS_BACKBUFFER).
        if((d->dwFlags&DDSD_BACKBUFFERCOUNT) && d->dwBackBufferCount>0) {
            DDSURFACEDESC2 bd=*d;
            bd.dwFlags&=~DDSD_BACKBUFFERCOUNT;
            bd.ddsCaps.dwCaps&=~(DDSCAPS_PRIMARYSURFACE|DDSCAPS_COMPLEX|DDSCAPS_FLIP);
            bd.ddsCaps.dwCaps|=DDSCAPS_BACKBUFFER;
            IDirectDrawSurface7* b=new IDirectDrawSurface7;
            hr=b->Alloc(&bd);
            if(hr!=DD_OK){b->Release();s->Release();*out=0;return hr;}
            s->attached=b; b->attachedFrom=s;   // attachment holds b's ref
        }
        return DD_OK;
    }
    HRESULT GetCaps(LPDDCAPS hal, LPDDCAPS) {
        if(hal){ memset(hal,0,sizeof(*hal)); hal->dwSize=sizeof(*hal);
                 hal->dwCaps=DDCAPS_3D|DDCAPS_BLT|DDCAPS_GDI;
                 hal->dwVidMemTotal=64*1024*1024; hal->dwVidMemFree=48*1024*1024; }
        return DD_OK;
    }
    HRESULT GetAvailableVidMem(LPDDSCAPS2, LPDWORD total, LPDWORD free_) {
        if(total)*total=64*1024*1024; if(free_)*free_=48*1024*1024;
        return DD_OK;
    }
    HRESULT GetDeviceIdentifier(LPDDDEVICEIDENTIFIER2 id, DWORD) {
        if(!id) return DDERR_INVALIDPARAMS;
        memset(id,0,sizeof(*id));
        strcpy(id->szDriver,"rowan-native");
        strcpy(id->szDescription,"Rowan Native (SDL/Vulkan backend)");
        id->dwWHQLLevel=1;
        return DD_OK;
    }
    HRESULT CreatePalette(DWORD, PALETTEENTRY* e, LPDIRECTDRAWPALETTE* out, IUnknown*) {
        if(!out) return DDERR_INVALIDPARAMS;
        IDirectDrawPalette* p=new IDirectDrawPalette;
        if(e) memcpy(p->entries,e,256*sizeof(PALETTEENTRY));
        *out=p; return DD_OK;
    }
    HRESULT CreateClipper(DWORD, LPDIRECTDRAWCLIPPER* out, IUnknown*) {
        if(!out) return DDERR_INVALIDPARAMS;
        *out=new IDirectDrawClipper; return DD_OK;
    }
    HRESULT EvictManagedTextures() { return DD_OK; }
    HRESULT GetDisplayMode(LPDDSURFACEDESC2 d) {
        if(!d) return DDERR_INVALIDPARAMS;
        memset(d,0,sizeof(*d)); d->dwSize=sizeof(*d);
        d->dwWidth=640; d->dwHeight=480;
        d->ddpfPixelFormat.dwSize=sizeof(DDPIXELFORMAT);
        d->ddpfPixelFormat.dwFlags=DDPF_RGB; d->ddpfPixelFormat.dwRGBBitCount=16;
        return DD_OK;
    }
    HRESULT GetFourCCCodes(LPDWORD n, LPDWORD c) { if(n)*n=0; return DD_OK; }
    HRESULT GetMonitorFrequency(LPDWORD f) { if(f)*f=60; return DD_OK; }
    HRESULT GetScanLine(LPDWORD l) { if(l)*l=0; return DD_OK; }
    HRESULT GetVerticalBlankStatus(LPBOOL b) { if(b)*b=TRUE; return DD_OK; }
    HRESULT WaitForVerticalBlank(DWORD,HANDLE) { return DD_OK; }
    HRESULT FlipToGDISurface() { return DD_OK; }
    HRESULT GetGammaRamp(DWORD,DDGAMMARAMP*) { return DD_OK; }
    HRESULT SetGammaRamp(DWORD,DDGAMMARAMP*) { return DD_OK; }
    HRESULT DuplicateSurface(LPDIRECTDRAWSURFACE7 s, LPDIRECTDRAWSURFACE7* out) {
        if(!s||!out) return DDERR_INVALIDPARAMS;
        s->AddRef(); *out=s; return DD_OK;
    }
    HRESULT EnumSurfaces(DWORD,LPDDSURFACEDESC2,LPVOID,LPDDENUMSURFACESCALLBACK7){return DD_OK;}
};

// One native "driver" is enumerated: the Rowan backend.  The callback
// contract is exercised for real so lib3d's mode enumeration works.  The
// GUID handed out is a persistent static — lib3d stores the pointer and
// passes it back to DirectDrawCreateEx.
inline HRESULT DirectDrawEnumerateEx(LPDDENUMCALLBACKEX cb, LPVOID ctx, DWORD) {
    if(!cb) return DDERR_INVALIDPARAMS;
    return cb((GUID*)&IID_IDirectDraw7,(LPSTR)"Display",(LPSTR)"Rowan Native",ctx,NULL)
           ? DD_OK : DDERR_GENERIC;
}
inline HRESULT DirectDrawEnumerateExA(LPDDENUMCALLBACKEX cb, LPVOID ctx, DWORD f) {
    return DirectDrawEnumerateEx(cb,ctx,f);
}
inline HRESULT DirectDrawCreateEx(GUID*, LPVOID* ppv, REFIID, IUnknown*) {
    if(!ppv) return DDERR_INVALIDPARAMS;
    *ppv=new IDirectDraw7; return DD_OK;
}
#endif // ROWAN_COMPAT_DDRAW_H
