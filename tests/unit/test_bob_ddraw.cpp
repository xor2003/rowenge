// BoB DirectDraw/Direct3D compat tests (T123): exercise the real rasterizer
// over malloc-backed surfaces — the same structs bob's lib3d drives in the
// NO_HARD_TNL path (pre-lit D3DTLVERTEX screen-space geometry).
//
// Surface chain under test:
//   DirectDrawCreateEx -> IDirectDraw7 -> SetDisplayMode(640x480x16)
//   -> CreateSurface(PRIMARY|FLIP|COMPLEX, backbuffer=1)   [primary+attached]
//   -> CreateSurface(ZBUFFER) + AddAttachedSurface
//   -> QueryInterface(IID_IDirect3D7) -> CreateDevice -> SetRenderTarget(back)
//   -> Clear / DrawPrimitive* -> primary->Flip()           [copies + presents]
#include "../harness.h"
#include <windows.h>
#include <ddraw.h>
#include <d3d.h>

namespace {

struct Frame {
    IDirectDraw7* dd;
    IDirect3D7* d3d;
    IDirectDrawSurface7* primary;   // front buffer
    IDirectDrawSurface7* back;      // attached back buffer = render target
    IDirectDrawSurface7* zbuf;
    IDirect3DDevice7* dev;

    Frame() : dd(0),d3d(0),primary(0),back(0),zbuf(0),dev(0) {}
    bool Init(DWORD w,DWORD h,DWORD bpp) {
        if(DirectDrawCreateEx(0,(LPVOID*)&dd,IID_IDirectDraw7,0)!=DD_OK||!dd)
            return false;
        dd->SetCooperativeLevel(0,DDSCL_FULLSCREEN|DDSCL_EXCLUSIVE);
        if(dd->SetDisplayMode(w,h,bpp,0,0)!=DD_OK) return false;

        DDSURFACEDESC2 d; memset(&d,0,sizeof(d)); d.dwSize=sizeof(d);
        d.dwFlags=DDSD_CAPS|DDSD_BACKBUFFERCOUNT;
        d.ddsCaps.dwCaps=DDSCAPS_PRIMARYSURFACE|DDSCAPS_FLIP|DDSCAPS_COMPLEX
                        |DDSCAPS_3DDEVICE;
        d.dwBackBufferCount=1;
        if(dd->CreateSurface(&d,&primary,0)!=DD_OK||!primary) return false;
        DDSCAPS2 caps; memset(&caps,0,sizeof(caps));
        caps.dwCaps=DDSCAPS_BACKBUFFER;
        if(primary->GetAttachedSurface(&caps,&back)!=DD_OK||!back) return false;

        memset(&d,0,sizeof(d)); d.dwSize=sizeof(d);
        d.dwFlags=DDSD_CAPS|DDSD_WIDTH|DDSD_HEIGHT;
        d.ddsCaps.dwCaps=DDSCAPS_ZBUFFER;
        d.dwWidth=w; d.dwHeight=h;
        if(dd->CreateSurface(&d,&zbuf,0)!=DD_OK||!zbuf) return false;
        if(back->AddAttachedSurface(zbuf)!=DD_OK) return false;

        if(dd->QueryInterface(IID_IDirect3D7,(LPVOID*)&d3d)!=DD_OK||!d3d)
            return false;
        if(d3d->CreateDevice(IID_IDirect3DHALDevice,back,&dev)!=DD_OK||!dev)
            return false;
        if(dev->SetRenderTarget(back,0)!=D3D_OK) return false;
        return true;
    }
    ~Frame() {
        if(dev)dev->Release();
        if(d3d)d3d->Release();
        if(zbuf)zbuf->Release();
        if(back)back->Release();
        if(primary)primary->Release();
        if(dd)dd->Release();
    }
};

// 16-bit surface access helpers (all tests run a 565 target).
inline uint16_t* Px(IDirectDrawSurface7* s,DWORD x,DWORD y)
    { return (uint16_t*)((BYTE*)s->desc.lpSurface+y*s->desc.lPitch)+x; }

inline D3DTLVERTEX TL(float x,float y,float z,float rhw,D3DCOLOR c,
                      float u=0,float v=0) {
    D3DTLVERTEX t;
    t.sx=x; t.sy=y; t.sz=z; t.rhw=rhw;
    t.color=c; t.specular=0xff000000;
    t.tu=u; t.tv=v;
    return t;
}

HRESULT DrawTris(IDirect3DDevice7* dev,D3DPRIMITIVETYPE pt,
                 D3DTLVERTEX* v,DWORD n) {
    dev->BeginScene();
    HRESULT hr=dev->DrawPrimitive(pt,D3DFVF_TLVERTEX,v,n,0);
    dev->EndScene();
    return hr;
}

} // namespace

