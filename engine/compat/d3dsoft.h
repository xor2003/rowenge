// Rowan software rasterizer (T123) — included at the bottom of ddraw.h
// once IDirectDrawSurface7 is complete.  BoB's lib3d is built NO_HARD_TNL:
// it transforms/clips/lights itself and feeds pre-lit screen-space vertices
// (D3DTLVERTEX family) via DrawPrimitiveVB/DrawIndexedPrimitiveVB
// (TRIANGLEFAN, LINELIST, POINTLIST) plus immediate DrawPrimitive calls.
// The device therefore only needs scan conversion + the DX7 texture-stage /
// blend / z-buffer pipeline — not transform or lighting.
//
// Everything runs on the malloc'd surface memory so results present through
// the ddraw.h Flip/Blt path.
#ifndef ROWAN_COMPAT_D3DSOFT_H
#define ROWAN_COMPAT_D3DSOFT_H

// BoB TUs sit under a global pack(1) (dosdefs.h); the port-layer
// COM classes/DD structs must keep natural alignment so every TU
// agrees on member offsets (ODR) — T123 BoBFrag OOB via CREdtBt.
#pragma pack(push,8)

#ifndef DDCKEY_SRCBLT
#define DDCKEY_SRCBLT   0x00000008   // ddraw.h color-key flag (source blt key)
#endif

#include <time.h>
#ifdef __SSE2__
#include <emmintrin.h>
#endif                            // clock() for ROWAN_PERF counters

// Vulkan backend entry points — defined in d3dvk.h (included after this
// header by ddraw.h); the software path calls them so a compiled-in backend
// can take draws, and they collapse to no-ops/false when it isn't.
struct IDirectDrawSurface7;
namespace Rast7 { struct State; struct Vert; }
bool RowanVkDraw(const Rast7::State&,D3DPRIMITIVETYPE,const Rast7::Vert*,DWORD);
bool RowanVkClear(IDirectDrawSurface7*,IDirectDrawSurface7*,int,DWORD,
                  LPD3DRECT,DWORD,D3DCOLOR,float);
void RowanVkReadSync(IDirectDrawSurface7*);
void RowanVkWriteSync(IDirectDrawSurface7*);
void RowanVkEndFrame();

