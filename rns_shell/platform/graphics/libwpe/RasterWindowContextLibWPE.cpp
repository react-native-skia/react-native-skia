/*
 * Copyright 2016 Google Inc.
 * Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "RasterWindowContextLibWPE.h"
#include "Performance.h"

using RnsShell::RasterWindowContext;
using RnsShell::DisplayParams;

namespace RnsShell {

std::unique_ptr<WindowContext> RasterWindowContextLibWPE::createContext(GLNativeWindowType window, PlatformDisplay* platformDisplay, const DisplayParams& params) {
    return std::unique_ptr<RasterWindowContextLibWPE>(new RasterWindowContextLibWPE(window, platformDisplay, params));
}

RasterWindowContextLibWPE::RasterWindowContextLibWPE(GLNativeWindowType window, PlatformDisplay* platformDisplay, const DisplayParams& params)
        : INHERITED(params)
        , display_((dynamic_cast<PlatformDisplayLibWPE*>(platformDisplay))->native())
        , window_(window) {

    width_ = display_->screen().width;
    height_ = display_->screen().height;

    initializeContext();
}

void RasterWindowContextLibWPE::setDisplayParams(const DisplayParams& params) {
    displayParams_ = params;
}

void RasterWindowContextLibWPE::initializeContext() {

    SkImageInfo info = SkImageInfo::Make(width_, height_, displayParams_.colorType_, kPremul_SkAlphaType,
                                         displayParams_.colorSpace_);
    backbufferSurface_ = SkSurface::MakeRaster(info, &displayParams_.surfaceProps_);
}

sk_sp<SkSurface> RasterWindowContextLibWPE::getBackbufferSurface() { return backbufferSurface_; }

void RasterWindowContextLibWPE::swapBuffers() {
    RNS_LOG_NOT_IMPL;
    // TODO : We need to blit the SkPixmap data to native window (window_) which is returned by libwpe.
    // So this part of code will be specific to display backend used in LIBWPE, so it is better to implement
    // new API(s) in libwpe to do this in respective backend in WPEBackend(fdo-rdk)
    // OR
    // Add required functionalities in this file for different backends

#if !defined(GOOGLE_STRIP_LOG) || (GOOGLE_STRIP_LOG <= INFO)
    RNS_GET_TIME_STAMP_US(start);
#endif

    SkPixmap pm;
    if (!backbufferSurface_->peekPixels(&pm)) {
        return;
    }

#if !defined(GOOGLE_STRIP_LOG) || (GOOGLE_STRIP_LOG <= INFO)
    RNS_GET_TIME_STAMP_US(end);
    Performance::takeSamples(end - start);
#endif
}

}  // namespace RnsShell

