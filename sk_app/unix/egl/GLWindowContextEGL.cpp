/*
 * Copyright 2016 Google Inc.
 * Copyright (C) 1994-2020 OpenTV, Inc. and Nagravision S.A.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/gl/GrGLInterface.h"
#include "GLWindowContext.h"
#include "egl/GLWindowContextEGL.h"

#if USE(EGL)

using sk_app::DisplayParams;
using sk_app::GLWindowContext;
using sk_app::window_context_factory::UnixWindowInfo;

namespace sk_app {

#if USE(OPENGL_ES)
static const char* gEGLAPIName = "OpenGL ES";
static const EGLenum gEGLAPIVersion = EGL_OPENGL_ES_API;
#else
static const char* gEGLAPIName = "OpenGL";
static const EGLenum gEGLAPIVersion = EGL_OPENGL_API;
#endif

const char* GLWindowContextEGL::errorString(int statusCode)
{
    static_assert(sizeof(int) >= sizeof(EGLint), "EGLint must not be wider than int");
    switch (statusCode) {
#define CASE_TO_STRING(code) case code: return #code
        CASE_TO_STRING(EGL_SUCCESS);
        CASE_TO_STRING(EGL_NOT_INITIALIZED);
        CASE_TO_STRING(EGL_BAD_ACCESS);
        CASE_TO_STRING(EGL_BAD_ALLOC);
        CASE_TO_STRING(EGL_BAD_ATTRIBUTE);
        CASE_TO_STRING(EGL_BAD_CONTEXT);
        CASE_TO_STRING(EGL_BAD_CONFIG);
        CASE_TO_STRING(EGL_BAD_CURRENT_SURFACE);
        CASE_TO_STRING(EGL_BAD_DISPLAY);
        CASE_TO_STRING(EGL_BAD_SURFACE);
        CASE_TO_STRING(EGL_BAD_MATCH);
        CASE_TO_STRING(EGL_BAD_PARAMETER);
        CASE_TO_STRING(EGL_BAD_NATIVE_PIXMAP);
        CASE_TO_STRING(EGL_BAD_NATIVE_WINDOW);
        CASE_TO_STRING(EGL_CONTEXT_LOST);
#undef CASE_TO_STRING
    default: return "EGL_UNKNOWN_ERROR";
    }
}

const char* GLWindowContextEGL::eglErrorString()
{
    return errorString(eglGetError());
}

bool GLWindowContextEGL::getEGLConfig(EGLDisplay display, EGLConfig* config, EGLSurfaceType surfaceType)
{
    std::array<EGLint, 4> rgbaSize = { 8, 8, 8, 8 };

    EGLint attributeList[] = {
#if USE(OPENGL_ES)
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
#else
        EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
#endif
        EGL_RED_SIZE, rgbaSize[0],
        EGL_GREEN_SIZE, rgbaSize[1],
        EGL_BLUE_SIZE, rgbaSize[2],
        EGL_ALPHA_SIZE, rgbaSize[3],
        EGL_STENCIL_SIZE, 8,
        EGL_SURFACE_TYPE, EGL_NONE,
        EGL_NONE
    };

    switch (surfaceType) {
    case GLWindowContextEGL::PbufferSurface:
        attributeList[13] = EGL_PBUFFER_BIT;
        break;
    case GLWindowContextEGL::PixmapSurface:
        attributeList[13] = EGL_PIXMAP_BIT;
        break;
    case GLWindowContextEGL::WindowSurface:
    case GLWindowContextEGL::Surfaceless:
        attributeList[13] = EGL_WINDOW_BIT;
        break;
    }

    EGLint count;
    if (!eglChooseConfig(display, attributeList, nullptr, 0, &count)) {
        SK_APP_LOG_ERROR("Cannot get count of available EGL configurations: %s\n", eglErrorString());
        return false;
    }

    EGLint numConfigsReturned;
    EGLConfig eglConfig = 0;
    if (!eglChooseConfig(display, attributeList, &eglConfig, 1, &numConfigsReturned) || numConfigsReturned == 0) {
        SK_APP_LOG_ERROR("Cannot get available EGL configurations : %s\n", eglErrorString());
        return false;
    } else {
        EGLint redSize, greenSize, blueSize, alphaSize;
        eglGetConfigAttrib(display, eglConfig, EGL_RED_SIZE, &redSize);
        eglGetConfigAttrib(display, eglConfig, EGL_GREEN_SIZE, &greenSize);
        eglGetConfigAttrib(display, eglConfig, EGL_BLUE_SIZE, &blueSize);
        eglGetConfigAttrib(display, eglConfig, EGL_ALPHA_SIZE, &alphaSize);
        if(( redSize == rgbaSize[0] && greenSize == rgbaSize[1] && blueSize == rgbaSize[2] && alphaSize == rgbaSize[3])) {
            *config = eglConfig;
        } else {
            SK_APP_NOT_IMPL // Walk through all the available eglConfig and select the matching values with rgbaSize / refer WPE code
            return false;
        }
    }
    return true;
}

bool GLWindowContextEGL::isExtensionSupported(const char* extensionList, const char* extension)
{
    if (!extensionList)
        return false;

    SkASSERT(extension);
    int extensionLen = strlen(extension);
    const char* extensionListPtr = extensionList;
    while ((extensionListPtr = strstr(extensionListPtr, extension))) {
        if (extensionListPtr[extensionLen] == ' ' || extensionListPtr[extensionLen] == '\0')
            return true;
        extensionListPtr += extensionLen;
    }
    return false;
}

EGLContext GLWindowContextEGL::createContextForEGLVersion(PlatformDisplay& platformDisplay, EGLConfig config, EGLContext sharingContext)
{
    static EGLint contextAttributes[7];
    static bool contextAttributesInitialized = false;

    if (!contextAttributesInitialized) {
        contextAttributesInitialized = true;

#if USE(OPENGL_ES)
        contextAttributes[0] = EGL_CONTEXT_CLIENT_VERSION;
        contextAttributes[1] = 2;
        contextAttributes[2] = EGL_NONE;
#else
        EGLContext context = EGL_NO_CONTEXT;

        if (platformDisplay.eglCheckVersion(1, 5)) {
            contextAttributes[0] = EGL_CONTEXT_MAJOR_VERSION;
            contextAttributes[1] = 3;
            contextAttributes[2] = EGL_CONTEXT_MINOR_VERSION;
            contextAttributes[3] = 2;
            contextAttributes[4] = EGL_CONTEXT_OPENGL_PROFILE_MASK;
            contextAttributes[5] = EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT;
            contextAttributes[6] = EGL_NONE;

            // Try to create a context.
            context = eglCreateContext(platformDisplay.eglDisplay(), config, sharingContext, contextAttributes);
        } else if (platformDisplay.eglCheckVersion(1, 4)) {
            const char* extensions = eglQueryString(platformDisplay.eglDisplay(), EGL_EXTENSIONS);
            if (isExtensionSupported(extensions, "EGL_KHR_create_context")) {
                contextAttributes[0] = EGL_CONTEXT_MAJOR_VERSION_KHR;
                contextAttributes[1] = 3;
                contextAttributes[2] = EGL_CONTEXT_MINOR_VERSION_KHR;
                contextAttributes[3] = 2;
                contextAttributes[4] = EGL_CONTEXT_OPENGL_PROFILE_MASK_KHR;
                contextAttributes[5] = EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT_KHR;
                contextAttributes[6] = EGL_NONE;

                // Try to create a context.
                context = eglCreateContext(platformDisplay.eglDisplay(), config, sharingContext, contextAttributes);
            }
        }
        if (context != EGL_NO_CONTEXT)
            return context;

        // required EGL version is not present, so just request whatever is available.
        contextAttributes[0] = EGL_NONE;
#endif //OPENGL_ES
    }

    return eglCreateContext(platformDisplay.eglDisplay(), config, sharingContext, contextAttributes);
}

GLWindowContextEGL::GLWindowContextEGL(const UnixWindowInfo& winInfo, PlatformDisplay& platformDisplay, const DisplayParams& params)
        : INHERITED(params)
        , fWindow(winInfo.fWindow)
        , fPlatformDisplay(platformDisplay)
        , fGLsurface()
        , fGLContext() {

    fWidth = winInfo.fWidth;
    fHeight = winInfo.fHeight;

    this->initializeContext();
}

EGLContext GLWindowContextEGL::createWindowContext(GLNativeWindowType window, PlatformDisplay& platformDisplay, EGLContext sharingContext)
{
    EGLDisplay display = platformDisplay.eglDisplay();
    EGLConfig config;
    if (!getEGLConfig(display, &config, WindowSurface)) {
        SK_APP_LOG_ERROR("Cannot obtain EGL window context configuration: %s\n", eglErrorString());
        return nullptr;
    }

    EGLContext context = createContextForEGLVersion(platformDisplay, config, sharingContext);
    if (context == EGL_NO_CONTEXT) {
        SK_APP_LOG_ERROR("Cannot create EGL window context: %s\n", eglErrorString());
        return nullptr;
    }

    EGLSurface surface = EGL_NO_SURFACE;
    switch (platformDisplay.type()) {
#if PLATFORM(X11)
    case PlatformDisplay::Type::X11:
        surface = createWindowSurfaceX11(display, config, window);
        break;
#endif
    }

    if (surface == EGL_NO_SURFACE) {
#if defined(EGL_PLATFORM_BRCM) && EGL_PLATFORM_BRCM
        SK_APP_LOG_ERROR("Cannot create EGL window surface: %s. Retrying with fallback. window %p\n", eglErrorString(), window);
#else
        SK_APP_LOG_ERROR("Cannot create EGL window surface: %s. Retrying with fallback. window %lu\n", eglErrorString(), reinterpret_cast<GLNativeWindowType>(window));
#endif
        surface = eglCreateWindowSurface(display, config, static_cast<EGLNativeWindowType>(window), nullptr);
    }

    if (surface == EGL_NO_SURFACE) {
        SK_APP_LOG_ERROR("Cannot create EGL window surface: %s\n", eglErrorString());
        eglDestroyContext(display, context);
        return nullptr;
    }
    fGLsurface = surface;

    eglGetConfigAttrib(display, config, EGL_STENCIL_SIZE, &fStencilBits);
    eglGetConfigAttrib(display, config, EGL_SAMPLES, &fSampleCount);
    fSampleCount = std::max(fSampleCount, 1);

    return context;
}

bool GLWindowContextEGL::createContext(GLNativeWindowType window, PlatformDisplay& platformDisplay)
{
    if (platformDisplay.eglDisplay() == EGL_NO_DISPLAY) {
        SK_APP_LOG_ERROR("Cannot create EGL context: invalid display: %s\n", eglErrorString());
        return false;
    }

    if (eglBindAPI(gEGLAPIVersion) == EGL_FALSE) {
        SK_APP_LOG_ERROR("Cannot create EGL context: error binding %s API : %s\n", gEGLAPIName, eglErrorString());
        return false;
    }

    EGLContext eglSharingContext = platformDisplay.sharingGLContext() ? platformDisplay.sharingGLContext() : createSharingContext(platformDisplay);
    auto context = window ? createWindowContext(window, platformDisplay, eglSharingContext) : nullptr;
    if (!context) {
        SK_APP_LOG_ERROR("Could not create EGL context\n");
        return false;
    }
    fGLContext = context;
    return true;
}

EGLContext GLWindowContextEGL::createSharingContext(PlatformDisplay& platformDisplay)
{
    EGLDisplay display = platformDisplay.eglDisplay();
    EGLConfig config;

    if (display == EGL_NO_DISPLAY) {
        SK_APP_LOG_ERROR("Cannot create EGL context: invalid display: %s\n", eglErrorString());
        return EGL_NO_CONTEXT;
    }

    if (eglBindAPI(gEGLAPIVersion) == EGL_FALSE) {
        SK_APP_LOG_ERROR("Cannot create EGL context: error binding %s API : %s\n", gEGLAPIName, eglErrorString());
        return EGL_NO_CONTEXT;
    }

    const char* extensions = eglQueryString(display, EGL_EXTENSIONS);
    if (isExtensionSupported(extensions, "EGL_KHR_surfaceless_context") && isExtensionSupported(extensions, "EGL_KHR_surfaceless_opengl")) {
        SK_APP_LOG_ERROR("Cannot create surfaceless EGL context: required extensions missing.\n");
        return EGL_NO_CONTEXT;
    }

    if (!getEGLConfig(display, &config, Surfaceless)) {
        SK_APP_LOG_ERROR("Cannot obtain EGL surfaceless configuration: %s\n", eglErrorString());
        return EGL_NO_CONTEXT;
    }

    EGLContext context = createContextForEGLVersion(platformDisplay, config, nullptr);
    if (context == EGL_NO_CONTEXT) {
        SK_APP_LOG_ERROR("Cannot create EGL surfaceless context %s\n", eglErrorString());
        return EGL_NO_CONTEXT;
    }
    return context;
}

sk_sp<const GrGLInterface> GLWindowContextEGL::onInitializeContext() {
    SkASSERT(!fGLContext);
    sk_sp<const GrGLInterface> interface;
    bool current = false;

    EGLDisplay display = fPlatformDisplay.eglDisplay();
    if(EGL_NO_DISPLAY == display) {
        SK_APP_LOG_ERROR("Cannot initialize EGL context: invalid display: %s\n", eglErrorString());
        return nullptr;
    }

    createContext(fWindow, PlatformDisplay::sharedDisplay());

    if( fGLContext && true == eglMakeCurrent(display, fGLsurface, fGLsurface, fGLContext)) {
        // Look to see if RenderDoc is attached. If so, re-create the context with a
        // core profile.
        interface = GrGLMakeNativeInterface();
        current = true;
        if (interface && interface->fExtensions.has("GL_EXT_debug_tool")) {
            SK_APP_LOG_ERROR("Extension has GL_EXT_debug_tool\n");
            interface.reset();
            eglMakeCurrent(display, nullptr, nullptr, nullptr);
            eglDestroyContext(display, fGLContext);
            fGLContext = nullptr;
            current = false;
        }
    }
    if (!fGLContext || false == current) {
        SK_APP_LOG_ERROR("No valid EGL context (%p) or No current context\n", fGLContext);
        return nullptr;
    }

    glClearStencil(0);
    glClearColor(0, 0, 0, 0);
    glStencilMask(0xffffffff);
    glClear(GL_STENCIL_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    if (!eglQuerySurface(display, fGLsurface, EGL_WIDTH, &fWidth)
        || !eglQuerySurface(display, fGLsurface, EGL_HEIGHT, &fHeight))
    {
        glViewport(0, 0, fWidth, fHeight);
    }

    swapInterval();
    return interface ? interface : GrGLMakeNativeInterface();
}

GLWindowContextEGL::~GLWindowContextEGL() {
    this->destroyContext();
}

void GLWindowContextEGL::onDestroyContext() {
    if (fGLContext) {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        eglMakeCurrent(fPlatformDisplay.eglDisplay(), nullptr, nullptr, nullptr);
        eglDestroyContext(fPlatformDisplay.eglDisplay(), fGLContext);
        fGLContext = nullptr;
    }
    if (fGLsurface) {
        eglDestroySurface(fPlatformDisplay.eglDisplay(), fGLsurface);
        fGLsurface = nullptr;
    }
}

void GLWindowContextEGL::onSwapBuffers() {
    if (fGLContext && fGLsurface) {
        eglSwapBuffers(fPlatformDisplay.eglDisplay(), fGLsurface);
    }
}

void GLWindowContextEGL::swapInterval()
{
    SK_APP_NOT_IMPL // Something similar to GLX_EXT_swap_control in GLX implementation ?
}

}  // namespace sk_app

#endif // USE(EGL)
