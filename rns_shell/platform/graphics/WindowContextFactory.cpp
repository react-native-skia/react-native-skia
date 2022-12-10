/*
 * Copyright 2016 Google Inc.
 * Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "rns_shell/platform/graphics/WindowContextFactory.h"

#if PLATFORM(X11) && USE(GLX)
#include "rns_shell/platform/graphics/gl/glx/GLWindowContextGLX.h"
#elif USE(EGL)
#include "rns_shell/platform/graphics/gl/egl/GLWindowContextEGL.h"
#endif

#ifndef RNS_SHELL_HAS_GPU_SUPPORT
#if PLATFORM(X11)
#include "rns_shell/platform/graphics/x11/RasterWindowContextX11.h"
#elif PLATFORM(LIBWPE)
#include "rns_shell/platform/graphics/libwpe/RasterWindowContextLibWPE.h"
#endif
#endif

#if PLATFORM(MAC)
#ifdef RNS_SHELL_HAS_GPU_SUPPORT
#include "rns_shell/platform/graphics/gl/nsgl/GLWindowContextNSGL.h"
#else
#include "rns_shell/platform/mac/RasterWindowContextMac.h"
#endif // RNS_SHELL_HAS_GPU_SUPPORT
#endif // PLATFORM(MAC)

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
#elif USE(NSGL)
    if(auto nsglContext = GLWindowContextNSGL::createContext(windowHandle, display, params))
        return nsglContext;
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
#elif PLATFORM(MAC)
    if(auto rasterContext = RasterWindowContextMac::createContext(windowHandle, display, params))
        return rasterContext;
#else
    RNS_LOG_NOT_IMPL;
#endif
    return nullptr;
}

#endif // RNS_SHELL_HAS_GPU_SUPPORT

}  // namespace WCF

}  // namespace RnsShell
