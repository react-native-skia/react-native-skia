/*
 * Copyright 2016 Google Inc.
 * Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

#include "include/gpu/gl/GrGLInterface.h"
#include "rns_shell/platform/graphics/gl/GLWindowContext.h"
#include "rns_shell/platform/graphics/PlatformDisplay.h"
#include "rns_shell/platform/graphics/WindowContextFactory.h"


namespace RnsShell {

class GLWindowContextGLX : public GLWindowContext {
public:
    static std::unique_ptr<WindowContext> createContext(GLNativeWindowType window, PlatformDisplay& platformDisplay, const DisplayParams& params);
    static GLXContext createSharingContext(PlatformDisplay& platformDisplay);

    GLWindowContextGLX(GLNativeWindowType, GLXFBConfig, PlatformDisplay&, const DisplayParams&, GLXContext);
    ~GLWindowContextGLX() override;

    void onSwapBuffers(std::vector<SkIRect> &damage) override;
    void onDestroyContext() override;
#if USE(RNS_SHELL_PARTIAL_UPDATES)
    bool onHasSwapBuffersWithDamage() override { RNS_LOG_NOT_IMPL; return false; }
    bool onHasBufferCopy() override { RNS_LOG_NOT_IMPL; return false; };
    int32_t getBufferAge() override { RNS_LOG_NOT_IMPL; return 0; }
#endif
protected:
    sk_sp<const GrGLInterface> onInitializeContext() override;

private:
    static std::unique_ptr<GLWindowContextGLX> createWindowContext(GLNativeWindowType, PlatformDisplay&, const DisplayParams&, GLXContext sharingContext = nullptr);
    bool makeContextCurrent() override;
    void swapInterval();

    Display*     display_;
    XVisualInfo* visualInfo_;
    GLNativeWindowType      window_;
    GLXContext   glContext_;

    typedef GLWindowContext INHERITED;
};

}  // namespace RnsShell

