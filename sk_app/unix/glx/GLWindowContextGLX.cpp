
/*
 * Copyright 2016 Google Inc.
 * Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/gl/GrGLInterface.h"
#include "GLWindowContext.h"
#include "glx/GLWindowContextGLX.h"

#if USE(GLX)

#include <GL/gl.h>

using sk_app::window_context_factory::UnixWindowInfo;
using sk_app::DisplayParams;
using sk_app::GLWindowContext;

namespace sk_app {

static bool gCtxErrorOccurred = false;
static int ctxErrorHandler(Display *dpy, XErrorEvent *ev) {
    gCtxErrorOccurred = true;
    return 0;
}

static PFNGLXSWAPINTERVALEXTPROC glXSwapIntervalEXT;

static bool hasEXTSwapControlExtension(Display* display)
{
    static bool initialized = false;
    if (initialized)
        return !!glXSwapIntervalEXT;

    initialized = true;

    const char* glxExtensions = glXQueryExtensionsString(display, DefaultScreen(display));
    if (glxExtensions) {
        if (strstr(glxExtensions, "GLX_EXT_swap_control")) {
            glXSwapIntervalEXT = (PFNGLXSWAPINTERVALEXTPROC)glXGetProcAddressARB((const GLubyte*)"glXSwapIntervalEXT");
            return !!glXSwapIntervalEXT;
        } else
            return false;
    }

    return false;
}

GLWindowContextGLX::GLWindowContextGLX(const UnixWindowInfo& winInfo, const DisplayParams& params)
        : INHERITED(params)
        , fDisplay(winInfo.native.fDisplay)
        , fFBConfig(winInfo.native.fFBConfig)
        , fVisualInfo(winInfo.native.fVisualInfo)
        , fWindow(winInfo.fWindow)
        , fGLContext() {
    fWidth = winInfo.fWidth;
    fHeight = winInfo.fHeight;

    this->initializeContext();
}

using CreateContextAttribsFn = GLXContext(Display*, GLXFBConfig, GLXContext, Bool, const int*);

bool GLWindowContextGLX::createWindowContext(XWindow window, PlatformDisplay& platformDisplay, GLXContext sharingContext)
{
    sk_sp<const GrGLInterface> interface;
    Display* display = (dynamic_cast<PlatformDisplayX11&>(platformDisplay)).native();
    bool current = false;

    SK_APP_UNUSED(display);
    SK_APP_UNUSED(window);

    GLXContext glxcontext = nullptr;

    // We attempt to use glXCreateContextAttribsARB as RenderDoc requires that the context be
    // created with this rather than glXCreateContext.
    CreateContextAttribsFn* createContextAttribs = (CreateContextAttribsFn*)glXGetProcAddressARB(
            (const GLubyte*)"glXCreateContextAttribsARB");

    if (createContextAttribs && fFBConfig) {
        // Install Xlib error handler that will set gCtxErrorOccurred
        int (*oldHandler)(Display*, XErrorEvent*) = XSetErrorHandler(&ctxErrorHandler);

        // Specifying 3.2 allows an arbitrarily high context version (so long as no 3.2 features
        // have been removed).
        for (int minor = 2; minor >= 0 && !glxcontext; --minor) {
            // Ganesh prefers a compatibility profile for possible NVPR support. However, RenderDoc
            // requires a core profile. Edit this code to use RenderDoc.
            for (int profile : {GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
                                GLX_CONTEXT_CORE_PROFILE_BIT_ARB}) {
                gCtxErrorOccurred = false;
                int attribs[] = {
                        GLX_CONTEXT_MAJOR_VERSION_ARB, 3, GLX_CONTEXT_MINOR_VERSION_ARB, minor,
                        GLX_CONTEXT_PROFILE_MASK_ARB, profile,
                        0
                };
                glxcontext = createContextAttribs(fDisplay, *fFBConfig, sharingContext, True, attribs);

                // Sync to ensure any errors generated are processed.
                XSync(fDisplay, False);
                if (gCtxErrorOccurred) { continue; }

                fGLContext = glxcontext;
                if (glxcontext && profile == GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB &&
                    makeContextCurrent()) {
                    current = true;
                    // Look to see if RenderDoc is attached. If so, re-create the context with a
                    // core profile.
                    interface = GrGLMakeNativeInterface();
                    if (interface && interface->fExtensions.has("GL_EXT_debug_tool")) {
                        interface.reset();
                        glXMakeCurrent(fDisplay, None, nullptr);
                        glXDestroyContext(fDisplay, glxcontext);
                        current = false;
                        fGLContext = glxcontext = nullptr;
                    }
                }
                if (glxcontext) {
                    break;
                }
            }
        }
        // Restore the original error handler
        XSetErrorHandler(oldHandler);
    }

    glXGetConfig(fDisplay, fVisualInfo, GLX_STENCIL_SIZE, &fStencilBits);
    glXGetConfig(fDisplay, fVisualInfo, GLX_SAMPLES_ARB, &fSampleCount);
    fSampleCount = std::max(fSampleCount, 1);

    return current;
}

bool GLWindowContextGLX::createContext(XWindow window, PlatformDisplay& platformDisplay)
{
    GLXContext glxSharingContext = nullptr;
    auto currentSet = window ? createWindowContext(window, platformDisplay, glxSharingContext) : false;
    return currentSet;
}


sk_sp<const GrGLInterface> GLWindowContextGLX::onInitializeContext() {
    SkASSERT(fDisplay);
    SkASSERT(!fGLContext);
    sk_sp<const GrGLInterface> interface;
    bool current = false;

    PlatformDisplay& pDisplay = PlatformDisplay::sharedDisplay();
    Display* display = dynamic_cast<sk_app::PlatformDisplayX11&>(pDisplay).native();

    current = createContext(fWindow, pDisplay);

    if (!fGLContext) {
        fGLContext = glXCreateContext(fDisplay, fVisualInfo, nullptr, GL_TRUE);
    }
    if (!fGLContext) {
        return nullptr;
    }

    if (!current && !makeContextCurrent()) {
        return nullptr;
    }

    glClearStencil(0);
    glClearColor(0, 0, 0, 0);
    glStencilMask(0xffffffff);
    glClear(GL_STENCIL_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    XWindow root;
    int x, y;
    unsigned int border_width, depth;
    XGetGeometry(fDisplay, fWindow, &root, &x, &y, (unsigned int*)&fWidth, (unsigned int*)&fHeight,
                 &border_width, &depth);
    glViewport(0, 0, fWidth, fHeight);

    swapInterval();
    return interface ? interface : GrGLMakeNativeInterface();
}

GLWindowContextGLX::~GLWindowContextGLX() {
    this->destroyContext();
}

void GLWindowContextGLX::onDestroyContext() {
    if (!fDisplay || !fGLContext) {
        return;
    }
    glXMakeCurrent(fDisplay, None, nullptr);
    glXDestroyContext(fDisplay, fGLContext);
    fGLContext = nullptr;
}

bool GLWindowContextGLX::makeContextCurrent()
{
    SkASSERT(fGLContext && fWindow);

    if (glXGetCurrentContext() == fGLContext)
        return true;

    if (fWindow)
        return glXMakeCurrent(fDisplay, fWindow, fGLContext);

    return false;
}


void GLWindowContextGLX::onSwapBuffers() {
    if (fDisplay && fGLContext) {
        glXSwapBuffers(fDisplay, fWindow);
    }
}

void GLWindowContextGLX::swapInterval()
{
    if (!hasEXTSwapControlExtension(fDisplay))
        return;
    glXSwapIntervalEXT(fDisplay, fWindow, fDisplayParams.fDisableVsync ? 0 : 1);
}

}  // anonymous namespace

#endif // USE(GLX)
