/*
 * Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "ReactSkia/utils/RnsUtils.h"

#if USE(EGL) && PLATFORM(X11)
#include <EGL/egl.h>
#include <X11/Xlib.h>

#include "rns_shell/platform/graphics/gl/egl/GLWindowContextEGL.h"
#include "rns_shell/platform/graphics/x11/PlatformDisplayX11.h"

namespace RnsShell {

EGLSurface GLWindowContextEGL::createWindowSurfaceX11(EGLDisplay display, EGLConfig config, GLNativeWindowType window) {
    return eglCreateWindowSurface(display, config, static_cast<EGLNativeWindowType>(window), nullptr);
}

} // namespace RnsShell

#endif // USE(EGL) && PLATFORM(X11)