namespace Rast7 {

struct PixFmt {                      // unpack/pack a surface pixel via masks
    DWORD bits, rM, gM, bM, aM;
    int   rS, gS, bS, aS, rW, gW, bW, aW;   // shift + field width
};
inline int lowbit(DWORD m){int s=0;
if(m)while(!(m&1)){m>>=1;s++;}
return s;}
inline int nbits (DWORD m){int n=0;while(m){n+=m&1;m>>=1;}return n;}
inline void DecodePF(const DDPIXELFORMAT& p, PixFmt& f){
    f.rM=p.dwRBitMask; f.gM=p.dwGBitMask; f.bM=p.dwBBitMask; f.aM=p.dwRGBAlphaBitMask;
    f.bits=p.dwRGBBitCount?p.dwRGBBitCount:32;
    f.rS=lowbit(f.rM); f.gS=lowbit(f.gM); f.bS=lowbit(f.bM); f.aS=lowbit(f.aM);
    f.rW=nbits(f.rM);  f.gW=nbits(f.gM);  f.bW=nbits(f.bM);  f.aW=nbits(f.aM);
}
// Force-inline the whole per-pixel path: on i386 GCC declines to inline
// these (size heuristics), leaving 6-10 nested calls + state reloads per
// pixel, which is the dominant render cost.
#if defined(__GNUC__)
#define R7INLINE inline __attribute__((always_inline))
#else
#define R7INLINE inline
#endif
R7INLINE DWORD scaleUp(DWORD v,int w){ if(!w)return 255; v&=((1u<<w)-1);
    return w>=8 ? (v>>(w-8)) : (v*255)/((1u<<w)-1); }
R7INLINE DWORD scaleDn(DWORD v,int w){ if(!w)return 0;
    return w>=8 ? (v<<(w-8)) : (v*((1u<<w)-1)+127)/255; }
R7INLINE DWORD Unpack(DWORD px, const PixFmt& f){ // -> 0xAARRGGBB
    return (scaleUp(px&f.rM? (px&f.rM)>>f.rS:0, f.rW)<<16)
         | (scaleUp(px&f.gM? (px&f.gM)>>f.gS:0, f.gW)<<8)
         | (scaleUp(px&f.bM? (px&f.bM)>>f.bS:0, f.bW))
         | ((f.aM? scaleUp((px&f.aM)>>f.aS,f.aW):255)<<24);
}
R7INLINE DWORD Pack(DWORD c, const PixFmt& f){
    return (scaleDn((c>>16)&255,f.rW)<<f.rS)|(scaleDn((c>>8)&255,f.gW)<<f.gS)
         | (scaleDn(c&255,f.bW)<<f.bS)|((f.aM?scaleDn((c>>24)&255,f.aW):0)<<f.aS);
}
R7INLINE DWORD ReadPx(const IDirectDrawSurface7* s,DWORD x,DWORD y){
    const BYTE*p=(const BYTE*)s->desc.lpSurface+y*s->desc.lPitch;
    DWORD b=s->desc.ddpfPixelFormat.dwRGBBitCount? s->desc.ddpfPixelFormat.dwRGBBitCount:32;
    return b==16?((const uint16_t*)p)[x]: b==8?p[x]:((const DWORD*)p)[x];
}
R7INLINE void WritePx(IDirectDrawSurface7* s,DWORD x,DWORD y,DWORD v){
    BYTE*p=(BYTE*)s->desc.lpSurface+y*s->desc.lPitch;
    DWORD b=s->desc.ddpfPixelFormat.dwRGBBitCount? s->desc.ddpfPixelFormat.dwRGBBitCount:32;
    if(b==16)((uint16_t*)p)[x]=(uint16_t)v; else if(b==8)p[x]=(uint8_t)v; else ((DWORD*)p)[x]=v;
}
// z-buffer access — the z surface's ZBufferBitDepth drives width.
R7INLINE int ZBits(const IDirectDrawSurface7* z){
    DWORD d=z->desc.ddpfPixelFormat.dwZBufferBitDepth;
    if(!d)d=z->desc.ddpfPixelFormat.dwRGBBitCount;
    return d?(int)d:16;
}
R7INLINE DWORD ReadZ(const IDirectDrawSurface7* z,DWORD x,DWORD y){
    const BYTE*p=(const BYTE*)z->desc.lpSurface+y*z->desc.lPitch;
    return ZBits(z)>=32?((const DWORD*)p)[x]: ZBits(z)>=24?((const DWORD*)p)[x]&0xffffff
         :((const uint16_t*)p)[x];
}
R7INLINE void WriteZ(IDirectDrawSurface7* z,DWORD x,DWORD y,DWORD v){
    BYTE*p=(BYTE*)z->desc.lpSurface+y*z->desc.lPitch;
    if(ZBits(z)>=32)((DWORD*)p)[x]=v;
    else if(ZBits(z)>=24)memcpy(p+x*3,&v,3);   // 24-bit: don't trample the next texel
    else ((uint16_t*)p)[x]=(uint16_t)v;
}

struct Vert {                        // decoded TLVERTEX-family vertex
    float x,y,z,rhw;
    DWORD col, spec;
    float u[3],v[3];
};

struct Surf { IDirectDrawSurface7* s; PixFmt f; DWORD bpp; };

// --- resolved pixel environment ---------------------------------------------
// SyncRast copies every per-pixel-invariant field of State into PixEnv once
// per draw call.  The scan loops run off a local copy plus __restrict surface
// pointers, so GCC can hoist the env loads out of the pixel loop (surface
// stores would otherwise force every State/env field to be reloaded per
// pixel).  Semantics are unchanged — same states, same evaluation order.
// Per-format texel/dest unpack LUT — scaleUp's exact channel math baked once
// per PixFmt so the pixel loop trades ~4 integer divides per texel for one
// indexed load.  Cached process-wide: the table is shared by every device/
// stage that meets the same format.  Only <=16bpp needs it (32bpp channels
// are 8-bit wide, so scaleUp's divide compiles away anyway).
struct FmtLut {
    PixFmt key;
    DWORD c[4][256];                     // per-channel ARGB contributions
    FmtLut* next;
    bool Match(const PixFmt& f) const {
        return key.bits==f.bits&&key.rM==f.rM&&key.gM==f.gM&&key.bM==f.bM
              &&key.aM==f.aM;
    }
};
inline const FmtLut* GetFmtLut(const PixFmt& f){
    if(f.bits!=16&&f.bits!=8)return 0;
    static FmtLut* head=0;               // single render thread; built once
    for(FmtLut* p=head;p;p=p->next)if(p->Match(f))return p;
    FmtLut* p=(FmtLut*)malloc(sizeof(FmtLut));
    p->key=f;
    const DWORD ms[4]={f.rM,f.gM,f.bM,f.aM}; const int ss[4]={f.rS,f.gS,f.bS,f.aS};
    const int ws[4]={f.rW,f.gW,f.bW,f.aW}; const int sh[4]={16,8,0,24};
    for(int k=0;k<4;k++){
        DWORD lim=ms[k]?(ms[k]>>ss[k]):0;
        for(DWORD i=0;i<=lim;i++)
            p->c[k][i]=(ms[k]?scaleUp(i,ws[k]):(k==3?255:0))<<sh[k];
    }
    p->next=head; head=p; return p;
}
R7INLINE DWORD UnpackL(const FmtLut* L,DWORD raw){
    const PixFmt& f=L->key;
    return L->c[0][(raw&f.rM)>>f.rS]|L->c[1][(raw&f.gM)>>f.gS]
          |L->c[2][(raw&f.bM)>>f.bS]|L->c[3][(raw&f.aM)>>f.aS];
}

struct PixEnv {
    IDirectDrawSurface7* rt; PixFmt rtf;
    BYTE* rtSurf; int rtPitch,rtBpp; const FmtLut* rtLut;
    IDirectDrawSurface7* zb; int zbits;
    BYTE* zbSurf; int zbPitch;
    DWORD ze,zwe,zfn,zbias;
    DWORD persp,spec,ablend,ckey,fog;
    DWORD fogTMode,fogVMode,fogCol;
    float fogStart,fogEnd;
    DWORD sb,db,tf;
    int nStage; DWORD fast;
    struct TS {
        IDirectDrawSurface7* s; PixFmt f;
        const BYTE* surf; int pitch,bpp,w,h;
        const PALETTEENTRY* pal;          // 8bpp palettized -> live entries
        const FmtLut* lut;                // <=16bpp unpalettized -> channel luts
        DWORD addr,bilin;
        DWORD cop,aop,ca1,ca2,aa1,aa2,tci;
        DWORD wMask,hMask;      // pow2 wrap masks (0 when not pow2)
        DWORD wPow2,hPow2;
    } ts[3];
};

struct State {
    IDirectDrawSurface7* rt; PixFmt rtf;
    IDirectDrawSurface7* zb; int zbits;
    Surf tex[8];
    DWORD rs[256]; DWORD ss[8][64];
    DWORD fogCol; float fogStart,fogEnd;
    DWORD clipX0,clipY0,clipX1,clipY1;
    PixEnv env;
};
// Truncation-based floor: (int)f truncs toward 0, subtract 1 when the result
// overshot (negative non-integer).  floorf() is a real call on i386 and runs
// per texel.
R7INLINE int IFloor(float f){int i=(int)f; return i-((float)i>f);}
inline void BuildEnv(const State& st,PixEnv& e){
    e.rt=st.rt; e.rtf=st.rtf;
    e.rtSurf=(BYTE*)(st.rt?st.rt->desc.lpSurface:0);
    e.rtPitch=st.rt?(int)st.rt->desc.lPitch:0;
    e.rtBpp=st.rt?(int)(st.rt->desc.ddpfPixelFormat.dwRGBBitCount
                ?st.rt->desc.ddpfPixelFormat.dwRGBBitCount:32):32;
    e.rtLut=GetFmtLut(st.rtf);
    e.zb=st.zb; e.zbits=st.zbits;
    e.zbSurf=(BYTE*)(st.zb?st.zb->desc.lpSurface:0);
    e.zbPitch=st.zb?(int)st.zb->desc.lPitch:0;
    e.ze=st.rs[D3DRENDERSTATE_ZENABLE];
    e.zwe=st.rs[D3DRENDERSTATE_ZWRITEENABLE];
    e.zfn=st.rs[D3DRENDERSTATE_ZFUNC];
    if(!e.zfn)e.zfn=D3DCMP_LESSEQUAL;
    e.zbias=st.rs[D3DRENDERSTATE_ZBIAS];
    e.persp=st.rs[D3DRENDERSTATE_TEXTUREPERSPECTIVE];
    e.spec=st.rs[D3DRENDERSTATE_SPECULARENABLE];
    e.ablend=st.rs[D3DRENDERSTATE_ALPHABLENDENABLE];
    e.ckey=st.rs[D3DRENDERSTATE_COLORKEYENABLE];
    e.fog=st.rs[D3DRENDERSTATE_FOGENABLE];
    e.fogTMode=st.rs[D3DRENDERSTATE_FOGTABLEMODE];
    e.fogVMode=st.rs[D3DRENDERSTATE_FOGVERTEXMODE];
    e.fogCol=st.fogCol; e.fogStart=st.fogStart; e.fogEnd=st.fogEnd;
    e.sb=st.rs[D3DRENDERSTATE_SRCBLEND];
    if(!e.sb)e.sb=D3DBLEND_ONE;
    e.db=st.rs[D3DRENDERSTATE_DESTBLEND];
    e.tf=st.rs[D3DRENDERSTATE_TEXTUREFACTOR];
    e.nStage=0;
    for(int i=0;i<3;i++){
        DWORD cop=st.ss[i][D3DTSS_COLOROP],aop=st.ss[i][D3DTSS_ALPHAOP];
        if(cop==D3DTOP_DISABLE&&aop==D3DTOP_DISABLE)break;
        PixEnv::TS& t=e.ts[e.nStage++];
        t.s=st.tex[i].s; t.f=st.tex[i].f;
        DWORD addr=st.ss[i][D3DTSS_ADDRESS];
        if(!addr)addr=D3DTADDRESS_WRAP;
        t.addr=addr;
        t.bilin=st.ss[i][D3DTSS_MAGFILTER]==D3DTFG_LINEAR
              ||st.ss[i][D3DTSS_MINFILTER]==D3DTFG_LINEAR
              ||st.ss[i][D3DTSS_MAGFILTER]==D3DTFG_ANISOTROPIC
              ||st.ss[i][D3DTSS_MINFILTER]==D3DTFG_ANISOTROPIC;
        t.tci=st.ss[i][D3DTSS_TEXCOORDINDEX]&3;
        if(t.tci>2)t.tci=2;
        t.cop=cop; t.aop=aop;
        t.ca1=st.ss[i][D3DTSS_COLORARG1]; t.ca2=st.ss[i][D3DTSS_COLORARG2];
        t.aa1=st.ss[i][D3DTSS_ALPHAARG1]; t.aa2=st.ss[i][D3DTSS_ALPHAARG2];
        DWORD W=t.s?t.s->desc.dwWidth:1, H=t.s?t.s->desc.dwHeight:1;
        t.wPow2=(W&(W-1))==0; t.hPow2=(H&(H-1))==0;
        t.wMask=W-1; t.hMask=H-1;
        t.surf=(const BYTE*)(t.s?t.s->desc.lpSurface:0);
        t.pitch=t.s?(int)t.s->desc.lPitch:0;
        t.w=(int)W; t.h=(int)H;
        t.bpp=t.s?(int)(t.s->desc.ddpfPixelFormat.dwRGBBitCount
                 ?t.s->desc.ddpfPixelFormat.dwRGBBitCount:32):32;
        t.pal=(t.bpp==8&&t.s&&t.s->pal)?t.s->pal->entries:0;
        t.lut=(t.bpp==8&&t.pal)?0:GetFmtLut(t.f);
    }
    // Dominant-config flag: 16bpp target + either no stages or a single
    // MODULATE stage over a pow2 WRAP bilinear 16bpp texture, with no
    // blend/fog/specular/key work.  PixFast handles that combination with
    // bit-identical results; anything else takes the general pipeline.
    e.fast=0;
    if(e.rtBpp==16&&e.rtf.rM==0xf800&&e.rtf.gM==0x07e0&&e.rtf.bM==0x001f
       &&!e.ablend&&!e.fog&&!e.spec&&!e.ckey){
        if(e.nStage==0)e.fast=1;
        else if(e.nStage==1){
            const PixEnv::TS& t=e.ts[0];
            if(t.s&&t.surf&&t.bpp==16&&t.f.rM==0xf800&&t.f.gM==0x07e0
               &&t.f.bM==0x001f&&t.bilin&&t.tci==0
               &&t.addr==D3DTADDRESS_WRAP&&t.wPow2&&t.hPow2
               &&t.cop==D3DTOP_MODULATE&&t.ca1==D3DTA_TEXTURE
               &&t.ca2==D3DTA_DIFFUSE
               &&(t.aop==D3DTOP_DISABLE
                  ||(t.aop==D3DTOP_SELECTARG1
                     &&(t.aa1==D3DTA_DIFFUSE||t.aa1==D3DTA_TEXTURE))))
                e.fast=(t.aa1==D3DTA_TEXTURE)?2:1;
        }
    }
#ifdef ROWAN_PERF_TRACE
    {static bool once=false;
    if(!once){once=true;
      const PixEnv::TS&t=e.ts[0];
      fprintf(stderr,"[env] fast=%u nStage=%d rtBpp=%d rM=%lx t.s=%p bpp=%d lut=%p bilin=%u tci=%u addr=%u wP=%u hP=%u cop=%u ca1=%lx ca2=%lx aop=%u aa1=%lx persp=%u ze=%u\n",
      e.fast,e.nStage,e.rtBpp,(unsigned long)e.rtf.rM,(void*)t.s,t.bpp,(void*)t.lut,t.bilin,t.tci,t.addr,t.wPow2,t.hPow2,t.cop,(unsigned long)t.ca1,(unsigned long)t.ca2,t.aop,(unsigned long)t.aa1,e.persp,e.ze);}}
#endif
}

// --- texture sampling -------------------------------------------------------
// 8bpp surfaces are palettized: the index means nothing until SetPalette's
// entries resolve it.
R7INLINE DWORD UnpackPx(const Surf& t,DWORD raw){
    if(t.s->desc.ddpfPixelFormat.dwRGBBitCount==8 && t.s->pal){
        const PALETTEENTRY& e=t.s->pal->entries[raw&255];
        return 0xff000000u|((DWORD)e.peRed<<16)|((DWORD)e.peGreen<<8)|e.peBlue;
    }
    return Unpack(raw,t.f);
}
// Wrap-modes resolved per axis; pow2 sizes use masks, the rest keep the
// original modulo path.  Same results, no integer divide.
R7INLINE int WrapC(int c,int m,DWORD pow2,DWORD mask,DWORD addr){
    if(addr==D3DTADDRESS_CLAMP){return c<0?0:c>=m?m-1:c;}
    if(addr==D3DTADDRESS_MIRROR){int p=2*m;c=((c%p)+p)%p;return c>=m?p-1-c:c;}
    if(pow2)return c&(int)mask;                            // WRAP default
    int r=c%m; return r<0?r+m:r;
}
R7INLINE DWORD UnpackPxS(IDirectDrawSurface7* s,const PixFmt& f,DWORD raw){
    if(s->desc.ddpfPixelFormat.dwRGBBitCount==8 && s->pal){
        const PALETTEENTRY& e=s->pal->entries[raw&255];
        return 0xff000000u|((DWORD)e.peRed<<16)|((DWORD)e.peGreen<<8)|e.peBlue;
    }
    return Unpack(raw,f);
}
// Fetch+unpack one texel through the cached stage addressing.  For palettized
// 8bpp the live palette resolves the index; <=16bpp unpalettized goes through
// the per-format LUT (identical values to Unpack()); 32bpp unpacks inline.
R7INLINE DWORD FetchT(const PixEnv::TS& t,int x,int y){
    const BYTE* r=t.surf+y*t.pitch;
    DWORD raw=t.bpp==16?(DWORD)((const uint16_t*)r)[x]
            :t.bpp==8?(DWORD)r[x]:((const DWORD*)r)[x];
    if(t.lut)return UnpackL(t.lut,raw);
    if(t.pal){const PALETTEENTRY& p=t.pal[raw&255];
        return 0xff000000u|((DWORD)p.peRed<<16)|((DWORD)p.peGreen<<8)|p.peBlue;}
    return Unpack(raw,t.f);
}
R7INLINE DWORD SampleTexE(const PixEnv::TS& t,float u,float v){
    if(!t.surf)return 0xffffffff;
    int W=t.w,H=t.h;
    // Point sampling takes floor(u*W); the -0.5 shift is only for the
    // bilinear weight basis (texel centers at (i+0.5)/W).
    float fx=t.bilin? u*W-0.5f : u*W;
    float fy=t.bilin? v*H-0.5f : v*H;
    int x0=IFloor(fx), y0=IFloor(fy);
    DWORD xA=(DWORD)WrapC(x0,W,t.wPow2,t.wMask,t.addr),
          yA=(DWORD)WrapC(y0,H,t.hPow2,t.hMask,t.addr);
    DWORD c00=FetchT(t,xA,yA);
    if(!t.bilin)return c00;
    DWORD xB=(DWORD)WrapC(x0+1,W,t.wPow2,t.wMask,t.addr),
          yB=(DWORD)WrapC(y0+1,H,t.hPow2,t.hMask,t.addr);
    DWORD c10=FetchT(t,xB,yA),
          c01=FetchT(t,xA,yB),
          c11=FetchT(t,xB,yB);
    int wx=(int)((fx-(float)x0)*256), wy=(int)((fy-(float)y0)*256);
    DWORD out=0;
    for(int i=0;i<4;i++){
        DWORD s0=(c00>>(i*8))&255,s1=(c10>>(i*8))&255,s2=(c01>>(i*8))&255,s3=(c11>>(i*8))&255;
        DWORD v0=(s0*(256-wx)+s1*wx)>>8, v1=(s2*(256-wx)+s3*wx)>>8;
        out|=(((v0*(256-wy)+v1*wy)>>8))<<(i*8);
    }
    return out;
}
// --- DX7 texture-stage evaluation -------------------------------------------
struct StageRes { DWORD col, alp; };
R7INLINE DWORD Arg(DWORD ta,DWORD tex,DWORD cur,DWORD dif,DWORD tfac){
    switch(ta&D3DTA_SELECTMASK){
        case D3DTA_DIFFUSE: return dif;
        case D3DTA_TEXTURE: return tex;
        case D3DTA_TFACTOR: return tfac;
        case D3DTA_CURRENT:
        default:            return cur;
    }
}
R7INLINE DWORD Complement(DWORD ta,DWORD v){ return (ta&D3DTA_COMPLEMENT)?v^0x00ffffff:v; }
R7INLINE DWORD AlphaRep (DWORD ta,DWORD v){
    if(ta&D3DTA_ALPHAREPLICATE){DWORD a=v>>24;return a|a<<8|a<<16;}
    return v;
}
R7INLINE DWORD BlendOp(DWORD op,DWORD a1,DWORD a2){
    int r=(a1>>16)&255,g=(a1>>8)&255,b=a1&255,r2=(a2>>16)&255,g2=(a2>>8)&255,b2=a2&255,R,G,B;
    switch(op){
        case D3DTOP_SELECTARG1: R=r;G=g;B=b;break;
        case D3DTOP_SELECTARG2: R=r2;G=g2;B=b2;break;
        case D3DTOP_ADD:        R=r+r2;G=g+g2;B=b+b2;break;
        case D3DTOP_ADDSIGNED:  R=r+r2-128;G=g+g2-128;B=b+b2-128;break;
        case D3DTOP_SUBTRACT:   R=r-r2;G=g-g2;B=b-b2;break;
        case D3DTOP_MODULATE4X:R=4*r*r2/255;G=4*g*g2/255;B=4*b*b2/255;break;
        default:                R=r*r2/255;G=g*g2/255;B=b*b2/255;break; // MODULATE(2X)
    }
    if(R<0)R=0;
    if(R>255)R=255;
    if(G<0)G=0;
    if(G>255)G=255;
    if(B<0)B=0;
    if(B>255)B=255;
    return (R<<16)|(G<<8)|B;
}
R7INLINE DWORD BlendOpAlpha(DWORD op,DWORD a1,DWORD a2){
    int a=(a1>>24)&255,b=(a2>>24)&255,R;
    switch(op){
        case D3DTOP_SELECTARG1: R=a;break;
        case D3DTOP_SELECTARG2: R=b;break;
        case D3DTOP_ADD:        R=a+b;break;
        case D3DTOP_MODULATE4X:R=4*a*b/255;break;
        default:                R=a*b/255;break;
    }
    return (DWORD)(R<0?0:R>255?255:R)<<24;
}
R7INLINE StageRes EvalStagesE(const PixEnv& e,const Vert& pv){
    DWORD cur=pv.col;
    DWORD dif=pv.col;
    DWORD tf=e.tf;
    for(int i=0;i<e.nStage;i++){
        const PixEnv::TS& t=e.ts[i];
        DWORD cop=t.cop;
        DWORD aop=t.aop;
        DWORD texC=0xffffffff;
        if(t.s){
#ifdef BENCH_NOTEX
            texC=0xff808080;
#else
            texC=SampleTexE(t,pv.u[t.tci],pv.v[t.tci]);
#endif
            // Source color key: keyed texels come out transparent.  The key
            // range applies to the unpacked RGB (matches how real drivers
            // key on the texel value for a single-color key).
            if(e.ckey && (t.s->ckeyFlags&DDCKEY_SRCBLT)){
                DWORD lo=t.s->ckey.dwColorSpaceLowValue&0xffffff;
                DWORD hi=t.s->ckey.dwColorSpaceHighValue&0xffffff;
                if((texC&0xffffff)>=lo&&(texC&0xffffff)<=hi)texC&=0x00ffffff;
            }
        }
        if(cop==D3DTOP_BLENDTEXTUREALPHA){
            DWORD tA=(texC>>24)&255;
            DWORD a1=Arg(t.ca1,texC,cur,dif,tf);
            DWORD a2=Arg(t.ca2,texC,cur,dif,tf);
            DWORD lo=((a1&0xffffff)*(255-tA)+(a2&0xffffff)*tA)/255;
            cur=(cur&0xff000000)|(lo&0xffffff);
        } else if(cop!=D3DTOP_DISABLE){
            DWORD a1=AlphaRep(t.ca1,Complement(t.ca1,
                       Arg(t.ca1,texC,cur,dif,tf)));
            DWORD a2=AlphaRep(t.ca2,Complement(t.ca2,
                       Arg(t.ca2,texC,cur,dif,tf)));
            cur=(cur&0xff000000)|BlendOp(cop,a1,a2);
        }
        if(aop!=D3DTOP_DISABLE){
            DWORD a1=Arg(t.aa1,texC,cur,dif,tf);
            DWORD a2=Arg(t.aa2,texC,cur,dif,tf);
            cur=(cur&0x00ffffff)|BlendOpAlpha(aop,a1,a2);
        }
    }
    StageRes r; r.col=cur; r.alp=cur>>24; return r;
}

// --- z / blend / fog ---------------------------------------------------------
R7INLINE bool ZTest(DWORD fn,DWORD nv,DWORD ov){
    switch(fn){case D3DCMP_NEVER:return false;case D3DCMP_LESS:return nv<ov;
        case D3DCMP_EQUAL:return nv==ov;case D3DCMP_LESSEQUAL:return nv<=ov;
        case D3DCMP_GREATER:return nv>ov;case D3DCMP_NOTEQUAL:return nv!=ov;
        case D3DCMP_GREATEREQUAL:return nv>=ov;default:return true;}
}
R7INLINE DWORD BlendChan(DWORD src,DWORD dst,DWORD blend,int sh){
    DWORD s=(src>>sh)&255,d=(dst>>sh)&255;
    switch(blend){
        case D3DBLEND_ZERO:          return 0;
        case D3DBLEND_ONE:           return 255;
        case D3DBLEND_SRCCOLOR:      return s;
        case D3DBLEND_INVSRCCOLOR:   return 255-s;
        case D3DBLEND_SRCALPHA:      return (src>>24)&255;
        case D3DBLEND_INVSRCALPHA:   return 255-((src>>24)&255);
        case D3DBLEND_DESTALPHA:     return (dst>>24)&255;
        case D3DBLEND_INVDESTALPHA:  return 255-((dst>>24)&255);
        case D3DBLEND_DESTCOLOR:     return d;
        case D3DBLEND_INVDESTCOLOR:  return 255-d;
        case D3DBLEND_SRCALPHASAT:   return min((src>>24)&255,255-((dst>>24)&255));
        default:                     return 255;
    }
}

// --- per-pixel pipeline ------------------------------------------------------
// Interpolated channel order inside the plane-fit arrays.
enum { IC_Z, IC_RHW, IC_CR, IC_CG, IC_CB, IC_CA,
       IC_SR, IC_SG, IC_SB, IC_SA,
       IC_U0, IC_V0, IC_U1, IC_V1, IC_U2, IC_V2, IC_N };
R7INLINE void Pack(const Vert& v,float* c,bool persp){
    float w=v.rhw!=0?v.rhw:1.0f;                 // persp stores u*rhw
    c[IC_Z]=v.z; c[IC_RHW]=v.rhw;
    c[IC_CR]=(v.col>>16)&255; c[IC_CG]=(v.col>>8)&255;
    c[IC_CB]=v.col&255; c[IC_CA]=(v.col>>24)&255;
    c[IC_SR]=(v.spec>>16)&255; c[IC_SG]=(v.spec>>8)&255;
    c[IC_SB]=v.spec&255; c[IC_SA]=(v.spec>>24)&255;
    for(int i=0;i<3;i++){c[IC_U0+i*2]=persp?v.u[i]*w:v.u[i];
                         c[IC_V0+i*2]=persp?v.v[i]*w:v.v[i];}
}
// Fast kernel for the dominant in-flight state: 565 target, z-buffered,
// either untextured or one MODULATE(diffuse,texture) stage over a pow2 WRAP
// bilinear 16bpp texture, no blend/fog/spec/key.  Every step reproduces the
// general pipeline's exact arithmetic for that state (same z math, same
// bilinear weights, same r*r2/255 modulate, same 565 down-pack).
R7INLINE void PixFast(const PixEnv& e,DWORD x,DWORD y,const float* c){
    // --- z test/write: identical block to the general path ------------------
    if(e.ze&&e.zb){
        BYTE* __restrict zrow=e.zbSurf+y*e.zbPitch;
        DWORD zmax=(e.zbits>=32)?0xffffffffu:(e.zbits>=24?0xffffffu:0xffffu);
        DWORD nz;
        if(e.ze==D3DZB_USEW){
            float w=c[IC_RHW]>1e-20f?1.0f/c[IC_RHW]:3.402823466e38f;
            if(!(w>0.0f))w=3.402823466e38f;
            memcpy(&nz,&w,4);
            if(e.zbits<32)nz>>=(32-e.zbits);
        } else {
            float zf=c[IC_Z]<0?0:c[IC_Z]>1?1:c[IC_Z];
            nz=(DWORD)(zf*zmax)+(e.zbias<<(e.zbits-8>0?e.zbits-8:0));
        }
        DWORD ov=e.zbits>=32?((const DWORD*)zrow)[x]
               :e.zbits>=24?((const DWORD*)zrow)[x]&0xffffff
               :((const uint16_t*)zrow)[x];
        if(!ZTest(e.zfn,nz,ov))return;
        if(e.zwe){
            if(e.zbits>=32)((DWORD*)zrow)[x]=nz;
            else if(e.zbits>=24)memcpy(zrow+x*3,&nz,3);
            else ((uint16_t*)zrow)[x]=(uint16_t)nz;
        }
    }
    int R=(int)c[IC_CR],G=(int)c[IC_CG],B=(int)c[IC_CB],A=(int)c[IC_CA];
    DWORD dif=((A<0?0:A>255?255:A)<<24)|((R<0?0:R>255?255:R)<<16)
             |((G<0?0:G>255?255:G)<<8)|(B<0?0:B>255?255:B);
    DWORD out=dif;
    if(e.nStage){
        const PixEnv::TS& t=e.ts[0];
        float w=e.persp?(c[IC_RHW]!=0?1.0f/c[IC_RHW]:1.0f):1.0f;
        float u=e.persp?c[IC_U0]*w:c[IC_U0];
        float v=e.persp?c[IC_V0]*w:c[IC_V0];
        float fx=u*t.w-0.5f, fy=v*t.h-0.5f;
        int x0=IFloor(fx), y0=IFloor(fy);
        int xA=x0&(int)t.wMask, xB=(x0+1)&(int)t.wMask;
        int yA=y0&(int)t.hMask, yB=(y0+1)&(int)t.hMask;
        const uint16_t* r0=(const uint16_t*)(t.surf+yA*t.pitch);
        const uint16_t* r1=(const uint16_t*)(t.surf+yB*t.pitch);
        // 565 -> ARGB: scaleUp(5)=v*255/31, scaleUp(6)=v*255/63 (constant
        // divisors -> magic multiplies, no table loads).
        DWORD p00=r0[xA],p10=r0[xB],p01=r1[xA],p11=r1[xB];
        DWORD c00=0xff000000|((((p00>>11)&31)*255/31)<<16)
               |((((p00>>5)&63)*255/63)<<8)|((p00&31)*255/31);
        DWORD c10=0xff000000|((((p10>>11)&31)*255/31)<<16)
               |((((p10>>5)&63)*255/63)<<8)|((p10&31)*255/31);
        DWORD c01=0xff000000|((((p01>>11)&31)*255/31)<<16)
               |((((p01>>5)&63)*255/63)<<8)|((p01&31)*255/31);
        DWORD c11=0xff000000|((((p11>>11)&31)*255/31)<<16)
               |((((p11>>5)&63)*255/63)<<8)|((p11&31)*255/31);
        int wx=(int)((fx-(float)x0)*256), wy=(int)((fy-(float)y0)*256);
        DWORD tex=0;
        for(int i=0;i<4;i++){
            DWORD s0=(c00>>(i*8))&255,s1=(c10>>(i*8))&255,
                  s2=(c01>>(i*8))&255,s3=(c11>>(i*8))&255;
            DWORD v0=(s0*(256-wx)+s1*wx)>>8, v1=(s2*(256-wx)+s3*wx)>>8;
            tex|=(((v0*(256-wy)+v1*wy)>>8))<<(i*8);
        }
        // MODULATE color = tex*dif per channel /255; alpha per the alpha op.
        DWORD a=(e.fast==2)?(tex>>24):(dif>>24);
        int r=((tex>>16)&255)*((dif>>16)&255)/255;
        int g=((tex>>8)&255)*((dif>>8)&255)/255;
        int b=(tex&255)*(dif&255)/255;
        out=((DWORD)a<<24)|((DWORD)r<<16)|((DWORD)g<<8)|(DWORD)b;
    }
    // 565 down-pack: scaleDn(v,5)<<11 | scaleDn(v,6)<<5 | scaleDn(v,5)
    uint16_t px=(uint16_t)(((((out>>16)&255)*31+127)/255<<11)
                |((((out>>8)&255)*63+127)/255<<5)
                |(((out&255)*31+127)/255));
    ((uint16_t*)(e.rtSurf+y*e.rtPitch))[x]=px;
}
R7INLINE void PixE(const PixEnv& e,DWORD x,DWORD y,const float* c){
    if(e.fast){PixFast(e,x,y,c);return;}
    Vert pv;
    pv.z=c[IC_Z]; pv.rhw=c[IC_RHW];
    float w=e.persp?(pv.rhw!=0?1.0f/pv.rhw:1.0f):1.0f;
    for(int i=0;i<3;i++){pv.u[i]=e.persp?c[IC_U0+i*2]*w:c[IC_U0+i*2];
                         pv.v[i]=e.persp?c[IC_V0+i*2]*w:c[IC_V0+i*2];}
    int R=(int)c[IC_CR],G=(int)c[IC_CG],B=(int)c[IC_CB],A=(int)c[IC_CA];
    pv.col=((A<0?0:A>255?255:A)<<24)|((R<0?0:R>255?255:R)<<16)
          |((G<0?0:G>255?255:G)<<8)|(B<0?0:B>255?255:B);
    // specular channels only feed SPECULARENABLE and vertex-fog — skip the
    // clamp+repack when neither is live (the common terrain path).
    if(e.spec||e.fog){
        int sR=(int)c[IC_SR],sG=(int)c[IC_SG],sB=(int)c[IC_SB],sA=(int)c[IC_SA];
        pv.spec=((sA<0?0:sA>255?255:sA)<<24)|((sR<0?0:sR>255?255:sR)<<16)
               |((sG<0?0:sG>255?255:sG)<<8)|(sB<0?0:sB>255?255:sB);
    } else pv.spec=0;

    // z test (against the attached z surface; z values 0..1 scaled to width)
    if(e.ze&&e.zb){
        BYTE* __restrict zrow=e.zbSurf+y*e.zbPitch;
        DWORD zmax=(e.zbits>=32)?0xffffffffu:(e.zbits>=24?0xffffffu:0xffffu);
        DWORD nz;
        if(e.ze==D3DZB_USEW){
            // W-buffer: depth is view-space w (= 1/rhw), not clip z.  The
            // interpolated rhw inverted gives the true plane depth at the
            // pixel — nearer means smaller w.  Positive float bit patterns
            // order-preserve as unsigned ints, and truncating the low bits
            // still preserves order for narrower buffers.
            float w=pv.rhw>1e-20f?1.0f/pv.rhw:3.402823466e38f;
            if(!(w>0.0f))w=3.402823466e38f;      // NaN/negative -> far
            memcpy(&nz,&w,4);
            if(e.zbits<32)nz>>=(32-e.zbits);
        } else {
            float zf=pv.z<0?0:pv.z>1?1:pv.z;
            nz=(DWORD)(zf*zmax)+(e.zbias<<(e.zbits-8>0?e.zbits-8:0));
        }
        DWORD ov=e.zbits>=32?((const DWORD*)zrow)[x]
               :e.zbits>=24?((const DWORD*)zrow)[x]&0xffffff
               :((const uint16_t*)zrow)[x];
        static const char* probeP=getenv("ROWAN_PIX");
        static int ppx=-1,ppy=-1;
        if(probeP&&ppx<0)sscanf(probeP,"%d,%d",&ppx,&ppy);
        if(probeP&&(int)x==ppx&&(int)y==ppy){
            float wf; memcpy(&wf,&nz,4);
            float of; memcpy(&of,&ov,4);
            fprintf(stderr,"[PIX] tex0=%p w=%g ov=%g fn=%u zwe=%u %s\n",
                (void*)(e.nStage?e.ts[0].s:0),wf,of,(unsigned)e.zfn,
                (unsigned)e.zwe,ZTest(e.zfn,nz,ov)?"PASS":"FAIL");}
        if(!ZTest(e.zfn,nz,ov))return;
        if(e.zwe){
            if(e.zbits>=32)((DWORD*)zrow)[x]=nz;
            else if(e.zbits>=24)memcpy(zrow+x*3,&nz,3);
            else ((uint16_t*)zrow)[x]=(uint16_t)nz;
        }
    }
    else {
        static const char* probeP0=getenv("ROWAN_PIX");
        static int ppx0=-1,ppy0=-1;
        if(probeP0&&ppx0<0)sscanf(probeP0,"%d,%d",&ppx0,&ppy0);
        if(probeP0&&(int)x==ppx0&&(int)y==ppy0)
            fprintf(stderr,"[PIX] tex0=%p NO-ZTEST ze=%u zb=%p\n",
                (void*)(e.nStage?e.ts[0].s:0),(unsigned)e.ze,(void*)e.zb);
    }
    StageRes sr;
#ifdef BENCH_NOSTAGE
    sr.col=pv.col; sr.alp=pv.col>>24;
#else
    sr=EvalStagesE(e,pv);
#endif
    DWORD out=sr.col;
    // specular add (post-texture, DX7 SPECULARENABLE)
    if(e.spec){
        int a=out>>24;
        int r=((out>>16)&255)+((pv.spec>>16)&255);
        int g=((out>>8)&255)+((pv.spec>>8)&255);
        int b=(out&255)+(pv.spec&255);
        out=(DWORD)a<<24|(DWORD)(r>255?255:r)<<16|(DWORD)(g>255?255:g)<<8|(DWORD)(b>255?255:b);
    }
    // fog: vertex fog factor lives in specular alpha; table fog derives from z
    if(e.fog){
        float f=-1;
        if(e.fogTMode==D3DFOG_LINEAR){
            float fs=e.fogStart,stE=e.fogEnd;
            f=(stE-fs)!=0?(stE-pv.z)/(stE-fs):0;
        } else if(e.fogVMode==D3DFOG_LINEAR)
            f=((pv.spec>>24)&255)/255.0f;
        if(f>=0){
            if(f>1)f=1;
            DWORD fc=e.fogCol;
            int fr=(fc>>16)&255,fg=(fc>>8)&255,fb=fc&255;
            int a=out>>24;
            int r=(int)(f*((out>>16)&255)+(1-f)*fr);
            int g=(int)(f*((out>>8)&255)+(1-f)*fg);
            int b=(int)(f*(out&255)+(1-f)*fb);
            out=(DWORD)a<<24|(DWORD)r<<16|(DWORD)g<<8|(DWORD)b;
        }
    }
    // __restrict on the row pointer: stores through it cannot alias the env
    // or the interpolant array, so all env fields stay hoisted in the loop.
    BYTE* __restrict rrow=e.rtSurf+y*e.rtPitch;
    DWORD rbpp=e.rtBpp;
    DWORD packed=Pack(out,e.rtf);
    if(e.ablend){
        DWORD raw=rbpp==16?((const uint16_t*)rrow)[x]:rbpp==8?rrow[x]:((const DWORD*)rrow)[x];
        DWORD dst=e.rtLut?UnpackL(e.rtLut,raw):Unpack(raw,e.rtf);
        DWORD res=0;
        for(int i=0;i<4;i++){
            DWORD sc=(out>>(i*8))&255,dc=(dst>>(i*8))&255;
            DWORD r=(sc*BlendChan(out,dst,e.sb,i*8)+dc*BlendChan(out,dst,e.db,i*8))/255;
            res|=(r>255?255:r)<<(i*8);
        }
        packed=Pack((out&0xff000000)|res,e.rtf);   // keep src alpha in pixel write
    }
    if(rbpp==16)((uint16_t*)rrow)[x]=(uint16_t)packed;
    else if(rbpp==8)rrow[x]=(uint8_t)packed;
    else ((DWORD*)rrow)[x]=packed;
}

// --- scan converter ----------------------------------------------------------
inline bool CullWinding(const State& st,const Vert&v0,const Vert&v1,const Vert&v2){
    DWORD cm=st.rs[D3DRENDERSTATE_CULLMODE];
    static const int nocull=getenv("ROWAN_NOCULL")?1:0;
    if(nocull||cm==D3DCULL_NONE||cm==0)return false;
    // Screen space is y-down: a positive signed area e appears clockwise
    // on the raster surface.  DX7 judges winding in screen space, so
    // D3DCULL_CW culls e>0 faces and D3DCULL_CCW culls e<0 faces.
    float e=(v1.x-v0.x)*(v2.y-v0.y)-(v2.x-v0.x)*(v1.y-v0.y);
    return cm==D3DCULL_CW ? e>0 : e<0;
}
inline void DrawTri(const State& st,const Vert A,const Vert B,const Vert C){
    if(!st.rt||!st.rt->desc.lpSurface)return;
    float d=(B.x-A.x)*(C.y-A.y)-(C.x-A.x)*(B.y-A.y);
    if(d==0)return;
    float ia=1.0f/d;
    bool persp=st.rs[D3DRENDERSTATE_TEXTUREPERSPECTIVE]!=0;
    float c0[IC_N],c1[IC_N],c2[IC_N];
    Pack(A,c0,persp); Pack(B,c1,persp); Pack(C,c2,persp);
    float dx[IC_N],dy[IC_N];
    for(int i=0;i<IC_N;i++){
        dx[i]=((c1[i]-c0[i])*(C.y-A.y)-(c2[i]-c0[i])*(B.y-A.y))*ia;
        dy[i]=((B.x-A.x)*(c2[i]-c0[i])-(C.x-A.x)*(c1[i]-c0[i]))*ia;
    }
    DWORD W=st.rt->desc.dwWidth,H=st.rt->desc.dwHeight;
    static const char* probeT=getenv("ROWAN_PIX");
    static int tpx=-1,tpy=-1;
    if(probeT&&tpx<0)sscanf(probeT,"%d,%d",&tpx,&tpy);
    float bx0=A.x<B.x?(A.x<C.x?A.x:C.x):(B.x<C.x?B.x:C.x);
    float bx1=A.x>B.x?(A.x>C.x?A.x:C.x):(B.x>C.x?B.x:C.x);
    float by0=A.y<B.y?(A.y<C.y?A.y:C.y):(B.y<C.y?B.y:C.y);
    float by1=A.y>B.y?(A.y>C.y?A.y:C.y):(B.y>C.y?B.y:C.y);
    if(probeT&&tpx>=bx0-2&&tpx<=bx1+2&&tpy>=by0-2&&tpy<=by1+2){
        float px=tpx+0.5f,py=tpy+0.5f;
        float sgn=d>0?1.f:-1.f;
        float w0=((C.x-B.x)*(py-B.y)-(C.y-B.y)*(px-B.x))*sgn;
        float w1=((A.x-C.x)*(py-C.y)-(A.y-C.y)*(px-C.x))*sgn;
        float w2=((B.x-A.x)*(py-A.y)-(B.y-A.y)*(px-A.x))*sgn;
        fprintf(stderr,"[TRI] tex0=%p A(%g,%g) B(%g,%g) C(%g,%g) d=%g W=%u H=%u w0=%g w1=%g w2=%g %s\n",
            (void*)st.tex[0].s,A.x,A.y,B.x,B.y,C.x,C.y,d,(unsigned)W,(unsigned)H,w0,w1,w2,
            (w0>=0&&w1>=0&&w2>=0)?"COVER":"miss");}
    float x0=A.x<B.x?(A.x<C.x?A.x:C.x):(B.x<C.x?B.x:C.x);
    float x1=A.x>B.x?(A.x>C.x?A.x:C.x):(B.x>C.x?B.x:C.x);
    float y0=A.y<B.y?(A.y<C.y?A.y:C.y):(B.y<C.y?B.y:C.y);
    float y1=A.y>B.y?(A.y>C.y?A.y:C.y):(B.y>C.y?B.y:C.y);
    int ix0=x0<0?0:(int)x0, ix1=x1>=(float)W?(int)W-1:(int)x1;
    int iy0=y0<0?0:(int)y0, iy1=y1>=(float)H?(int)H-1:(int)y1;
    // half-space edge functions E(P)=(p2.x-p1.x)*(P.y-p1.y)-(p2.y-p1.y)*(P.x-p1.x)
    // for directed edges B->C, C->A, A->B; interior iff all >=0 (winding-signed)
    float sgn=d>0?1.f:-1.f;
    static const bool perf=getenv("ROWAN_PERF")!=0;
    static uint64_t pTris=0,pScan=0,pCov=0; static double pT0=0;
    // Half-space edge functions evaluated once per bbox corner then stepped
    // in y across rows.  Per row the covered x-range is solved analytically —
    // each edge contributes a lower bound (dwx>0) or upper bound (dwx<0) to
    // the pixel-centre coverage test — so the scan never iterates an
    // uncovered pixel.  Double precision keeps the bounds numerically at
    // least as accurate as the float per-pixel evaluation this replaces.
    const double sgnD=sgn;
    const double dwx0=(B.y-C.y)*sgnD, dwy0=(C.x-B.x)*sgnD;
    const double dwx1=(C.y-A.y)*sgnD, dwy1=(A.x-C.x)*sgnD;
    const double dwx2=(A.y-B.y)*sgnD, dwy2=(B.x-A.x)*sgnD;
    const double px0=ix0+0.5;
    double rw0=((C.x-B.x)*(iy0+0.5-B.y)-(C.y-B.y)*(px0-B.x))*sgnD;
    double rw1=((A.x-C.x)*(iy0+0.5-C.y)-(A.y-C.y)*(px0-C.x))*sgnD;
    double rw2=((B.x-A.x)*(iy0+0.5-A.y)-(B.y-A.y)*(px0-A.x))*sgnD;
    const PixEnv& e=st.env;
    float cur[IC_N];
    for(int y=iy0;y<=iy1;y++){
        float py=y+0.5f;
        // w_i(x) = w_i + dwx_i*(x-ix0) >= 0  ->  x >= ix0 - w_i/dwx_i (dwx>0)
        double lo=ix0, hi=ix1;
        if(dwx0>0){double b=ix0-rw0/dwx0;
        if(b>lo)lo=b;}
        else if(dwx0<0){double b=ix0-rw0/dwx0;
        if(b<hi)hi=b;}
        else if(rw0<0)goto nrow;
        if(dwx1>0){double b=ix0-rw1/dwx1;
        if(b>lo)lo=b;}
        else if(dwx1<0){double b=ix0-rw1/dwx1;
        if(b<hi)hi=b;}
        else if(rw1<0)goto nrow;
        if(dwx2>0){double b=ix0-rw2/dwx2;
        if(b>lo)lo=b;}
        else if(dwx2<0){double b=ix0-rw2/dwx2;
        if(b<hi)hi=b;}
        else if(rw2<0)goto nrow;
        if(lo<ix0)lo=ix0;
        if(hi>ix1)hi=ix1;
        if(lo<=hi){
            int xs=(int)ceil(lo), xe=(int)floor(hi);
            for(int i=0;i<IC_N;i++)
                cur[i]=c0[i]+dx[i]*(xs+0.5f-A.x)+dy[i]*(py-A.y);
#ifdef BENCH_TSC
            static uint64_t tPix=0,tInt=0,tN=0; static double tt0=0;
#endif
            for(int x=xs;x<=xe;x++){
#ifdef BENCH_TSC
                uint64_t t0=__builtin_ia32_rdtsc();
                PixE(e,x,y,cur);
                uint64_t t1=__builtin_ia32_rdtsc();
                for(int i=0;i<IC_N;i++)cur[i]+=dx[i];
                tPix+=t1-t0; tInt+=__builtin_ia32_rdtsc()-t1; tN++;
                double tw=double(clock())/CLOCKS_PER_SEC;
                if(tw-tt0>5){fprintf(stderr,"[tsc] pix=%llu int=%llu n=%llu -> %llu cyc/px (%llu interp)\n",
                  (unsigned long long)tPix,(unsigned long long)tInt,(unsigned long long)tN,
                  (unsigned long long)(tPix/tN),(unsigned long long)(tInt/tN));
                  tPix=tInt=tN=0;tt0=tw;}
#else
                PixE(e,x,y,cur);
                if(perf)pCov++;
#ifdef __SSE2__
                {_mm_storeu_ps(cur+0,_mm_add_ps(_mm_loadu_ps(cur+0),_mm_loadu_ps(dx+0)));
                 _mm_storeu_ps(cur+4,_mm_add_ps(_mm_loadu_ps(cur+4),_mm_loadu_ps(dx+4)));
                 _mm_storeu_ps(cur+8,_mm_add_ps(_mm_loadu_ps(cur+8),_mm_loadu_ps(dx+8)));
                 _mm_storeu_ps(cur+12,_mm_add_ps(_mm_loadu_ps(cur+12),_mm_loadu_ps(dx+12)));
                 cur[16]+=dx[16];}
#else
                for(int i=0;i<IC_N;i++)cur[i]+=dx[i];
#endif
                if(perf)pScan++;
#endif
            }
        }
nrow:
        rw0+=dwy0;rw1+=dwy1;rw2+=dwy2;
    }
    if(perf){pTris++;double t=double(clock())/CLOCKS_PER_SEC;
        if(t-pT0>5.0){fprintf(stderr,"[perf] tris=%llu scan=%llu cov=%llu (%.1fM scanned/s)\n",
            (unsigned long long)pTris,(unsigned long long)pScan,(unsigned long long)pCov,
            pScan/(t-pT0)/1e6);pTris=pScan=pCov=0;pT0=t;}}
}
inline void DrawLine(const State& st,const Vert&A,const Vert&B){
    if(!st.rt||!st.rt->desc.lpSurface)return;
    // DDA with the same per-pixel pipeline; flat interpolant steps
    int steps=(int)fmaxf(fabsf(B.x-A.x),fabsf(B.y-A.y));
    if(steps<1)steps=1;
    bool persp=st.rs[D3DRENDERSTATE_TEXTUREPERSPECTIVE]!=0;
    float ca[IC_N],cb[IC_N],c[IC_N];
    Pack(A,ca,persp); Pack(B,cb,persp);
    for(int i=0;i<IC_N;i++)cb[i]=(cb[i]-ca[i])/steps;
    memcpy(c,ca,sizeof(c));
    int x=(int)(A.x+0.5f),y=(int)(A.y+0.5f);
    float fx=(B.x-A.x)/steps,fy=(B.y-A.y)/steps;
    for(int i=0;i<=steps;i++){
        if(x>=0&&y>=0&&x<(int)st.rt->desc.dwWidth&&y<(int)st.rt->desc.dwHeight)
            PixE(st.env,x,y,c);
        for(int j=0;j<IC_N;j++)c[j]+=cb[j];
        x=(int)(A.x+0.5f+fx*i); y=(int)(A.y+0.5f+fy*i);
    }
}

inline void EmitPrim(const State& st,D3DPRIMITIVETYPE pt,const Vert* v,DWORD n){
    if(RowanVkDraw(st,pt,v,n)) return;          // GPU path handled the batch
    RowanVkReadSync(st.rt);                     // land GPU content for software
    RowanVkReadSync(st.zb);                     //   and the depth values
    static const char* dumpR=getenv("ROWAN_DUMP");
    if(dumpR&&n>=3&&pt>=4){
        int x0=0,y0=0,x1=0,y1=0;
        if(sscanf(dumpR,"%d,%d,%d,%d",&x0,&y0,&x1,&y1)==4){
            float mnx=1e30f,mny=1e30f,mxx=-1e30f,mxy=-1e30f;
            for(DWORD i=0;i<n;i++){
                if(v[i].x<mnx)mnx=v[i].x;
                if(v[i].x>mxx)mxx=v[i].x;
                if(v[i].y<mny)mny=v[i].y;
                if(v[i].y>mxy)mxy=v[i].y;}
            if(mxx>x0&&mnx<x1&&mxy>y0&&mny<y1){
                IDirectDrawSurface7*t=st.tex[0].s;
                fprintf(stderr,"[PRIM] pt=%d n=%d x[%d..%d] y[%d..%d] cm=%u zb=%p tex0=%p %ux%u/%u col0=%08lx spec0=%08lx rhw0=%g z0=%g cop=%u ca1=%x ca2=%x aop=%u aa1=%x aa2=%x ablend=%u",
                    (int)pt,(int)n,(int)mnx,(int)mxx,(int)mny,(int)mxy,
                    (unsigned)st.rs[D3DRENDERSTATE_CULLMODE],st.zb,t,
                    t?(unsigned)t->desc.dwWidth:0,t?(unsigned)t->desc.dwHeight:0,
                    t?(unsigned)t->desc.ddpfPixelFormat.dwRGBBitCount:0,
                    (unsigned long)v[0].col,(unsigned long)v[0].spec,v[0].rhw,v[0].z,
                    (unsigned)st.ss[0][D3DTSS_COLOROP],
                    (unsigned)st.ss[0][D3DTSS_COLORARG1],
                    (unsigned)st.ss[0][D3DTSS_COLORARG2],
                    (unsigned)st.ss[0][D3DTSS_ALPHAOP],
                    (unsigned)st.ss[0][D3DTSS_ALPHAARG1],
                    (unsigned)st.ss[0][D3DTSS_ALPHAARG2],
                    (unsigned)st.rs[D3DRENDERSTATE_ALPHABLENDENABLE]);
                fprintf(stderr," rt=%p ze=%u zwe=%u zfn=%u",
                    (void*)st.rt,
                    (unsigned)st.rs[D3DRENDERSTATE_ZENABLE],
                    (unsigned)st.rs[D3DRENDERSTATE_ZWRITEENABLE],
                    (unsigned)st.rs[D3DRENDERSTATE_ZFUNC]);
                if(getenv("ROWAN_DUMPV"))
                    for(DWORD i=0;i<n;i++)
                        fprintf(stderr," v%u(%g,%g r%g z%g u%g v%g)",i,v[i].x,v[i].y,v[i].rhw,v[i].z,v[i].u[0],v[i].v[0]);
                if(getenv("ROWAN_DUMPTEX")&&t&&t->desc.lpSurface){
                    const Surf& ts=st.tex[0];
                    int tw=(int)t->desc.dwWidth, th=(int)t->desc.dwHeight;
                    int tbpp=(int)(t->desc.ddpfPixelFormat.dwRGBBitCount
                            ?t->desc.ddpfPixelFormat.dwRGBBitCount:32);
                    const BYTE* tsur=(const BYTE*)t->desc.lpSurface;
                    int tpit=(int)t->desc.lPitch;
                    for(DWORD i=0;i<n&&i<4;i++){
                        int tx=(int)(v[i].u[0]*tw), ty=(int)(v[i].v[0]*th);
                        if(tx<0)tx=0;
                        if(tx>=tw)tx=tw-1;
                        if(ty<0)ty=0;
                        if(ty>=th)ty=th-1;
                        const BYTE* r=tsur+ty*tpit;
                        DWORD raw=tbpp==16?(DWORD)((const uint16_t*)r)[tx]
                                 :tbpp==8?(DWORD)r[tx]:((const DWORD*)r)[tx];
                        fprintf(stderr," tex%u[%d,%d]=%08lx",i,tx,ty,
                            (unsigned long)UnpackPxS(t,ts.f,raw));
                    }
                    // black-row profile: fraction of zero/lum<8 texels per
                    // 16-row band across the prim's v range
                    float vmn=1e30f,vmx=-1e30f,umn=1e30f,umx=-1e30f;
                    for(DWORD i=0;i<n;i++){
                        if(v[i].v[0]<vmn)vmn=v[i].v[0];
                        if(v[i].v[0]>vmx)vmx=v[i].v[0];
                        if(v[i].u[0]<umn)umn=v[i].u[0];
                        if(v[i].u[0]>umx)umx=v[i].u[0];}
                    int y0=(int)(vmn*th), y1=(int)(vmx*th);
                    int x0=(int)(umn*tw), x1=(int)(umx*tw);
                    if(y0<0)y0=0;
                    if(y1>=th)y1=th-1;
                    if(x0<0)x0=0;
                    if(x1>=tw)x1=tw-1;
                    fprintf(stderr," rows:");
                    for(int y=y0;y<=y1;y+=8){
                        int dark=0,tot=0;
                        for(int x=x0;x<=x1;x+=4){
                            const BYTE* r=tsur+y*tpit;
                            DWORD raw=tbpp==16?(DWORD)((const uint16_t*)r)[x]
                                     :tbpp==8?(DWORD)r[x]:((const DWORD*)r)[x];
                            DWORD c=UnpackPxS(t,ts.f,raw);
                            int lum=((c>>16)&255)+((c>>8)&255)+(c&255);
                            tot++;
                            if(lum<24)dark++;
                        }
                        fprintf(stderr," %d:%d/%d",y,dark,tot);
                    }
                }
                fprintf(stderr,"\n");
            }
        }
    }
    if(pt==D3DPT_TRIANGLELIST){for(DWORD i=2;i<n;i+=3)
        if(!CullWinding(st,v[i-2],v[i-1],v[i]))DrawTri(st,v[i-2],v[i-1],v[i]);}
    else if(pt==D3DPT_TRIANGLESTRIP){for(DWORD i=2;i<n;i++){
        const Vert&a=v[i-2],&b=v[i-1],&c=v[i];
        if(i&1){if(!CullWinding(st,a,c,b))DrawTri(st,a,c,b);}
        else    {if(!CullWinding(st,a,b,c))DrawTri(st,a,b,c);}}}
    else if(pt==D3DPT_TRIANGLEFAN){for(DWORD i=2;i<n;i++)
        if(!CullWinding(st,v[0],v[i-1],v[i]))DrawTri(st,v[0],v[i-1],v[i]);}
    else if(pt==D3DPT_LINELIST){for(DWORD i=1;i<n;i+=2)DrawLine(st,v[i-1],v[i]);}
    else if(pt==D3DPT_LINESTRIP){for(DWORD i=1;i<n;i++)DrawLine(st,v[i-1],v[i]);}
    else if(pt==D3DPT_POINTLIST){for(DWORD i=0;i<n;i++){
        int x=(int)(v[i].x+0.5f),y=(int)(v[i].y+0.5f);
        if(st.rt&&x>=0&&y>=0&&x<(int)st.rt->desc.dwWidth&&y<(int)st.rt->desc.dwHeight){
            float c[IC_N]; Pack(v[i],c,false); PixE(st.env,x,y,c);
        }}}
    RowanVkWriteSync(st.rt);                    // CPU drew to the target
    RowanVkWriteSync(st.zb);                    //   and possibly the zbuffer
}

// --- FVF decode --------------------------------------------------------------
#ifndef D3DFVF_PSIZE
#define D3DFVF_PSIZE 0x020
#endif
#ifndef D3DFVF_XYZW
#define D3DFVF_XYZW  0x4002
#endif
inline DWORD FvfStride(DWORD f){
    DWORD n=0;
    DWORD pos=f&D3DFVF_POSITION_MASK;
    if(pos==D3DFVF_XYZRHW||pos==D3DFVF_XYZW) n+=4*4;
    else if(pos==D3DFVF_XYZ)    n+=3*4;
    else if(pos>=D3DFVF_XYZB1&&pos<=D3DFVF_XYZB5) n+=4*4+((pos-D3DFVF_XYZB1)>>1)*4;
    if(f&D3DFVF_NORMAL)   n+=3*4;
    if(f&D3DFVF_PSIZE)    n+=4;
    if(f&D3DFVF_DIFFUSE)  n+=4;
    if(f&D3DFVF_SPECULAR) n+=4;
    DWORD tc=(f&D3DFVF_TEXCOUNT_MASK)>>8;
    n+=tc*2*4;
    return n;
}
// Pull the TLVERTEX-relevant fields out of any FVF record.
inline void DecodeVert(DWORD fvf,const BYTE* p,Vert& v){
    memset(&v,0,sizeof(v)); v.col=v.spec=0xffffffff; v.rhw=1.0f;
    DWORD pos=fvf&D3DFVF_POSITION_MASK;
    if(pos==D3DFVF_XYZRHW||pos==D3DFVF_XYZW){
        v.x=((float*)p)[0]; v.y=((float*)p)[1]; v.z=((float*)p)[2]; v.rhw=((float*)p)[3];
        p+=16;
    } else if(pos==D3DFVF_XYZ){
        v.x=((float*)p)[0]; v.y=((float*)p)[1]; v.z=((float*)p)[2]; p+=12;
    } else if(pos>=D3DFVF_XYZB1&&pos<=D3DFVF_XYZB5){
        p+=16+((pos-D3DFVF_XYZB1)>>1)*4;           // skip pos+blend weights
    }
    if(fvf&D3DFVF_NORMAL)p+=12;
    if(fvf&D3DFVF_PSIZE)p+=4;
    if(fvf&D3DFVF_DIFFUSE){v.col=*(DWORD*)p;p+=4;}
    if(fvf&D3DFVF_SPECULAR){v.spec=*(DWORD*)p;p+=4;}
    DWORD tc=(fvf&D3DFVF_TEXCOUNT_MASK)>>8;
    for(DWORD i=0;i<tc&&i<3;i++){v.u[i]=((float*)p)[0];v.v[i]=((float*)p)[1];p+=8;}
    for(int i=0;i<3;i++)if(i>=(int)tc){v.u[i]=v.u[0];v.v[i]=v.v[0];}
}

} // namespace Rast7

