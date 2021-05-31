/*
 * Copyright 2016 Google Inc.
 * Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/gl/GrGLInterface.h"
#include "gl/GLWindowContext.h"

#if USE(GLX)

#include <GL/gl.h>

#include "glx/GLWindowContextGLX.h"
#include "Performance.h"

using RnsShell::DisplayParams;
using RnsShell::GLWindowContext;

namespace RnsShell {

static bool gCtxErrorOccurred = false;
static int ctxErrorHandler(Display *dpy, XErrorEvent *ev) {
    gCtxErrorOccurred = true;
    return 0;
}

static PFNGLXSWAPINTERVALEXTPROC glXSwapIntervalEXT;
static PFNGLXSWAPINTERVALMESAPROC glXSwapIntervalMESA;

static bool hasEXTSwapControlExtension(Display* display)
{
    static bool initialized = false;
    if (initialized)
        return !!glXSwapIntervalEXT || !!glXSwapIntervalMESA;
    initialized = true;

    const char* glxExtensions = glXQueryExtensionsString(display, DefaultScreen(display));
    if (glxExtensions) {
        if (strstr(glxExtensions, "GLX_EXT_swap_control")) {
            glXSwapIntervalEXT = (PFNGLXSWAPINTERVALEXTPROC)glXGetProcAddressARB((const GLubyte*)"glXSwapIntervalEXT");
            return !!glXSwapIntervalEXT;
        } else if (strstr(glxExtensions, "GLX_MESA_swap_control")) {
            glXSwapIntervalMESA = (PFNGLXSWAPINTERVALMESAPROC)glXGetProcAddressARB((const GLubyte*)"glXSwapIntervalMESA");
            return !!glXSwapIntervalMESA;
        } else {
            RNS_LOG_WARN("Neither EXT nor MESA GLX_Swap_Control is supported");
            return false;
        }
    }

    return false;
}

GLWindowContextGLX::GLWindowContextGLX(GLNativeWindowType window, GLXFBConfig config, PlatformDisplay& platformDisplay, const DisplayParams& params, GLXContext context)
        : INHERITED(params)
        , display_((dynamic_cast<PlatformDisplayX11&>(platformDisplay)).native())
        , visualInfo_(glXGetVisualFromFBConfig(display_, config))
        , window_(window)
        , glContext_(context) {

    glXGetConfig(display_, visualInfo_, GLX_STENCIL_SIZE, &stencilBits_);
    glXGetConfig(display_, visualInfo_, GLX_SAMPLES_ARB, &sampleCount_);
    sampleCount_ = std::max(sampleCount_, 1);

    this->initializeContext();
    RNS_LOG_DEBUG("GLWindowContextGLX constructed with WH(" << width_ << " x " << height_ << "SampleCount & StencilBits : " << sampleCount_ << "," << stencilBits_);
}

using CreateContextAttribsFn = GLXContext(Display*, GLXFBConfig, GLXContext, Bool, const int*);

std::unique_ptr<GLWindowContextGLX> GLWindowContextGLX::createWindowContext(GLNativeWindowType window, PlatformDisplay& platformDisplay, const DisplayParams& params, GLXContext sharingContext) {
    sk_sp<const GrGLInterface> interface;
    Display* display = (dynamic_cast<PlatformDisplayX11&>(platformDisplay)).native();
    bool current = false;

    GLXContext glxcontext = nullptr;

    XWindowAttributes attributes;
    if (!XGetWindowAttributes(display, static_cast<Window>(window), &attributes))
        return nullptr;

    XVisualInfo visualInfo;
    visualInfo.visualid = XVisualIDFromVisual(attributes.visual);

    int numConfigs = 0;
    GLXFBConfig config = nullptr;
    std::unique_ptr<GLXFBConfig> configs(glXGetFBConfigs(display, DefaultScreen(display), &numConfigs));
    for (int i = 0; i < numConfigs; i++) {
        std::unique_ptr<XVisualInfo> glxVisualInfo(glXGetVisualFromFBConfig(display, configs.get()[i]));
        if (!glxVisualInfo)
            continue;

        if (glxVisualInfo.get()->visualid == visualInfo.visualid) {
            config = configs.get()[i];
            break;
        }
    }

    // We attempt to use glXCreateContextAttribsARB as RenderDoc requires that the context be
    // created with this rather than glXCreateContext.
    CreateContextAttribsFn* createContextAttribs = (CreateContextAttribsFn*)glXGetProcAddressARB(
            (const GLubyte*)"glXCreateContextAttribsARB");

    if (createContextAttribs && config) {
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
                glxcontext = createContextAttribs(display, config, sharingContext, True, attribs);

                // Sync to ensure any errors generated are processed.
                //XSync(display_, False);
                if (gCtxErrorOccurred) { continue; }

                if (glxcontext && profile == GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB &&
                    glXMakeCurrent(display, window, glxcontext)) {
                    current = true;
                    // Look to see if RenderDoc is attached. If so, re-create the context with a
                    // core profile.
                    interface = GrGLMakeNativeInterface();
                    if (interface && interface->fExtensions.has("GL_EXT_debug_tool")) {
                        interface.reset();
                        glXMakeCurrent(display, None, nullptr);
                        glXDestroyContext(display, glxcontext);
                        current = false;
                        glxcontext = nullptr;
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
    if(glxcontext == nullptr)
        return nullptr;

    return std::unique_ptr<GLWindowContextGLX>(new GLWindowContextGLX(window, config, platformDisplay, params, glxcontext));
}

std::unique_ptr<WindowContext> GLWindowContextGLX::createContext(GLNativeWindowType window, PlatformDisplay& platformDisplay, const DisplayParams& params) {
    GLXContext glxSharingContext = createSharingContext(platformDisplay);
    auto context = window ? createWindowContext(window, platformDisplay, params, glxSharingContext) : nullptr;
    if (!context) {
        RNS_LOG_ERROR("Could not create GLX context\n");
        return nullptr;
    }
    return std::move(context);
}

GLXContext GLWindowContextGLX::createSharingContext(PlatformDisplay& platformDisplay) {
    // TODO This is not required for now.
    return nullptr;
}

sk_sp<const GrGLInterface> GLWindowContextGLX::onInitializeContext() {
    sk_sp<const GrGLInterface> interface;

    PlatformDisplay& pDisplay = PlatformDisplay::sharedDisplay();
    Display* display = dynamic_cast<RnsShell::PlatformDisplayX11&>(pDisplay).native();

    if (!glContext_) {
        glContext_ = glXCreateContext(display_, visualInfo_, nullptr, GL_TRUE);
    }
    if (!glContext_) {
        return nullptr;
    }

    if (!makeContextCurrent()) {
        return nullptr;
    }

    glClearStencil(0);
    glClearColor(0, 0, 0, 0);
    glStencilMask(0xffffffff);
    glClear(GL_STENCIL_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    Window root;
    int x, y;
    unsigned int border_width, depth;
    XGetGeometry(display_, window_, &root, &x, &y, (unsigned int*)&width_, (unsigned int*)&height_,
                 &border_width, &depth);
    glViewport(0, 0, width_, height_);

    if (hasEXTSwapControlExtension(display_))
        swapInterval();
    return interface ? interface : GrGLMakeNativeInterface();
}

GLWindowContextGLX::~GLWindowContextGLX() {
    this->destroyContext();
}

void GLWindowContextGLX::onDestroyContext() {
    if (!display_ || !glContext_) {
        return;
    }
    glXMakeCurrent(display_, None, nullptr);
    glXDestroyContext(display_, glContext_);
    glContext_ = nullptr;
}

bool GLWindowContextGLX::makeContextCurrent() {
    SkASSERT(glContext_ && window_);

    if (glXGetCurrentContext() == glContext_)
        return true;

    if (window_)
        return glXMakeCurrent(display_, window_, glContext_);

    return false;
}


void GLWindowContextGLX::onSwapBuffers() {
    if (display_ && glContext_) {
#if !defined(GOOGLE_STRIP_LOG) || (GOOGLE_STRIP_LOG <= INFO)
        RNS_GET_TIME_STAMP_US(start);
#endif
        glXSwapBuffers(display_, window_);
#if !defined(GOOGLE_STRIP_LOG) || (GOOGLE_STRIP_LOG <= INFO)
        RNS_GET_TIME_STAMP_US(end);
        Performance::takeSamples(end - start);
#endif
    }
}

void GLWindowContextGLX::swapInterval() {
    if(glXSwapIntervalEXT) {
        glXSwapIntervalEXT(display_, window_, displayParams_.disableVsync_ ? 0 : 1);
    } else if (glXSwapIntervalMESA) {
        glXSwapIntervalMESA(displayParams_.disableVsync_ ? 0 : 1);
    } else {
        RNS_LOG_WARN("No GLX Swap Control extensions available");
    }
}

}  // namespace RnsShell

#endif // USE(GLX)