// --- surface chain ------------------------------------------------------------
REGISTER_TEST(test_bob_ddraw_surface_chain) {
    Frame f;
    CHECK(f.Init(640,480,16));
    CHECK_EQ(f.primary->desc.dwWidth,640u);       // inherited from mode
    CHECK_EQ(f.primary->desc.dwHeight,480u);
    CHECK_EQ(f.primary->desc.ddpfPixelFormat.dwRGBBitCount,16u);
    CHECK(f.primary->IsPrimary());
    CHECK_EQ(f.back->desc.dwWidth,640u);          // attached back buffer made
    CHECK_EQ(f.back->desc.ddpfPixelFormat.dwRBitMask,0xf800u);
    CHECK_EQ(f.back->attached,(IDirectDrawSurface7*)f.zbuf);
    CHECK_EQ(Rast7::ZBits(f.zbuf),16);
    // Lock hands back real memory.
    DDSURFACEDESC2 ld;
    CHECK_EQ(f.back->Lock(0,&ld,0,0),DD_OK);
    CHECK(ld.lpSurface!=0);
    f.back->Unlock(0);
}

// --- Clear: target + zbuffer ---------------------------------------------------
REGISTER_TEST(test_bob_ddraw_clear) {
    Frame f;
    CHECK(f.Init(320,240,16));
    f.dev->Clear(0,0,D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,0x00ff0000,1.0f,0);
    CHECK_EQ(*Px(f.back,0,0),0xf800);             // red packed to 565
    CHECK_EQ(*Px(f.back,319,239),0xf800);
    CHECK_EQ(Rast7::ReadZ(f.zbuf,0,0),0xffffu);   // z=1.0 -> far plane
    CHECK_EQ(Rast7::ReadZ(f.zbuf,319,239),0xffffu);
    // Rect clear honours bounds.
    f.dev->Clear(0,0,D3DCLEAR_TARGET,0,0,0);
    D3DRECT r={10,10,20,20};
    f.dev->Clear(1,&r,D3DCLEAR_TARGET,0x0000ff00,0,0);
    CHECK_EQ(*Px(f.back,15,15),0x07e0);           // green inside rect
    CHECK_EQ(*Px(f.back,9,15),0);                 // untouched outside
    CHECK_EQ(*Px(f.back,25,15),0);
}

// --- DrawPrimitive: gouraud triangle, no texture ------------------------------
REGISTER_TEST(test_bob_ddraw_flat_triangle) {
    Frame f;
    CHECK(f.Init(64,64,16));
    f.dev->Clear(0,0,D3DCLEAR_TARGET,0,1.0f,0);
    D3DTLVERTEX tri[3]={TL(8,8,0.5f,1,0xffff0000),TL(56,8,0.5f,1,0xffff0000),
                        TL(32,56,0.5f,1,0xffff0000)};
    CHECK_EQ(DrawTris(f.dev,D3DPT_TRIANGLELIST,tri,3),D3D_OK);
    CHECK_EQ(*Px(f.back,32,20),0xf800);           // interior = red
    CHECK_EQ(*Px(f.back,32,60),0);                // below the triangle
    CHECK_EQ(*Px(f.back,2,2),0);                  // outside bbox
}

