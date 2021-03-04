/*
 * Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef PlatformDisplay_DEFINED
#define PlatformDisplay_DEFINED

#include <iostream>
#include "SkAppUtil.h"

#ifdef SKA_HAS_GPU_SUPPORT
#include "GLWindowContext.h"
#else
typedef uint64_t GLNativeWindowType;
#endif

namespace sk_app {

class PlatformDisplay {
public:
    SKA_EXPORT static PlatformDisplay& sharedDisplay();
    SKA_EXPORT static PlatformDisplay& sharedDisplayForCompositing();
    virtual ~PlatformDisplay();

    enum class Type {
#if PLATFORM(X11)
        X11,
#endif
#if PLATFORM(WAYLAND)
        Wayland,
#endif
#if PLATFORM(DFB)
        DFB,
#endif
#if PLATFORM(WIN)
        Windows,
#endif
#if PLATFORM(LIBWPE) || USE(WPE_RENDERER)
        WPE,
#endif
    };

    virtual Type type() const = 0;

#if USE(EGL)
    SKA_EXPORT EGLContext* sharingGLContext();
#elif USE(GLX)
    SKA_EXPORT GLXContext* sharingGLContext();
#endif

#if USE(EGL)
    EGLDisplay eglDisplay() const;
    bool eglCheckVersion(int major, int minor) const;
#endif

protected:
    explicit PlatformDisplay(bool);
    bool fNativeDisplayOwned { false };

#if USE(EGL)
    virtual void initializeEGLDisplay();

    EGLDisplay fEglDisplay;
#endif

#if USE(EGL)
    std::unique_ptr<EGLContext> fSharingGLContext;
#elif USE(GLX)
    std::unique_ptr<GLXContext> fSharingGLContext;
#endif

private:
    static std::unique_ptr<PlatformDisplay> createPlatformDisplay();

#if USE(EGL)
    void terminateEGLDisplay();

    bool fEglDisplayInitialized { false };
    int fEglMajorVersion { 0 };
    int fEglMinorVersion { 0 };
#endif

};
} // namespace sk_app

#endif // PlatformDisplay_DEFINED
