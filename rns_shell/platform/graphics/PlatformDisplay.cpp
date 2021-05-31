/*
 * Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "ReactSkia/utils/RnsUtils.h"

#if USE(EGL)
#include "egl/GLWindowContextEGL.h"
#elif USE(GLX)
#include "glx/GLWindowContextGLX.h"
#endif

#include "PlatformDisplay.h"
#if PLATFORM(X11)
#include "x11/PlatformDisplayX11.h"
#elif PLATFORM(LIBWPE) || USE(WPE_RENDERER)
#include "libwpe/PlatformDisplayLibWPE.h"
#endif

namespace RnsShell {

static PlatformDisplay* s_sharedDisplayForCompositing;

PlatformDisplay::PlatformDisplay(bool displayOwned)
    : nativeDisplayOwned_(displayOwned)
#if USE(EGL)
    , eglDisplay_(EGL_NO_DISPLAY)
#endif
{
}

PlatformDisplay::~PlatformDisplay() {
#if USE(EGL)
    if (eglDisplay_ != EGL_NO_DISPLAY) {
        eglTerminate(eglDisplay_);
        eglDisplay_ = EGL_NO_DISPLAY;
    }
#endif
    if(this == s_sharedDisplayForCompositing)
        s_sharedDisplayForCompositing = nullptr;
}

#if USE(EGL) || USE(GLX)
GLWindowContext* PlatformDisplay::sharingGLContext() {
    RNS_LOG_TODO("Implement " << __func__ << " in GLWindowContext to call GLWindowContextEGL/GLX");
#if 0 // TODO implement this function in GLWindowContext to call GLWindowContextEGL
    if (!sharingGLContext_)
        sharingGLContext_ = GLWindowContext::createSharingContext();
#endif
    return sharingGLContext_ ? sharingGLContext_.get() : nullptr;
}
#endif

#if USE(EGL)
EGLDisplay PlatformDisplay::eglDisplay() const {
    if (!eglDisplayInitialized_)
        const_cast<PlatformDisplay*>(this)->initializeEGLDisplay();
    return eglDisplay_;
}

bool PlatformDisplay::eglCheckVersion(int major, int minor) const {
    if (!eglDisplayInitialized_)
        const_cast<PlatformDisplay*>(this)->initializeEGLDisplay();

    return (eglMajorVersion_ > major) || ((eglMajorVersion_ == major) && (eglMinorVersion_ >= minor));
}

void PlatformDisplay::initializeEGLDisplay() {
    eglDisplayInitialized_ = true;

    if (eglDisplay_ == EGL_NO_DISPLAY) {
        eglDisplay_ = eglGetDisplay(EGL_DEFAULT_DISPLAY);
        if (eglDisplay_ == EGL_NO_DISPLAY) {
            RNS_LOG_ERROR("Cannot get default EGL display : " << GLWindowContextEGL::eglErrorString());
            return;
        }
    }

    EGLint majorVersion, minorVersion;
    if (eglInitialize(eglDisplay_, &majorVersion, &minorVersion) == EGL_FALSE) {
        RNS_LOG_ERROR("EGLDisplay Initialization failed : " << GLWindowContextEGL::eglErrorString());
        terminateEGLDisplay();
        return;
    }

    eglMajorVersion_ = majorVersion;
    eglMinorVersion_ = minorVersion;
}

void PlatformDisplay::terminateEGLDisplay() {
    sharingGLContext_ = nullptr;
    SkASSERT(eglDisplayInitialized_);
    if (eglDisplay_ == EGL_NO_DISPLAY)
        return;
    eglTerminate(eglDisplay_);
    eglDisplay_ = EGL_NO_DISPLAY;
}

#endif // USE(EGL)

std::unique_ptr<PlatformDisplay> PlatformDisplay::createPlatformDisplay() {

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
#elif PLATFORM(LIBWPE)
    return PlatformDisplayLibWPE::create();
#endif

    return nullptr;
}

PlatformDisplay& PlatformDisplay::sharedDisplay() {
#if PLATFORM(X11) || PLATFORM(LIBWPE)
    static std::once_flag onceFlag;
    static std::unique_ptr<PlatformDisplay> display;
    std::call_once(onceFlag, []{
        display = createPlatformDisplay();
    });
    return *display;
#else
    #error "!!!!!!!!!! Atleast one Platform needs to be selected !!!!!!!!!!"
#endif
}

PlatformDisplay& PlatformDisplay::sharedDisplayForCompositing() {
    return s_sharedDisplayForCompositing ? *s_sharedDisplayForCompositing : sharedDisplay();
}

bool PlatformDisplay::initialize() {
    if(s_sharedDisplayForCompositing)
        return true;
    if(!(s_sharedDisplayForCompositing = &sharedDisplay()))
        return false;

#if PLATFORM(LIBWPE)
    return dynamic_cast<RnsShell::PlatformDisplayLibWPE*>(s_sharedDisplayForCompositing)->initialize(wpe_renderer_host_create_client());
#else
    return true;
#endif
}

} // namespace RnsShell