// --- IDirect3DDevice7 out-of-line members ------------------------------------
static_assert(sizeof(Rast7::State)<=sizeof(((IDirect3DDevice7*)0)->rastMem),
              "Rast7::State outgrew IDirect3DDevice7::rastMem — bump the buffer");

inline HRESULT IDirect3D7::CreateDevice(REFCLSID, LPDIRECTDRAWSURFACE7 surf,
                                        LPDIRECT3DDEVICE7* out) {
    if(!out) return DDERR_INVALIDPARAMS;
    IDirect3DDevice7* dev=new IDirect3DDevice7;
    dev->renderTarget=surf; RowanSurf7AddRef(surf);
    *out=dev; return D3D_OK;
}

inline HRESULT IDirect3DDevice7::SetTexture(DWORD s, LPDIRECTDRAWSURFACE7 t) {
    if(s<8){
        RowanSurf7AddRef(t);RowanSurf7Release(textures[s]);textures[s]=t;
        rastStamp++;
        if(sbRec) recTex|=(BYTE)(1<<s);
    }
    return D3D_OK;
}
inline HRESULT IDirect3DDevice7::GetTexture(DWORD s, LPDIRECTDRAWSURFACE7* t) {
    if(!t)return DDERR_INVALIDPARAMS;
    *t=(s<8)?textures[s]:0; RowanSurf7AddRef(*t); return D3D_OK;
}
inline HRESULT IDirect3DDevice7::SetRenderTarget(LPDIRECTDRAWSURFACE7 t, DWORD) {
    RowanSurf7AddRef(t); RowanSurf7Release(renderTarget);
    renderTarget=t; rastStamp++; return D3D_OK;
}
inline HRESULT IDirect3DDevice7::GetRenderTarget(LPDIRECTDRAWSURFACE7* t) {
    if(!t)return DDERR_INVALIDPARAMS;
    *t=renderTarget; RowanSurf7AddRef(*t); return D3D_OK;
}

