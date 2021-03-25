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

#ifdef SKA_HAS_GPU_SUPPORT
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
#endif

std::unique_ptr<WindowContext> MakeRasterForUnix(const UnixWindowInfo& winInfo,
                                                 const DisplayParams& params) {
#if PLATFORM(X11)
    std::unique_ptr<WindowContext> ctx(new RasterWindowContextX11(winInfo, params));
#elif PLATFORM(LIBWPE)
    std::unique_ptr<WindowContext> ctx(new RasterWindowContextLibWPE(winInfo, params));
#else
    SK_APP_NOT_IMPL
    std::unique_ptr<WindowContext> ctx(nullptr);
#endif
    if (!ctx->isValid()) {
        ctx = nullptr;
    }
    return ctx;
}


}  // namespace window_context_factory

}  // namespace sk_app
