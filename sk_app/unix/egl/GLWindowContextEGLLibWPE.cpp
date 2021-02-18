/*
 * Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GLWindowContextEGL.h"

#if USE(EGL) && USE(WPE_RENDERER)
#include "libwpe/PlatformDisplayLibWPE.h"

#if PLATFORM(WAYLAND)
#include <wayland-egl.h>
#endif

#include <EGL/egl.h>
#include <wpe/wpe-egl.h>

namespace sk_app {

EGLSurface GLWindowContextEGL::createWindowSurfaceWPE(EGLDisplay display, EGLConfig config, GLNativeWindowType window)
{
    return eglCreateWindowSurface(display, config, reinterpret_cast<EGLNativeWindowType>(window), nullptr);
}

void GLWindowContextEGL::destroyWPETarget()
{
    if (fWpeTarget)
        wpe_renderer_backend_egl_offscreen_target_destroy(fWpeTarget);
}

} // namespace sk_app

#endif // USE(EGL) && USE(WPE_RENDERER)