// --- edge coverage: fan tri anchored far from the diagonal (A.x!=A.y) ---------
// Regression for the w2 half-space bug: the A->B edge test used (px-A.y)
// instead of (px-A.x), shifting the edge anchor to (A.y,A.y).  Triangles whose
// first vertex had x==y rendered fine while cockpit polys deep in the frame
// lost a triangular slice along that edge (missing instrument panel).
REGISTER_TEST(test_bob_ddraw_edge_coverage) {
    Frame f;
    CHECK(f.Init(64,64,16));
    f.dev->Clear(0,0,D3DCLEAR_TARGET,0,1.0f,0);
    f.dev->SetRenderState(D3DRENDERSTATE_CULLMODE,D3DCULL_NONE);
    // Same shape as the BoB instrument-panel fan, scaled: fan apex low-left,
    // interior point (10,30) sits between the A->B edge and the fan centroid.
    D3DTLVERTEX fan[5]={TL(4,56,0.5f,1,0xffff0000),TL(4,8,0.5f,1,0xffff0000),
                        TL(60,20,0.5f,1,0xffff0000),TL(64,26,0.5f,1,0xffff0000),
                        TL(20,56,0.5f,1,0xffff0000)};
    CHECK_EQ(DrawTris(f.dev,D3DPT_TRIANGLEFAN,fan,5),D3D_OK);
    CHECK_EQ(*Px(f.back,10,30),0xf800);           // inside first fan tri
    CHECK_EQ(*Px(f.back,10,58),0);                // below the fan
}

// --- culling: CW winding culled under D3DCULL_CW ------------------------------
REGISTER_TEST(test_bob_ddraw_cull) {
    Frame f;
    CHECK(f.Init(64,64,16));
    f.dev->Clear(0,0,D3DCLEAR_TARGET,0,1.0f,0);
    f.dev->SetRenderState(D3DRENDERSTATE_CULLMODE,D3DCULL_CW);
    // (8,8)->(56,8)->(32,56) is visually CW in y-down space -> culled.
    D3DTLVERTEX cw[3]={TL(8,8,0.5f,1,0xffff0000),TL(56,8,0.5f,1,0xffff0000),
                       TL(32,56,0.5f,1,0xffff0000)};
    CHECK_EQ(DrawTris(f.dev,D3DPT_TRIANGLELIST,cw,3),D3D_OK);
    CHECK_EQ(*Px(f.back,32,20),0);                // culled: nothing drawn
    // CCW copy of the same triangle survives.
    D3DTLVERTEX ccw[3]={TL(8,8,0.5f,1,0xffff0000),TL(32,56,0.5f,1,0xffff0000),
                        TL(56,8,0.5f,1,0xffff0000)};
    CHECK_EQ(DrawTris(f.dev,D3DPT_TRIANGLELIST,ccw,3),D3D_OK);
    CHECK_EQ(*Px(f.back,32,20),0xf800);
}

// --- z-buffer: nearer triangle wins -------------------------------------------
REGISTER_TEST(test_bob_ddraw_ztest) {
    Frame f;
    CHECK(f.Init(64,64,16));
    f.dev->Clear(0,0,D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,0,1.0f,0);
    f.dev->SetRenderState(D3DRENDERSTATE_ZENABLE,D3DZB_TRUE);
    f.dev->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE,TRUE);
    f.dev->SetRenderState(D3DRENDERSTATE_ZFUNC,D3DCMP_LESSEQUAL);
    // far red first, near green second covering the same pixels.
    D3DTLVERTEX far_[3]={TL(8,8,0.9f,1,0xffff0000),TL(56,8,0.9f,1,0xffff0000),
                         TL(32,56,0.9f,1,0xffff0000)};
    D3DTLVERTEX near_[3]={TL(8,8,0.1f,1,0xff00ff00),TL(56,8,0.1f,1,0xff00ff00),
                          TL(32,56,0.1f,1,0xff00ff00)};
    CHECK_EQ(DrawTris(f.dev,D3DPT_TRIANGLELIST,far_,3),D3D_OK);
    CHECK_EQ(DrawTris(f.dev,D3DPT_TRIANGLELIST,near_,3),D3D_OK);
    CHECK_EQ(*Px(f.back,32,20),0x07e0);           // green (near) won
    // Drawing the far triangle again must now fail the z test.
    CHECK_EQ(DrawTris(f.dev,D3DPT_TRIANGLELIST,far_,3),D3D_OK);
    CHECK_EQ(*Px(f.back,32,20),0x07e0);           // still green
}

