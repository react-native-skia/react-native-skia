
/*
 * Copyright 2016 Google Inc.
 * Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef WindowContextFactory_DEFINED
#define WindowContextFactory_DEFINED

#include "SkAppUtil.h"

// webgpu_cpp.h and X.h don't get along. Include this first, before X11 defines None, Success etc.
#ifdef SK_DAWN
#include "dawn/webgpu_cpp.h"
#endif

#if PLATFORM(X11)
#include <X11/Xlib.h>
#include "x11/PlatformDisplayX11.h"
#elif PLATFORM(LIBWPE) || USE(WPE_RENDERER)
#include "libwpe/PlatformDisplayLibWPE.h"
#endif

#include <memory>

namespace sk_app {

class WindowContext;
struct DisplayParams;

namespace window_context_factory {

#if PLATFORM(X11)
struct XlibWindowInfo {
    Display*     fDisplay;
#if USE(GLX)
    GLXFBConfig* fFBConfig;
    XVisualInfo* fVisualInfo;
#endif
};
#endif

struct UnixWindowInfo {
#if PLATFORM(X11)
    XlibWindowInfo native;
#endif
    GLNativeWindowType       fWindow;
    int         fWidth;
    int         fHeight;
};

std::unique_ptr<WindowContext> MakeVulkanForUnix(const UnixWindowInfo&, const DisplayParams&);

std::unique_ptr<WindowContext> MakeGLForUnix(const UnixWindowInfo&, const DisplayParams&);

#ifdef SK_DAWN
std::unique_ptr<WindowContext> MakeDawnVulkanForUnix(const UnixWindowInfo&, const DisplayParams&);
#endif

std::unique_ptr<WindowContext> MakeRasterForUnix(const UnixWindowInfo&, const DisplayParams&);

}  // namespace window_context_factory

}  // namespace sk_app

#if PLATFORM(X11)
#include "x11/RasterWindowContextX11.h"
#elif PLATFORM(LIBWPE) || USE(WPE_RENDERER)
#include "libwpe/RasterWindowContextLibWPE.h"
#endif

#endif
