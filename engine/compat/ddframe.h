// ddframe.h — latest-wins DirectDraw present queue (header-only, no SDL).
//
// The 3D worker's Flip feeds RowanDDPresentHook, which lands here; the main
// thread pumps queued frames out through an injected sink.  Splitting the
// queue from the SDL glue (migalley/Bob/port/dd_present.cpp) makes the
// hand-off logic unit-testable — engine/tests/unit/test_ddframe.cpp drives
// it with capture fakes and real malloc-backed compat surfaces.
//
// Collaborators (dependency-injected via Install):
//   Sink   — what the main thread does with a frame (SDL renderer blit,
//            test capture, ...)
//   Notify — how the worker wakes the main thread (SDL_PushEvent, counter…)
#pragma once
#include "ddraw.h"   // IDirectDrawSurface7
#include <mutex>

class DDFramePump {
public:
    struct Sink {
        virtual ~Sink() {}
        virtual void Present(IDirectDrawSurface7* primary) = 0;
    };
    struct Notify {
        virtual ~Notify() {}
        virtual void Wakeup() = 0;
    };

    void Install(Sink* sink, Notify* notify) { sink_ = sink; notify_ = notify; }

    // Worker side: queue the newest primary frame and wake the main thread.
    // A still-queued older frame is released — latest wins, so the pump can
    // never fall behind the renderer.
    void Queue(IDirectDrawSurface7* primary)
    {
        if (!primary || !primary->desc.lpSurface) return;
        primary->AddRef();
        IDirectDrawSurface7* old;
        {
            std::lock_guard<std::mutex> lk(mu_);
            old = pending_;
            pending_ = primary;
        }
        if (old) old->Release();
        if (notify_) notify_->Wakeup();
    }

    // Main side: present whatever is queued.  True if a frame went out.
    bool Pump()
    {
        IDirectDrawSurface7* s;
        {
            std::lock_guard<std::mutex> lk(mu_);
            s = pending_;
            pending_ = nullptr;
        }
        if (!s) return false;
        if (sink_) sink_->Present(s);
        s->Release();
        return true;
    }

private:
    std::mutex mu_;
    IDirectDrawSurface7* pending_ = nullptr;
    Sink*   sink_   = nullptr;
    Notify* notify_ = nullptr;
};