// ---- state blocks ----------------------------------------------------------
// PIXELSTATE captures the DX7 pixel render states (D3DHAL list), every
// texture-stage state, and the bound textures.  VERTEXSTATE captures the
// remaining render states only (transforms/lights/material are not tracked).
static const DWORD RowanPixRS[] = {
    7,8,9,10,14,15,16,19,20,23,24,25,26,27,33,36,37,38,40,47,
    52,53,54,55,56,57,58,59,60,128,129,130,131,132,133,134,135 };
inline void RowanSBlockFill(RowanSBlock* b, DWORD st, const RowanSBlock* cur) {
    memcpy(b,cur,sizeof(*b));
    memset(b->rsM,0,sizeof(b->rsM));
    memset(b->ssM,0,sizeof(b->ssM));
    b->texM=0;
    if(st==D3DSBT_ALL || st==D3DSBT_PIXELSTATE) {
        for(unsigned i=0;i<sizeof(RowanPixRS)/sizeof(*RowanPixRS);i++)
            b->rsM[RowanPixRS[i]>>3]|=(BYTE)(1<<(RowanPixRS[i]&7));
        memset(b->ssM,0xff,sizeof(b->ssM));
        b->texM=0xff;
    }
    if(st==D3DSBT_ALL || st==D3DSBT_VERTEXSTATE) {
        BYTE pix[32]; memset(pix,0,sizeof(pix));
        for(unsigned i=0;i<sizeof(RowanPixRS)/sizeof(*RowanPixRS);i++)
            pix[RowanPixRS[i]>>3]|=(BYTE)(1<<(RowanPixRS[i]&7));
        if(st==D3DSBT_ALL)
            memset(b->rsM,0xff,sizeof(b->rsM));
        else
            for(int i=0;i<32;i++) b->rsM[i]=(BYTE)~pix[i];
    }
}
inline HRESULT IDirect3DDevice7::CreateStateBlock(D3DSTATEBLOCKTYPE st, LPDWORD tok) {
    if(!tok) return DDERR_INVALIDPARAMS;
    if(st!=D3DSBT_ALL && st!=D3DSBT_PIXELSTATE && st!=D3DSBT_VERTEXSTATE)
        return DDERR_INVALIDPARAMS;
    RowanSBlock* b=new RowanSBlock;
    RowanSBlock cur={};
    memcpy(cur.rs,renderStates,sizeof(cur.rs));
    memcpy(cur.ss,stageStates,sizeof(cur.ss));
    memcpy(cur.tex,textures,sizeof(cur.tex));
    RowanSBlockFill(b,st,&cur);
    for(int i=0;i<8;i++) RowanSurf7AddRef(b->tex[i]);
    DWORD t=++stateBlockCount;
    sblocks[t]=b; *tok=t;
    return D3D_OK;
}
inline HRESULT IDirect3DDevice7::ApplyStateBlock(DWORD tok) {
    std::map<DWORD,RowanSBlock*>::iterator it=sblocks.find(tok);
    if(it==sblocks.end()) return DDERR_INVALIDPARAMS;
    RowanSBlock* b=it->second;
    for(int i=0;i<256;i++)
        if(b->rsM[i>>3]&(BYTE)(1<<(i&7))) renderStates[i]=b->rs[i];
    for(int s=0;s<8;s++) for(int i=0;i<64;i++)
        if(b->ssM[(s*64+i)>>3]&(BYTE)(1<<((s*64+i)&7))) stageStates[s][i]=b->ss[s][i];
    for(int s=0;s<8;s++) if(b->texM&(BYTE)(1<<s)) {
        RowanSurf7AddRef(b->tex[s]); RowanSurf7Release(textures[s]);
        textures[s]=b->tex[s];
    }
    rastStamp++;
    return D3D_OK;
}
inline HRESULT IDirect3DDevice7::CaptureStateBlock(DWORD tok) {
    std::map<DWORD,RowanSBlock*>::iterator it=sblocks.find(tok);
    if(it==sblocks.end()) return DDERR_INVALIDPARAMS;
    RowanSBlock* b=it->second;
    for(int i=0;i<256;i++)
        if(b->rsM[i>>3]&(BYTE)(1<<(i&7))) b->rs[i]=renderStates[i];
    for(int s=0;s<8;s++) for(int i=0;i<64;i++)
        if(b->ssM[(s*64+i)>>3]&(BYTE)(1<<((s*64+i)&7))) b->ss[s][i]=stageStates[s][i];
    for(int s=0;s<8;s++) if(b->texM&(BYTE)(1<<s)) {
        RowanSurf7AddRef(textures[s]); RowanSurf7Release(b->tex[s]);
        b->tex[s]=textures[s];
    }
    return D3D_OK;
}
inline HRESULT IDirect3DDevice7::DeleteStateBlock(DWORD tok) {
    std::map<DWORD,RowanSBlock*>::iterator it=sblocks.find(tok);
    if(it==sblocks.end()) return DDERR_INVALIDPARAMS;
    RowanSBlock* b=it->second;
    for(int i=0;i<8;i++) RowanSurf7Release(b->tex[i]);
    delete b; sblocks.erase(it);
    return D3D_OK;
}
inline HRESULT IDirect3DDevice7::BeginStateBlock() {
    if(sbRec) return DDERR_GENERIC;
    sbRec=true;
    memset(recRS,0,sizeof(recRS)); memset(recSS,0,sizeof(recSS)); recTex=0;
    return D3D_OK;
}
inline HRESULT IDirect3DDevice7::EndStateBlock(LPDWORD t) {
    if(!sbRec) return DDERR_GENERIC;
    sbRec=false;
    RowanSBlock* b=new RowanSBlock;
    memcpy(b->rs,renderStates,sizeof(b->rs));
    memcpy(b->ss,stageStates,sizeof(b->ss));
    memcpy(b->tex,textures,sizeof(b->tex));
    memcpy(b->rsM,recRS,sizeof(b->rsM));
    memcpy(b->ssM,recSS,sizeof(b->ssM));
    b->texM=recTex;
    for(int i=0;i<8;i++) RowanSurf7AddRef(b->tex[i]);
    DWORD tok=++stateBlockCount;
    sblocks[tok]=b;
    if(t) *t=tok;
    return D3D_OK;
}

