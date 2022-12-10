/*
 * Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "ReactSkia/utils/RnsUtils.h"

#if PLATFORM(WAYLAND)
#include <wayland-egl.h>
#endif
#include <wpe/wpe-egl.h>

#include "rns_shell/platform/graphics/gl/egl/GLWindowContextEGL.h"

#if USE(EGL) && USE(WPE_RENDERER)

namespace RnsShell {

EGLSurface GLWindowContextEGL::createWindowSurfaceWPE(EGLDisplay display, EGLConfig config, GLNativeWindowType window){
    return eglCreateWindowSurface(display, config, reinterpret_cast<EGLNativeWindowType>(window), nullptr);
}

void GLWindowContextEGL::destroyWPETarget() {
    if (wpeTarget_)
        wpe_renderer_backend_egl_offscreen_target_destroy(wpeTarget_);
}

} // namespace RnsShell
#endif // USE(EGL) && USE(WPE_RENDERER)
