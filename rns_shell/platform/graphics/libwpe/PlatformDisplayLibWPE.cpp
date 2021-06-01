/*
 * Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "ReactSkia/utils/RnsUtils.h"

#if USE(WPE_RENDERER)

#if USE(EGL)
#if PLATFORM(WAYLAND)
#include <wayland-egl.h> // wayland-egl.h defines WL_EGL_PLATFORM
#endif

#include <EGL/egl.h>
#include "egl/GLWindowContextEGL.h"
#endif

#include "PlatformDisplayLibWPE.h"

namespace RnsShell {

Display::Display()
    :viewBackend_(wpe_view_backend_create()) {
}

Display::~Display() {
    if(viewBackend_) {
        wpe_view_backend_destroy(viewBackend_);
        viewBackend_ = nullptr;
    }
}

std::unique_ptr<PlatformDisplayLibWPE> PlatformDisplayLibWPE::create() {
    static Display *display = new Display();
    if (!display)
        return nullptr;

    return std::unique_ptr<PlatformDisplayLibWPE>(new PlatformDisplayLibWPE(display));
}

PlatformDisplayLibWPE::PlatformDisplayLibWPE(Display *display)
    : PlatformDisplay(false)
    , display_(display) {

}

PlatformDisplayLibWPE::~PlatformDisplayLibWPE() {
    wpe_renderer_backend_egl_destroy(rendererBackend_);
}

bool PlatformDisplayLibWPE::initialize(int hostFd) {
    rendererBackend_ = wpe_renderer_backend_egl_create(hostFd);
#if USE(EGL)
    eglDisplay_ = eglGetDisplay(wpe_renderer_backend_egl_get_native_display(rendererBackend_));
    if (eglDisplay_ == EGL_NO_DISPLAY) {
        RNS_LOG_ERROR("PlatformDisplayLibWPE - Couldn't create the EGL display : " << GLWindowContextEGL::eglErrorString());
        return false;
    }

    PlatformDisplay::initializeEGLDisplay();
    return eglDisplay_ != EGL_NO_DISPLAY;
#else
    return true;
#endif
}

} // namespace RnsShell

#endif // USE(WPE_RENDERER) && USE(EGL)