inline void IDirect3DDevice7::SyncRast() {
    // Skip the ~3.6KB state copy + env rebuild when nothing that the pipeline
    // consults has changed (draw calls per poly make this the common case).
    IDirectDrawSurface7* att=renderTarget?renderTarget->attached:0;
    if(rast&&rastSynced==rastStamp&&rast->rt==renderTarget
        &&rast->zb==(att&&(att->desc.ddsCaps.dwCaps&DDSCAPS_ZBUFFER)?att:0))
        return;
    rastSynced=rastStamp;
    if(!rast){rast=(Rast7::State*)rastMem;memset(rast,0,sizeof(*rast));}
    rast->rt=renderTarget;
    rast->zb=0; rast->zbits=16;
    if(renderTarget){
        Rast7::DecodePF(renderTarget->desc.ddpfPixelFormat,rast->rtf);
        if(att && (att->desc.ddsCaps.dwCaps&DDSCAPS_ZBUFFER)){
            rast->zb=att; rast->zbits=Rast7::ZBits(att);
        }
    }
    memcpy(rast->rs,renderStates,sizeof(rast->rs));
    memcpy(rast->ss,stageStates,sizeof(rast->ss));
    for(int i=0;i<8;i++){
        rast->tex[i].s=textures[i];
        if(textures[i]){
            Rast7::DecodePF(textures[i]->desc.ddpfPixelFormat,rast->tex[i].f);
            rast->tex[i].bpp=textures[i]->desc.ddpfPixelFormat.dwRGBBitCount;
        }
    }
    rast->fogCol=renderStates[D3DRENDERSTATE_FOGCOLOR];
    memcpy(&rast->fogStart,&renderStates[D3DRENDERSTATE_FOGSTART],4);
    memcpy(&rast->fogEnd,&renderStates[D3DRENDERSTATE_FOGEND],4);
    Rast7::BuildEnv(*rast,rast->env);
}