// --- textured triangle: stage0 MODULATE ---------------------------------------
REGISTER_TEST(test_bob_ddraw_texture_modulate) {
    Frame f;
    CHECK(f.Init(64,64,16));
    // 2x2 565 texture: red green / blue white
    DDSURFACEDESC2 d; memset(&d,0,sizeof(d)); d.dwSize=sizeof(d);
    d.dwFlags=DDSD_CAPS|DDSD_WIDTH|DDSD_HEIGHT|DDSD_PIXELFORMAT;
    d.ddsCaps.dwCaps=DDSCAPS_TEXTURE;
    d.dwWidth=2; d.dwHeight=2;
    d.ddpfPixelFormat.dwSize=sizeof(DDPIXELFORMAT);
    d.ddpfPixelFormat.dwFlags=DDPF_RGB; d.ddpfPixelFormat.dwRGBBitCount=16;
    d.ddpfPixelFormat.dwRBitMask=0xf800; d.ddpfPixelFormat.dwGBitMask=0x07e0;
    d.ddpfPixelFormat.dwBBitMask=0x001f;
    IDirectDrawSurface7* tex=0;
    CHECK_EQ(f.dd->CreateSurface(&d,&tex,0),DD_OK);
    CHECK(tex!=0);
    *Px(tex,0,0)=0xf800; *Px(tex,1,0)=0x07e0;
    *Px(tex,0,1)=0x001f; *Px(tex,1,1)=0xffff;

    f.dev->Clear(0,0,D3DCLEAR_TARGET,0,1.0f,0);
    f.dev->SetTexture(0,tex);
    f.dev->SetTextureStageState(0,D3DTSS_COLOROP,D3DTOP_MODULATE);
    f.dev->SetTextureStageState(0,D3DTSS_COLORARG1,D3DTA_TEXTURE);
    f.dev->SetTextureStageState(0,D3DTSS_COLORARG2,D3DTA_DIFFUSE);
    f.dev->SetTextureStageState(0,D3DTSS_ALPHAOP,D3DTOP_SELECTARG1);
    f.dev->SetTextureStageState(0,D3DTSS_ALPHAARG1,D3DTA_DIFFUSE);
    f.dev->SetTextureStageState(0,D3DTSS_MAGFILTER,D3DTFG_POINT);
    f.dev->SetTextureStageState(0,D3DTSS_MINFILTER,D3DTFG_POINT);
    // white-diffuse quad over the whole 64x64 target: uv spans 0..1.
    D3DTLVERTEX quad[4]={TL(0,0,0.5f,1,0xffffffff,0,0),
                         TL(64,0,0.5f,1,0xffffffff,1,0),
                         TL(64,64,0.5f,1,0xffffffff,1,1),
                         TL(0,64,0.5f,1,0xffffffff,0,1)};
    CHECK_EQ(DrawTris(f.dev,D3DPT_TRIANGLEFAN,quad,4),D3D_OK);
    CHECK_EQ(*Px(f.back,8,8)&0xf800,0xf800);      // top-left texel ~ red
    CHECK_EQ(*Px(f.back,8,8)&0x07e0,0);           // no green in it
    CHECK_EQ(*Px(f.back,40,8)&0x07e0,0x07e0);     // top-right ~ green
    CHECK_EQ(*Px(f.back,8,40)&0x001f,0x001f);     // bottom-left ~ blue
    tex->Release();
}

// --- alpha blend: SRCALPHA / INVSRCALPHA ---------------------------------------
REGISTER_TEST(test_bob_ddraw_alphablend) {
    Frame f;
    CHECK(f.Init(64,64,16));
    f.dev->Clear(0,0,D3DCLEAR_TARGET,0x000000ff,1.0f,0);  // dst = full blue
    f.dev->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE,TRUE);
    f.dev->SetRenderState(D3DRENDERSTATE_SRCBLEND,D3DBLEND_SRCALPHA);
    f.dev->SetRenderState(D3DRENDERSTATE_DESTBLEND,D3DBLEND_INVSRCALPHA);
    // Untextured alpha: games take the interpolated vertex alpha — the DX7
    // default ALPHAARG1 is TEXTURE which would yield 255 with nothing bound.
    f.dev->SetTextureStageState(0,D3DTSS_ALPHAARG1,D3DTA_DIFFUSE);
    // half-alpha red quad over a corner.
    D3DTLVERTEX tri[3]={TL(0,0,0.5f,1,0x80ff0000),TL(64,0,0.5f,1,0x80ff0000),
                        TL(0,64,0.5f,1,0x80ff0000)};
    CHECK_EQ(DrawTris(f.dev,D3DPT_TRIANGLELIST,tri,3),D3D_OK);
    uint16_t px=*Px(f.back,10,10);
    DWORD r=(px>>11)&0x1f, b=px&0x1f;
    // 0x80/255 alpha -> ~50% red + ~50% blue. 565: r~15/16, b~15/16.
    CHECK(r>=13&&r<=18);
    CHECK(b>=13&&b<=18);
}

