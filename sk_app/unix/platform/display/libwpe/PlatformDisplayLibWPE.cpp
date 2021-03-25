/*
 * Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "PlatformDisplayLibWPE.h"

#if USE(WPE_RENDERER)

#include <wpe/wpe.h>

#if USE(EGL)
#include "egl/GLWindowContextEGL.h"

#if PLATFORM(WAYLAND)
#include <wayland-egl.h> // wayland-egl.h defines WL_EGL_PLATFORM
#endif

#include <EGL/egl.h>
#endif // USE(EGL)

#include <wpe/wpe-egl.h>

namespace sk_app {

Display::Display()
    :fViewBackend(wpe_view_backend_create())
{
}

Display::~Display()
{
    if(fViewBackend) {
        wpe_view_backend_destroy(fViewBackend);
        fViewBackend = nullptr;
    }
}

std::unique_ptr<PlatformDisplayLibWPE> PlatformDisplayLibWPE::create()
{
    static Display *display = new Display();
    if (!display)
        return nullptr;

    return std::unique_ptr<PlatformDisplayLibWPE>(new PlatformDisplayLibWPE(display));
}

PlatformDisplayLibWPE::PlatformDisplayLibWPE(Display *display)
    : PlatformDisplay(false)
    , fDisplay(display)
{

}

PlatformDisplayLibWPE::~PlatformDisplayLibWPE()
{
    wpe_renderer_backend_egl_destroy(fRendererBackend);
}

bool PlatformDisplayLibWPE::initialize(int hostFd)
{
    fRendererBackend = wpe_renderer_backend_egl_create(hostFd);
#if USE(EGL)
    fEglDisplay = eglGetDisplay(wpe_renderer_backend_egl_get_native_display(fRendererBackend));
    if (fEglDisplay == EGL_NO_DISPLAY) {
        SK_APP_LOG_ERROR("PlatformDisplayLibWPE - Couldn't create the EGL display %s\n", GLWindowContextEGL::eglErrorString());
        return false;
    }

    PlatformDisplay::initializeEGLDisplay();
    return fEglDisplay != EGL_NO_DISPLAY;
#else
    return true;
#endif
}

} // namespace sk_app

#endif // USE(WPE_RENDERER) && USE(EGL)