inline HRESULT IDirect3DDevice7::EndScene() {
    inScene=false; RowanVkEndFrame(); return D3D_OK;
}

inline HRESULT IDirect3DDevice7::Clear(DWORD nRect,LPD3DRECT rects,DWORD flags,
                                       D3DCOLOR col,D3DVALUE z,DWORD) {
    if(!renderTarget) return D3D_OK;
    Rast7::PixFmt pf; Rast7::DecodePF(renderTarget->desc.ddpfPixelFormat,pf);
    IDirectDrawSurface7* zb=0; int zbits=16;
    IDirectDrawSurface7* a=renderTarget->attached;
    if(a&&(a->desc.ddsCaps.dwCaps&DDSCAPS_ZBUFFER)){zb=a;zbits=Rast7::ZBits(a);}
    if(RowanVkClear(renderTarget,zb,zbits,nRect,rects,flags,col,(float)z))
        return D3D_OK;
    RowanVkReadSync(renderTarget);      // land GPU content before CPU writes
    RowanVkReadSync(zb);                //   (rect clears may preserve depth)
    DWORD nv=nRect?nRect:1;
    D3DRECT full={0,0,(LONG)renderTarget->desc.dwWidth,(LONG)renderTarget->desc.dwHeight};
    LONG sw=(LONG)renderTarget->desc.dwWidth, sh=(LONG)renderTarget->desc.dwHeight;
    for(DWORD i=0;i<nv;i++){
        D3DRECT r=nRect?rects[i]:full;
        if(r.x1<0)r.x1=0;
        if(r.y1<0)r.y1=0;
        if(r.x2>sw)r.x2=sw;
        if(r.y2>sh)r.y2=sh;
        if((flags&D3DCLEAR_TARGET)&&renderTarget->desc.lpSurface){
            DWORD px=Rast7::Pack(col,pf);
            for(LONG y=r.y1;y<r.y2;y++)
                for(LONG x=r.x1;x<r.x2;x++)
                    Rast7::WritePx(renderTarget,x,y,px);
        }
        if((flags&D3DCLEAR_ZBUFFER)&&zb&&zb->desc.lpSurface){
            DWORD nz;
            if(renderStates[D3DRENDERSTATE_ZENABLE]==D3DZB_USEW){
                // W-buffer stores float bit patterns (see Pixel()); the
                // far sentinel scales FLT_MAX by the requested depth so
                // z=1 still clears to "farther than anything drawable".
                float w=z<0?0.0f:(z>=1?3.402823466e38f:3.402823466e38f*z);
                memcpy(&nz,&w,4);
                if(zbits<32)nz>>=(32-zbits);
            } else {
                DWORD zmax=(zbits>=32)?0xffffffffu:(zbits>=24?0xffffffu:0xffffu);
                float zf=z<0?0:z>1?1:z;
                nz=(DWORD)(zf*zmax);
            }
            for(LONG y=r.y1;y<r.y2;y++)
                for(LONG x=r.x1;x<r.x2;x++)
                    Rast7::WriteZ(zb,x,y,nz);
        }
    }
    if(flags&D3DCLEAR_TARGET) RowanVkWriteSync(renderTarget);
    if((flags&D3DCLEAR_ZBUFFER)&&zb) RowanVkWriteSync(zb);
    return D3D_OK;
}

