/*
 * Copyright 2016 Google Inc.
 * Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "WindowContextFactory.h"
#if PLATFORM(X11) && USE(GLX)
#include "glx/GLWindowContextGLX.h"
#elif USE(EGL)
#include "egl/GLWindowContextEGL.h"
#endif

namespace sk_app {

namespace window_context_factory {

std::unique_ptr<WindowContext> MakeGLForUnix(const UnixWindowInfo& winInfo,
                                             const DisplayParams& params) {

    PlatformDisplay& pDisplay = PlatformDisplay::sharedDisplay();

#if USE(GLX)
    std::unique_ptr<WindowContext> ctx(new GLWindowContextGLX(winInfo, params));
#elif USE(EGL)
    std::unique_ptr<WindowContext> ctx(new GLWindowContextEGL(winInfo, pDisplay, params));
#endif
    if (!ctx->isValid()) {
        return nullptr;
    }
    return ctx;
}

std::unique_ptr<WindowContext> MakeRasterForUnix(const UnixWindowInfo& info,
                                                 const DisplayParams& params) {
    SK_APP_NOT_IMPL
#if PLATFORM(X11)
    std::unique_ptr<WindowContext> ctx(new RasterWindowContext_xlib(
            info.native.fDisplay, info.fWindow, info.fWidth, info.fHeight, params));
    if (!ctx->isValid()) {
        ctx = nullptr;
    }
    return ctx;
#else
    return nullptr;
#endif
}


}  // namespace window_context_factory

}  // namespace sk_app