// --- indexed draw + vertex buffer path -----------------------------------------
REGISTER_TEST(test_bob_ddraw_indexed_and_vb) {
    Frame f;
    CHECK(f.Init(64,64,16));
    f.dev->Clear(0,0,D3DCLEAR_TARGET,0,1.0f,0);
    D3DTLVERTEX v[4]={TL(0,0,0.5f,1,0xffffffff),TL(64,0,0.5f,1,0xffffffff),
                      TL(64,64,0.5f,1,0xffffffff),TL(0,64,0.5f,1,0xffffffff)};
    WORD idx[6]={0,1,2, 0,2,3};
    f.dev->BeginScene();
    HRESULT hr=f.dev->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,D3DFVF_TLVERTEX,
                                           v,4,idx,6,0);
    f.dev->EndScene();
    CHECK_EQ(hr,D3D_OK);
    CHECK_EQ(*Px(f.back,32,32),0xffff);           // whole quad covered

    // Same quad through a real vertex buffer + DrawIndexedPrimitiveVB.
    D3DVERTEXBUFFERDESC vd; memset(&vd,0,sizeof(vd));
    vd.dwSize=sizeof(vd); vd.dwCaps=D3DVBCAPS_WRITEONLY;
    vd.dwFVF=D3DFVF_TLVERTEX; vd.dwNumVertices=4;
    IDirect3DVertexBuffer7* vb=0;
    CHECK_EQ(f.d3d->CreateVertexBuffer(&vd,&vb,0),D3D_OK);
    CHECK(vb!=0);
    LPVOID mem=0;
    CHECK_EQ(vb->Lock(0,&mem,0),D3D_OK);
    memcpy(mem,v,sizeof(v));
    vb->Unlock();
    f.dev->Clear(0,0,D3DCLEAR_TARGET,0,1.0f,0);
    f.dev->BeginScene();
    hr=f.dev->DrawIndexedPrimitiveVB(D3DPT_TRIANGLELIST,vb,0,4,idx,6,0);
    f.dev->EndScene();
    CHECK_EQ(hr,D3D_OK);
    CHECK_EQ(*Px(f.back,32,32),0xffff);
    vb->Release();
}

// --- lines and points -----------------------------------------------------------
REGISTER_TEST(test_bob_ddraw_lines_points) {
    Frame f;
    CHECK(f.Init(64,64,16));
    f.dev->Clear(0,0,D3DCLEAR_TARGET,0,1.0f,0);
    D3DTLVERTEX ln[2]={TL(4,4,0.5f,1,0xff00ff00),TL(60,4,0.5f,1,0xff00ff00)};
    f.dev->BeginScene();
    HRESULT hr=f.dev->DrawPrimitive(D3DPT_LINELIST,D3DFVF_TLVERTEX,ln,2,0);
    f.dev->EndScene();
    CHECK_EQ(hr,D3D_OK);
    CHECK_EQ(*Px(f.back,32,4),0x07e0);            // horizontal line mid
    D3DTLVERTEX pt[1]={TL(20,20,0.5f,1,0xff0000ff)};
    f.dev->BeginScene();
    hr=f.dev->DrawPrimitive(D3DPT_POINTLIST,D3DFVF_TLVERTEX,pt,1,0);
    f.dev->EndScene();
    CHECK_EQ(hr,D3D_OK);
    CHECK_EQ(*Px(f.back,20,20),0x001f);           // point landed
}

// --- Flip copies back->primary and fires the present hook -----------------------
static IDirectDrawSurface7* g_presented;
static void TestPresent(IDirectDrawSurface7* s) { g_presented=s; }
REGISTER_TEST(test_bob_ddraw_flip_present) {
    g_presented=0;
    RowanDDPresentHook=&TestPresent;
    Frame f;
    CHECK(f.Init(64,64,16));
    f.dev->Clear(0,0,D3DCLEAR_TARGET,0x00ff0000,1.0f,0);
    CHECK_EQ(*Px(f.primary,10,10),0);             // front still black
    CHECK_EQ(f.primary->Flip(0,0),DD_OK);
    CHECK_EQ(g_presented,(IDirectDrawSurface7*)f.primary);  // hook fired
    CHECK_EQ(*Px(f.primary,10,10),0xf800);        // back pixels reached front
}
