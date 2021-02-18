/*
 * Copyright 2016 Google Inc.
 * Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef RasterWindowContext_unix_DEFINED
#define RasterWindowContext_unix_DEFINED

#include "include/core/SkSurface.h"
#include "WindowContextFactory.h"
#include "RasterWindowContext.h"

namespace sk_app {

#if PLATFORM(X11)
class RasterWindowContext_xlib : public RasterWindowContext {
public:
    RasterWindowContext_xlib(Display*, XWindow, int width, int height, const DisplayParams&);

    sk_sp<SkSurface> getBackbufferSurface() override;
    void swapBuffers() override;
    bool isValid() override { return SkToBool(fWindow); }
    void resize(int  w, int h) override;
    void setDisplayParams(const DisplayParams& params) override;

protected:
    sk_sp<SkSurface> fBackbufferSurface;
    Display* fDisplay;
    XWindow  fWindow;
    GC       fGC;
    typedef RasterWindowContext INHERITED;
};
#endif

}   // namespace sk_app

#endif //RasterWindowContext_unix_DEFINED
