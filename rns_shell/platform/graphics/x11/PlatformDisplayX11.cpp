/*
 * Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "ReactSkia/utils/RnsUtils.h"

#if PLATFORM(X11)
#include <X11/Xlib.h>

#if USE(EGL)
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include "egl/GLWindowContextEGL.h"
#elif USE(GLX)
#include <GL/glx.h>
#include "glx/GLWindowContextGLX.h"
#endif

#include "PlatformDisplayX11.h"

namespace RnsShell {

std::unique_ptr<PlatformDisplay> PlatformDisplayX11::create() {
    XInitThreads();
    Display* display = XOpenDisplay(getenv("DISPLAY"));
    if (!display)
        return nullptr;

    return std::unique_ptr<PlatformDisplayX11>(new PlatformDisplayX11(display, true));
}

std::unique_ptr<PlatformDisplay> PlatformDisplayX11::create(Display* display) {
    return std::unique_ptr<PlatformDisplayX11>(new PlatformDisplayX11(display, false));
}

PlatformDisplayX11::PlatformDisplayX11(Display* display, bool displayOwned)
    : PlatformDisplay(displayOwned)
    , display_(display) {
    SkSize screenDimension = screenSize();

    setCurrentScreenSize(screenDimension.width(),screenDimension.height());
}

PlatformDisplayX11::~PlatformDisplayX11() {
#if USE(EGL) || USE(GLX)
    // Clear the sharing context before releasing the display
    sharingGLContext_ = nullptr;
#endif
    if (nativeDisplayOwned_ == true)
        XCloseDisplay(display_);
}

#if USE(EGL)
void PlatformDisplayX11::initializeEGLDisplay()
{
#if defined(EGL_KHR_platform_x11)
    const char* extensions = eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS);
    if (GLWindowContextEGL::isExtensionSupported(extensions, "EGL_KHR_platform_base")) {
        if (auto* getPlatformDisplay = reinterpret_cast<PFNEGLGETPLATFORMDISPLAYEXTPROC>(eglGetProcAddress("eglGetPlatformDisplay")))
            eglDisplay_ = getPlatformDisplay(EGL_PLATFORM_X11_KHR, display_, nullptr);
    } else if (GLWindowContextEGL::isExtensionSupported(extensions, "EGL_EXT_platform_base")) {
        if (auto* getPlatformDisplay = reinterpret_cast<PFNEGLGETPLATFORMDISPLAYEXTPROC>(eglGetProcAddress("eglGetPlatformDisplayEXT")))
            eglDisplay_ = getPlatformDisplay(EGL_PLATFORM_X11_EXT, display_, nullptr);
    } else
#endif
    eglDisplay_ = eglGetDisplay(display_);

    PlatformDisplay::initializeEGLDisplay();
}
#endif // USE(EGL)

SkSize PlatformDisplayX11::screenSize() {
    XWindowAttributes winAttr;
    Window rootWin = DefaultRootWindow(display_); // Root Window always returns the updated screen size.
    SkSize size = SkSize::MakeEmpty();

    XGetWindowAttributes(display_, rootWin, &winAttr);
    size.set(winAttr.width, winAttr.height);

    return size;
}

} // namespace RnsShell

#endif // PLATFORM(X11)

