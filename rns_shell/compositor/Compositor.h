/*
* Copyright 2016 Google Inc.
* Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/
#pragma once

#include "third_party/skia/include/core/SkRect.h"

#include "Window.h"
#include "WindowContext.h"
#include "PlatformDisplay.h"
#include "layers/Layer.h"

#define RNS_TARGET_FPS_US 16666.7 // In Microseconds

namespace RnsShell {

class Compositor {
    RNS_MAKE_NONCOPYABLE(Compositor);
public:

    static std::unique_ptr<Compositor> create(SkRect& viewPort, float scaleFactor = 1.0);
    Compositor(SkRect& viewPort, float scaleFactor);
    virtual ~Compositor();

    Layer* rootLayer() { return rootLayer_.get(); }
    void setRootLayer(SharedLayer rootLayer);
    void setViewportSize(const SkRect& viewportSize);
    SkRect& viewport() { return attributes_.viewportSize; }
    void invalidate();
    void commit();

private:

    void createWindowContext();
    void renderLayerTree();

    SharedLayer rootLayer_;
    std::unique_ptr<Window> window_;
    std::unique_ptr<WindowContext> windowContext_;
    sk_sp<SkSurface> backBuffer_;
    uint64_t nativeWindowHandle_;

    struct {
        //Lock lock;
        SkRect viewportSize;
        float scaleFactor { 1 };
        bool needsResize { false };
        bool rendersNextFrame { false };
    } attributes_;
};

}   // namespace RnsShell
