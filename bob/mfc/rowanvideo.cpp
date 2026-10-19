// Native AVI playback behind the DirectShow interfaces BoB's frontend uses.
// Implements the port's streams.h surface (IGraphBuilder/IMediaControl/
// IMediaEventEx/IVideoWindow/IBasicAudio) with an ffmpeg decode pair:
// rawvideo pipe -> SDL overlay window, s16le pipe -> SDL_QueueAudio device.
// BoB's clips are real RIFF AVIs (MPEG-4/MP3).
#include "stdafx.h"
#include "mig.h"
#include "winmove.h"
#include "vfw.h"
#include <streams.h>      // port shim: DirectShow iface decls

#include <SDL.h>
#include "io_compat.h"   // resolve_ci_path: '\'->'/' + case-insensitive comps
#include <atomic>
#include <thread>
#include <string>
#include <vector>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

struct RowanVideoGraph;

// Each interface subobject's IUnknown::lpVtbl points at the embedded vtbl of
// one of these; `off` is the byte distance from that subobject's IUnknown
// back to the owning RowanVideoGraph.
struct RvVtbl { IUnknownVtbl vt; ptrdiff_t off; };

struct RowanVideoGraph : IGraphBuilder, IMediaControl, IMediaEventEx,
                         IVideoWindow, IBasicAudio
{
    std::atomic<long> refs{1};
    std::atomic<bool> stopFlag{false}, running{false}, eof{false};
    std::string file;
    long owner = 0, notifyHwnd = 0;
    long notifyMsg = 0;
    SDL_Window* vwin = nullptr;
    SDL_Renderer* vrend = nullptr;
    SDL_Texture* vtex = nullptr;
    SDL_AudioDeviceID adev = 0;
    pid_t vpid = -1, apid = -1;
    int vfd = -1, afd = -1;
    std::thread vthr, athr;
    long vx = 0, vy = 0, vw = 384, vh = 288;
    bool fullscreen = false;

    static RvVtbl vt_gb, vt_mc, vt_me, vt_vw, vt_ba;
    static HRESULT RowanQI(IUnknown* This, const void* riid, void** ppv);
    static ULONG   RowanAR(IUnknown* This);
    static ULONG   RowanRL(IUnknown* This);
    static RowanVideoGraph* Self(IUnknown* p) {
        const RvVtbl* vp = (const RvVtbl*)p->lpVtbl;
        return (RowanVideoGraph*)((char*)p - vp->off);
    }

    RowanVideoGraph()
    {
        vt_gb.off = (char*)(IUnknown*)(IGraphBuilder*)this - (char*)this;
        vt_mc.off = (char*)(IUnknown*)(IMediaControl*)this - (char*)this;
        vt_me.off = (char*)(IUnknown*)(IMediaEventEx*)this - (char*)this;
        vt_vw.off = (char*)(IUnknown*)(IVideoWindow*)this - (char*)this;
        vt_ba.off = (char*)(IUnknown*)(IBasicAudio*)this - (char*)this;
        ((IGraphBuilder*)this)->lpVtbl = &vt_gb.vt;
        ((IMediaControl*)this)->lpVtbl = &vt_mc.vt;
        ((IMediaEventEx*)this)->lpVtbl = &vt_me.vt;
        ((IVideoWindow*)this)->lpVtbl  = &vt_vw.vt;
        ((IBasicAudio*)this)->lpVtbl   = &vt_ba.vt;
    }
    ~RowanVideoGraph() { StopAll(); }

    HRESULT QI(const GUID& riid, void** ppv)
    {
        if (!ppv) return E_POINTER;
        if      (riid == IID_IUnknown || riid == IID_IGraphBuilder)
            *ppv = (IGraphBuilder*)this;
        else if (riid == IID_IMediaControl) *ppv = (IMediaControl*)this;
        else if (riid == IID_IMediaEventEx) *ppv = (IMediaEventEx*)this;
        else if (riid == IID_IVideoWindow)  *ppv = (IVideoWindow*)this;
        else if (riid == IID_IBasicAudio)   *ppv = (IBasicAudio*)this;
        else { *ppv = 0; return E_NOINTERFACE; }
        ++refs; return S_OK;
    }

    // ---- playback core ------------------------------------------------------
    void VideoPump()
    {
        const size_t fsz = (size_t)vw * vh * 2;
        std::vector<unsigned char> buf(fsz);
        while (!stopFlag) {
            size_t got = 0;
            while (got < fsz) {
                ssize_t n = read(vfd, buf.data() + got, fsz - got);
                if (n <= 0) goto out;
                got += (size_t)n;
            }
            if (vtex) {
                SDL_UpdateTexture(vtex, nullptr, buf.data(), vw * 2);
                SDL_RenderClear(vrend);
                SDL_RenderCopy(vrend, vtex, nullptr, nullptr);
                SDL_RenderPresent(vrend);
            }
        }
    out:
        eof = true;
        if (notifyHwnd && notifyMsg && !stopFlag)
            if (CWnd* w = CWnd::FromHandle((HWND)notifyHwnd))
                w->PostMessage((UINT)notifyMsg, 0, 0);
    }
    void AudioPump()
    {
        unsigned char ab[8192];
        while (!stopFlag) {
            ssize_t n = read(afd, ab, sizeof ab);
            if (n <= 0) break;
            if (adev) SDL_QueueAudio(adev, ab, (Uint32)n);
        }
    }
    static pid_t SpawnFF(const std::string& f, bool audio, int* fdout,
                         const char* scalearg)
    {
        int p[2];
        if (pipe(p)) return -1;
        pid_t pid = fork();
        if (pid == 0) {
            dup2(p[1], STDOUT_FILENO);
            close(p[0]); close(p[1]);
            int dn = open(audio ? "/tmp/ff_audio_err.log" : "/tmp/ff_video_err.log",
                          O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (dn < 0) dn = open("/dev/null", O_WRONLY);
            dup2(dn, STDERR_FILENO);
            if (!audio)
                execlp("ffmpeg", "ffmpeg", "-nostdin", "-v", "error", "-re",
                       "-i", f.c_str(), "-map", "0:v:0", "-vf", scalearg,
                       "-f", "rawvideo", "-pix_fmt", "rgb565", "-", (char*)nullptr);
            else
                execlp("ffmpeg", "ffmpeg", "-nostdin", "-v", "error", "-re",
                       "-i", f.c_str(), "-map", "0:a:0", "-f", "s16le",
                       "-ac", "2", "-ar", "22050", "-", (char*)nullptr);
            {
                char eb[80];
                int en = snprintf(eb, sizeof eb, "execlp ffmpeg failed errno=%d\n", errno);
                (void)en; write(STDERR_FILENO, eb, strlen(eb));
            }
            _exit(127);
        }
        close(p[1]);
        *fdout = p[0];
        return pid;
    }

    void StartAll()
    {
        if (running || file.empty()) return;
        fprintf(stderr, "[rowanvideo] Run() file=%s owner=%ld pos=%ld,%ld %ldx%ld\n",
                file.c_str(), owner, vx, vy, vw, vh);
        running = true; stopFlag = false; eof = false;

        int wx = 0, wy = 0, physW = 0, physH = 0;
        double sx = 1.0, sy = 1.0;
        WindowBackend* be = backend_from_hwnd((HWND)owner);
        if (be && be->window) {
            SDL_GetWindowPosition(be->window, &wx, &wy);
            SDL_GetWindowSize(be->window, &physW, &physH);
            if (be->virt_w > 0 && be->virt_h > 0) {
                sx = (double)physW / be->virt_w;
                sy = (double)physH / be->virt_h;
            }
        }
        int ox = wx + (int)(vx * sx), oy = wy + (int)(vy * sy);
        int ow = (int)(vw * sx), oh = (int)(vh * sy);
        if (fullscreen && be && be->window) { ox = wx; oy = wy; ow = physW; oh = physH; }

        vwin = SDL_CreateWindow("rowan-video", ox, oy, ow, oh,
                                SDL_WINDOW_BORDERLESS | SDL_WINDOW_SKIP_TASKBAR);
        if (vwin) {
            vrend = SDL_CreateRenderer(vwin, -1, SDL_RENDERER_SOFTWARE);
            if (vrend)
                vtex = SDL_CreateTexture(vrend, SDL_PIXELFORMAT_RGB565,
                                         SDL_TEXTUREACCESS_STREAMING, vw, vh);
            // NB: SDL_CreateRenderer may recreate the X window (new XID),
            // so window-level setup must run AFTER it, not before.
            // Empty input shape: clicks fall through to the game window
            // (intro-skip via click relies on this).
            MakeX11WindowClickThrough(vwin);
            if (be && be->window)
                // Child-of-main-window semantics: tracks window moves and
                // clips to it, like DirectShow's put_Owner + WS_CHILD.
                MFC_ReparentOverlayToWindow(vwin, be->window,
                    fullscreen ? 0 : (int)(vx * sx),
                    fullscreen ? 0 : (int)(vy * sy));
            else
                SDL_SetWindowAlwaysOnTop(vwin, SDL_TRUE);   // fallback
        }

        std::string flt = "scale=" + std::to_string(vw) + ":" + std::to_string(vh);
        vpid = SpawnFF(file, false, &vfd, flt.c_str());
        apid = SpawnFF(file, true, &afd, nullptr);
        if (afd >= 0) {
            SDL_AudioSpec want = {}, got;
            want.freq = 22050; want.format = AUDIO_S16SYS;
            want.channels = 2; want.samples = 2048;
            adev = SDL_OpenAudioDevice(nullptr, 0, &want, &got, 0);
            if (adev) SDL_PauseAudioDevice(adev, 0);
        }
        if (vfd >= 0) vthr = std::thread(&RowanVideoGraph::VideoPump, this);
        if (afd >= 0) athr = std::thread(&RowanVideoGraph::AudioPump, this);
    }

    void StopAll()
    {
        fprintf(stderr, "[rowanvideo] StopAll running=%d\n", (int)running);
        stopFlag = true;
        if (vpid > 0) kill(vpid, SIGKILL);
        if (apid > 0) kill(apid, SIGKILL);
        if (vfd >= 0) close(vfd), vfd = -1;
        if (afd >= 0) close(afd), afd = -1;
        if (vthr.joinable()) vthr.join();
        if (athr.joinable()) athr.join();
        if (vpid > 0) { int st; waitpid(vpid, &st, 0); vpid = -1; }
        if (apid > 0) { int st; waitpid(apid, &st, 0); apid = -1; }
        if (adev) { SDL_CloseAudioDevice(adev); adev = 0; }
        if (vtex) { SDL_DestroyTexture(vtex); vtex = nullptr; }
        if (vrend) { SDL_DestroyRenderer(vrend); vrend = nullptr; }
        if (vwin) { SDL_DestroyWindow(vwin); vwin = nullptr; }
        running = false;
    }

    // ---- IGraphBuilder ------------------------------------------------------
    HRESULT RenderFile(const WCHAR* f, const WCHAR*) override
    {
        char narrow[MAX_PATH];
        WideCharToMultiByte(0, 0, f, -1, narrow, sizeof narrow, nullptr, nullptr);
        file = resolve_ci_path(narrow);
        fprintf(stderr, "[rowanvideo] RenderFile %s -> %s\n", narrow, file.c_str());
        return S_OK;
    }
    // ---- IMediaControl ------------------------------------------------------
    HRESULT Run() override { StartAll(); return S_OK; }
    HRESULT Pause() override { return S_OK; }
    HRESULT Stop() override { StopAll(); return S_OK; }
    HRESULT StopWhenReady() override { StopAll(); return S_OK; }
    HRESULT GetState(long, long* pfs) override
        { if (pfs) *pfs = running ? 2 : 0; return S_OK; }
    HRESULT RenderFile(const WCHAR* f) override
        { return RenderFile(f, nullptr); }
    HRESULT AddSourceFilter(const WCHAR*, IUnknown**) override { return E_NOTIMPL; }
    HRESULT get_FilterCollection(IUnknown**) override { return E_NOTIMPL; }
    HRESULT get_RegFilterCollection(IUnknown**) override { return E_NOTIMPL; }
    // ---- IMediaEventEx ------------------------------------------------------
    HRESULT GetEvent(long* c, long* p1, long* p2, long) override
    {
        if (eof) { if (c)*c = EC_COMPLETE; if(p1)*p1=0; if(p2)*p2=0; eof=false; return S_OK; }
        return E_FAIL;
    }
    HRESULT WaitForCompletion(long, long* c) override
        { if (c)*c = EC_COMPLETE; return S_OK; }
    HRESULT CancelDefaultHandling(long) override { return S_OK; }
    HRESULT RestoreDefaultHandling(long) override { return S_OK; }
    HRESULT FreeEventParams(long, long, long) override { return S_OK; }
    HRESULT SetNotifyWindow(OAHWND h, long m, long) override
        { notifyHwnd = h; notifyMsg = m; return S_OK; }
    HRESULT SetNotifyFlags(long) override { return S_OK; }
    HRESULT GetNotifyFlags(long* f) override { if(f)*f=0; return S_OK; }
    // ---- IVideoWindow -------------------------------------------------------
    HRESULT put_Caption(const WCHAR*) override { return S_OK; }
    HRESULT get_Caption(WCHAR**) override { return E_NOTIMPL; }
    HRESULT put_WindowStyle(long) override { return S_OK; }
    HRESULT get_WindowStyle(long*) override { return S_OK; }
    HRESULT put_WindowStyleEx(long) override { return S_OK; }
    HRESULT get_WindowStyleEx(long*) override { return S_OK; }
    HRESULT put_AutoShow(long) override { return S_OK; }
    HRESULT get_AutoShow(long*) override { return S_OK; }
    HRESULT put_WindowState(long) override { return S_OK; }
    HRESULT get_WindowState(long*) override { return S_OK; }
    HRESULT put_BackgroundPalette(long) override { return S_OK; }
    HRESULT get_BackgroundPalette(long*) override { return S_OK; }
    HRESULT put_Visible(long v) override
        { if (vwin) { if (v) SDL_ShowWindow(vwin); else SDL_HideWindow(vwin); } return S_OK; }
    HRESULT get_Visible(long*) override { return S_OK; }
    HRESULT put_Left(long l) override { vx = l; return S_OK; }
    HRESULT get_Left(long* p) override { if(p)*p=vx; return S_OK; }
    HRESULT put_Width(long w) override { vw = w; return S_OK; }
    HRESULT get_Width(long* p) override { if(p)*p=vw; return S_OK; }
    HRESULT put_Top(long t) override { vy = t; return S_OK; }
    HRESULT get_Top(long* p) override { if(p)*p=vy; return S_OK; }
    HRESULT put_Height(long h) override { vh = h; return S_OK; }
    HRESULT get_Height(long* p) override { if(p)*p=vh; return S_OK; }
    HRESULT put_Owner(OAHWND o) override { owner = o; return S_OK; }
    HRESULT get_Owner(OAHWND* p) override { if(p)*p=owner; return S_OK; }
    HRESULT put_MessageDrain(OAHWND) override { return S_OK; }
    HRESULT get_MessageDrain(OAHWND*) override { return S_OK; }
    HRESULT get_BorderColor(long*) override { return E_NOTIMPL; }
    HRESULT put_BorderColor(long) override { return S_OK; }
    HRESULT get_FullScreenMode(long* p) override { if(p)*p=fullscreen; return S_OK; }
    HRESULT put_FullScreenMode(long f) override { fullscreen = !!f; return S_OK; }
    HRESULT SetWindowForeground(long) override { return S_OK; }
    HRESULT NotifyOwnerMessage(OAHWND, long, long, long) override { return S_OK; }
    HRESULT SetWindowPosition(long l, long t, long w, long h) override
        { vx=l; vy=t; vw=w; vh=h; return S_OK; }
    HRESULT GetWindowPosition(long* l, long* t, long* w, long* h) override
        { if(l)*l=vx; if(t)*t=vy; if(w)*w=vw; if(h)*h=vh; return S_OK; }
    HRESULT GetMinIdealImageSize(long*, long*) override { return E_NOTIMPL; }
    HRESULT GetMaxIdealImageSize(long*, long*) override { return E_NOTIMPL; }
    HRESULT GetRestorePosition(long*, long*, long*, long*) override { return E_NOTIMPL; }
    HRESULT HideCursor(long) override { return S_OK; }
    HRESULT IsCursorHidden(long*) override { return S_OK; }
    // ---- IBasicAudio --------------------------------------------------------
    HRESULT put_Volume(long) override { return S_OK; }
    HRESULT get_Volume(long*) override { return S_OK; }
    HRESULT put_Balance(long) override { return S_OK; }
    HRESULT get_Balance(long*) override { return S_OK; }
};

HRESULT RowanVideoGraph::RowanQI(IUnknown* This, const void* riid, void** ppv)
    { return Self(This)->QI(*(const GUID*)riid, ppv); }
ULONG RowanVideoGraph::RowanAR(IUnknown* This) { return ++Self(This)->refs; }
ULONG RowanVideoGraph::RowanRL(IUnknown* This)
{
    RowanVideoGraph* g = Self(This);
    ULONG r = --g->refs;
    if (!r) { g->StopAll(); delete g; }
    return r;
}
RvVtbl RowanVideoGraph::vt_gb = {{RowanQI, RowanAR, RowanRL}, 0};
RvVtbl RowanVideoGraph::vt_mc = {{RowanQI, RowanAR, RowanRL}, 0};
RvVtbl RowanVideoGraph::vt_me = {{RowanQI, RowanAR, RowanRL}, 0};
RvVtbl RowanVideoGraph::vt_vw = {{RowanQI, RowanAR, RowanRL}, 0};
RvVtbl RowanVideoGraph::vt_ba = {{RowanQI, RowanAR, RowanRL}, 0};

extern "C" HRESULT RowanVideo_CreateGraph(void** ppv)
{
    fprintf(stderr, "[rowanvideo] CreateGraph\n");
    if (!ppv) return E_POINTER;
    *ppv = (IGraphBuilder*)new RowanVideoGraph;
    return S_OK;
}
