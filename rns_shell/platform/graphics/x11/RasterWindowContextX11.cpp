/*
 * Copyright 2016 Google Inc.
 * Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "rns_shell/platform/graphics/x11/RasterWindowContextX11.h"

#include "rns_shell/common/Performance.h"

#if PLATFORM(X11)
using RnsShell::RasterWindowContext;
using RnsShell::DisplayParams;

namespace RnsShell {

std::unique_ptr<WindowContext> RasterWindowContextX11::createContext(GLNativeWindowType window, PlatformDisplay* platformDisplay, const DisplayParams& params) {
    return std::unique_ptr<RasterWindowContextX11>(new RasterWindowContextX11(window, platformDisplay, params));
}

RasterWindowContextX11::RasterWindowContextX11(GLNativeWindowType window, PlatformDisplay* platformDisplay, const DisplayParams& params)
        : INHERITED(params)
        , display_((dynamic_cast<PlatformDisplayX11*>(platformDisplay))->native())
        , window_(window) {

    XWindowAttributes attrs;
    XGetWindowAttributes(display_, window_, &attrs);
    width_ = attrs.width;
    height_= attrs.height;

    gc_ = XCreateGC(display_, window, 0, nullptr);
    initializeContext();
}

void RasterWindowContextX11::setDisplayParams(const DisplayParams& params) {
    displayParams_ = params;
    RNS_LOG_NOT_IMPL;
}

void RasterWindowContextX11::initializeContext() {

    SkImageInfo info = SkImageInfo::Make(width_, height_, displayParams_.colorType_, kPremul_SkAlphaType,
                                         displayParams_.colorSpace_);
    backbufferSurface_ = SkSurface::MakeRaster(info, &displayParams_.surfaceProps_);

}

sk_sp<SkSurface> RasterWindowContextX11::getBackbufferSurface() { return backbufferSurface_; }

void RasterWindowContextX11::swapBuffers(std::vector<SkIRect> &damage) {
#if !defined(GOOGLE_STRIP_LOG) || (GOOGLE_STRIP_LOG <= INFO)
    RNS_GET_TIME_STAMP_US(start);
#endif
    SkPixmap pm;
    if (!backbufferSurface_->peekPixels(&pm)) {
        return;
    }
    int bitsPerPixel = pm.info().bytesPerPixel() * 8;
    XImage image;
    memset(&image, 0, sizeof(image));
    image.width = pm.width();
    image.height = pm.height();
    image.format = ZPixmap;
    image.data = (char*) pm.addr();
    image.byte_order = LSBFirst;
    image.bitmap_unit = bitsPerPixel;
    image.bitmap_bit_order = LSBFirst;
    image.bitmap_pad = bitsPerPixel;
    image.depth = 24;
    image.bytes_per_line = pm.rowBytes() - pm.width() * pm.info().bytesPerPixel();
    image.bits_per_pixel = bitsPerPixel;
    if (!XInitImage(&image)) {
        return;
    }
    XPutImage(display_, window_, gc_, &image, 0, 0, 0, 0, pm.width(), pm.height());
#if !defined(GOOGLE_STRIP_LOG) || (GOOGLE_STRIP_LOG <= INFO)
    RNS_GET_TIME_STAMP_US(end);
    Performance::takeSamples(end - start);
#endif
}

#if USE(RNS_SHELL_PARTIAL_UPDATES)
bool RasterWindowContextX11::hasBufferCopy() {
    // With current implementation We are using offscreen bitmap to draw and then copying this bitmap to window.
    // This means both bitmap and window has same data after frame display ( swapbuffer )
    return true;
}
#endif

}  // namespace RnsShell

#endif // PLATFORM(X11)
