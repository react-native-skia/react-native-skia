
/*
 * Copyright 2016 Google Inc.
 * Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GLWindowContextEGL_DEFINED
#define GLWindowContextEGL_DEFINED

#include "include/gpu/gl/GrGLInterface.h"
#include "GLWindowContext.h"
#include "PlatformDisplay.h"
#include "WindowContextFactory.h"

using sk_app::window_context_factory::UnixWindowInfo;
using sk_app::DisplayParams;
using sk_app::GLWindowContext;

namespace sk_app {

class GLWindowContextEGL : public GLWindowContext {
public:
    GLWindowContextEGL(const UnixWindowInfo&, PlatformDisplay&, const DisplayParams&);

    static const char* errorString(int statusCode);
    static const char* eglErrorString();

    ~GLWindowContextEGL() override;

    static bool isExtensionSupported(const char* extensionList, const char* extension);

    void onSwapBuffers() override;
    void onDestroyContext() override;

protected:
    sk_sp<const GrGLInterface> onInitializeContext() override;

private:
    static EGLContext createContextForEGLVersion(PlatformDisplay&, EGLConfig, EGLContext);

    GLWindowContextEGL(void*, const DisplayParams&);

    enum EGLSurfaceType { PbufferSurface, WindowSurface, PixmapSurface, Surfaceless };

    EGLContext createWindowContext(GLNativeWindowType window, PlatformDisplay& platformDisplay, EGLContext sharingContext);
    EGLContext createSharingContext(PlatformDisplay& platformDisplay);
    bool createContext(GLNativeWindowType window, PlatformDisplay& platformDisplay);
    bool makeContextCurrent();
    void swapInterval();

#if PLATFORM(X11)
    static EGLSurface createWindowSurfaceX11(EGLDisplay, EGLConfig, GLNativeWindowType);
#endif

    static bool getEGLConfig(EGLDisplay, EGLConfig*, EGLSurfaceType);

    GLNativeWindowType      fWindow;

    PlatformDisplay& fPlatformDisplay;
    EGLSurface fGLsurface { nullptr };
    EGLContext fGLContext { nullptr };
    EGLSurfaceType fType;

    typedef GLWindowContext INHERITED;
};

}  // anonymous namespace

#endif // GLWindowContextEGL_DEFINED
