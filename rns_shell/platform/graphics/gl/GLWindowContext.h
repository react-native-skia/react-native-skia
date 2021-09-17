/*
 * Copyright 2016 Google Inc.
 * Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#pragma once

#include "ReactSkia/utils/RnsUtils.h"

<<<<<<< HEAD
#define GL_GLEXT_PROTOTYPES 1
#if USE(OPENGL_ES)
#include <GLES2/gl2.h>
#include <GLES3/gl3.h>
#include <GLES2/gl2ext.h>
#else
=======
#if USE(OPENGL_ES)
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#else
#define GL_GLEXT_PROTOTYPES 1
>>>>>>> RNS Shell Implementation  (#8)
#include <GL/gl.h>
#endif // USE(OPENGL_ES)

#if USE(EGL)
<<<<<<< HEAD
#define EGL_GLEXT_PROTOTYPES 1
=======
>>>>>>> RNS Shell Implementation  (#8)
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <EGL/eglplatform.h>
#elif USE(GLX)
#include <GL/glx.h>
#endif // USE(EGL)

#include "include/gpu/gl/GrGLInterface.h"

#include "include/core/SkRefCnt.h"
#include "include/core/SkSurface.h"

#include "PlatformDisplay.h"
#include "WindowContext.h"

#if USE(EGL)
typedef EGLNativeWindowType GLNativeWindowType;
#else
typedef uint64_t GLNativeWindowType;
#endif

namespace RnsShell {

class GLWindowContext : public WindowContext {
public:
    sk_sp<SkSurface> getBackbufferSurface() override;

    bool isValid() override { return SkToBool(backendContext_.get()); }

<<<<<<< HEAD
<<<<<<< HEAD
    void swapBuffers(std::vector<SkIRect> &damage) override;
#if USE(RNS_SHELL_PARTIAL_UPDATES)
    bool hasSwapBuffersWithDamage() override;
    bool hasBufferCopy() override;
#endif
=======
    void swapBuffers() override;
>>>>>>> RNS Shell Implementation  (#8)
=======
    void swapBuffers(std::vector<SkIRect> &damage) override;
    bool hasSwapBuffersWithDamage() override;
>>>>>>> Munez graphics (#20)

    void setDisplayParams(const DisplayParams& params) override;
    static std::unique_ptr<GLWindowContext> createContextForWindow(GLNativeWindowType windowHandle, PlatformDisplay* = nullptr);

protected:
    GLWindowContext(const DisplayParams&);
    // This should be called by subclass constructor. It is also called when window/display
    // parameters change. This will in turn call onInitializeContext().
    void initializeContext();
    virtual sk_sp<const GrGLInterface> onInitializeContext() = 0;

    // This should be called by subclass destructor. It is also called when window/display
    // parameters change prior to initializing a new GL context. This will in turn call
    // onDestroyContext().
    void destroyContext();
    virtual void onDestroyContext() = 0;
<<<<<<< HEAD
<<<<<<< HEAD
    virtual void onSwapBuffers(std::vector<SkIRect> &damage) = 0;
#if USE(RNS_SHELL_PARTIAL_UPDATES)
    virtual bool onHasSwapBuffersWithDamage() = 0;
    virtual bool onHasBufferCopy() = 0;
#endif
=======
    virtual void onSwapBuffers() = 0;
=======
    virtual void onSwapBuffers(std::vector<SkIRect> &damage) = 0;
    virtual bool onHasSwapBuffersWithDamage() = 0;
>>>>>>> Munez graphics (#20)

>>>>>>> RNS Shell Implementation  (#8)
    sk_sp<const GrGLInterface> backendContext_;
    sk_sp<SkSurface>           surface_;
};

}   // namespace RnsShell
