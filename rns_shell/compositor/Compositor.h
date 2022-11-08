/*
* Copyright 2016 Google Inc.
* Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/
#pragma once

#include <list>

#include "third_party/skia/include/core/SkRect.h"

#include "WindowContext.h"
#include "PlatformDisplay.h"
#include "layers/Layer.h"

#define RNS_TARGET_FPS_US 16666.7 // In Microseconds
#define RNS_SHELL_MAX_FRAME_DAMAGE_HISTORY 5

namespace RnsShell {

typedef uint64_t PlatformDisplayID;

class Compositor {
    RNS_MAKE_NONCOPYABLE(Compositor);
public:

    class Client {
    public:
        virtual uint64_t nativeSurfaceHandle() = 0;
        virtual void didRenderFrame() = 0;
    };

    static std::unique_ptr<Compositor> create(Client&, PlatformDisplayID, SkSize&, float scaleFactor = 1.0);
    Compositor(Client&, PlatformDisplayID, SkSize&, float);
    virtual ~Compositor();

    Layer* rootLayer() { return rootLayer_.get(); }
    void setRootLayer(SharedLayer rootLayer);
    void setViewportSize(const SkSize& viewportSize);
    SkSize& viewport() { return attributes_.viewportSize; }
    void invalidate();
    void begin(); // Call this before modifying render layer tree
    void commit(bool immediate); // Commit the changes in render layer tree - immediately/schedule
#if USE(RNS_SHELL_PARTIAL_UPDATES)
    bool supportsPartialUpdates() { return supportPartialUpdate_; } // Wheather compositor can support partial paint and update
    void addDamageRect(SkIRect damage) { if(supportPartialUpdate_ && !damage.isEmpty()) surfaceDamage_.push_back(damage); }
#endif

#ifdef RNS_SHELL_HAS_GPU_SUPPORT
    GrDirectContext* getDirectContext(); // interface to expose directcontext of gpu backend
#endif
    static SkRect beginClip(PaintContext& context, bool useClipRegion=false);

private:

    void createWindowContext();
    void renderLayerTree();
#if USE(RNS_SHELL_PARTIAL_UPDATES) && ENABLE(RNS_SHELL_BUFFER_AGE)
    SkRect beginClip();
#endif
    std::mutex isMutating; // Lock the renderLayer tree while updating and rendering

    Client& client_;
    SharedLayer rootLayer_;
    std::unique_ptr<WindowContext> windowContext_;
    sk_sp<SkSurface> backBuffer_;
    GLNativeWindowType nativeWindowHandle_;

#if USE(RNS_SHELL_PARTIAL_UPDATES)
    bool supportPartialUpdate_;
#endif
    std::vector<SkIRect> surfaceDamage_;
#if USE(RNS_SHELL_PARTIAL_UPDATES) && ENABLE(RNS_SHELL_BUFFER_AGE)
    std::list<FrameDamages> frameDamageHistory_;
#endif
    struct {
        //Lock lock;
        SkSize viewportSize;
        float scaleFactor { 1 };
        bool needsResize { false };
        bool rendersNextFrame { false };
    } attributes_;
};

}   // namespace RnsShell
