/*
 * Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "PlatformDisplay.h"

#if USE(EGL)
#include "egl/GLWindowContextEGL.h"
#endif

#if PLATFORM(X11)
#include "x11/PlatformDisplayX11.h"
#endif

#if PLATFORM(LIBWPE) || USE(WPE_RENDERER)
#include "libwpe/PlatformDisplayLibWPE.h"
#endif

namespace sk_app {

static PlatformDisplay* s_sharedDisplayForCompositing;

PlatformDisplay::PlatformDisplay(bool displayOwned)
    : fNativeDisplayOwned(displayOwned)
#if USE(EGL)
    , fEglDisplay(EGL_NO_DISPLAY)
#endif
{
}

PlatformDisplay::~PlatformDisplay()
{
#if USE(EGL)
    if (fEglDisplay != EGL_NO_DISPLAY) {
        eglTerminate(fEglDisplay);
        fEglDisplay = EGL_NO_DISPLAY;
    }
#endif
    if(this == s_sharedDisplayForCompositing)
        s_sharedDisplayForCompositing = nullptr;
}

#if USE(EGL)
EGLContext* PlatformDisplay::sharingGLContext()
{
#if 0 // TODO implement this function in GLWindowContex to call GLWindowContextEGL
    if (!fSharingGLContext)
        fSharingGLContext = GLWindowContext::createSharingContext();
#endif
    return fSharingGLContext ? fSharingGLContext.get() : nullptr;
}
#elif USE(GLX)
GLXContext* PlatformDisplay::sharingGLContext()
{
#if 0 // TODO implement this function in GLWindowContext to call GLWindowContextGLX
    if (!fSharingGLContext)
        fSharingGLContext = GLWindowContext::createSharingContext();
#endif
    return fSharingGLContext ? fSharingGLContext.get() : nullptr;
}
#endif

#if USE(EGL)
EGLDisplay PlatformDisplay::eglDisplay() const
{
    if (!fEglDisplayInitialized)
        const_cast<PlatformDisplay*>(this)->initializeEGLDisplay();
    return fEglDisplay;
}

bool PlatformDisplay::eglCheckVersion(int major, int minor) const
{
    if (!fEglDisplayInitialized)
        const_cast<PlatformDisplay*>(this)->initializeEGLDisplay();

    return (fEglMajorVersion > major) || ((fEglMajorVersion == major) && (fEglMinorVersion >= minor));
}

void PlatformDisplay::initializeEGLDisplay()
{
    fEglDisplayInitialized = true;

    if (fEglDisplay == EGL_NO_DISPLAY) {
        fEglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
        if (fEglDisplay == EGL_NO_DISPLAY) {
            SK_APP_LOG_ERROR("Cannot get default EGL display : %s\n", GLWindowContextEGL::eglErrorString());
            return;
        }
    }

    EGLint majorVersion, minorVersion;
    if (eglInitialize(fEglDisplay, &majorVersion, &minorVersion) == EGL_FALSE) {
        SK_APP_LOG_ERROR("EGLDisplay Initialization failed : %s\n", GLWindowContextEGL::eglErrorString());
        terminateEGLDisplay();
        return;
    }

    fEglMajorVersion = majorVersion;
    fEglMinorVersion = minorVersion;
}

void PlatformDisplay::terminateEGLDisplay()
{
    fSharingGLContext = nullptr;
    SkASSERT(fEglDisplayInitialized);
    if (fEglDisplay == EGL_NO_DISPLAY)
        return;
    eglTerminate(fEglDisplay);
    fEglDisplay = EGL_NO_DISPLAY;
}

#endif // USE(EGL)

std::unique_ptr<PlatformDisplay> PlatformDisplay::createPlatformDisplay()
{

#if PLATFORM(WAYLAND)
    if (auto platformDisplay = PlatformDisplayWayland::create())
        return platformDisplay;
    else
        return PlatformDisplayWayland::create(nullptr);
#endif

#if PLATFORM(X11)
    if (auto platformDisplay = PlatformDisplayX11::create())
        return platformDisplay;
    else
        return PlatformDisplayX11::create(nullptr);
#endif

#if PLATFORM(DFB)
    return PlatformDisplayDfb::create();
#endif

#if PLATFORM(WIN)
    return PlatformDisplayWin::create();
#elif USE(WPE_RENDERER)
    return PlatformDisplayLibWPE::create();
#endif

    return nullptr;
}

PlatformDisplay& PlatformDisplay::sharedDisplay()
{
#if PLATFORM(X11) || PLATFORM(LIBWPE) || USE(WPE_RENDERER)
    static std::once_flag onceFlag;
    static std::unique_ptr<PlatformDisplay> display;
    std::call_once(onceFlag, []{
        display = createPlatformDisplay();
    });
    return *display;
#else
    SK_APP_NOT_IMPL;
#endif
}

PlatformDisplay& PlatformDisplay::sharedDisplayForCompositing()
{
    return s_sharedDisplayForCompositing ? *s_sharedDisplayForCompositing : sharedDisplay();
}

} // namespace sk_app
