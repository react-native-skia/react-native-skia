
/*
 * Copyright 2016 Google Inc.
 * Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GLWindowContextGLX_DEFINED
#define GLWindowContextGLX_DEFINED

#include "include/gpu/gl/GrGLInterface.h"
#include "GLWindowContext.h"
#include "WindowContextFactory.h"

using sk_app::window_context_factory::UnixWindowInfo;
using sk_app::DisplayParams;
using sk_app::GLWindowContext;

namespace sk_app {

class GLWindowContextGLX : public GLWindowContext {
public:
    GLWindowContextGLX(const UnixWindowInfo&, const DisplayParams&);
    ~GLWindowContextGLX() override;

    void onSwapBuffers() override;
    void onDestroyContext() override;

protected:
    sk_sp<const GrGLInterface> onInitializeContext() override;

private:
    GLWindowContextGLX(void*, const DisplayParams&);
    bool createWindowContext(XWindow window, PlatformDisplay& platformDisplay, GLXContext sharingContext);
    bool createContext(XWindow window, PlatformDisplay& platformDisplay);
    bool makeContextCurrent();
    void swapInterval();

    Display*     fDisplay;
    GLXFBConfig* fFBConfig;
    XVisualInfo* fVisualInfo;
    XWindow      fWindow;
    GLXContext   fGLContext;

    typedef GLWindowContext INHERITED;
};

}  // anonymous namespace

#endif // GLWindowContextGLX_DEFINED
