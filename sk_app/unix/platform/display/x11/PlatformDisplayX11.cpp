/*
 * Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "PlatformDisplayX11.h"

#if USE(EGL)
#include "egl/GLWindowContextEGL.h"
#endif

#if PLATFORM(X11)
#include <X11/Xlib.h>

#if USE(EGL)
#include <EGL/egl.h>
#include <EGL/eglext.h>
#elif USE(GLX)
#include <GL/glx.h>
#endif

namespace sk_app {

std::unique_ptr<PlatformDisplay> PlatformDisplayX11::create()
{
    XInitThreads();
    Display* display = XOpenDisplay(getenv("DISPLAY"));
    if (!display)
        return nullptr;

    return std::unique_ptr<PlatformDisplayX11>(new PlatformDisplayX11(display, true));
}

std::unique_ptr<PlatformDisplay> PlatformDisplayX11::create(Display* display)
{
    return std::unique_ptr<PlatformDisplayX11>(new PlatformDisplayX11(display, false));
}

PlatformDisplayX11::PlatformDisplayX11(Display* display, bool displayOwned)
    : PlatformDisplay(displayOwned)
    , fDisplay(display)
{
}

PlatformDisplayX11::~PlatformDisplayX11()
{
#if USE(EGL) || USE(GLX)
    // Clear the sharing context before releasing the display
    fSharingGLContext = nullptr;
#endif
    if (fNativeDisplayOwned == true)
        XCloseDisplay(fDisplay);
}

#if USE(EGL)
void PlatformDisplayX11::initializeEGLDisplay()
{
#if defined(EGL_KHR_platform_x11)
    const char* extensions = eglQueryString(nullptr, EGL_EXTENSIONS);
    if (GLWindowContextEGL::isExtensionSupported(extensions, "EGL_KHR_platform_base")) {
        if (auto* getPlatformDisplay = reinterpret_cast<PFNEGLGETPLATFORMDISPLAYEXTPROC>(eglGetProcAddress("eglGetPlatformDisplay")))
            fEglDisplay = getPlatformDisplay(EGL_PLATFORM_X11_KHR, fDisplay, nullptr);
    } else if (GLWindowContextEGL::isExtensionSupported(extensions, "EGL_EXT_platform_base")) {
        if (auto* getPlatformDisplay = reinterpret_cast<PFNEGLGETPLATFORMDISPLAYEXTPROC>(eglGetProcAddress("eglGetPlatformDisplayEXT")))
            fEglDisplay = getPlatformDisplay(EGL_PLATFORM_X11_KHR, fDisplay, nullptr);
    } else
#endif
    fEglDisplay = eglGetDisplay(fDisplay);

    PlatformDisplay::initializeEGLDisplay();
}
#endif // USE(EGL)

} // namespace sk_app

#endif // PLATFORM(X11)

