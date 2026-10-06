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

namespace Rast7 {

struct PixFmt {                      // unpack/pack a surface pixel via masks
    DWORD bits, rM, gM, bM, aM;
    int   rS, gS, bS, aS, rW, gW, bW, aW;   // shift + field width
};
inline int lowbit(DWORD m){int s=0;if(m)while(!(m&1)){m>>=1;s++;}return s;}
inline int nbits (DWORD m){int n=0;while(m){n+=m&1;m>>=1;}return n;}
inline void DecodePF(const DDPIXELFORMAT& p, PixFmt& f){
    f.rM=p.dwRBitMask; f.gM=p.dwGBitMask; f.bM=p.dwBBitMask; f.aM=p.dwRGBAlphaBitMask;
    f.bits=p.dwRGBBitCount?p.dwRGBBitCount:32;
    f.rS=lowbit(f.rM); f.gS=lowbit(f.gM); f.bS=lowbit(f.bM); f.aS=lowbit(f.aM);
    f.rW=nbits(f.rM);  f.gW=nbits(f.gM);  f.bW=nbits(f.bM);  f.aW=nbits(f.aM);
}
inline DWORD scaleUp(DWORD v,int w){ if(!w)return 255; v&=((1u<<w)-1);
    return w>=8 ? (v>>(w-8)) : (v*255)/((1u<<w)-1); }
inline DWORD scaleDn(DWORD v,int w){ if(!w)return 0;
    return w>=8 ? (v<<(w-8)) : (v*((1u<<w)-1)+127)/255; }
inline DWORD Unpack(DWORD px, const PixFmt& f){ // -> 0xAARRGGBB
    return (scaleUp(px&f.rM? (px&f.rM)>>f.rS:0, f.rW)<<16)
         | (scaleUp(px&f.gM? (px&f.gM)>>f.gS:0, f.gW)<<8)
         | (scaleUp(px&f.bM? (px&f.bM)>>f.bS:0, f.bW))
         | ((f.aM? scaleUp((px&f.aM)>>f.aS,f.aW):255)<<24);
}
inline DWORD Pack(DWORD c, const PixFmt& f){
    return (scaleDn((c>>16)&255,f.rW)<<f.rS)|(scaleDn((c>>8)&255,f.gW)<<f.gS)
         | (scaleDn(c&255,f.bW)<<f.bS)|((f.aM?scaleDn((c>>24)&255,f.aW):0)<<f.aS);
}
inline DWORD ReadPx(const IDirectDrawSurface7* s,DWORD x,DWORD y){
    const BYTE*p=(const BYTE*)s->desc.lpSurface+y*s->desc.lPitch;
    DWORD b=s->desc.ddpfPixelFormat.dwRGBBitCount? s->desc.ddpfPixelFormat.dwRGBBitCount:32;
    return b==16?((const uint16_t*)p)[x]: b==8?p[x]:((const DWORD*)p)[x];
}
inline void WritePx(IDirectDrawSurface7* s,DWORD x,DWORD y,DWORD v){
    BYTE*p=(BYTE*)s->desc.lpSurface+y*s->desc.lPitch;
    DWORD b=s->desc.ddpfPixelFormat.dwRGBBitCount? s->desc.ddpfPixelFormat.dwRGBBitCount:32;
    if(b==16)((uint16_t*)p)[x]=(uint16_t)v; else if(b==8)p[x]=(uint8_t)v; else ((DWORD*)p)[x]=v;
}
// z-buffer access — the z surface's ZBufferBitDepth drives width.
inline int ZBits(const IDirectDrawSurface7* z){
    DWORD d=z->desc.ddpfPixelFormat.dwZBufferBitDepth;
    if(!d)d=z->desc.ddpfPixelFormat.dwRGBBitCount; return d?(int)d:16;
}
inline DWORD ReadZ(const IDirectDrawSurface7* z,DWORD x,DWORD y){
    const BYTE*p=(const BYTE*)z->desc.lpSurface+y*z->desc.lPitch;
    return ZBits(z)>=32?((const DWORD*)p)[x]: ZBits(z)>=24?((const DWORD*)p)[x]&0xffffff
         :((const uint16_t*)p)[x];
}
inline void WriteZ(IDirectDrawSurface7* z,DWORD x,DWORD y,DWORD v){
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

struct State {
    IDirectDrawSurface7* rt; PixFmt rtf;
    IDirectDrawSurface7* zb; int zbits;
    Surf tex[8];
    DWORD rs[256]; DWORD ss[8][64];
    DWORD fogCol; float fogStart,fogEnd;
    DWORD clipX0,clipY0,clipX1,clipY1;
};

// --- texture sampling -------------------------------------------------------
// 8bpp surfaces are palettized: the index means nothing until SetPalette's
// entries resolve it.
inline DWORD UnpackPx(const Surf& t,DWORD raw){
    if(t.s->desc.ddpfPixelFormat.dwRGBBitCount==8 && t.s->pal){
        const PALETTEENTRY& e=t.s->pal->entries[raw&255];
        return 0xff000000u|((DWORD)e.peRed<<16)|((DWORD)e.peGreen<<8)|e.peBlue;
    }
    return Unpack(raw,t.f);
}
inline DWORD SampleTex(const Surf& t,float u,float v,DWORD addrMode,bool bilinear){
    if(!t.s||!t.s->desc.lpSurface)return 0xffffffff;
    int W=(int)t.s->desc.dwWidth,H=(int)t.s->desc.dwHeight;
    // Point sampling takes floor(u*W); the -0.5 shift is only for the
    // bilinear weight basis (texel centers at (i+0.5)/W).
    float fx=bilinear? u*W-0.5f : u*W;
    float fy=bilinear? v*H-0.5f : v*H;
    int x0=(int)floorf(fx), y0=(int)floorf(fy);
    auto wrap=[&](int c,int m)->int{
        if(addrMode==D3DTADDRESS_CLAMP){return c<0?0:c>=m?m-1:c;}
        if(addrMode==D3DTADDRESS_MIRROR){int p=2*m;c=((c%p)+p)%p;return c>=m?p-1-c:c;}
        int r=c%m; return r<0?r+m:r;                       // WRAP default
    };
    DWORD xA=(DWORD)wrap(x0,W), yA=(DWORD)wrap(y0,H);
    DWORD c00=UnpackPx(t,ReadPx(t.s,xA,yA));
    if(!bilinear)return c00;
    DWORD xB=(DWORD)wrap(x0+1,W), yB=(DWORD)wrap(y0+1,H);
    DWORD c10=UnpackPx(t,ReadPx(t.s,xB,yA)),
          c01=UnpackPx(t,ReadPx(t.s,xA,yB)),
          c11=UnpackPx(t,ReadPx(t.s,xB,yB));
    int wx=(int)((fx-floorf(fx))*256), wy=(int)((fy-floorf(fy))*256);
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
inline DWORD Arg(DWORD ta,DWORD tex,DWORD cur,DWORD dif,DWORD tfac){
    switch(ta&D3DTA_SELECTMASK){
        case D3DTA_DIFFUSE: return dif;
        case D3DTA_TEXTURE: return tex;
        case D3DTA_TFACTOR: return tfac;
        case D3DTA_CURRENT:
        default:            return cur;
    }
}
inline DWORD Complement(DWORD ta,DWORD v){ return (ta&D3DTA_COMPLEMENT)?v^0x00ffffff:v; }
inline DWORD AlphaRep (DWORD ta,DWORD v){
    if(ta&D3DTA_ALPHAREPLICATE){DWORD a=v>>24;return a|a<<8|a<<16;}
    return v;
}
inline DWORD BlendOp(DWORD op,DWORD a1,DWORD a2){
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
    if(R<0)R=0;if(R>255)R=255;if(G<0)G=0;if(G>255)G=255;if(B<0)B=0;if(B>255)B=255;
    return (R<<16)|(G<<8)|B;
}
inline DWORD BlendOpAlpha(DWORD op,DWORD a1,DWORD a2){
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
inline StageRes EvalStages(const State& st,const Vert& pv){
    DWORD cur=pv.col;
    DWORD dif=pv.col;
    DWORD tf=st.rs[D3DRENDERSTATE_TEXTUREFACTOR];
    for(int i=0;i<3;i++){
        DWORD cop=st.ss[i][D3DTSS_COLOROP];
        DWORD aop=st.ss[i][D3DTSS_ALPHAOP];
        if(cop==D3DTOP_DISABLE&&aop==D3DTOP_DISABLE)break;
        DWORD texC=0xffffffff;
        if(st.tex[i].s){
            DWORD addr=st.ss[i][D3DTSS_ADDRESS]; if(!addr)addr=D3DTADDRESS_WRAP;
            bool bilin = st.ss[i][D3DTSS_MAGFILTER]==D3DTFG_LINEAR
                      || st.ss[i][D3DTSS_MINFILTER]==D3DTFG_LINEAR
                      || st.ss[i][D3DTSS_MAGFILTER]==D3DTFG_ANISOTROPIC
                      || st.ss[i][D3DTSS_MINFILTER]==D3DTFG_ANISOTROPIC;
            int ti=(int)(st.ss[i][D3DTSS_TEXCOORDINDEX]&3); if(ti>2)ti=2;
            texC=SampleTex(st.tex[i],pv.u[ti],pv.v[ti],addr,bilin);
            // Source color key: keyed texels come out transparent.  The key
            // range applies to the unpacked RGB (matches how real drivers
            // key on the texel value for a single-color key).
            if(st.rs[D3DRENDERSTATE_COLORKEYENABLE] &&
               (st.tex[i].s->ckeyFlags&DDCKEY_SRCBLT)){
                DWORD lo=st.tex[i].s->ckey.dwColorSpaceLowValue&0xffffff;
                DWORD hi=st.tex[i].s->ckey.dwColorSpaceHighValue&0xffffff;
                if((texC&0xffffff)>=lo&&(texC&0xffffff)<=hi)texC&=0x00ffffff;
            }
        }
        if(cop==D3DTOP_BLENDTEXTUREALPHA){
            DWORD tA=(texC>>24)&255;
            DWORD a1=Arg(st.ss[i][D3DTSS_COLORARG1],texC,cur,dif,tf);
            DWORD a2=Arg(st.ss[i][D3DTSS_COLORARG2],texC,cur,dif,tf);
            DWORD lo=((a1&0xffffff)*(255-tA)+(a2&0xffffff)*tA)/255;
            cur=(cur&0xff000000)|(lo&0xffffff);
        } else if(cop!=D3DTOP_DISABLE){
            DWORD a1=AlphaRep(st.ss[i][D3DTSS_COLORARG1],
                       Complement(st.ss[i][D3DTSS_COLORARG1],
                       Arg(st.ss[i][D3DTSS_COLORARG1],texC,cur,dif,tf)));
            DWORD a2=AlphaRep(st.ss[i][D3DTSS_COLORARG2],
                       Complement(st.ss[i][D3DTSS_COLORARG2],
                       Arg(st.ss[i][D3DTSS_COLORARG2],texC,cur,dif,tf)));
            cur=(cur&0xff000000)|BlendOp(cop,a1,a2);
        }
        if(aop!=D3DTOP_DISABLE){
            DWORD a1=Arg(st.ss[i][D3DTSS_ALPHAARG1],texC,cur,dif,tf);
            DWORD a2=Arg(st.ss[i][D3DTSS_ALPHAARG2],texC,cur,dif,tf);
            cur=(cur&0x00ffffff)|BlendOpAlpha(aop,a1,a2);
        }
    }
    StageRes r; r.col=cur; r.alp=cur>>24; return r;
}

// --- z / blend / fog ---------------------------------------------------------
inline bool ZTest(DWORD fn,DWORD nv,DWORD ov){
    switch(fn){case D3DCMP_NEVER:return false;case D3DCMP_LESS:return nv<ov;
        case D3DCMP_EQUAL:return nv==ov;case D3DCMP_LESSEQUAL:return nv<=ov;
        case D3DCMP_GREATER:return nv>ov;case D3DCMP_NOTEQUAL:return nv!=ov;
        case D3DCMP_GREATEREQUAL:return nv>=ov;default:return true;}
}
inline DWORD BlendChan(DWORD src,DWORD dst,DWORD blend,int sh){
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
inline void Pack(const Vert& v,float* c,bool persp){
    float w=v.rhw!=0?v.rhw:1.0f;                 // persp stores u*rhw
    c[IC_Z]=v.z; c[IC_RHW]=v.rhw;
    c[IC_CR]=(v.col>>16)&255; c[IC_CG]=(v.col>>8)&255;
    c[IC_CB]=v.col&255; c[IC_CA]=(v.col>>24)&255;
    c[IC_SR]=(v.spec>>16)&255; c[IC_SG]=(v.spec>>8)&255;
    c[IC_SB]=v.spec&255; c[IC_SA]=(v.spec>>24)&255;
    for(int i=0;i<3;i++){c[IC_U0+i*2]=persp?v.u[i]*w:v.u[i];
                         c[IC_V0+i*2]=persp?v.v[i]*w:v.v[i];}
}
inline void Pixel(const State& st,DWORD x,DWORD y,const float* c){
    Vert pv;
    pv.z=c[IC_Z]; pv.rhw=c[IC_RHW];
    float w=pv.rhw!=0?1.0f/pv.rhw:1.0f;
    bool persp=st.rs[D3DRENDERSTATE_TEXTUREPERSPECTIVE]!=0;
    for(int i=0;i<3;i++){pv.u[i]=persp?c[IC_U0+i*2]*w:c[IC_U0+i*2];
                         pv.v[i]=persp?c[IC_V0+i*2]*w:c[IC_V0+i*2];}
    int R=(int)c[IC_CR],G=(int)c[IC_CG],B=(int)c[IC_CB],A=(int)c[IC_CA];
    pv.col=((A<0?0:A>255?255:A)<<24)|((R<0?0:R>255?255:R)<<16)
          |((G<0?0:G>255?255:G)<<8)|(B<0?0:B>255?255:B);
    int sR=(int)c[IC_SR],sG=(int)c[IC_SG],sB=(int)c[IC_SB],sA=(int)c[IC_SA];
    pv.spec=((sA<0?0:sA>255?255:sA)<<24)|((sR<0?0:sR>255?255:sR)<<16)
           |((sG<0?0:sG>255?255:sG)<<8)|(sB<0?0:sB>255?255:sB);

    // z test (against the attached z surface; z values 0..1 scaled to width)
    DWORD ze=st.rs[D3DRENDERSTATE_ZENABLE];
    if(ze&&st.zb){
        DWORD zmax=(st.zbits>=32)?0xffffffffu:(st.zbits>=24?0xffffffu:0xffffu);
        float zf=pv.z<0?0:pv.z>1?1:pv.z;
        DWORD nz=(DWORD)(zf*zmax)+(st.rs[D3DRENDERSTATE_ZBIAS]<<(st.zbits-8>0?st.zbits-8:0));
        DWORD fn=st.rs[D3DRENDERSTATE_ZFUNC]; if(!fn)fn=D3DCMP_LESSEQUAL;
        if(!ZTest(fn,nz,ReadZ(st.zb,x,y)))return;
        if(st.rs[D3DRENDERSTATE_ZWRITEENABLE])WriteZ(st.zb,x,y,nz);
    }
    StageRes sr=EvalStages(st,pv);
    DWORD out=sr.col;
    // specular add (post-texture, DX7 SPECULARENABLE)
    if(st.rs[D3DRENDERSTATE_SPECULARENABLE]){
        int a=out>>24;
        int r=((out>>16)&255)+((pv.spec>>16)&255);
        int g=((out>>8)&255)+((pv.spec>>8)&255);
        int b=(out&255)+(pv.spec&255);
        out=(DWORD)a<<24|(DWORD)(r>255?255:r)<<16|(DWORD)(g>255?255:g)<<8|(DWORD)(b>255?255:b);
    }
    // fog: vertex fog factor lives in specular alpha; table fog derives from z
    if(st.rs[D3DRENDERSTATE_FOGENABLE]){
        float f=-1;
        if(st.rs[D3DRENDERSTATE_FOGTABLEMODE]==D3DFOG_LINEAR){
            float fs,stE; memcpy(&fs,&st.rs[D3DRENDERSTATE_FOGSTART],4);
            memcpy(&stE,&st.rs[D3DRENDERSTATE_FOGEND],4);
            f=(stE-fs)!=0?(stE-pv.z)/(stE-fs):0;
        } else if(st.rs[D3DRENDERSTATE_FOGVERTEXMODE]==D3DFOG_LINEAR)
            f=((pv.spec>>24)&255)/255.0f;
        if(f>=0){
            if(f>1)f=1;
            DWORD fc=st.fogCol;
            int fr=(fc>>16)&255,fg=(fc>>8)&255,fb=fc&255;
            int a=out>>24;
            int r=(int)(f*((out>>16)&255)+(1-f)*fr);
            int g=(int)(f*((out>>8)&255)+(1-f)*fg);
            int b=(int)(f*(out&255)+(1-f)*fb);
            out=(DWORD)a<<24|(DWORD)r<<16|(DWORD)g<<8|(DWORD)b;
        }
    }
    if(st.rs[D3DRENDERSTATE_ALPHABLENDENABLE]){
        DWORD dst=Unpack(ReadPx(st.rt,x,y),st.rtf);
        DWORD sb=st.rs[D3DRENDERSTATE_SRCBLEND],db=st.rs[D3DRENDERSTATE_DESTBLEND];
        if(!sb)sb=D3DBLEND_ONE;
        DWORD res=0;
        for(int i=0;i<4;i++){
            DWORD sc=(out>>(i*8))&255,dc=(dst>>(i*8))&255;
            DWORD r=(sc*BlendChan(out,dst,sb,i*8)+dc*BlendChan(out,dst,db,i*8))/255;
            res|=(r>255?255:r)<<(i*8);
        }
        out=(out&0xff000000)|res;           // keep src alpha in pixel write
    }
    WritePx(st.rt,x,y,Pack(out,st.rtf));
}

// --- scan converter ----------------------------------------------------------
inline bool CullWinding(const State& st,const Vert&v0,const Vert&v1,const Vert&v2){
    DWORD cm=st.rs[D3DRENDERSTATE_CULLMODE];
    if(cm==D3DCULL_NONE||cm==0)return false;
    // Screen space is y-down: visually-clockwise windings have positive
    // signed area.  DX7 culls those under D3DCULL_CW, negative under CCW.
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
    float x0=A.x<B.x?(A.x<C.x?A.x:C.x):(B.x<C.x?B.x:C.x);
    float x1=A.x>B.x?(A.x>C.x?A.x:C.x):(B.x>C.x?B.x:C.x);
    float y0=A.y<B.y?(A.y<C.y?A.y:C.y):(B.y<C.y?B.y:C.y);
    float y1=A.y>B.y?(A.y>C.y?A.y:C.y):(B.y>C.y?B.y:C.y);
    int ix0=x0<0?0:(int)x0, ix1=x1>=(float)W?(int)W-1:(int)x1;
    int iy0=y0<0?0:(int)y0, iy1=y1>=(float)H?(int)H-1:(int)y1;
    // half-space edge functions E(P)=(p2.x-p1.x)*(P.y-p1.y)-(p2.y-p1.y)*(P.x-p1.x)
    // for directed edges B->C, C->A, A->B; interior iff all >=0 (winding-signed)
    float sgn=d>0?1.f:-1.f;
    float cur[IC_N];
    for(int y=iy0;y<=iy1;y++){
        float py=y+0.5f;
        for(int i=0;i<IC_N;i++)
            cur[i]=c0[i]+dx[i]*(ix0+0.5f-A.x)+dy[i]*(py-A.y);
        for(int x=ix0;x<=ix1;x++){
            float px=x+0.5f;
            float w0=((C.x-B.x)*(py-B.y)-(C.y-B.y)*(px-B.x))*sgn;
            float w1=((A.x-C.x)*(py-C.y)-(A.y-C.y)*(px-C.x))*sgn;
            float w2=((B.x-A.x)*(py-A.y)-(B.y-A.y)*(px-A.y))*sgn;
            if(w0>=0&&w1>=0&&w2>=0)Pixel(st,x,y,cur);
            for(int i=0;i<IC_N;i++)cur[i]+=dx[i];
        }
    }
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
            Pixel(st,x,y,c);
        for(int j=0;j<IC_N;j++)c[j]+=cb[j];
        x=(int)(A.x+0.5f+fx*i); y=(int)(A.y+0.5f+fy*i);
    }
}

inline void EmitPrim(const State& st,D3DPRIMITIVETYPE pt,const Vert* v,DWORD n){
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
            float c[IC_N]; Pack(v[i],c,false); Pixel(st,x,y,c);
        }}}
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
    if(s<8){RowanSurf7AddRef(t);RowanSurf7Release(textures[s]);textures[s]=t;}
    return D3D_OK;
}
inline HRESULT IDirect3DDevice7::GetTexture(DWORD s, LPDIRECTDRAWSURFACE7* t) {
    if(!t)return DDERR_INVALIDPARAMS;
    *t=(s<8)?textures[s]:0; RowanSurf7AddRef(*t); return D3D_OK;
}
inline HRESULT IDirect3DDevice7::SetRenderTarget(LPDIRECTDRAWSURFACE7 t, DWORD) {
    RowanSurf7AddRef(t); RowanSurf7Release(renderTarget);
    renderTarget=t; return D3D_OK;
}
inline HRESULT IDirect3DDevice7::GetRenderTarget(LPDIRECTDRAWSURFACE7* t) {
    if(!t)return DDERR_INVALIDPARAMS;
    *t=renderTarget; RowanSurf7AddRef(*t); return D3D_OK;
}

inline void IDirect3DDevice7::SyncRast() {
    if(!rast){rast=(Rast7::State*)rastMem;memset(rast,0,sizeof(*rast));}
    rast->rt=renderTarget;
    rast->zb=0; rast->zbits=16;
    if(renderTarget){
        Rast7::DecodePF(renderTarget->desc.ddpfPixelFormat,rast->rtf);
        IDirectDrawSurface7* a=renderTarget->attached;
        if(a && (a->desc.ddsCaps.dwCaps&DDSCAPS_ZBUFFER)){
            rast->zb=a; rast->zbits=Rast7::ZBits(a);
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
}

inline HRESULT IDirect3DDevice7::Clear(DWORD nRect,LPD3DRECT rects,DWORD flags,
                                       D3DCOLOR col,D3DVALUE z,DWORD) {
    if(!renderTarget) return D3D_OK;
    Rast7::PixFmt pf; Rast7::DecodePF(renderTarget->desc.ddpfPixelFormat,pf);
    IDirectDrawSurface7* zb=0; int zbits=16;
    IDirectDrawSurface7* a=renderTarget->attached;
    if(a&&(a->desc.ddsCaps.dwCaps&DDSCAPS_ZBUFFER)){zb=a;zbits=Rast7::ZBits(a);}
    DWORD nv=nRect?nRect:1;
    D3DRECT full={0,0,(LONG)renderTarget->desc.dwWidth,(LONG)renderTarget->desc.dwHeight};
    LONG sw=(LONG)renderTarget->desc.dwWidth, sh=(LONG)renderTarget->desc.dwHeight;
    for(DWORD i=0;i<nv;i++){
        D3DRECT r=nRect?rects[i]:full;
        if(r.x1<0)r.x1=0; if(r.y1<0)r.y1=0;
        if(r.x2>sw)r.x2=sw; if(r.y2>sh)r.y2=sh;
        if((flags&D3DCLEAR_TARGET)&&renderTarget->desc.lpSurface){
            DWORD px=Rast7::Pack(col,pf);
            for(LONG y=r.y1;y<r.y2;y++)
                for(LONG x=r.x1;x<r.x2;x++)
                    Rast7::WritePx(renderTarget,x,y,px);
        }
        if((flags&D3DCLEAR_ZBUFFER)&&zb&&zb->desc.lpSurface){
            DWORD zmax=(zbits>=32)?0xffffffffu:(zbits>=24?0xffffffu:0xffffu);
            float zf=z<0?0:z>1?1:z;
            DWORD nz=(DWORD)(zf*zmax);
            for(LONG y=r.y1;y<r.y2;y++)
                for(LONG x=r.x1;x<r.x2;x++)
                    Rast7::WriteZ(zb,x,y,nz);
        }
    }
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
#pragma pack(pop)
#endif // ROWAN_COMPAT_D3DSOFT_H
