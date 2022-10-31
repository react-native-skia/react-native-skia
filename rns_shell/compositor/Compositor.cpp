/*
* Copyright 2016 Google Inc.
* Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "include/core/SkPaint.h"
#include "include/core/SkSurface.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkRegion.h"

#include "ReactSkia/utils/RnsLog.h"

#include "platform/linux/TaskLoop.h"
#include "WindowContextFactory.h"
#ifdef RNS_SHELL_HAS_GPU_SUPPORT
#include "GLWindowContext.h"
#endif

#include "Compositor.h"

namespace RnsShell {

std::unique_ptr<Compositor> Compositor::create(Client& compositorClient, PlatformDisplayID displayID, SkSize& viewPortSize, float scaleFactor) {
    RNS_LOG_INFO("Create New Compositor");
    return std::make_unique<Compositor>(compositorClient, displayID, viewPortSize, scaleFactor);
}

Compositor::Compositor(Client& client, PlatformDisplayID displayID, SkSize& viewportSize, float scaleFactor)
    :client_(client)
    ,rootLayer_(nullptr) {

    nativeWindowHandle_ = reinterpret_cast<GLNativeWindowType>(client_.nativeSurfaceHandle());
    if(nativeWindowHandle_) {
       createWindowContext();
    }

    if(windowContext_) {
        backBuffer_ = windowContext_->getBackbufferSurface();
    } else {
        RNS_LOG_ERROR("Invalid windowContext for nativeWindowHandle : " << nativeWindowHandle_);
        return;
    }

    {
        //locker(attributes_.lock);
        if(viewportSize.isEmpty())
            attributes_.viewportSize = SkSize::Make(windowContext_->width(), windowContext_->height());
        else
            attributes_.viewportSize = viewportSize;
        attributes_.scaleFactor = scaleFactor;
        attributes_.needsResize = !viewportSize.isEmpty();
    }
#if USE(RNS_SHELL_PARTIAL_UPDATES)
    supportPartialUpdate_ = windowContext_->hasSwapBuffersWithDamage() || windowContext_->hasBufferCopy();
    RNS_LOG_DEBUG("Support for Swapbuffer with damage rect : " << windowContext_->hasSwapBuffersWithDamage() <<
                  " Support for Copy buffer : " <<  windowContext_->hasBufferCopy());
#endif
    RNS_LOG_DEBUG("Native Window Handle : " << nativeWindowHandle_ << " Window Context : " << windowContext_.get() << "Back Buffer : " << backBuffer_.get());
}

Compositor::~Compositor() {
}

void Compositor::createWindowContext() {
    RNS_LOG_ASSERT((nativeWindowHandle_), "Invalid Native Window Handle");
    windowContext_ = WCF::createContextForWindow(nativeWindowHandle_, &PlatformDisplay::sharedDisplayForCompositing(), DisplayParams());

    if (!windowContext_ || !windowContext_->makeContextCurrent())
        return;
    //TODO swap interval
}

void Compositor::invalidate() {
    RNS_LOG_TODO("Destroy GL context and Surface");
    windowContext_ = nullptr;
    backBuffer_ = nullptr;
}

SkRect Compositor::beginClip(PaintContext& context, bool useClipRegion) {
    SkRect clipBound = SkRect::MakeEmpty();
    if(context.damageRect.size() == 0)
        return clipBound;

    // Use clipRegion for clipping
    if(useClipRegion) {
        SkRegion clipRgn(SkIRect::MakeEmpty());
        clipRgn.setRects(context.damageRect.data(),context.damageRect.size());

        if(clipRgn.getBounds().isEmpty()) {
            return clipBound;
        }

        context.canvas->clipRegion(clipRgn);
        clipBound = SkRect::Make(clipRgn.getBounds());

    } else {
        // Use clipPath for clipping
        SkPath clipPath = SkPath();
        for (auto& rect : context.damageRect) {
            RNS_LOG_DEBUG("Add Damage " << rect.x() << " " << rect.y() << " " << rect.width() << " " << rect.height());
            clipPath.addRect(rect.left(), rect.top(), rect.right(), rect.bottom());
        }

        if(clipPath.getBounds().isEmpty()) {
            return clipBound;
        }

        context.canvas->clipPath(clipPath);
        clipBound = clipPath.getBounds();
    }

    return clipBound;
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
        SkSize viewportSize;
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
        auto canvas = backBuffer_->getCanvas();
        SkAutoCanvasRestore save(canvas, true);
        SkRect clipBound = SkRect::MakeEmpty();
        PaintContext paintContext = {
            canvas,  // canvas
            surfaceDamage_, // damage rects
#if USE(RNS_SHELL_PARTIAL_UPDATES)
            supportPartialUpdate_,
#endif
            clipBound, // After prePaint we need to update this with beginClip
            nullptr, // GrDirectContext
            {0,0} //scrollOffset is zero for rootLayer
        };
        RNS_PROFILE_API_OFF("Render Tree Pre-Paint", rootLayer_.get()->prePaint(paintContext));
        clipBound = beginClip(paintContext);
        /* Check if paint required*/
        if(!rootLayer_.get()->needsPainting(paintContext)) return;
#ifdef RNS_SHELL_HAS_GPU_SUPPORT
        WindowContext::grTransactionBegin();
#endif
        RNS_PROFILE_API_OFF("Render Tree Paint", rootLayer_.get()->paint(paintContext));
        RNS_PROFILE_API_OFF("SkSurface Flush & Submit", backBuffer_->flushAndSubmit());
#ifdef RNS_SHELL_HAS_GPU_SUPPORT
        WindowContext::grTransactionEnd();
#endif
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
        RNS_PROFILE_API_OFF("SwapBuffers", windowContext_->swapBuffers(surfaceDamage_));
        client_.didRenderFrame();
    }
}

void Compositor::begin() {
    // Lock until render tree has rendered current tree
    isMutating.lock();
    surfaceDamage_.clear(); // Clear the previous damage rects.
}

void Compositor::commit(bool immediate=false) {
    if(!windowContext_) {
        isMutating.unlock();
        return;
    }

    if(immediate) {
       RNS_PROFILE_API_OFF("RenderTree Immediate:", renderLayerTree());
       // Unlock here, after rendering of tree is done for immediate rendering
       isMutating.unlock();
       return;
    }

    // Unlock here, to ensure updates and rendering of tree is synchronous
    isMutating.unlock();
    TaskLoop::main().dispatch([&]() {
        std::scoped_lock lock(isMutating); // Lock to make sure render tree is not mutated during the rendering
        RNS_PROFILE_API_OFF("RenderTree Scheduled:", renderLayerTree());
    });
}

void Compositor::setRootLayer(SharedLayer rootLayer) {
  if (rootLayer_.get() == rootLayer.get())
    return;
  RNS_LOG_INFO("Set Root Layer : " << rootLayer.get());
  rootLayer_ = rootLayer;
}

void Compositor::setViewportSize(const SkSize& viewportSize) {
    //locker(attributes_.lock);
    if(viewportSize.width() == attributes_.viewportSize.width() &&
        viewportSize.height() == attributes_.viewportSize.height()) {
        return;
    }
    attributes_.viewportSize = viewportSize;
    attributes_.needsResize = true;
    commit();
}

#ifdef RNS_SHELL_HAS_GPU_SUPPORT
GrDirectContext* Compositor::getDirectContext( ) {
  return windowContext_ ? windowContext_->directContext() : nullptr;
}
#endif

}   // namespace RnsShell
