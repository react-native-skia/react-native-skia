/*
 * Copyright 2016 Google Inc.
 * Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "RasterWindowContext_unix.h"

using sk_app::RasterWindowContext;
using sk_app::DisplayParams;
#if !PLATFORM(X11)
typedef int XWindow;
#endif

namespace sk_app {

#if PLATFORM(X11)
RasterWindowContext_xlib::RasterWindowContext_xlib(Display* display, XWindow window, int width,
                                                   int height, const DisplayParams& params)
        : INHERITED(params)
        , fDisplay(display)
        , fWindow(window) {
    fGC = XCreateGC(fDisplay, fWindow, 0, nullptr);
    this->resize(width, height);
    fWidth = width;
    fHeight = height;
}

void RasterWindowContext_xlib::setDisplayParams(const DisplayParams& params) {
    fDisplayParams = params;
    XWindowAttributes attrs;
    XGetWindowAttributes(fDisplay, fWindow, &attrs);
    this->resize(attrs.width, attrs.height);
}

void RasterWindowContext_xlib::resize(int  w, int h) {
    SkImageInfo info = SkImageInfo::Make(w, h, fDisplayParams.fColorType, kPremul_SkAlphaType,
                                         fDisplayParams.fColorSpace);
    fBackbufferSurface = SkSurface::MakeRaster(info, &fDisplayParams.fSurfaceProps);

}

sk_sp<SkSurface> RasterWindowContext_xlib::getBackbufferSurface() { return fBackbufferSurface; }

void RasterWindowContext_xlib::swapBuffers() {
    SkPixmap pm;
    if (!fBackbufferSurface->peekPixels(&pm)) {
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
    XPutImage(fDisplay, fWindow, fGC, &image, 0, 0, 0, 0, pm.width(), pm.height());
}
#endif

}  // namespace sk_app