inline HRESULT IDirect3DDevice7::DrawPrimitive(D3DPRIMITIVETYPE pt,DWORD fvf,
                                             LPVOID verts,DWORD n,DWORD) {
    if(!verts||!n) return D3D_OK;
    SyncRast();
    DWORD stride=Rast7::FvfStride(fvf);
    Rast7::Vert* dv=(Rast7::Vert*)alloca(n*sizeof(Rast7::Vert));
    for(DWORD i=0;i<n;i++) Rast7::DecodeVert(fvf,(const BYTE*)verts+i*stride,dv[i]);
    Rast7::EmitPrim(*rast,pt,dv,n);
    return D3D_OK;
}
inline HRESULT IDirect3DDevice7::DrawIndexedPrimitive(D3DPRIMITIVETYPE pt,DWORD fvf,
        LPVOID verts,DWORD nv,LPWORD idx,DWORD ni,DWORD) {
    if(!verts||!idx||!ni) return D3D_OK;
    SyncRast();
    DWORD stride=Rast7::FvfStride(fvf);
    Rast7::Vert* dv=(Rast7::Vert*)alloca(ni*sizeof(Rast7::Vert));
    for(DWORD i=0;i<ni;i++)
        Rast7::DecodeVert(fvf,(const BYTE*)verts+idx[i]*stride,dv[i]);
    Rast7::EmitPrim(*rast,pt,dv,ni);
    return D3D_OK;
}
inline HRESULT IDirect3DDevice7::DrawPrimitiveVB(D3DPRIMITIVETYPE pt,
        LPDIRECT3DVERTEXBUFFER7 vb,DWORD start,DWORD n,DWORD) {
    if(!vb||!vb->data||!n) return D3D_OK;
    SyncRast();
    DWORD stride=Rast7::FvfStride(vb->fvf);
    Rast7::Vert* dv=(Rast7::Vert*)alloca(n*sizeof(Rast7::Vert));
    for(DWORD i=0;i<n;i++)
        Rast7::DecodeVert(vb->fvf,vb->data+(start+i)*stride,dv[i]);
    Rast7::EmitPrim(*rast,pt,dv,n);
    return D3D_OK;
}
inline HRESULT IDirect3DDevice7::DrawIndexedPrimitiveVB(D3DPRIMITIVETYPE pt,
        LPDIRECT3DVERTEXBUFFER7 vb,DWORD start,DWORD nv,LPWORD idx,DWORD ni,DWORD) {
    if(!vb||!vb->data||!idx||!ni) return D3D_OK;
    SyncRast();
    DWORD stride=Rast7::FvfStride(vb->fvf);
    Rast7::Vert* dv=(Rast7::Vert*)alloca(ni*sizeof(Rast7::Vert));
    for(DWORD i=0;i<ni;i++)
        Rast7::DecodeVert(vb->fvf,vb->data+(start+idx[i])*stride,dv[i]);
    Rast7::EmitPrim(*rast,pt,dv,ni);
    return D3D_OK;
}

