// ---------------------------------------------------------------------------
// d3dvk.h — Vulkan backend for the DX7 compat device.
//
// Renders IDirect3DDevice7::DrawPrimitive* batches into an offscreen Vulkan
// image, then lazily copies the result back into the render target's
// lpSurface so the existing Lock/Blt/Flip/present paths see the same pixels
// they see under the software rasterizer.  d3dsoft.h stays the fallback and
// the behavioural oracle — every pixel semantic here is a port of
// EvalStagesE/PixE, not a re-derivation of the DX7 docs.
//
// CPU->GPU sync:  surface->cpuGen bumps on every CPU write (Unlock/Blt-dst/
//                 SetPalette); the next DrawPrimitive* re-uploads the render
//                 target / texture images whose generation changed.
// GPU->CPU sync:  RowanVkReadSync(surf) — called by Lock/Blt-src/Flip —
//                 waits for the in-flight submit and lands the readback copy
//                 into lpSurface.
//
// Frame model: draws record into one command buffer; at EndScene the
// buffer is closed with an image->linear-buffer copy appended and submitted
// WITHOUT waiting (GPU renders while the CPU does 2D/present work).  The
// first touch of lpSurface afterwards waits the fence and memcpy's the
// rows across.  All entry points hold vkMutex.
//
// Environment: ROWAN_VK=0 forces the software path.
// ---------------------------------------------------------------------------
#ifndef ROWAN_COMPAT_D3DVK_H
#define ROWAN_COMPAT_D3DVK_H

#if defined(__GNUC__) && defined(__linux__)
#define ROWAN_VK_LOADED 1
#include <vulkan/vulkan.h>
#include <dlfcn.h>
#include <mutex>
#include <unordered_map>
#include <alloca.h>
#include <time.h>
#else
#define ROWAN_VK_LOADED 0
#endif

#if ROWAN_VK_LOADED
#include "shaders/d3dvk_spv.h"

namespace RastVk {

// --- function pointers ------------------------------------------------------
// Loaded once from libvulkan.so.1 so the binary still runs on systems with no
// Vulkan at all (headless CI, missing driver).
struct PFN {
    PFN_vkGetInstanceProcAddr GetInstanceProcAddr;
    PFN_vkCreateInstance CreateInstance;
    PFN_vkDestroyInstance DestroyInstance;
    PFN_vkEnumeratePhysicalDevices EnumeratePhysicalDevices;
    PFN_vkGetPhysicalDeviceProperties GetPhysicalDeviceProperties;
    PFN_vkGetPhysicalDeviceQueueFamilyProperties GetPhysicalDeviceQueueFamilyProperties;
    PFN_vkGetPhysicalDeviceMemoryProperties GetPhysicalDeviceMemoryProperties;
    PFN_vkCreateDevice CreateDevice;
    PFN_vkGetDeviceProcAddr GetDeviceProcAddr;
    PFN_vkDestroyDevice DestroyDevice;
    PFN_vkGetDeviceQueue GetDeviceQueue;
    PFN_vkDeviceWaitIdle DeviceWaitIdle;
    PFN_vkCreateCommandPool CreateCommandPool;
    PFN_vkDestroyCommandPool DestroyCommandPool;
    PFN_vkAllocateCommandBuffers AllocateCommandBuffers;
    PFN_vkBeginCommandBuffer BeginCommandBuffer;
    PFN_vkEndCommandBuffer EndCommandBuffer;
    PFN_vkResetCommandBuffer ResetCommandBuffer;
    PFN_vkCmdBeginRenderPass CmdBeginRenderPass;
    PFN_vkCmdEndRenderPass CmdEndRenderPass;
    PFN_vkCmdBindPipeline CmdBindPipeline;
    PFN_vkCmdBindVertexBuffers CmdBindVertexBuffers;
    PFN_vkCmdBindDescriptorSets CmdBindDescriptorSets;
    PFN_vkCmdDraw CmdDraw;
    PFN_vkCmdPushConstants CmdPushConstants;
    PFN_vkCmdSetViewport CmdSetViewport;
    PFN_vkCmdSetScissor CmdSetScissor;
    PFN_vkCmdPipelineBarrier CmdPipelineBarrier;
    PFN_vkCmdCopyBufferToImage CmdCopyBufferToImage;
    PFN_vkCmdCopyImageToBuffer CmdCopyImageToBuffer;
    PFN_vkCmdClearColorImage CmdClearColorImage;
    PFN_vkCmdClearDepthStencilImage CmdClearDepthStencilImage;
    PFN_vkCmdClearAttachments CmdClearAttachments;
    PFN_vkQueueSubmit QueueSubmit;
    PFN_vkCreateFence CreateFence;
    PFN_vkDestroyFence DestroyFence;
    PFN_vkWaitForFences WaitForFences;
    PFN_vkResetFences ResetFences;
    PFN_vkCreateBuffer CreateBuffer;
    PFN_vkDestroyBuffer DestroyBuffer;
    PFN_vkBindBufferMemory BindBufferMemory;
    PFN_vkCreateImage CreateImage;
    PFN_vkDestroyImage DestroyImage;
    PFN_vkBindImageMemory BindImageMemory;
    PFN_vkCreateImageView CreateImageView;
    PFN_vkDestroyImageView DestroyImageView;
    PFN_vkCreateSampler CreateSampler;
    PFN_vkDestroySampler DestroySampler;
    PFN_vkCreateShaderModule CreateShaderModule;
    PFN_vkDestroyShaderModule DestroyShaderModule;
    PFN_vkCreateDescriptorSetLayout CreateDescriptorSetLayout;
    PFN_vkDestroyDescriptorSetLayout DestroyDescriptorSetLayout;
    PFN_vkCreateDescriptorPool CreateDescriptorPool;
    PFN_vkResetDescriptorPool ResetDescriptorPool;
    PFN_vkAllocateDescriptorSets AllocateDescriptorSets;
    PFN_vkUpdateDescriptorSets UpdateDescriptorSets;
    PFN_vkCreatePipelineLayout CreatePipelineLayout;
    PFN_vkDestroyPipelineLayout DestroyPipelineLayout;
    PFN_vkCreateGraphicsPipelines CreateGraphicsPipelines;
    PFN_vkDestroyPipeline DestroyPipeline;
    PFN_vkCreateRenderPass CreateRenderPass;
    PFN_vkDestroyRenderPass DestroyRenderPass;
    PFN_vkCreateFramebuffer CreateFramebuffer;
    PFN_vkDestroyFramebuffer DestroyFramebuffer;
    PFN_vkAllocateMemory AllocateMemory;
    PFN_vkFreeMemory FreeMemory;
    PFN_vkMapMemory MapMemory;
    PFN_vkUnmapMemory UnmapMemory;
    PFN_vkGetBufferMemoryRequirements GetBufferMemoryRequirements;
    PFN_vkGetImageMemoryRequirements GetImageMemoryRequirements;
};
static PFN vk;
#define V(f) vk.f

// --- shared push-constant block (128B, mirrors shaders/d3dvk.{vert,frag}) ---
struct PC {
    float    xform[4];     // scale.xy, bias.zw (VS)
    float    zmisc[4];     // x: zbiasN(VS); y,z: fogStart,fogEnd (FS)
    float    fogCol[4];
    float    tfac[4];
    uint32_t cfg[4];       // x: nStage|persp<<4|spec<<5|fogT<<6|fogV<<7|ckey<<8|usew<<9
                           // y: ckeyLo rgb, z: ckeyHi rgb
    uint32_t stg[3][4];    // per stage: ops, args, 2 unused (GLSL uvec4 stride)
};

// GPU vertex = Rast7::Vert fields re-laid for interleaved texcoords.
struct GVert {
    float x,y,z,rhw;
    uint32_t col,spec;
    float u0,v0,u1,v1,u2,v2;
};

struct Tex {
    VkImage img; VkDeviceMemory mem; VkImageView view;
    DWORD gen;             // surface cpuGen at last upload
    VkFormat fmt;
    DWORD w,h;
};

// One GPU render-target set per (rt,zb) surface pair.  BoB draws each frame
// into the back buffer plus small offscreen targets (gauge/mirror prerenders
// at 128x128, 256x256); keeping the images/framebuffer alive across target
// switches avoids a destroy/recreate + forced re-upload per switch.
struct Tgt {
    IDirectDrawSurface7 *rt,*zb;
    VkImage rtImg; VkDeviceMemory rtMem; VkImageView rtView; VkFormat rtFmt;
    DWORD   W,H;
    DWORD   rtSurfGen;      // cpuGen value mirrored into rtImg
    VkImage zImg; VkDeviceMemory zMem; VkImageView zView; VkFormat zFmt;
    int     zbits;
    DWORD   zbSurfGen;      // cpuGen value mirrored into zImg
    VkRenderPass rp; VkFramebuffer fb;

    bool gpuOwns;       // rtImg content ahead of rt->lpSurface
    bool zbAhead;       // zImg content ahead of zb->lpSurface
    bool zbUseW;        // depth encoding when zbAhead was set
    bool zbWanted;      // a CPU-side read of zb was requested since last drain
    bool copyOwed;      // gpu/zb content newer than the last recorded readback
    bool rbPending;     // in-flight submit ends with this target's colour copy
    bool zbPending;     //   readback also carried the depth image
    VkDeviceSize rbOff; // stable readback slot inside rb (colour at rbOff,
                        // depth at rbOff+zbSlotOff)
    VkDeviceSize zbSlotOff;
};

struct TgtKey {
    IDirectDrawSurface7 *rt,*zb;
    bool operator==(const TgtKey&o)const{ return rt==o.rt && zb==o.zb; }
};
struct TgtKeyHash {
    size_t operator()(const TgtKey&k)const{
        return std::hash<IDirectDrawSurface7*>()(k.rt)*31
             + std::hash<IDirectDrawSurface7*>()(k.zb);
    }
};

struct Dev {
    bool tried,avail;
    void* lib;
    VkInstance inst; VkPhysicalDevice pd; VkDevice dev; VkQueue queue;
    uint32_t qfam; VkPhysicalDeviceMemoryProperties memProps;
    VkCommandPool cmdPool; VkCommandBuffer cmd; VkFence fence;
    bool cmdRec;        // command buffer open
    bool inPass;        // render pass open inside it
    bool inFlight;      // submitted, fence not yet waited
    bool poolFail;      // descriptor pool exhausted during last BindState

    std::unordered_map<TgtKey,Tgt,TgtKeyHash> tgts;
    Tgt* cur;
    IDirectDrawSurface7* failed;   // rt that alloc-failed once
    VkDeviceSize rbAlloc;          // bump allocator for per-target rb slots

    VkBuffer vb; VkDeviceMemory vbMem; BYTE* vbMap; VkDeviceSize vbCap,vbOff;
    VkBuffer rb; VkDeviceMemory rbMem; BYTE* rbMap; VkDeviceSize rbCap;
    VkBuffer ub; VkDeviceMemory ubMem; BYTE* ubMap; VkDeviceSize ubCap,ubOff;

    VkShaderModule vs,fs,fsW;   // fsW: w-buffer variant (unconditional FragDepth)
    VkDescriptorSetLayout dsl; VkDescriptorPool dp; VkPipelineLayout pl;
    VkImage dummyImg; VkDeviceMemory dummyMem; VkImageView dummyView;
    VkSampler dummySamp;

