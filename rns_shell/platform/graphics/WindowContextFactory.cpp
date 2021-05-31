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

#ifndef RNS_SHELL_HAS_GPU_SUPPORT
#if PLATFORM(X11)
#include "x11/RasterWindowContextX11.h"
#elif PLATFORM(LIBWPE)
#include "libwpe/RasterWindowContextLibWPE.h"
#endif
#endif

namespace RnsShell {

namespace WCF { //window_context_factory

#ifdef RNS_SHELL_HAS_GPU_SUPPORT
std::unique_ptr<WindowContext> createContextForWindow(GLNativeWindowType windowHandle, PlatformDisplay* platformDisplay,
                                             const DisplayParams& params) {
    PlatformDisplay& display = platformDisplay ? *platformDisplay : PlatformDisplay::sharedDisplay();
#if USE(GLX)
    if(auto glXContext = GLWindowContextGLX::createContext(windowHandle, display, params))
        return glXContext;
#elif USE(EGL)
    if(auto eglContext = GLWindowContextEGL::createContext(windowHandle, display, params))
        return eglContext;
#endif
    return nullptr;
}
#else
std::unique_ptr<WindowContext> createContextForWindow(GLNativeWindowType windowHandle, PlatformDisplay* platformDisplay,
                                             const DisplayParams& params) {
    PlatformDisplay* display = platformDisplay ? platformDisplay : &PlatformDisplay::sharedDisplay();
#if PLATFORM(X11)
    if(auto rasterContext = RasterWindowContextX11::createContext(windowHandle, display, params))
        return rasterContext;
#elif PLATFORM(LIBWPE)
    if(auto rasterContext = RasterWindowContextLibWPE::createContext(windowHandle, display, params))
        return rasterContext;
#else
    RNS_LOG_NOT_IMPL;
#endif
    return nullptr;
}

#endif // RNS_SHELL_HAS_GPU_SUPPORT

}  // namespace WCF

}  // namespace RnsShell
