/*
 * Copyright 2016 Google Inc.
 * Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

#include "include/gpu/gl/GrGLInterface.h"

#include "gl/GLWindowContext.h"
#include "PlatformDisplay.h"
#include "WindowContextFactory.h"

#if PLATFORM(LIBWPE)
struct wpe_renderer_backend_egl_offscreen_target;
#endif

using RnsShell::DisplayParams;
using RnsShell::GLWindowContext;

namespace RnsShell {

class GLWindowContextEGL : public GLWindowContext {
public:
    static std::unique_ptr<WindowContext> createContext(GLNativeWindowType window, PlatformDisplay& platformDisplay, const DisplayParams& params);
    static EGLContext createSharingContext(PlatformDisplay& platformDisplay);

    GLWindowContextEGL(GLNativeWindowType window, EGLConfig config, PlatformDisplay& platformDisplay, const DisplayParams& params, EGLContext context, EGLSurface surface);
    ~GLWindowContextEGL() override;

    static const char* errorString(int statusCode);
    static const char* eglErrorString();

    static bool isExtensionSupported(const char* extensionList, const char* extension);

    void onSwapBuffers(std::vector<SkIRect> &damage) override;
    void onDestroyContext() override;
#if USE(RNS_SHELL_PARTIAL_UPDATES)
    bool onHasSwapBuffersWithDamage() override;
    bool onHasBufferCopy() override;
#endif
protected:
    sk_sp<const GrGLInterface> onInitializeContext() override;

private:

    GLWindowContextEGL(void*, const DisplayParams&);
#if USE(WPE_RENDERER)
    GLWindowContextEGL(PlatformDisplay&, EGLContext, EGLSurface, struct wpe_renderer_backend_egl_offscreen_target*);
#endif

    enum EGLSurfaceType { PbufferSurface, WindowSurface, PixmapSurface, Surfaceless };
    static EGLContext createContextForEGLVersion(PlatformDisplay&, EGLConfig, EGLContext);
#if PLATFORM(X11)
    static EGLSurface createWindowSurfaceX11(EGLDisplay, EGLConfig, GLNativeWindowType);
#elif PLATFORM(LIBWPE) || USE(WPE_RENDERER)
    static EGLSurface createWindowSurfaceWPE(EGLDisplay, EGLConfig, GLNativeWindowType);
#endif
    static bool getEGLConfig(EGLDisplay, EGLConfig*, EGLSurfaceType);
    static std::unique_ptr<GLWindowContextEGL> createWindowContext(GLNativeWindowType, PlatformDisplay&, const DisplayParams&, EGLContext sharingContext = nullptr);

#if USE(WPE_RENDERER)
    void destroyWPETarget();
#endif
    bool makeContextCurrent() override;
    void swapInterval();

    GLNativeWindowType      window_;
#if USE(WPE_RENDERER)
    struct wpe_renderer_backend_egl_offscreen_target* wpeTarget_ { nullptr };
#endif

#if USE(RNS_SHELL_PARTIAL_UPDATES)
    std::vector<EGLint> rectsToInts(EGLDisplay display, EGLSurface surface, const std::vector<SkIRect>& rects);
#if USE(RNS_SHELL_COPY_BUFFERS)
    void eglInitializeOffscreenFrameBuffer();
    void eglDeleteOffscreenFrameBuffer();
    void eglBlitAndSwapBuffers();
#endif //RNS_SHELL_COPY_BUFFERS
#endif //RNS_SHELL_PARTIAL_UPDATES

    PlatformDisplay& platformDisplay_;
    EGLSurface glSurface_ { nullptr };
    EGLContext glContext_ { nullptr };
    EGLSurfaceType surfaceType_;

#if USE(RNS_SHELL_PARTIAL_UPDATES) &&  USE(RNS_SHELL_COPY_BUFFERS)
    GLuint offScreenFbo_ { 0 };
    GLuint colorTexture_ { 0 };
    GLuint depthStencilTexture_ { 0 };
#endif


    typedef GLWindowContext INHERITED;
};

}  // namespace RnsShell