    std::unordered_map<IDirectDrawSurface7*,Tex> texes;
    std::unordered_map<uint64_t,VkPipeline> pipes;
    std::unordered_map<uint32_t,VkSampler> samps;
    struct SetKey { VkImageView v0,v1,v2; uint32_t sc;
        bool operator==(const SetKey&o)const{
            return v0==o.v0 && v1==o.v1 && v2==o.v2 && sc==o.sc; } };
    struct SetKeyHash {
        size_t operator()(const SetKey&k)const{
            return std::hash<VkImageView>()(k.v0)*131
                 + std::hash<VkImageView>()(k.v1)*17
                 + std::hash<VkImageView>()(k.v2)*3 + k.sc; } };
    std::unordered_map<SetKey,VkDescriptorSet,SetKeyHash> sets;
};
// Leaked on purpose: the render thread can still be inside TexFor()/Draw() when
// exit() runs static destructors, so destroying D's maps there is a use-after-free.
// () is load-bearing: it zero-initializes the POD members like static storage did.
static Dev& D = *new Dev();
static std::mutex vkMutex;

// --- perf counters (ROWAN_VK_PERF=1) -----------------------------------------
// Per-second rollup of where the frame time goes: fence wait, readback
// copies, CPU->GPU uploads, draws.  Zero cost when the env var is unset.
struct Perf {
    uint64_t waitNs,rtNs,zbNs,upRTns,upZBns,upTexNs;
    uint32_t nFlush,nDraws,nUpRT,nUpZB,nUpTex,nPipeMk;
    uint64_t lastLog;
};
static Perf& P = *new Perf();
static uint64_t NowNs() {
    timespec ts; clock_gettime(CLOCK_MONOTONIC,&ts);
    return (uint64_t)ts.tv_sec*1000000000ull+(uint64_t)ts.tv_nsec;
}
static int PerfOn() {
    static int on=-1; if(on<0)on=getenv("ROWAN_VK_PERF")?1:0; return on;
}
static void PerfLog() {
    if(!PerfOn()) return;
    uint64_t n=NowNs();
    if(!P.lastLog){P.lastLog=n;return;}
    if(n-P.lastLog<2000000000ull) return;
    double s=(n-P.lastLog)/1e9;
    fprintf(stderr,"[vkperf] %.1fs wait=%.1fms rtcp=%.1f zbcp=%.1f "
            "upRT=%.1f(%u) upZB=%.1f(%u) upTex=%.1f(%u) "
            "fl=%u dr=%u pipes=%u\n",
            s,P.waitNs/1e6,P.rtNs/1e6,P.zbNs/1e6,
            P.upRTns/1e6,P.nUpRT,P.upZBns/1e6,P.nUpZB,
            P.upTexNs/1e6,P.nUpTex,P.nFlush,P.nDraws,P.nPipeMk);
    P.waitNs=P.rtNs=P.zbNs=P.upRTns=P.upZBns=P.upTexNs=0;
    P.nFlush=P.nDraws=P.nUpRT=P.nUpZB=P.nUpTex=P.nPipeMk=0;
    P.lastLog=n;
}

// --- small helpers -------------------------------------------------------------
static uint32_t MemType(uint32_t bits, VkMemoryPropertyFlags want) {
    for(uint32_t i=0;i<D.memProps.memoryTypeCount;i++)
        if((bits&(1u<<i)) && (D.memProps.memoryTypes[i].propertyFlags&want)==want)
            return i;
    return 0xffffffffu;
}
static VkDeviceMemory AllocMem(VkDeviceSize size, uint32_t typeBits,
                               VkMemoryPropertyFlags want) {
    uint32_t mt=MemType(typeBits,want);
    if(mt==0xffffffffu) return 0;
    VkMemoryAllocateInfo ai={VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,0,size,mt};
    VkDeviceMemory m;
    return V(AllocateMemory)(D.dev,&ai,0,&m)==VK_SUCCESS ? m : 0;
}
static void Drain();          // fwd
static void Flush(bool wait); // fwd

// --- command-buffer lifecycle ---------------------------------------------------
static void BeginCmd() {
    if(D.inFlight) Drain();     // GPU must be done before vb is rewritten
    if(D.cmdRec) return;
    V(ResetCommandBuffer)(D.cmd,0);
    VkCommandBufferBeginInfo bi={VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    V(BeginCommandBuffer)(D.cmd,&bi);
    D.cmdRec=true; D.vbOff=0; D.ubOff=0;
}
static void EndPass() { if(D.inPass){V(CmdEndRenderPass)(D.cmd);D.inPass=false;} }
// Wait for the in-flight submit and land the readback into lpSurface.
static void Drain() {
    if(!D.inFlight) return;
    uint64_t t0=NowNs();
    V(WaitForFences)(D.dev,1,&D.fence,VK_TRUE,~0ull);
    D.inFlight=false;
    if(PerfOn()) P.waitNs+=NowNs()-t0;
    for(std::unordered_map<TgtKey,Tgt,TgtKeyHash>::iterator ti=D.tgts.begin();ti!=D.tgts.end();++ti){
        Tgt& t=ti->second;
        if(t.rbPending&&t.rt){
            uint64_t tc=NowNs();
            DWORD bpp=t.rt->desc.ddpfPixelFormat.dwRGBBitCount?
                      t.rt->desc.ddpfPixelFormat.dwRGBBitCount/8:2;
            VkDeviceSize row=(VkDeviceSize)t.W*bpp;
            BYTE* dst=(BYTE*)t.rt->desc.lpSurface;
            for(DWORD y=0;y<t.H;y++)
                memcpy(dst+y*t.rt->desc.lPitch,D.rbMap+t.rbOff+y*row,(size_t)row);
            t.rbPending=false; t.gpuOwns=false;
            t.rtSurfGen=t.rt->cpuGen;      // surface now mirrors the image
            if(PerfOn()) P.rtNs+=NowNs()-tc;
        }
        // The depth copy is consumed only when the CPU actually asked for the
        // z surface (ReadSync/Lock).  BoB never locks it mid-flight, so the
        // per-pixel float->int conversion is skipped most frames.
        if(t.zbPending&&t.zb&&t.zbWanted){
            uint64_t tz=NowNs();
            // Inverse of UploadZB's translation; 16-bit non-w copies bytewise.
            bool usew=t.zbUseW;
            VkDeviceSize pxB=(t.zFmt==VK_FORMAT_D16_UNORM)?2:4;
            bool direct=(t.zbits==16&&!usew);
            const BYTE* src=D.rbMap+t.rbOff+t.zbSlotOff;
            if(direct){
                for(DWORD y=0;y<t.H;y++)
                    memcpy((BYTE*)t.zb->desc.lpSurface+y*t.zb->desc.lPitch,
                           src+y*pxB*t.W,(size_t)(pxB*t.W));
            } else {
                for(DWORD y=0;y<t.H;y++)for(DWORD x=0;x<t.W;x++){
                    float f=(pxB==2)?((const uint16_t*)src)[y*t.W+x]/65535.0f
                                    :((const float*)src)[y*t.W+x];
                    DWORD nz;
                    if(usew){
                        float w=(f>=1.0f)?3.402823466e38f:f/(1.0f-f);
                        memcpy(&nz,&w,4);
                        if(t.zbits<32)nz>>=(32-t.zbits);
                    } else {
                        double zmax=t.zbits>=32?4294967295.0:
                                   t.zbits>=24?16777215.0:65535.0;
                        nz=(DWORD)(f*zmax);
                    }
                    Rast7::WriteZ(t.zb,x,y,nz);
                }
            }
            t.zbPending=false; t.zbAhead=false; t.zbWanted=false;
            t.zbSurfGen=t.zb->cpuGen;
            if(PerfOn()) P.zbNs+=NowNs()-tz;
        }
    }
    PerfLog();
}
// Close the buffer (appending the readback copy for every target that owes
// one) and submit.  wait=false leaves the GPU racing ahead of the CPU.
static void Flush(bool wait) {
    if(!D.cmdRec) { if(wait) Drain(); return; }
    EndPass();
    for(std::unordered_map<TgtKey,Tgt,TgtKeyHash>::iterator ti=D.tgts.begin();ti!=D.tgts.end();++ti){
        Tgt& t=ti->second;
        if(!t.copyOwed) continue;
        if(t.rt&&t.gpuOwns){
        DWORD bpp=t.rt->desc.ddpfPixelFormat.dwRGBBitCount?
                  t.rt->desc.ddpfPixelFormat.dwRGBBitCount/8:2;
        VkDeviceSize row=(VkDeviceSize)t.W*bpp;
        if(t.rbOff+row*t.H<=D.rbCap){
            VkImageMemoryBarrier b={VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
            b.oldLayout=VK_IMAGE_LAYOUT_GENERAL; b.newLayout=VK_IMAGE_LAYOUT_GENERAL;
            b.srcQueueFamilyIndex=b.dstQueueFamilyIndex=VK_QUEUE_FAMILY_IGNORED;
            b.image=t.rtImg; b.subresourceRange.aspectMask=VK_IMAGE_ASPECT_COLOR_BIT;
            b.subresourceRange.levelCount=1; b.subresourceRange.layerCount=1;
            b.srcAccessMask=VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
                           |VK_ACCESS_TRANSFER_WRITE_BIT;
            b.dstAccessMask=VK_ACCESS_TRANSFER_READ_BIT;
            V(CmdPipelineBarrier)(D.cmd,VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                VK_PIPELINE_STAGE_TRANSFER_BIT,0,0,0,0,0,1,&b);
            VkBufferImageCopy c={};
            c.bufferOffset=t.rbOff;
            c.bufferRowLength=t.W; c.bufferImageHeight=t.H;
            c.imageExtent.width=t.W; c.imageExtent.height=t.H;
            c.imageExtent.depth=1;
            c.imageSubresource.aspectMask=VK_IMAGE_ASPECT_COLOR_BIT;
            c.imageSubresource.layerCount=1;
            V(CmdCopyImageToBuffer)(D.cmd,t.rtImg,VK_IMAGE_LAYOUT_GENERAL,D.rb,1,&c);
            b.srcAccessMask=VK_ACCESS_TRANSFER_READ_BIT;
            b.dstAccessMask=VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            V(CmdPipelineBarrier)(D.cmd,VK_PIPELINE_STAGE_TRANSFER_BIT,
                VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,0,0,0,0,0,1,&b);
            t.rbPending=true;
        }
        }
        if(t.zb&&t.zbAhead&&t.zImg){
        VkDeviceSize pxB=(t.zFmt==VK_FORMAT_D16_UNORM)?2:4;
        VkDeviceSize zoff=t.rbOff+t.zbSlotOff;
        if(zoff+pxB*t.W*t.H<=D.rbCap){
            VkImageMemoryBarrier b={VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
            b.oldLayout=VK_IMAGE_LAYOUT_GENERAL; b.newLayout=VK_IMAGE_LAYOUT_GENERAL;
            b.srcQueueFamilyIndex=b.dstQueueFamilyIndex=VK_QUEUE_FAMILY_IGNORED;
            b.image=t.zImg; b.subresourceRange.aspectMask=VK_IMAGE_ASPECT_DEPTH_BIT;
            b.subresourceRange.levelCount=1; b.subresourceRange.layerCount=1;
            b.srcAccessMask=VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            b.dstAccessMask=VK_ACCESS_TRANSFER_READ_BIT;
            V(CmdPipelineBarrier)(D.cmd,
                VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
                VK_PIPELINE_STAGE_TRANSFER_BIT,0,0,0,0,0,1,&b);
            VkBufferImageCopy c={};
            c.bufferOffset=zoff;
            c.bufferRowLength=t.W; c.bufferImageHeight=t.H;
            c.imageExtent.width=t.W; c.imageExtent.height=t.H;
            c.imageExtent.depth=1;
            c.imageSubresource.aspectMask=VK_IMAGE_ASPECT_DEPTH_BIT;
            c.imageSubresource.layerCount=1;
            V(CmdCopyImageToBuffer)(D.cmd,t.zImg,VK_IMAGE_LAYOUT_GENERAL,D.rb,1,&c);
            b.srcAccessMask=VK_ACCESS_TRANSFER_READ_BIT;
            b.dstAccessMask=VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            V(CmdPipelineBarrier)(D.cmd,VK_PIPELINE_STAGE_TRANSFER_BIT,
                VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,0,0,0,0,0,1,&b);
            t.zbPending=true;
        }
        }
        t.copyOwed=false;
    }
    if(V(EndCommandBuffer)(D.cmd)==VK_SUCCESS){
        VkSubmitInfo si={VK_STRUCTURE_TYPE_SUBMIT_INFO};
        si.commandBufferCount=1; si.pCommandBuffers=&D.cmd;
        V(ResetFences)(D.dev,1,&D.fence);
        V(QueueSubmit)(D.queue,1,&si,D.fence);
        D.inFlight=true;
    }
    D.cmdRec=false;
    if(PerfOn()) P.nFlush++;
    if(wait) Drain();
}

// --- image helpers ---------------------------------------------------------------
// The VkFormat whose byte layout matches lpSurface; everything else is
// converted to B8G8R8A8 at upload.
static VkFormat SurfFmt(IDirectDrawSurface7* s) {
    const DDPIXELFORMAT& p=s->desc.ddpfPixelFormat;
    DWORD b=p.dwRGBBitCount;
    if(b==16){
        if(p.dwRBitMask==0xf800&&p.dwGBitMask==0x07e0&&p.dwBBitMask==0x001f)
            return VK_FORMAT_R5G6B5_UNORM_PACK16;
        if(p.dwRBitMask==0x7c00&&p.dwGBitMask==0x03e0&&p.dwBBitMask==0x001f
           &&p.dwRGBAlphaBitMask==0x8000)
            return VK_FORMAT_A1R5G5B5_UNORM_PACK16;
    }
    if(b==32&&p.dwRBitMask==0x00ff0000&&p.dwGBitMask==0x0000ff00
       &&p.dwBBitMask==0x000000ff)
        return VK_FORMAT_B8G8R8A8_UNORM;   // X8R8G8B8 or A8R8G8B8
    return VK_FORMAT_B8G8R8A8_UNORM;       // convert path
}
static bool DirectFmt(IDirectDrawSurface7* s) {   // lpSurface -> VkImage bytewise
    const DDPIXELFORMAT& p=s->desc.ddpfPixelFormat;
    DWORD b=p.dwRGBBitCount;
    if(b==16&&(p.dwRBitMask==0xf800||(p.dwRBitMask==0x7c00&&p.dwRGBAlphaBitMask==0x8000)))
        return true;
    if(b==32&&p.dwRBitMask==0x00ff0000&&p.dwGBitMask==0x0000ff00
       &&p.dwBBitMask==0x000000ff) return true;
    return false;
}
static VkImage MkImage(VkFormat f, DWORD w, DWORD h, VkImageUsageFlags use,
                       VkDeviceMemory* out) {
    VkImageCreateInfo ii={VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    ii.imageType=VK_IMAGE_TYPE_2D; ii.format=f;
    ii.extent.width=w; ii.extent.height=h; ii.extent.depth=1;
    ii.mipLevels=1; ii.arrayLayers=1; ii.samples=VK_SAMPLE_COUNT_1_BIT;
    ii.tiling=VK_IMAGE_TILING_OPTIMAL; ii.usage=use;
    ii.initialLayout=VK_IMAGE_LAYOUT_UNDEFINED;
    VkImage img;
    if(V(CreateImage)(D.dev,&ii,0,&img)!=VK_SUCCESS) return 0;
    VkMemoryRequirements mr; V(GetImageMemoryRequirements)(D.dev,img,&mr);
    VkDeviceMemory m=AllocMem(mr.size,mr.memoryTypeBits,
                              VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    if(!m) m=AllocMem(mr.size,mr.memoryTypeBits,0);
    if(!m||V(BindImageMemory)(D.dev,img,m,0)!=VK_SUCCESS){
        if(m)V(FreeMemory)(D.dev,m,0);
        V(DestroyImage)(D.dev,img,0); return 0;
    }
    *out=m; return img;
}
static VkImageView MkView(VkImage img, VkFormat f, VkImageAspectFlags asp) {
    VkImageViewCreateInfo vi={VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    vi.image=img; vi.viewType=VK_IMAGE_VIEW_TYPE_2D; vi.format=f;
    vi.subresourceRange.aspectMask=asp;
    vi.subresourceRange.levelCount=1; vi.subresourceRange.layerCount=1;
    VkImageView v;
    return V(CreateImageView)(D.dev,&vi,0,&v)==VK_SUCCESS ? v : 0;
}
static void ImgBarrier(VkImage img, VkAccessFlags src, VkAccessFlags dst,
                       VkPipelineStageFlags ss, VkPipelineStageFlags ds,
                       VkImageAspectFlags asp) {
    VkImageMemoryBarrier b={VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
    b.srcAccessMask=src; b.dstAccessMask=dst;
    b.oldLayout=VK_IMAGE_LAYOUT_GENERAL; b.newLayout=VK_IMAGE_LAYOUT_GENERAL;
    b.srcQueueFamilyIndex=b.dstQueueFamilyIndex=VK_QUEUE_FAMILY_IGNORED;
    b.image=img; b.subresourceRange.aspectMask=asp;
    b.subresourceRange.levelCount=1; b.subresourceRange.layerCount=1;
    V(CmdPipelineBarrier)(D.cmd,ss,ds,0,0,0,0,0,1,&b);
}
// one-time UNDEFINED -> GENERAL (GENERAL serves as attachment, transfer and
// sampled layout alike — trading a little perf for zero layout bookkeeping).
static void ToGeneral(VkImage img, VkImageAspectFlags asp) {
    Flush(true);    // drain: ResetCommandBuffer would nuke recorded work
    VkCommandBufferBeginInfo bi={VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    V(ResetCommandBuffer)(D.cmd,0);
    V(BeginCommandBuffer)(D.cmd,&bi);
    VkImageMemoryBarrier b={VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
    b.oldLayout=VK_IMAGE_LAYOUT_UNDEFINED; b.newLayout=VK_IMAGE_LAYOUT_GENERAL;
    b.srcQueueFamilyIndex=b.dstQueueFamilyIndex=VK_QUEUE_FAMILY_IGNORED;
    b.srcAccessMask=0;
    b.dstAccessMask=VK_ACCESS_MEMORY_READ_BIT|VK_ACCESS_MEMORY_WRITE_BIT;
    b.image=img; b.subresourceRange.aspectMask=asp;
    b.subresourceRange.levelCount=1; b.subresourceRange.layerCount=1;
    V(CmdPipelineBarrier)(D.cmd,VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,0,0,0,0,0,1,&b);
    V(EndCommandBuffer)(D.cmd);
    VkSubmitInfo si={VK_STRUCTURE_TYPE_SUBMIT_INFO};
    si.commandBufferCount=1; si.pCommandBuffers=&D.cmd;
    V(QueueSubmit)(D.queue,1,&si,VK_NULL_HANDLE);
    V(DeviceWaitIdle)(D.dev);
    D.cmdRec=false; D.vbOff=0;
}
static VkBuffer MkBuf(VkDeviceSize sz, VkBufferUsageFlags use,
                      VkMemoryPropertyFlags props,
                      VkDeviceMemory* out, BYTE** map) {
    VkBufferCreateInfo bi={VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    bi.size=sz; bi.usage=use; bi.sharingMode=VK_SHARING_MODE_EXCLUSIVE;
    VkBuffer b;
    if(V(CreateBuffer)(D.dev,&bi,0,&b)!=VK_SUCCESS) return 0;
    VkMemoryRequirements mr; V(GetBufferMemoryRequirements)(D.dev,b,&mr);
    VkDeviceMemory m=AllocMem(mr.size,mr.memoryTypeBits,props);
    if(!m||V(BindBufferMemory)(D.dev,b,m,0)!=VK_SUCCESS){
        if(m)V(FreeMemory)(D.dev,m,0);
        V(DestroyBuffer)(D.dev,b,0); return 0;
    }
    if(map){
        void* p=0;
        if(V(MapMemory)(D.dev,m,0,sz,0,&p)!=VK_SUCCESS){
            V(DestroyBuffer)(D.dev,b,0); V(FreeMemory)(D.dev,m,0); return 0;
        }
        *map=(BYTE*)p;
    }
    *out=m; return b;
}

// --- init ------------------------------------------------------------------------
static bool LoadFns() {
    D.lib=dlopen("libvulkan.so.1",RTLD_NOW|RTLD_LOCAL);
    if(!D.lib) D.lib=dlopen("libvulkan.so",RTLD_NOW|RTLD_LOCAL);
    if(!D.lib){
        if(getenv("ROWAN_VK_DEBUG"))
            fprintf(stderr,"[vk] dlopen fail: %s\n",dlerror());
        return false;
    }
    vk.GetInstanceProcAddr=(PFN_vkGetInstanceProcAddr)
        dlsym(D.lib,"vkGetInstanceProcAddr");
    if(!vk.GetInstanceProcAddr) return false;
    // dlsym straight out of the loader: GetInstanceProcAddr(NULL,...) only
    // guarantees global functions on strict loaders — instance-level entry
    // points (GetDeviceProcAddr, EnumeratePhysicalDevices) can come back 0.
#define L(n) vk.n=(PFN_vk##n)dlsym(D.lib,"vk"#n); if(!vk.n)return false
    L(CreateInstance); L(DestroyInstance); L(EnumeratePhysicalDevices);
    L(GetPhysicalDeviceProperties); L(GetPhysicalDeviceQueueFamilyProperties);
    L(GetPhysicalDeviceMemoryProperties); L(CreateDevice); L(GetDeviceProcAddr);
#undef L
    return true;
}
static bool LoadDevFns() {
#define L(n) vk.n=(PFN_vk##n)vk.GetDeviceProcAddr(D.dev,"vk"#n); if(!vk.n)return false
    L(DestroyDevice); L(GetDeviceQueue); L(DeviceWaitIdle);
    L(CreateCommandPool); L(DestroyCommandPool); L(AllocateCommandBuffers);
    L(BeginCommandBuffer); L(EndCommandBuffer); L(ResetCommandBuffer);
    L(CmdBeginRenderPass); L(CmdEndRenderPass); L(CmdBindPipeline);
    L(CmdBindVertexBuffers); L(CmdBindDescriptorSets); L(CmdDraw);
    L(CmdPushConstants); L(CmdSetViewport); L(CmdSetScissor);
    L(CmdPipelineBarrier); L(CmdCopyBufferToImage); L(CmdCopyImageToBuffer);
    L(CmdClearColorImage); L(CmdClearDepthStencilImage); L(CmdClearAttachments);
    L(QueueSubmit); L(CreateFence); L(DestroyFence); L(WaitForFences); L(ResetFences);
    L(CreateBuffer); L(DestroyBuffer); L(BindBufferMemory);
    L(CreateImage); L(DestroyImage); L(BindImageMemory);
    L(CreateImageView); L(DestroyImageView); L(CreateSampler); L(DestroySampler);
    L(CreateShaderModule); L(DestroyShaderModule);
    L(CreateDescriptorSetLayout); L(DestroyDescriptorSetLayout);
    L(CreateDescriptorPool); L(ResetDescriptorPool);
    L(AllocateDescriptorSets); L(UpdateDescriptorSets);
    L(CreatePipelineLayout); L(DestroyPipelineLayout);
    L(CreateGraphicsPipelines); L(DestroyPipeline);
    L(CreateRenderPass); L(DestroyRenderPass);
    L(CreateFramebuffer); L(DestroyFramebuffer);
    L(AllocateMemory); L(FreeMemory); L(MapMemory); L(UnmapMemory);
    L(GetBufferMemoryRequirements); L(GetImageMemoryRequirements);
#undef L
    return true;
}
static bool Init() {
    if(D.tried) return D.avail;
    D.tried=true;
    const char* env=getenv("ROWAN_VK");
    if(env&&env[0]=='0'&&env[1]==0) return false;
#define VK_FAIL(x) do{ if(getenv("ROWAN_VK_DEBUG")) {\
        fprintf(stderr,"[vk] init fail: %s\n",x);} \
        return false; }while(0)
    if(!LoadFns()){VK_FAIL("LoadFns");}
    VkApplicationInfo app={VK_STRUCTURE_TYPE_APPLICATION_INFO};
    app.pApplicationName="rowan-compat"; app.apiVersion=VK_API_VERSION_1_0;
    VkInstanceCreateInfo ici={VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
    ici.pApplicationInfo=&app;
    if(V(CreateInstance)(&ici,0,&D.inst)!=VK_SUCCESS) VK_FAIL("CreateInstance");
    uint32_t n=0;
    V(EnumeratePhysicalDevices)(D.inst,&n,0);
    if(!n) VK_FAIL("no physical devices");
    VkPhysicalDevice* pds=(VkPhysicalDevice*)alloca(n*sizeof(*pds));
    V(EnumeratePhysicalDevices)(D.inst,&n,pds);
    int best=-1,bscore=-1;
    for(uint32_t i=0;i<n;i++){
        VkPhysicalDeviceProperties pr;
        V(GetPhysicalDeviceProperties)(pds[i],&pr);
        int s= pr.deviceType==VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU ? 3
             : pr.deviceType==VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU ? 2
             : pr.deviceType==VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU ? 1 : 0;
        if(s>bscore){bscore=s;best=(int)i;}
    }
    if(best<0) VK_FAIL("device pick");
    D.pd=pds[best];
    uint32_t qn=0;
    V(GetPhysicalDeviceQueueFamilyProperties)(D.pd,&qn,0);
    VkQueueFamilyProperties* qp=(VkQueueFamilyProperties*)alloca(qn*sizeof(*qp));
    V(GetPhysicalDeviceQueueFamilyProperties)(D.pd,&qn,qp);
    D.qfam=qn;
    for(uint32_t i=0;i<qn;i++)
        if(qp[i].queueFlags&VK_QUEUE_GRAPHICS_BIT){D.qfam=i;break;}
    if(D.qfam>=qn) VK_FAIL("no gfx queue");
    V(GetPhysicalDeviceMemoryProperties)(D.pd,&D.memProps);
    float prio=1.0f;
    VkDeviceQueueCreateInfo qi={VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
    qi.queueFamilyIndex=D.qfam; qi.queueCount=1; qi.pQueuePriorities=&prio;
    VkDeviceCreateInfo di={VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
    di.queueCreateInfoCount=1; di.pQueueCreateInfos=&qi;
    if(V(CreateDevice)(D.pd,&di,0,&D.dev)!=VK_SUCCESS) VK_FAIL("CreateDevice");
    if(!LoadDevFns()) VK_FAIL("LoadDevFns");
    V(GetDeviceQueue)(D.dev,D.qfam,0,&D.queue);
    VkCommandPoolCreateInfo pi={VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
    pi.flags=VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    pi.queueFamilyIndex=D.qfam;
    if(V(CreateCommandPool)(D.dev,&pi,0,&D.cmdPool)!=VK_SUCCESS) VK_FAIL("cmdpool");
    VkCommandBufferAllocateInfo cb={VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    cb.commandPool=D.cmdPool; cb.level=VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cb.commandBufferCount=1;
    if(V(AllocateCommandBuffers)(D.dev,&cb,&D.cmd)!=VK_SUCCESS) VK_FAIL("cmdbuf");
    VkFenceCreateInfo fi={VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
    fi.flags=VK_FENCE_CREATE_SIGNALED_BIT;
    if(V(CreateFence)(D.dev,&fi,0,&D.fence)!=VK_SUCCESS) VK_FAIL("fence");
    D.vbCap=8u<<20;
    D.vb=MkBuf(D.vbCap,VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &D.vbMem,&D.vbMap);
    D.rbCap=64u<<20;                   // per-target readback slots
    D.rb=MkBuf(D.rbCap,VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &D.rbMem,&D.rbMap);
    D.ubCap=8u<<20;
    D.ub=MkBuf(D.ubCap,VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &D.ubMem,&D.ubMap);
    if(!D.vb||!D.rb||!D.ub) VK_FAIL("buffers");
    VkShaderModuleCreateInfo sm={VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
    sm.codeSize=d3dvk_vert_spv_len; sm.pCode=(const uint32_t*)d3dvk_vert_spv;
    if(V(CreateShaderModule)(D.dev,&sm,0,&D.vs)!=VK_SUCCESS) VK_FAIL("vs shader");
    sm.codeSize=d3dvk_frag_spv_len; sm.pCode=(const uint32_t*)d3dvk_frag_spv;
    if(V(CreateShaderModule)(D.dev,&sm,0,&D.fs)!=VK_SUCCESS) VK_FAIL("fs shader");
    sm.codeSize=d3dvk_fragw_spv_len; sm.pCode=(const uint32_t*)d3dvk_fragw_spv;
    if(V(CreateShaderModule)(D.dev,&sm,0,&D.fsW)!=VK_SUCCESS) VK_FAIL("fsw shader");
    VkDescriptorSetLayoutBinding bnd[3];
    for(int i=0;i<3;i++){
        memset(&bnd[i],0,sizeof(bnd[i]));
        bnd[i].binding=i;
        bnd[i].descriptorType=VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        bnd[i].descriptorCount=1; bnd[i].stageFlags=VK_SHADER_STAGE_FRAGMENT_BIT;
    }
    VkDescriptorSetLayoutCreateInfo dl={VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    dl.bindingCount=3; dl.pBindings=bnd;
    if(V(CreateDescriptorSetLayout)(D.dev,&dl,0,&D.dsl)!=VK_SUCCESS) VK_FAIL("dsl");
    VkPushConstantRange pr={VK_SHADER_STAGE_VERTEX_BIT|VK_SHADER_STAGE_FRAGMENT_BIT,0,128};
    VkPipelineLayoutCreateInfo pli={VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
    pli.setLayoutCount=1; pli.pSetLayouts=&D.dsl;
    pli.pushConstantRangeCount=1; pli.pPushConstantRanges=&pr;
    if(V(CreatePipelineLayout)(D.dev,&pli,0,&D.pl)!=VK_SUCCESS) VK_FAIL("layout");
    VkDescriptorPoolSize ps={VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,1024};
    VkDescriptorPoolCreateInfo dpi={VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    dpi.maxSets=341; dpi.poolSizeCount=1; dpi.pPoolSizes=&ps;
    if(V(CreateDescriptorPool)(D.dev,&dpi,0,&D.dp)!=VK_SUCCESS) VK_FAIL("pool");
    D.dummyImg=MkImage(VK_FORMAT_B8G8R8A8_UNORM,1,1,
        VK_IMAGE_USAGE_SAMPLED_BIT|VK_IMAGE_USAGE_TRANSFER_DST_BIT,&D.dummyMem);
    if(D.dummyImg){
        ToGeneral(D.dummyImg,VK_IMAGE_ASPECT_COLOR_BIT);
        D.dummyView=MkView(D.dummyImg,VK_FORMAT_B8G8R8A8_UNORM,
                           VK_IMAGE_ASPECT_COLOR_BIT);
        uint32_t wht=0xffffffff; memcpy(D.ubMap,&wht,4);
        BeginCmd();
        VkBufferImageCopy c={};
        c.imageExtent.width=1; c.imageExtent.height=1; c.imageExtent.depth=1;
        c.imageSubresource.aspectMask=VK_IMAGE_ASPECT_COLOR_BIT;
        c.imageSubresource.layerCount=1;
        V(CmdCopyBufferToImage)(D.cmd,D.ub,D.dummyImg,VK_IMAGE_LAYOUT_GENERAL,1,&c);
        Flush(true);
    }
    VkSamplerCreateInfo sci={VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
    V(CreateSampler)(D.dev,&sci,0,&D.dummySamp);
    // Predictable capacities — keep first-frames free of rehash stalls.
    D.tgts.reserve(16);  D.texes.reserve(512); D.pipes.reserve(64);
    D.samps.reserve(64); D.sets.reserve(256);
    D.avail=true;
    if(getenv("ROWAN_VK_DEBUG")){
        VkPhysicalDeviceProperties pr; V(GetPhysicalDeviceProperties)(D.pd,&pr);
        fprintf(stderr,"[vk] backend up: %s\n",pr.deviceName);
    }
#undef VK_FAIL
    return true;
}

// --- render-target setup -----------------------------------------------------------
static VkFormat DepthFmt(int bits) {
    if(bits>=32) return VK_FORMAT_D32_SFLOAT;
    return VK_FORMAT_D16_UNORM;          // 24bpp lands on D16 — close enough
}
static void TgtDestroy(Tgt& t) {
    if(t.fb){V(DestroyFramebuffer)(D.dev,t.fb,0);t.fb=0;}
    if(t.rtView){V(DestroyImageView)(D.dev,t.rtView,0);t.rtView=0;}
    if(t.rtImg){V(DestroyImage)(D.dev,t.rtImg,0);V(FreeMemory)(D.dev,t.rtMem,0);t.rtImg=0;}
    if(t.zView){V(DestroyImageView)(D.dev,t.zView,0);t.zView=0;}
    if(t.zImg){V(DestroyImage)(D.dev,t.zImg,0);V(FreeMemory)(D.dev,t.zMem,0);t.zImg=0;}
    if(t.rp){V(DestroyRenderPass)(D.dev,t.rp,0);t.rp=0;}
}
// Image/framebuffer creation for one target (fresh Tgt or dims/format change).
static bool TgtAlloc(Tgt& t,IDirectDrawSurface7* rt,IDirectDrawSurface7* zb,
                     VkFormat cf,VkFormat zf,int zbits) {
    TgtDestroy(t);
    t.rt=rt; t.zb=zb; t.rtFmt=cf;
    t.W=rt->desc.dwWidth; t.H=rt->desc.dwHeight;
    t.zFmt=zf; t.zbits=zbits;
    t.zbSurfGen=zb?zb->cpuGen-1:0;   // fresh depth image uploads once
    t.zbAhead=false; t.zbWanted=false;
    t.rtImg=MkImage(cf,t.W,t.H,
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT|VK_IMAGE_USAGE_TRANSFER_SRC_BIT
        |VK_IMAGE_USAGE_TRANSFER_DST_BIT,&t.rtMem);
    if(!t.rtImg) return false;
    ToGeneral(t.rtImg,VK_IMAGE_ASPECT_COLOR_BIT);
    t.rtView=MkView(t.rtImg,cf,VK_IMAGE_ASPECT_COLOR_BIT);
    if(!t.rtView) return false;
    if(zb){
        t.zImg=MkImage(zf,t.W,t.H,
            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
            |VK_IMAGE_USAGE_TRANSFER_SRC_BIT
            |VK_IMAGE_USAGE_TRANSFER_DST_BIT,&t.zMem);
        if(!t.zImg) return false;
        ToGeneral(t.zImg,VK_IMAGE_ASPECT_DEPTH_BIT);
        t.zView=MkView(t.zImg,zf,VK_IMAGE_ASPECT_DEPTH_BIT);
        if(!t.zView) return false;
    }
    VkAttachmentDescription att[2]; memset(att,0,sizeof(att));
    att[0].format=cf; att[0].samples=VK_SAMPLE_COUNT_1_BIT;
    att[0].loadOp=VK_ATTACHMENT_LOAD_OP_LOAD;
    att[0].storeOp=VK_ATTACHMENT_STORE_OP_STORE;
    att[0].stencilLoadOp=VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    att[0].stencilStoreOp=VK_ATTACHMENT_STORE_OP_DONT_CARE;
    att[0].initialLayout=VK_IMAGE_LAYOUT_GENERAL;
    att[0].finalLayout=VK_IMAGE_LAYOUT_GENERAL;
    if(zb){ att[1]=att[0]; att[1].format=zf;
            att[1].stencilLoadOp=VK_ATTACHMENT_LOAD_OP_LOAD; }
    VkAttachmentReference cr={0,VK_IMAGE_LAYOUT_GENERAL};
    VkAttachmentReference dr={1,VK_IMAGE_LAYOUT_GENERAL};
    VkSubpassDescription sp={};
    sp.pipelineBindPoint=VK_PIPELINE_BIND_POINT_GRAPHICS;
    sp.colorAttachmentCount=1; sp.pColorAttachments=&cr;
    sp.pDepthStencilAttachment=zb?&dr:0;
    VkRenderPassCreateInfo ri={VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
    ri.attachmentCount=zb?2:1; ri.pAttachments=att;
    ri.subpassCount=1; ri.pSubpasses=&sp;
    if(V(CreateRenderPass)(D.dev,&ri,0,&t.rp)!=VK_SUCCESS) return false;
    VkImageView views[2]={t.rtView,t.zView};
    VkFramebufferCreateInfo fbi={VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
    fbi.renderPass=t.rp; fbi.attachmentCount=zb?2:1; fbi.pAttachments=views;
    fbi.width=t.W; fbi.height=t.H; fbi.layers=1;
    if(V(CreateFramebuffer)(D.dev,&fbi,0,&t.fb)!=VK_SUCCESS) return false;
    t.rtSurfGen=rt->cpuGen-1;   // image starts stale -> first draw uploads
    t.gpuOwns=false;
    return true;
}
// Find or create the GPU target for this (rt,zb) pair.  Switching targets
// keeps the old one's images alive — the game ping-pongs between the back
// buffer and small prerender targets every frame, and a destroy/recreate
// per switch used to drain the pipeline each time.
static bool TgtFor(const Rast7::State& st) {
    IDirectDrawSurface7* rt=st.rt;
    if(!rt||!rt->desc.lpSurface||!DirectFmt(rt)) return false;
    if(rt==D.failed) return false;         // this rt alloc-failed before
    VkFormat cf=SurfFmt(rt);
    IDirectDrawSurface7* zb=st.zb;
    VkFormat zf=zb?DepthFmt(st.zbits):VK_FORMAT_UNDEFINED;
    TgtKey k={rt,zb};
    std::unordered_map<TgtKey,Tgt,TgtKeyHash>::iterator it=D.tgts.find(k);
    if(it!=D.tgts.end()){
        Tgt& t=it->second;
        if(t.rtFmt==cf&&t.W==rt->desc.dwWidth&&t.H==rt->desc.dwHeight
           &&t.zFmt==zf){
            if(D.cur!=&t){EndPass();D.cur=&t;}
            return true;
        }
        // Same surface pair, new dims/format: rare — drop and recreate below
        // (the readback slot may also be undersized for the new dims).
        Flush(true);
        if(D.cur==&t)D.cur=0;
        TgtDestroy(t);
        D.tgts.erase(it);
    }
    // New target.  Bound the cache: evict the first non-current target when
    // full (its lpSurface stays CPU-authoritative; re-entry re-uploads).
    if(D.tgts.size()>=8){
        Flush(true);
        for(std::unordered_map<TgtKey,Tgt,TgtKeyHash>::iterator e=D.tgts.begin();e!=D.tgts.end();++e)
            if(&e->second!=D.cur){TgtDestroy(e->second);D.tgts.erase(e);break;}
    }
    // Readback slot: colour bytes (worst case 4bpp) + aligned depth bytes.
    VkDeviceSize need=(VkDeviceSize)rt->desc.dwWidth*rt->desc.dwHeight*4;
    VkDeviceSize zoff=(need+255)&~(VkDeviceSize)255;
    need=zoff+(VkDeviceSize)rt->desc.dwWidth*rt->desc.dwHeight*4;
    if(D.rbAlloc+need>D.rbCap){
        // Compact: land everything, then re-slot every surviving target.
        Flush(true);
        D.rbAlloc=0;
        for(std::unordered_map<TgtKey,Tgt,TgtKeyHash>::iterator e=D.tgts.begin();e!=D.tgts.end();++e){
            Tgt& t=e->second;
            VkDeviceSize cw=(VkDeviceSize)t.W*t.H*4;
            t.rbOff=D.rbAlloc; t.zbSlotOff=(cw+255)&~(VkDeviceSize)255;
            D.rbAlloc+=t.zbSlotOff+(VkDeviceSize)t.W*t.H*4;
        }
        if(D.rbAlloc+need>D.rbCap){D.failed=rt;return false;}
    }
    Tgt t; memset(&t,0,sizeof(t));
    t.rbOff=D.rbAlloc; t.zbSlotOff=zoff;
    if(!TgtAlloc(t,rt,zb,cf,zf,st.zbits)){D.failed=rt;return false;}
    D.rbAlloc+=need;
    std::pair<std::unordered_map<TgtKey,Tgt,TgtKeyHash>::iterator,bool> r=
        D.tgts.insert(std::make_pair(k,t));
    D.sets.clear();
    D.failed=0;
    EndPass(); D.cur=&r.first->second;
    return true;
}

// --- texture upload -----------------------------------------------------------------
static Tex* TexFor(IDirectDrawSurface7* s) {
    if(!s||!s->desc.lpSurface) return 0;
    std::unordered_map<IDirectDrawSurface7*,Tex>::iterator it=D.texes.find(s);
    if(it!=D.texes.end()) return &it->second;
    VkFormat f=SurfFmt(s);
    DWORD w=s->desc.dwWidth,h=s->desc.dwHeight;
    if(!w||!h) return 0;
    Tex t; memset(&t,0,sizeof(t));
    t.img=MkImage(f,w,h,VK_IMAGE_USAGE_SAMPLED_BIT|VK_IMAGE_USAGE_TRANSFER_DST_BIT,&t.mem);
    if(!t.img) return 0;
    ToGeneral(t.img,VK_IMAGE_ASPECT_COLOR_BIT);
    t.view=MkView(t.img,f,VK_IMAGE_ASPECT_COLOR_BIT);
    if(!t.view){V(DestroyImage)(D.dev,t.img,0);V(FreeMemory)(D.dev,t.mem,0);return 0;}
    t.fmt=f; t.w=w; t.h=h; t.gen=s->cpuGen-1;   // != cpuGen -> uploads once
    std::pair<std::unordered_map<IDirectDrawSurface7*,Tex>::iterator,bool> r=
        D.texes.insert(std::make_pair(s,t));
    return &r.first->second;
}
// lpSurface -> B8G8R8A8 rows for formats without a native VkFormat.
static void ConvertUpload(IDirectDrawSurface7* s,BYTE* dst,DWORD pitch) {
    Rast7::PixFmt f; Rast7::DecodePF(s->desc.ddpfPixelFormat,f);
    Rast7::Surf ss; ss.s=s; ss.f=f; ss.bpp=f.bits;
    for(DWORD y=0;y<s->desc.dwHeight;y++){
        uint32_t* row=(uint32_t*)(dst+y*pitch);
        for(DWORD x=0;x<s->desc.dwWidth;x++){
            // 0xAARRGGBB little-endian bytes are B,G,R,A = B8G8R8A8 verbatim.
            row[x]=Rast7::UnpackPx(ss,Rast7::ReadPx(s,x,y));
        }
    }
}
// Staging is bump-allocated (D.ubOff, reset per cmd buffer by BeginCmd after
// the in-flight drain): every recorded copy reads the offset written at record
// time, so several uploads can share one submit.
static void UploadTex(Tex& t,IDirectDrawSurface7* s) {
    if(t.gen==s->cpuGen) return;
    uint64_t t0=NowNs();
    BeginCmd(); EndPass();
    DWORD bpp=s->desc.ddpfPixelFormat.dwRGBBitCount/8;
    bool direct=DirectFmt(s);
    VkDeviceSize rowBytes=direct?(VkDeviceSize)t.w*bpp:(VkDeviceSize)t.w*4;
    VkDeviceSize need=rowBytes*t.h;
    if(need>D.ubCap-D.ubOff){Flush(true);BeginCmd();}
    if(need>D.ubCap){t.gen=s->cpuGen;return;}
    BYTE* stage=D.ubMap+D.ubOff;
    if(direct)
        for(DWORD y=0;y<t.h;y++)
            memcpy(stage+y*rowBytes,(const BYTE*)s->desc.lpSurface+y*s->desc.lPitch,
                   (size_t)rowBytes);
    else
        ConvertUpload(s,stage,(DWORD)rowBytes);
    ImgBarrier(t.img,VK_ACCESS_SHADER_READ_BIT,VK_ACCESS_TRANSFER_WRITE_BIT,
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_IMAGE_ASPECT_COLOR_BIT);
    VkBufferImageCopy c={};
    c.bufferOffset=D.ubOff;
    c.bufferRowLength=t.w; c.bufferImageHeight=t.h;
    c.imageExtent.width=t.w; c.imageExtent.height=t.h; c.imageExtent.depth=1;
    c.imageSubresource.aspectMask=VK_IMAGE_ASPECT_COLOR_BIT;
    c.imageSubresource.layerCount=1;
    V(CmdCopyBufferToImage)(D.cmd,D.ub,t.img,VK_IMAGE_LAYOUT_GENERAL,1,&c);
    ImgBarrier(t.img,VK_ACCESS_TRANSFER_WRITE_BIT,VK_ACCESS_SHADER_READ_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        VK_IMAGE_ASPECT_COLOR_BIT);
    D.ubOff+=(need+255)&~(VkDeviceSize)255;
    t.gen=s->cpuGen;
    if(PerfOn()){P.upTexNs+=NowNs()-t0;P.nUpTex++;}
}

// --- pipeline + state -----------------------------------------------------------------
static VkBlendFactor BlendF(DWORD b) {
    switch(b){
        case D3DBLEND_ZERO:          return VK_BLEND_FACTOR_ZERO;
        case D3DBLEND_ONE:           return VK_BLEND_FACTOR_ONE;
        case D3DBLEND_SRCCOLOR:      return VK_BLEND_FACTOR_SRC_COLOR;
        case D3DBLEND_INVSRCCOLOR:   return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
        case D3DBLEND_SRCALPHA:      return VK_BLEND_FACTOR_SRC_ALPHA;
        case D3DBLEND_INVSRCALPHA:   return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        case D3DBLEND_DESTALPHA:     return VK_BLEND_FACTOR_DST_ALPHA;
        case D3DBLEND_INVDESTALPHA:  return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
        case D3DBLEND_DESTCOLOR:     return VK_BLEND_FACTOR_DST_COLOR;
        case D3DBLEND_INVDESTCOLOR:  return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
        case D3DBLEND_SRCALPHASAT:   return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
        default:                     return VK_BLEND_FACTOR_ONE;
    }
}
static VkCompareOp CmpF(DWORD f) {
    switch(f){
        case D3DCMP_NEVER:       return VK_COMPARE_OP_NEVER;
        case D3DCMP_LESS:        return VK_COMPARE_OP_LESS;
        case D3DCMP_EQUAL:       return VK_COMPARE_OP_EQUAL;
        case D3DCMP_LESSEQUAL:   return VK_COMPARE_OP_LESS_OR_EQUAL;
        case D3DCMP_GREATER:     return VK_COMPARE_OP_GREATER;
        case D3DCMP_NOTEQUAL:    return VK_COMPARE_OP_NOT_EQUAL;
        case D3DCMP_GREATEREQUAL:return VK_COMPARE_OP_GREATER_OR_EQUAL;
        default:                 return VK_COMPARE_OP_ALWAYS;
    }
}
static VkPipeline PipeFor(VkPrimitiveTopology topo,const Rast7::State& st) {
    DWORD ze=st.rs[D3DRENDERSTATE_ZENABLE];
    DWORD zfn=st.rs[D3DRENDERSTATE_ZFUNC]; if(!zfn)zfn=D3DCMP_LESSEQUAL;
    bool zt = ze!=0&&ze!=D3DZB_FALSE&&D.cur->zb!=0;
    bool zw = st.rs[D3DRENDERSTATE_ZWRITEENABLE]!=0;
    bool ab = st.rs[D3DRENDERSTATE_ALPHABLENDENABLE]!=0;
    DWORD sb=st.rs[D3DRENDERSTATE_SRCBLEND]; if(!sb)sb=D3DBLEND_ONE;
    DWORD db=st.rs[D3DRENDERSTATE_DESTBLEND];
    if(sb==D3DBLEND_BOTHSRCALPHA){sb=D3DBLEND_SRCALPHA;db=D3DBLEND_INVSRCALPHA;}
    if(sb==D3DBLEND_BOTHINVSRCALPHA){sb=D3DBLEND_INVSRCALPHA;db=D3DBLEND_SRCALPHA;}
    uint64_t key=(uint64_t)topo
               | (uint64_t)(zfn&15)<<3
               | (uint64_t)(zt?1:0)<<7 | (uint64_t)(zw?1:0)<<8
               | (uint64_t)(ze==D3DZB_USEW?1:0)<<9
               | (uint64_t)(ab?1:0)<<10
               | (uint64_t)(sb&31)<<11 | (uint64_t)(db&31)<<16
               | (uint64_t)(D.cur->rtFmt&0x3ff)<<21 | (uint64_t)(D.cur->zFmt&0x3ff)<<31;
    std::unordered_map<uint64_t,VkPipeline>::iterator it=D.pipes.find(key);
    if(it!=D.pipes.end()) return it->second;
    VkVertexInputBindingDescription bd={0,sizeof(GVert),VK_VERTEX_INPUT_RATE_VERTEX};
    VkVertexInputAttributeDescription ad[8]; memset(ad,0,sizeof(ad));
    ad[0].location=0; ad[0].format=VK_FORMAT_R32G32_SFLOAT;  ad[0].offset=0;
    ad[1].location=1; ad[1].format=VK_FORMAT_R32_SFLOAT;     ad[1].offset=8;
    ad[2].location=2; ad[2].format=VK_FORMAT_R32_SFLOAT;     ad[2].offset=12;
    ad[3].location=3; ad[3].format=VK_FORMAT_B8G8R8A8_UNORM; ad[3].offset=16;
    ad[4].location=4; ad[4].format=VK_FORMAT_B8G8R8A8_UNORM; ad[4].offset=20;
    ad[5].location=5; ad[5].format=VK_FORMAT_R32G32_SFLOAT;  ad[5].offset=24;
    ad[6].location=6; ad[6].format=VK_FORMAT_R32G32_SFLOAT;  ad[6].offset=32;
    ad[7].location=7; ad[7].format=VK_FORMAT_R32G32_SFLOAT;  ad[7].offset=40;
    VkPipelineVertexInputStateCreateInfo vi={VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
    vi.vertexBindingDescriptionCount=1; vi.pVertexBindingDescriptions=&bd;
    vi.vertexAttributeDescriptionCount=8; vi.pVertexAttributeDescriptions=ad;
    VkPipelineInputAssemblyStateCreateInfo ia={VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
    ia.topology=topo;
    VkPipelineViewportStateCreateInfo vp={VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
    vp.viewportCount=1; vp.scissorCount=1;
    VkDynamicState dyn[2]={VK_DYNAMIC_STATE_VIEWPORT,VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dy={VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
    dy.dynamicStateCount=2; dy.pDynamicStates=dyn;
    VkPipelineRasterizationStateCreateInfo rs={VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
    rs.polygonMode=VK_POLYGON_MODE_FILL; rs.cullMode=VK_CULL_MODE_NONE;
    rs.frontFace=VK_FRONT_FACE_COUNTER_CLOCKWISE; rs.lineWidth=1.0f;
    VkPipelineMultisampleStateCreateInfo ms={VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
    ms.rasterizationSamples=VK_SAMPLE_COUNT_1_BIT;
    VkPipelineDepthStencilStateCreateInfo ds={VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
    ds.depthTestEnable=zt?VK_TRUE:VK_FALSE;
    ds.depthWriteEnable=(zt&&zw)?VK_TRUE:VK_FALSE;
    ds.depthCompareOp=CmpF(zfn);
    VkPipelineColorBlendAttachmentState ba={};
    ba.colorWriteMask=VK_COLOR_COMPONENT_R_BIT|VK_COLOR_COMPONENT_G_BIT
                     |VK_COLOR_COMPONENT_B_BIT|VK_COLOR_COMPONENT_A_BIT;
    ba.blendEnable=ab?VK_TRUE:VK_FALSE;
    ba.srcColorBlendFactor=BlendF(sb); ba.dstColorBlendFactor=BlendF(db);
    ba.colorBlendOp=VK_BLEND_OP_ADD;
    ba.srcAlphaBlendFactor=ba.srcColorBlendFactor;
    ba.dstAlphaBlendFactor=ba.dstColorBlendFactor;
    ba.alphaBlendOp=VK_BLEND_OP_ADD;
    VkPipelineColorBlendStateCreateInfo cb={VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
    cb.attachmentCount=1; cb.pAttachments=&ba;
    VkPipelineShaderStageCreateInfo ss[2]; memset(ss,0,sizeof(ss));
    ss[0].sType=VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    ss[0].stage=VK_SHADER_STAGE_VERTEX_BIT; ss[0].module=D.vs; ss[0].pName="main";
    ss[1]=ss[0]; ss[1].stage=VK_SHADER_STAGE_FRAGMENT_BIT;
    ss[1].module=(ze==D3DZB_USEW)?D.fsW:D.fs;
    VkGraphicsPipelineCreateInfo gi={VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
    gi.stageCount=2; gi.pStages=ss;
    gi.pVertexInputState=&vi; gi.pInputAssemblyState=&ia;
    gi.pViewportState=&vp; gi.pRasterizationState=&rs;
    gi.pMultisampleState=&ms; gi.pDepthStencilState=&ds;
    gi.pColorBlendState=&cb; gi.pDynamicState=&dy;
    gi.layout=D.pl; gi.renderPass=D.cur->rp;
    VkPipeline p;
    if(V(CreateGraphicsPipelines)(D.dev,VK_NULL_HANDLE,1,&gi,0,&p)!=VK_SUCCESS)
        return 0;
    D.pipes[key]=p;
    if(PerfOn()) P.nPipeMk++;
    return p;
}
static VkSampler SampFor(const Rast7::State& st,int i) {
    DWORD addr=st.ss[i][D3DTSS_ADDRESS]; if(!addr)addr=D3DTADDRESS_WRAP;
    bool bilin=st.ss[i][D3DTSS_MAGFILTER]==D3DTFG_LINEAR
            ||st.ss[i][D3DTSS_MINFILTER]==D3DTFG_LINEAR
            ||st.ss[i][D3DTSS_MAGFILTER]==D3DTFG_ANISOTROPIC
            ||st.ss[i][D3DTSS_MINFILTER]==D3DTFG_ANISOTROPIC;
    uint32_t key=(addr&7)|((bilin?1u:0u)<<3);
    std::unordered_map<uint32_t,VkSampler>::iterator it=D.samps.find(key);
    if(it!=D.samps.end()) return it->second;
    VkSamplerCreateInfo si={VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
    si.magFilter=si.minFilter=bilin?VK_FILTER_LINEAR:VK_FILTER_NEAREST;
    si.mipmapMode=VK_SAMPLER_MIPMAP_MODE_NEAREST;
    si.addressModeU=si.addressModeV=
        addr==D3DTADDRESS_CLAMP ? VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE
      : addr==D3DTADDRESS_MIRROR? VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT
      : VK_SAMPLER_ADDRESS_MODE_REPEAT;
    si.addressModeW=si.addressModeU;
    si.maxLod=0.0f;
    VkSampler s;
    if(V(CreateSampler)(D.dev,&si,0,&s)!=VK_SUCCESS) return D.dummySamp;
    D.samps[key]=s;
    return s;
}
// Upload the stage textures the next draw samples (outside the render pass),
// then push constants + bind the cached descriptor set inside it.
static void PrepTextures(const Rast7::State& st,int nStage) {
    for(int i=0;i<nStage;i++){
        IDirectDrawSurface7* s=st.tex[i].s;
        if(!s) continue;
        Tex* t=TexFor(s);
        if(t&&t->gen!=s->cpuGen) UploadTex(*t,s);
    }
}
static void BindState(const Rast7::State& st,int nStage) {
    PC pc; memset(&pc,0,sizeof(pc));
    pc.xform[0]= 2.0f/D.cur->W; pc.xform[1]= 2.0f/D.cur->H;
    pc.xform[2]=-1.0f;       pc.xform[3]=-1.0f;
    float zmax=D.cur->zbits>=32?4294967295.0f:D.cur->zbits>=24?16777215.0f:65535.0f;
    DWORD zb=st.rs[D3DRENDERSTATE_ZBIAS];
    pc.zmisc[0]=zmax>0?(float)zb*(float)(1u<<(D.cur->zbits>8?D.cur->zbits-8:0))/zmax:0;
    memcpy(&pc.zmisc[1],&st.fogStart,4);
    memcpy(&pc.zmisc[2],&st.fogEnd,4);
    DWORD fc=st.fogCol;
    pc.fogCol[0]=((fc>>16)&255)/255.0f; pc.fogCol[1]=((fc>>8)&255)/255.0f;
    pc.fogCol[2]=(fc&255)/255.0f;       pc.fogCol[3]=((fc>>24)&255)/255.0f;
    DWORD tf=st.rs[D3DRENDERSTATE_TEXTUREFACTOR];
    pc.tfac[0]=((tf>>16)&255)/255.0f; pc.tfac[1]=((tf>>8)&255)/255.0f;
    pc.tfac[2]=(tf&255)/255.0f;       pc.tfac[3]=((tf>>24)&255)/255.0f;
    uint32_t cfgx=(uint32_t)nStage;
    if(st.rs[D3DRENDERSTATE_TEXTUREPERSPECTIVE])cfgx|=16;
    if(st.rs[D3DRENDERSTATE_SPECULARENABLE])    cfgx|=32;
    if(st.rs[D3DRENDERSTATE_FOGENABLE]){
        if(st.rs[D3DRENDERSTATE_FOGTABLEMODE]==D3DFOG_LINEAR) cfgx|=64;
        else if(st.rs[D3DRENDERSTATE_FOGVERTEXMODE]==D3DFOG_LINEAR) cfgx|=128;
    }
    if(st.rs[D3DRENDERSTATE_COLORKEYENABLE])cfgx|=256;
    if(st.rs[D3DRENDERSTATE_ZENABLE]==D3DZB_USEW)cfgx|=512;
    pc.cfg[0]=cfgx; pc.cfg[1]=pc.cfg[2]=0;
    static int trace=-1; if(trace<0)trace=getenv("ROWAN_VK_TRACE")?1:0;
    if(trace)
        fprintf(stderr,"[vk] cfgx=%08x ze=%lu zfn=%lu zwe=%lu\n",cfgx,
            (unsigned long)st.rs[D3DRENDERSTATE_ZENABLE],
            (unsigned long)st.rs[D3DRENDERSTATE_ZFUNC],
            (unsigned long)st.rs[D3DRENDERSTATE_ZWRITEENABLE]);
    VkImageView views[3]={D.dummyView,D.dummyView,D.dummyView};
    VkSampler  samps[3]={D.dummySamp,D.dummySamp,D.dummySamp};
    uint32_t sc=0;
    for(int i=0;i<nStage;i++){
        IDirectDrawSurface7* s=st.tex[i].s;
        if(s){
            Tex* t=TexFor(s);
            if(t) views[i]=t->view;
            if((cfgx&256)&&(s->ckeyFlags&DDCKEY_SRCBLT)&&!pc.cfg[1]&&!pc.cfg[2]){
                pc.cfg[1]=s->ckey.dwColorSpaceLowValue&0xffffff;
                pc.cfg[2]=s->ckey.dwColorSpaceHighValue&0xffffff;
            }
        }
        samps[i]=SampFor(st,i);
        sc^=(uint32_t)(uintptr_t)(void*)samps[i];
        DWORD tci=st.ss[i][D3DTSS_TEXCOORDINDEX]&3; if(tci>2)tci=2;
        pc.stg[i][0]=(st.ss[i][D3DTSS_COLOROP]&255)
                    |(st.ss[i][D3DTSS_ALPHAOP]&255)<<8
                    |tci<<16
                    |(s?1u:0u)<<24;
        pc.stg[i][1]=(st.ss[i][D3DTSS_COLORARG1]&0xff)
                    |(st.ss[i][D3DTSS_COLORARG2]&0xff)<<8
                    |(st.ss[i][D3DTSS_ALPHAARG1]&0xff)<<16
                    |(st.ss[i][D3DTSS_ALPHAARG2]&0xff)<<24;
    }
    V(CmdPushConstants)(D.cmd,D.pl,
        VK_SHADER_STAGE_VERTEX_BIT|VK_SHADER_STAGE_FRAGMENT_BIT,0,128,&pc);
    Dev::SetKey k; k.v0=views[0]; k.v1=views[1]; k.v2=views[2]; k.sc=sc;
    std::unordered_map<Dev::SetKey,VkDescriptorSet,Dev::SetKeyHash>::iterator it=D.sets.find(k);
    VkDescriptorSet set=0;
    if(it==D.sets.end()){
        VkDescriptorSetAllocateInfo ai={VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
        ai.descriptorPool=D.dp; ai.descriptorSetCount=1; ai.pSetLayouts=&D.dsl;
        if(V(AllocateDescriptorSets)(D.dev,&ai,&set)!=VK_SUCCESS){
            // pool exhausted mid-recording — flag it; Draw drains, resets and
            // retries the bind once.
            D.poolFail=true; return;
        }
        VkDescriptorImageInfo ii[3]; VkWriteDescriptorSet wr[3];
        for(int i=0;i<3;i++){
            ii[i].imageLayout=VK_IMAGE_LAYOUT_GENERAL;
            ii[i].imageView=views[i]; ii[i].sampler=samps[i];
            memset(&wr[i],0,sizeof(wr[i]));
            wr[i].sType=VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            wr[i].dstSet=set; wr[i].dstBinding=i; wr[i].descriptorCount=1;
            wr[i].descriptorType=VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            wr[i].pImageInfo=&ii[i];
        }
        V(UpdateDescriptorSets)(D.dev,3,wr,0,0);
        D.sets[k]=set;
    } else set=it->second;
    V(CmdBindDescriptorSets)(D.cmd,VK_PIPELINE_BIND_POINT_GRAPHICS,D.pl,
                           0,1,&set,0,0);
}

// --- rt upload (CPU writes made while the GPU owned it) -------------------------------
static void UploadRT() {
    Tgt& t=*D.cur;
    if(!t.rt||t.rtSurfGen==t.rt->cpuGen) return;
    uint64_t t0=NowNs();
    BeginCmd(); EndPass();
    DWORD bpp=t.rt->desc.ddpfPixelFormat.dwRGBBitCount/8;
    if(!bpp)bpp=2;
    VkDeviceSize tight=(VkDeviceSize)t.W*bpp;
    VkDeviceSize need=tight*t.H;
    if(need>D.ubCap-D.ubOff){Flush(true);BeginCmd();EndPass();}
    if(need>D.ubCap) return;
    BYTE* stage=D.ubMap+D.ubOff;
    for(DWORD y=0;y<t.H;y++)
        memcpy(stage+y*tight,(const BYTE*)t.rt->desc.lpSurface
               +y*t.rt->desc.lPitch,(size_t)tight);
    ImgBarrier(t.rtImg,VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        VK_ACCESS_TRANSFER_WRITE_BIT,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,VK_IMAGE_ASPECT_COLOR_BIT);
    VkBufferImageCopy c={};
    c.bufferOffset=D.ubOff;
    c.bufferRowLength=t.W; c.bufferImageHeight=t.H;
    c.imageExtent.width=t.W; c.imageExtent.height=t.H; c.imageExtent.depth=1;
    c.imageSubresource.aspectMask=VK_IMAGE_ASPECT_COLOR_BIT;
    c.imageSubresource.layerCount=1;
    V(CmdCopyBufferToImage)(D.cmd,D.ub,t.rtImg,VK_IMAGE_LAYOUT_GENERAL,1,&c);
    ImgBarrier(t.rtImg,VK_ACCESS_TRANSFER_WRITE_BIT,
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,VK_IMAGE_ASPECT_COLOR_BIT);
    D.ubOff+=(need+255)&~(VkDeviceSize)255;
    t.rtSurfGen=t.rt->cpuGen;
    if(PerfOn()){P.upRTns+=NowNs()-t0;P.nUpRT++;}
}

// z surface -> depth image.  16-bit non-w buffers are bytewise-identical to
// D16_UNORM (fast copy); w-buffered and wider surfaces translate per-pixel
// into the GPU encoding (float for D32, mapped w/(1+w) for USEW).
static void UploadZB(bool usew) {
    Tgt& t=*D.cur;
    if(!t.zb||t.zbSurfGen==t.zb->cpuGen) return;
    uint64_t t0=NowNs();
    BeginCmd(); EndPass();
    int zb=t.zbits;
    bool direct=(zb==16&&!usew);
    VkDeviceSize pxB=(t.zFmt==VK_FORMAT_D16_UNORM)?2:4;
    VkDeviceSize need=pxB*t.W*t.H;
    if(need>D.ubCap-D.ubOff){Flush(true);BeginCmd();EndPass();}
    if(need>D.ubCap) return;
    BYTE* stage=D.ubMap+D.ubOff;
    if(direct){
        for(DWORD y=0;y<t.H;y++)
            memcpy(stage+y*pxB*t.W,
                   (const BYTE*)t.zb->desc.lpSurface+y*t.zb->desc.lPitch,
                   (size_t)(pxB*t.W));
    } else {
        for(DWORD y=0;y<t.H;y++)for(DWORD x=0;x<t.W;x++){
            DWORD nz=Rast7::ReadZ(t.zb,x,y);
            if(pxB==2){
                uint16_t v;
                if(usew){ DWORD b=nz<<(32-zb); float w; memcpy(&w,&b,4);
                          v=(uint16_t)((w/(1.0f+w))*65535.0f); }
                else v=(uint16_t)(zb>=24?(nz>>(zb-16)):nz);
                ((uint16_t*)stage)[y*t.W+x]=v;
            } else {
                float v;
                if(usew){ memcpy(&v,&nz,4); v=v/(1.0f+v); }
                else v=(float)((double)nz/(zb>=32?4294967295.0:zb>=24?16777215.0:65535.0));
                ((float*)stage)[y*t.W+x]=v;
            }
        }
    }
    ImgBarrier(t.zImg,VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
        VK_ACCESS_TRANSFER_WRITE_BIT,
        VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,VK_IMAGE_ASPECT_DEPTH_BIT);
    VkBufferImageCopy c={};
    c.bufferOffset=D.ubOff;
    c.bufferRowLength=t.W; c.bufferImageHeight=t.H;
    c.imageExtent.width=t.W; c.imageExtent.height=t.H; c.imageExtent.depth=1;
    c.imageSubresource.aspectMask=VK_IMAGE_ASPECT_DEPTH_BIT;
    c.imageSubresource.layerCount=1;
    V(CmdCopyBufferToImage)(D.cmd,D.ub,t.zImg,VK_IMAGE_LAYOUT_GENERAL,1,&c);
    ImgBarrier(t.zImg,VK_ACCESS_TRANSFER_WRITE_BIT,
        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT|VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,VK_IMAGE_ASPECT_DEPTH_BIT);
    D.ubOff+=(need+255)&~(VkDeviceSize)255;
    t.zbSurfGen=t.zb->cpuGen;
    if(PerfOn()){P.upZBns+=NowNs()-t0;P.nUpZB++;}
}

static void EnsurePass() {
    BeginCmd();
    if(D.inPass) return;
    VkRenderPassBeginInfo bi={VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
    bi.renderPass=D.cur->rp; bi.framebuffer=D.cur->fb;
    bi.renderArea.extent.width=D.cur->W; bi.renderArea.extent.height=D.cur->H;
    V(CmdBeginRenderPass)(D.cmd,&bi,VK_SUBPASS_CONTENTS_INLINE);
    D.inPass=true;
    VkViewport vp={0,0,(float)D.cur->W,(float)D.cur->H,0,1};
    VkRect2D r={{0,0},{D.cur->W,D.cur->H}};
    V(CmdSetViewport)(D.cmd,0,1,&vp);
    V(CmdSetScissor)(D.cmd,0,1,&r);
}

// --- public API ----------------------------------------------------------------------
// Draw a decoded-vertex batch on the GPU.  Returns false when the caller
// should use the software rasterizer instead.
static bool Draw(const Rast7::State& st,D3DPRIMITIVETYPE pt,
                 const Rast7::Vert* v,DWORD n) {
    if(!n) return true;
    std::lock_guard<std::mutex> lk(vkMutex);
    if(PerfOn()) P.nDraws++;
    if(!Init()||!TgtFor(st)) return false;
    VkPrimitiveTopology topo;
    switch(pt){
        case D3DPT_TRIANGLELIST: case D3DPT_TRIANGLESTRIP: case D3DPT_TRIANGLEFAN:
            topo=VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; break;
        case D3DPT_LINELIST: case D3DPT_LINESTRIP:
            topo=VK_PRIMITIVE_TOPOLOGY_LINE_LIST; break;
        case D3DPT_POINTLIST: topo=VK_PRIMITIVE_TOPOLOGY_POINT_LIST; break;
        default: return false;
    }
    int nStage=0;
    for(int i=0;i<3;i++){
        DWORD cop=st.ss[i][D3DTSS_COLOROP],aop=st.ss[i][D3DTSS_ALPHAOP];
        if(cop==D3DTOP_DISABLE&&aop==D3DTOP_DISABLE)break;
        nStage=i+1;
    }
    UploadRT();
    UploadZB(st.rs[D3DRENDERSTATE_ZENABLE]==D3DZB_USEW);
    PrepTextures(st,nStage);
    EnsurePass();
    VkPipeline p=PipeFor(topo,st);
    if(!p) return false;
    BindState(st,nStage);
    V(CmdBindPipeline)(D.cmd,VK_PIPELINE_BIND_POINT_GRAPHICS,p);
    if(D.poolFail){                       // descriptor pool ran dry mid-bind:
        D.poolFail=false;                 // drain, reset the pool, re-enter the
        Flush(true);                      // pass and bind again (rare path).
        V(ResetDescriptorPool)(D.dev,D.dp,0); D.sets.clear();
        EnsurePass();
        BindState(st,nStage);
        if(D.poolFail){D.poolFail=false;return false;}
        V(CmdBindPipeline)(D.cmd,VK_PIPELINE_BIND_POINT_GRAPHICS,p);
    }
    // Worst-case expansion is 3 verts per input vert (strip/fan -> tri list).
    if(D.vbOff+(VkDeviceSize)n*3*sizeof(GVert)>D.vbCap){
        Flush(true);EnsurePass();
        V(CmdBindPipeline)(D.cmd,VK_PIPELINE_BIND_POINT_GRAPHICS,p);
        BindState(st,nStage);
    }
    DWORD cap=(DWORD)(D.vbCap/sizeof(GVert));
    DWORD first=(DWORD)(D.vbOff/sizeof(GVert));
    GVert* out=(GVert*)(D.vbMap+D.vbOff);
    DWORD em=0;
#define GV(g,s) {g.x=s.x;g.y=s.y;g.z=s.z;g.rhw=s.rhw;g.col=s.col;g.spec=s.spec;\
    g.u0=s.u[0];g.v0=s.v[0];g.u1=s.u[1];g.v1=s.v[1];g.u2=s.u[2];g.v2=s.v[2];}
#define EM3(a,b,c) {if(first+em+3<=cap){\
    GV(out[em],a)GV(out[em+1],b)GV(out[em+2],c)em+=3;}}
// Software places lines/points at int(v+0.5) — pixel centers — while the
// vertex->ndc map targets fragment centers identically for triangles.  GVH
// shifts only line/point emission by +0.5 so both agree.
#define GVH(g,s) {GV(g,s) g.x+=0.5f;g.y+=0.5f;}
#define EM2(a,b)   {if(first+em+2<=cap){\
    GVH(out[em],a)GVH(out[em+1],b)em+=2;}}
#define EM1(a)     {if(first+em+1<=cap){\
    GVH(out[em],a)em++;}}
    if(pt==D3DPT_TRIANGLELIST){
        for(DWORD i=2;i<n;i+=3)
            if(!Rast7::CullWinding(st,v[i-2],v[i-1],v[i]))
                EM3(v[i-2],v[i-1],v[i])
    } else if(pt==D3DPT_TRIANGLESTRIP){
        for(DWORD i=2;i<n;i++){
            const Rast7::Vert&a=v[i-2],&b=v[i-1],&c=v[i];
            if(i&1){if(!Rast7::CullWinding(st,a,c,b))EM3(a,c,b)}
            else   {if(!Rast7::CullWinding(st,a,b,c))EM3(a,b,c)}
        }
    } else if(pt==D3DPT_TRIANGLEFAN){
        for(DWORD i=2;i<n;i++)
            if(!Rast7::CullWinding(st,v[0],v[i-1],v[i]))
                EM3(v[0],v[i-1],v[i])
    } else if(pt==D3DPT_LINELIST){
        for(DWORD i=1;i<n;i+=2)EM2(v[i-1],v[i])
    } else if(pt==D3DPT_LINESTRIP){
        for(DWORD i=1;i<n;i++)EM2(v[i-1],v[i])
    } else if(pt==D3DPT_POINTLIST){
        for(DWORD i=0;i<n;i++)EM1(v[i])
    }
#undef GV
#undef GVH
#undef EM3
#undef EM2
#undef EM1
    if(!em) return true;
    VkDeviceSize off=D.vbOff;
    V(CmdBindVertexBuffers)(D.cmd,0,1,&D.vb,&off);
    V(CmdDraw)(D.cmd,em,1,0,0);
    D.vbOff+=em*sizeof(GVert);
    D.cur->gpuOwns=true; D.cur->copyOwed=true;
    if(st.zb&&st.rs[D3DRENDERSTATE_ZWRITEENABLE]){
        D.cur->zbAhead=true;
        D.cur->zbUseW=(st.rs[D3DRENDERSTATE_ZENABLE]==D3DZB_USEW);
    }
    return true;
}

// DX7 Clear -> vkCmdClearAttachments inside the pass (covers rect lists too).
static bool Clear(IDirectDrawSurface7* rt,IDirectDrawSurface7* zb,int zbits,
                  DWORD nRect,LPD3DRECT rects,DWORD flags,D3DCOLOR col,float z) {
    std::lock_guard<std::mutex> lk(vkMutex);
    if(!Init()||!rt) return false;
    // The vk path only clears surfaces it already renders to — a rt we never
    // drew to has nothing on the GPU, so the caller falls back to the CPU
    // clear (which stays authoritative and uploads on first draw).
    Tgt* tgt=0;
    for(std::unordered_map<TgtKey,Tgt,TgtKeyHash>::iterator e=D.tgts.begin();e!=D.tgts.end();++e)
        if(e->second.rt==rt){tgt=&e->second;break;}
    if(!tgt) return false;
    if(D.cur!=tgt){EndPass();D.cur=tgt;}
    UploadRT();
    EnsurePass();
    VkClearAttachment att[2]; uint32_t na=0;
    VkClearColorValue cc;
    cc.float32[0]=((col>>16)&255)/255.0f; cc.float32[1]=((col>>8)&255)/255.0f;
    cc.float32[2]=(col&255)/255.0f;       cc.float32[3]=((col>>24)&255)/255.0f;
    if(flags&D3DCLEAR_TARGET){
        att[na].aspectMask=VK_IMAGE_ASPECT_COLOR_BIT;
        att[na].colorAttachment=0; att[na].clearValue.color=cc; na++;
    }
    if((flags&D3DCLEAR_ZBUFFER)&&zb&&zb==tgt->zb){
        att[na].aspectMask=VK_IMAGE_ASPECT_DEPTH_BIT;
        att[na].clearValue.depthStencil.depth=z<0?0:z>1?1:z;
        att[na].clearValue.depthStencil.stencil=0; na++;
    }
    if(!na) return true;
    VkClearRect r; r.baseArrayLayer=0; r.layerCount=1;
    if(nRect){
        for(DWORD i=0;i<nRect;i++){
            LONG x1=rects[i].x1<0?0:rects[i].x1, y1=rects[i].y1<0?0:rects[i].y1;
            LONG x2=rects[i].x2>(LONG)tgt->W?(LONG)tgt->W:rects[i].x2;
            LONG y2=rects[i].y2>(LONG)tgt->H?(LONG)tgt->H:rects[i].y2;
            if(x2<=x1||y2<=y1) continue;
            r.rect.offset.x=x1; r.rect.offset.y=y1;
            r.rect.extent.width=(DWORD)(x2-x1); r.rect.extent.height=(DWORD)(y2-y1);
            V(CmdClearAttachments)(D.cmd,na,att,1,&r);
        }
    } else {
        r.rect.offset.x=0; r.rect.offset.y=0;
        r.rect.extent.width=tgt->W; r.rect.extent.height=tgt->H;
        V(CmdClearAttachments)(D.cmd,na,att,1,&r);
    }
    if(flags&D3DCLEAR_TARGET){tgt->gpuOwns=true;tgt->copyOwed=true;}
    if((flags&D3DCLEAR_ZBUFFER)&&zb&&zb==tgt->zb){
        tgt->zbAhead=true; tgt->copyOwed=true;
        tgt->zbUseW=false;
    }
    return true;
}
// EndScene = the frame boundary: submit and land the readback.  Code outside
// the hooked surface methods reads lpSurface directly after EndScene (tests,
// the SDL present path), so the GPU image must be resident by the time
// EndScene returns — the fence wait is the frame's GPU render time.
static void EndFrame() {
    std::lock_guard<std::mutex> lk(vkMutex);
    if(D.avail) Flush(true);
}

// --- surface-facing hooks ------------------------------------------------------------
static void ReadSync(IDirectDrawSurface7* s) {
    std::lock_guard<std::mutex> lk(vkMutex);
    if(!D.avail||!s) return;
    for(std::unordered_map<TgtKey,Tgt,TgtKeyHash>::iterator e=D.tgts.begin();e!=D.tgts.end();++e){
        Tgt& t=e->second;
        if(s==t.rt&&t.gpuOwns){Flush(false);Drain();}
        // A CPU-side z read is the only thing that makes the buffered depth
        // copy worth converting — set zbWanted before Drain lands it.
        if(s==t.zb&&t.zbAhead){t.zbWanted=true;Flush(false);Drain();}
    }
}
static void WriteSync(IDirectDrawSurface7* s) {
    if(!s) return;
    std::lock_guard<std::mutex> lk(vkMutex);
    __atomic_add_fetch(&s->cpuGen,1,__ATOMIC_SEQ_CST);
    // CPU just wrote s: any buffered GPU readback for it is now stale, and
    // its images are behind lpSurface again until the next upload.
    for(std::unordered_map<TgtKey,Tgt,TgtKeyHash>::iterator e=D.tgts.begin();e!=D.tgts.end();++e){
        Tgt& t=e->second;
        if(s==t.rt){t.rbPending=false;t.gpuOwns=false;}
        if(s==t.zb){t.zbPending=false;t.zbAhead=false;t.zbWanted=false;}
    }
}
static void SurfaceGone(IDirectDrawSurface7* s) {
    std::lock_guard<std::mutex> lk(vkMutex);
    if(!D.avail||!s) return;
    Flush(true);
    D.sets.clear();                    // bound sets may reference destroyed views
    std::unordered_map<IDirectDrawSurface7*,Tex>::iterator it=D.texes.find(s);
    if(it!=D.texes.end()){
        Tex& t=it->second;
        if(t.view)V(DestroyImageView)(D.dev,t.view,0);
        if(t.img){V(DestroyImage)(D.dev,t.img,0);V(FreeMemory)(D.dev,t.mem,0);}
        D.texes.erase(it);
    }
    for(std::unordered_map<TgtKey,Tgt,TgtKeyHash>::iterator e=D.tgts.begin();e!=D.tgts.end();){
        Tgt& t=e->second;
        if(t.rt==s||t.zb==s){
            if(D.cur==&t)D.cur=0;
            TgtDestroy(t);
            e=D.tgts.erase(e);
        } else ++e;
    }
    if(s==D.failed)D.failed=0;
}

} // namespace RastVk
#endif // ROWAN_VK_LOADED

// Hook entry points used by ddraw.h's surface methods (RowanBltFormat-style);
// compile to no-ops when the loader is absent.
inline void RowanVkReadSync(IDirectDrawSurface7* s)
#if ROWAN_VK_LOADED
    { RastVk::ReadSync(s); }
#else
    { (void)s; }
#endif
inline void RowanVkWriteSync(IDirectDrawSurface7* s)
#if ROWAN_VK_LOADED
    { RastVk::WriteSync(s); }
#else
    { (void)s; }
#endif
inline void RowanVkSurfaceGone(IDirectDrawSurface7* s)
#if ROWAN_VK_LOADED
    { RastVk::SurfaceGone(s); }
#else
    { (void)s; }
#endif

// Device-facing entry points (EmitPrim/Clear/EndScene).  Each returns false
// when the Vulkan path can't honour the call, leaving the software fallback
// to run — callers then ReadSync first so GPU content lands in lpSurface.
inline bool RowanVkDraw(const Rast7::State& st,D3DPRIMITIVETYPE pt,
                        const Rast7::Vert* v,DWORD n)
#if ROWAN_VK_LOADED
    { return RastVk::Draw(st,pt,v,n); }
#else
    { (void)st;(void)pt;(void)v;(void)n; return false; }
#endif
inline bool RowanVkClear(IDirectDrawSurface7* rt,IDirectDrawSurface7* zb,
                         int zbits,DWORD nRect,LPD3DRECT rects,
                         DWORD flags,D3DCOLOR col,float z)
#if ROWAN_VK_LOADED
    { return RastVk::Clear(rt,zb,zbits,nRect,rects,flags,col,z); }
#else
    { (void)rt;(void)zb;(void)zbits;(void)nRect;(void)rects;
      (void)flags;(void)col;(void)z; return false; }
#endif
inline void RowanVkEndFrame()
#if ROWAN_VK_LOADED
    { RastVk::EndFrame(); }
#else
    {}
#endif

#endif // ROWAN_COMPAT_D3DVK_H