// --- format-converting blit (ddraw.h Blt/BltFast delegate here) -------------
// Real DD converts pixel formats during Blt; the fast path in ddraw.h only
// handles identical formats. Nearest-neighbour for stretches (mip upload and
// HUD sheets both take this path). Source colour-keyed blits compare the raw
// source pixel, same as the hardware.
inline HRESULT RowanBltFormat(IDirectDrawSurface7* dst, LPRECT dstR,
                              IDirectDrawSurface7* src, LPRECT srcR,
                              DWORD flags, LPDDBLTFX fx) {
    if(!dst||!src||!dst->desc.lpSurface||!src->desc.lpSurface) return DD_OK;
    LONG dx=dstR?dstR->left:0, dy=dstR?dstR->top:0;
    LONG sx0=srcR?srcR->left:0, sy0=srcR?srcR->top:0;
    DWORD srcW=srcR?(DWORD)(srcR->right-srcR->left):src->desc.dwWidth;
    DWORD srcH=srcR?(DWORD)(srcR->bottom-srcR->top):src->desc.dwHeight;
    DWORD dstW=dstR?(DWORD)(dstR->right-dstR->left):dst->desc.dwWidth;
    DWORD dstH=dstR?(DWORD)(dstR->bottom-dstR->top):dst->desc.dwHeight;
    if(!srcW||!srcH||!dstW||!dstH) return DD_OK;
    Rast7::PixFmt sf,df;
    Rast7::DecodePF(src->desc.ddpfPixelFormat,sf);
    Rast7::DecodePF(dst->desc.ddpfPixelFormat,df);
    Rast7::Surf ss{src,sf,sf.bits};
    DWORD klo=0,khi=0; bool keyed=false;
    if(flags&DDBLT_KEYSRCOVERRIDE && fx){
        klo=fx->ddckSrcColorkey.dwColorSpaceLowValue;
        khi=fx->ddckSrcColorkey.dwColorSpaceHighValue; keyed=true;
    } else if((flags&DDBLT_KEYSRC) && (src->ckeyFlags&DDCKEY_SRCBLT)){
        klo=src->ckey.dwColorSpaceLowValue; khi=src->ckey.dwColorSpaceHighValue;
        keyed=true;
    }
    DWORD sbits=src->desc.ddpfPixelFormat.dwRGBBitCount?src->desc.ddpfPixelFormat.dwRGBBitCount:32;
    DWORD smask=sbits>=32?0xffffffffu:((1u<<sbits)-1);
    if(getenv("ROWAN_DUMPBLT")&&dy>=560)
        fprintf(stderr,"[BLT] dst=(%d,%d %ux%u) src=(%d,%d %ux%u) srcsurf=%ux%u/%u dstsurf=%ux%u/%u key=%u\n",
            (int)dx,(int)dy,(unsigned)dstW,(unsigned)dstH,(int)sx0,(int)sy0,(unsigned)srcW,(unsigned)srcH,
            (unsigned)src->desc.dwWidth,(unsigned)src->desc.dwHeight,(unsigned)sbits,
            (unsigned)dst->desc.dwWidth,(unsigned)dst->desc.dwHeight,
            (unsigned)dst->desc.ddpfPixelFormat.dwRGBBitCount,(unsigned)keyed);
    for(DWORD y=0;y<dstH;y++){
        LONG ddy=dy+(LONG)y;
        if(ddy<0||ddy>=(LONG)dst->desc.dwHeight) continue;
        DWORD sy=(DWORD)sy0+y*srcH/dstH;
        if(sy>=src->desc.dwHeight) continue;
        for(DWORD x=0;x<dstW;x++){
            LONG ddx=dx+(LONG)x;
            if(ddx<0||ddx>=(LONG)dst->desc.dwWidth) continue;
            DWORD sx=(DWORD)sx0+x*srcW/dstW;
            if(sx>=src->desc.dwWidth) continue;
            DWORD raw=Rast7::ReadPx(src,sx,sy);
            if(keyed){DWORD v=raw&smask;
            if(v>=klo&&v<=khi)continue;}
            DWORD c=Rast7::UnpackPx(ss,raw);
            Rast7::WritePx(dst,(DWORD)ddx,(DWORD)ddy,Rast7::Pack(c,df));
        }
    }
    return DD_OK;
}
#pragma pack(pop)
#endif // ROWAN_COMPAT_D3DSOFT_H
