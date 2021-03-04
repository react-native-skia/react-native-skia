/*
 * Copyright 2016 Google Inc.
 * Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "RasterWindowContextLibWPE.h"

using sk_app::RasterWindowContext;
using sk_app::DisplayParams;

namespace sk_app {
RasterWindowContextLibWPE::RasterWindowContextLibWPE(const UnixWindowInfo& winInfo, const DisplayParams& params)
        : INHERITED(params)
        , fWindow(winInfo.fWindow) {
    SK_APP_NOT_IMPL
    this->resize(winInfo.fWidth, winInfo.fHeight);
    fWidth = winInfo.fWidth;
    fHeight = winInfo.fHeight;
}

void RasterWindowContextLibWPE::setDisplayParams(const DisplayParams& params) {
    fDisplayParams = params;
}

void RasterWindowContextLibWPE::resize(int  w, int h) {
    SK_APP_NOT_IMPL
    SkImageInfo info = SkImageInfo::Make(w, h, fDisplayParams.fColorType, kPremul_SkAlphaType,
                                         fDisplayParams.fColorSpace);
    fBackbufferSurface = SkSurface::MakeRaster(info, &fDisplayParams.fSurfaceProps);
    SK_APP_UNUSED(info);
}

sk_sp<SkSurface> RasterWindowContextLibWPE::getBackbufferSurface() { return fBackbufferSurface; }

void RasterWindowContextLibWPE::swapBuffers() {
    SK_APP_NOT_IMPL
    SkPixmap pm;
    if (!fBackbufferSurface->peekPixels(&pm)) {
        return;
    }
    // TODO : We need to blit the SkPixmap data to native window (fWindow) which is returned by libwpe.
    // So this part of code will be specific to display backend used in LIBWPE, so it is better to implemet
    // a new API(s) in libwpe to do this in respective backend in WPEBackend(fdo-rdk)
}

}  // namespace sk_app

