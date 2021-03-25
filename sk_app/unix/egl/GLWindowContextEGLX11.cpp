/*
 * Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GLWindowContextEGL.h"

#if USE(EGL) && PLATFORM(X11)
#include "x11/PlatformDisplayX11.h"
#include <EGL/egl.h>
#include <X11/Xlib.h>

namespace sk_app {

EGLSurface GLWindowContextEGL::createWindowSurfaceX11(EGLDisplay display, EGLConfig config, GLNativeWindowType window)
{
    return eglCreateWindowSurface(display, config, static_cast<EGLNativeWindowType>(window), nullptr);
}

} // namespace sk_app

#endif // USE(EGL) && PLATFORM(X11)
