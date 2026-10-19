// DDFramePump unit tests (engine/compat/ddframe.h): latest-wins queue
// semantics, refcount balance, invalid-frame rejection, injected
// sink/notify collaborators — all without SDL.  The compat surfaces are
// real malloc-backed IDirectDrawSurface7 objects (same header-inline impl
// the game uses), so refcount and pixel-buffer behaviour match production.

#include "../harness.h"
#include <ddraw.h>
#include <ddframe.h>

namespace {

IDirectDrawSurface7* MakeSurface(DWORD w, DWORD h, DWORD bpp)
{
    auto* s = new IDirectDrawSurface7;
    DDSURFACEDESC2 d;
    std::memset(&d, 0, sizeof(d));
    d.dwSize = sizeof(d);
    d.dwWidth = w; d.dwHeight = h;
    d.ddpfPixelFormat.dwRGBBitCount = bpp;
    CHECK_EQ(s->Alloc(&d), DD_OK);
    CHECK(s->desc.lpSurface != nullptr);
    return s;
}

struct CaptureSink : DDFramePump::Sink {
    IDirectDrawSurface7* last = nullptr;
    int calls = 0;
    void Present(IDirectDrawSurface7* s) override { last = s; ++calls; }
};

struct CountNotify : DDFramePump::Notify {
    int calls = 0;
    void Wakeup() override { ++calls; }
};

} // namespace

// Two frames queued before a pump: only the newest is presented, the older
// is released inside Queue (never reaches the sink).
REGISTER_TEST(ddframe_latest_wins)
{
    DDFramePump pump; CaptureSink sink; CountNotify notify;
    pump.Install(&sink, &notify);

    IDirectDrawSurface7* a = MakeSurface(8, 8, 16);
    IDirectDrawSurface7* b = MakeSurface(8, 8, 16);

    pump.Queue(a);                       // refCnt: caller(1) + queue(1) = 2
    pump.Queue(b);                       // drops queued a -> a back to 1
    CHECK_EQ((long long)a->refCnt, 1);
    CHECK_EQ((long long)b->refCnt, 2);
    CHECK_EQ(notify.calls, 2);           // one wakeup per queue

    CHECK(pump.Pump());
    CHECK_EQ(sink.calls, 1);
    CHECK(sink.last == b);
    CHECK_EQ((long long)b->refCnt, 1);   // pump released the queue ref

    CHECK(!pump.Pump());                 // queue drained
    CHECK_EQ(sink.calls, 1);

    delete a;
    delete b;
}

// Null pointers and surfaces without backing store are dropped before the
// queue — and never notify the main thread.
REGISTER_TEST(ddframe_drops_invalid)
{
    DDFramePump pump; CaptureSink sink; CountNotify notify;
    pump.Install(&sink, &notify);

    pump.Queue(nullptr);
    auto* unalloc = new IDirectDrawSurface7;   // lpSurface stays null
    pump.Queue(unalloc);

    CHECK_EQ(notify.calls, 0);
    CHECK(!pump.Pump());
    CHECK_EQ(sink.calls, 0);
    delete unalloc;
}

// With no sink installed the queue still drains and balances refcounts —
// Pump must not leak the frame just because nobody is presenting.
REGISTER_TEST(ddframe_no_sink_still_drains)
{
    DDFramePump pump; CountNotify notify;
    pump.Install(nullptr, &notify);

    IDirectDrawSurface7* s = MakeSurface(4, 4, 32);
    pump.Queue(s);
    CHECK_EQ(notify.calls, 1);
    CHECK(pump.Pump());
    CHECK_EQ((long long)s->refCnt, 1);
    CHECK(!pump.Pump());
    delete s;
}

// No notify installed: queuing must still work (main thread just isn't
// woken — e.g. an alternative pump driven by a timer).
REGISTER_TEST(ddframe_no_notify)
{
    DDFramePump pump; CaptureSink sink;
    pump.Install(&sink, nullptr);

    IDirectDrawSurface7* s = MakeSurface(4, 4, 16);
    pump.Queue(s);
    CHECK(pump.Pump());
    CHECK(sink.last == s);
    delete s;
}

// Interleaved queue/pump sequence: each pump yields exactly the most
// recent frame, in order.
REGISTER_TEST(ddframe_in_order_stream)
{
    DDFramePump pump; CaptureSink sink; CountNotify notify;
    pump.Install(&sink, &notify);

    IDirectDrawSurface7* frames[4];
    for (auto*& f : frames) f = MakeSurface(8, 8, 16);

    pump.Queue(frames[0]); CHECK(pump.Pump()); CHECK(sink.last == frames[0]);
    pump.Queue(frames[1]);
    pump.Queue(frames[2]);               // 2 supersedes 1 in the queue
    CHECK(pump.Pump()); CHECK(sink.last == frames[2]);
    pump.Queue(frames[3]); CHECK(pump.Pump()); CHECK(sink.last == frames[3]);
    CHECK(!pump.Pump());

    CHECK_EQ(sink.calls, 3);             // frame[1] never presented
    CHECK_EQ(notify.calls, 4);
    for (auto* f : frames) {
        CHECK_EQ((long long)f->refCnt, 1);
        delete f;
    }
}
