/*
* Copyright 2016 Google Inc.
* Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "include/core/SkPaint.h"
#include "include/core/SkSurface.h"
#include "include/core/SkCanvas.h"

#include "ReactSkia/utils/RnsLog.h"

#include "platform/linux/TaskLoop.h"
#include "WindowContextFactory.h"
#ifdef RNS_SHELL_HAS_GPU_SUPPORT
#include "GLWindowContext.h"
#endif

#include "Compositor.h"

namespace RnsShell {

std::unique_ptr<Compositor> Compositor::create(SkRect& viewPort, float scaleFactor) {
    return std::make_unique<Compositor>(viewPort, scaleFactor);
}

Compositor::Compositor(SkRect& viewportSize, float scaleFactor)
    :rootLayer_(nullptr)
    ,window_(Window::createNativeWindow(&PlatformDisplay::sharedDisplayForCompositing())) {

    nativeWindowHandle_ = window_ ? window_->nativeWindowHandle() : 0;
    if(nativeWindowHandle_) {
       createWindowContext();
    }

    if(windowContext_) {
        backBuffer_ = windowContext_->getBackbufferSurface();
    }
    {
        //locker(attributes_.lock);
        if(viewportSize.isEmpty())
            attributes_.viewportSize = SkRect::MakeWH(windowContext_->width(), windowContext_->height());
        else
            attributes_.viewportSize = viewportSize;
        attributes_.scaleFactor = scaleFactor;
        attributes_.needsResize = !viewportSize.isEmpty();
    }
    RNS_LOG_DEBUG("Native Window Handle : " << nativeWindowHandle_ << " Window Context : " << windowContext_.get() << "Back Buffer : " << backBuffer_.get());
}

Compositor::~Compositor() {
}

void Compositor::createWindowContext() {
    windowContext_ = WCF::createContextForWindow(reinterpret_cast<GLNativeWindowType>(nativeWindowHandle_),
                        &PlatformDisplay::sharedDisplayForCompositing(), DisplayParams());

    if (!windowContext_ || !windowContext_->makeContextCurrent())
        return;
    //TODO swap interval
}

void Compositor::invalidate() {
    RNS_LOG_TODO("Destroy GL context and Surface");
    windowContext_ = nullptr;
    backBuffer_ = nullptr;
}

void Compositor::renderLayerTree() {

    if(!windowContext_)
        return;

#if !defined(GOOGLE_STRIP_LOG) || (GOOGLE_STRIP_LOG <= INFO)
  static double prevTime = SkTime::GetMSecs();
  RNS_LOG_INFO_EVERY_N(60, "Called RenderLayerTree(" << std::this_thread::get_id()) << ") : after " << SkTime::GetMSecs() - prevTime << " ms";
  prevTime = SkTime::GetMSecs();
#endif

    if(backBuffer_ == nullptr || rootLayer_.get() == nullptr) {
        RNS_LOG_ERROR("No backbuffer : " << backBuffer_ << " or rootlayer" << rootLayer_.get());
    } else {
        SkRect viewportSize;
        float scaleFactor;
        bool needsResize;
        {
            //locker(attributes_.lock);
            viewportSize = attributes_.viewportSize;
            scaleFactor = attributes_.scaleFactor;
            needsResize = attributes_.needsResize;

            // Reset the needsResize attribute to false
            attributes_.needsResize = false;
        }

#ifdef RNS_SHELL_HAS_GPU_SUPPORT
        if (needsResize)
            glViewport(0, 0, viewportSize.width(), viewportSize.height());
#endif
        RNS_PROFILE_API_OFF("Render Tree Pre-Paint", rootLayer_.get()->prePaint(backBuffer_.get()));
        RNS_PROFILE_API_OFF("Render Tree Paint", rootLayer_.get()->paint(backBuffer_.get()));
        RNS_PROFILE_API_OFF("SkSurface Flush & Submit", backBuffer_->flushAndSubmit());
#ifdef RNS_ENABLE_FRAME_RATE_CONTROL
        {
            static double prevSwapTimestamp = SkTime::GetNSecs() * 1e-3;
            double diffUs = SkTime::GetNSecs() * 1e-3 - prevSwapTimestamp;
            int diff = RNS_TARGET_FPS_US - diffUs;
            RNS_LOG_DEBUG(" SwapBuffer Gap : ( " << diffUs << " us)" << " FrameRateTarget : ( " << RNS_TARGET_FPS_US << " us ) " << diff);
            if(diff > 0 ) {
                RNS_LOG_TRACE("ZZZzzzz for : " << diff * 1e-3 << " ms");
                std::this_thread::sleep_for (std::chrono::microseconds(diff));
            }
            prevSwapTimestamp = SkTime::GetNSecs() * 1e-3;
        }
#endif
        RNS_PROFILE_API_OFF("SwapBuffers", windowContext_->swapBuffers());
        window_->didRenderFrame();
    }
}

void Compositor::begin() {
    // Locke until render tree has rendered current tree
    std::scoped_lock lock(isMutating);
}

void Compositor::commit() {

    if(!windowContext_)
        return;

    TaskLoop::main().dispatch([&]() {
        std::scoped_lock lock(isMutating); // Lock to make sure render tree is not mutated during the rendering
        RNS_PROFILE_API_OFF("RenderTree :", renderLayerTree());
    });
}

void Compositor::setRootLayer(SharedLayer rootLayer) {
  if (rootLayer_.get() == rootLayer.get())
    return;
  RNS_LOG_INFO("Set Root Layer : " << rootLayer.get());
  rootLayer_ = rootLayer;
}

void Compositor::setViewportSize(const SkRect& viewportSize) {
    //locker(attributes_.lock);
    if(viewportSize.width() == attributes_.viewportSize.width() &&
        viewportSize.height() == attributes_.viewportSize.height()) {
        return;
    }
    attributes_.viewportSize = viewportSize;
    attributes_.needsResize = true;
    commit();
}

}   // namespace RnsShell
