/*
 * Copyright 2016 Google Inc.
 * Copyright (C) 1994-2020 OpenTV, Inc. and Nagravision S.A.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/gl/GrGLInterface.h"

#include "gl/GLWindowContext.h"
#include "egl/GLWindowContextEGL.h"

#include "Performance.h"

#if USE(EGL)

using RnsShell::DisplayParams;
using RnsShell::GLWindowContext;

namespace RnsShell {

#if USE(OPENGL_ES)
static const char* gEGLAPIName = "OpenGL ES";
static const EGLenum gEGLAPIVersion = EGL_OPENGL_ES_API;
#else
static const char* gEGLAPIName = "OpenGL";
static const EGLenum gEGLAPIVersion = EGL_OPENGL_API;
#endif

#if defined(EGL_KHR_partial_update) && EGL_KHR_partial_update
static PFNEGLSETDAMAGEREGIONKHRPROC eglSetDamageRegion = nullptr;
#else
static void* eglSetDamageRegion = nullptr;
#endif

#if defined(EGL_EXT_swap_buffers_with_damage) && EGL_EXT_swap_buffers_with_damage
static PFNEGLSWAPBUFFERSWITHDAMAGEEXTPROC eglSwapBuffersWithDamage = nullptr;
#else
static void* eglSwapBuffersWithDamage = nullptr;
#endif

const char* GLWindowContextEGL::errorString(int statusCode) {
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

const char* GLWindowContextEGL::eglErrorString() {
    return errorString(eglGetError());
}

bool GLWindowContextEGL::getEGLConfig(EGLDisplay display, EGLConfig* config, EGLSurfaceType surfaceType) {
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
        RNS_LOG_ERROR("Cannot get count of available EGL configurations : " << eglErrorString());
        return false;
    }

    EGLint numConfigsReturned;
    EGLConfig eglConfig = 0;
    if (!eglChooseConfig(display, attributeList, &eglConfig, 1, &numConfigsReturned) || numConfigsReturned == 0) {
        RNS_LOG_ERROR("Cannot get available EGL configurations : " << eglErrorString());
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
            RNS_LOG_NOT_IMPL; // Walk through all the available eglConfig and select the matching values with rgbaSize / refer WPE code
            return false;
        }
    }
    return true;
}

bool GLWindowContextEGL::isExtensionSupported(const char* extensionList, const char* extension) {
    if (!extensionList)
        return false;

    RNS_LOG_ASSERT(extension, "Invalid Extension");
    int extensionLen = strlen(extension);
    const char* extensionListPtr = extensionList;
    while ((extensionListPtr = strstr(extensionListPtr, extension))) {
        if (extensionListPtr[extensionLen] == ' ' || extensionListPtr[extensionLen] == '\0')
            return true;
        extensionListPtr += extensionLen;
    }
    return false;
}

EGLContext GLWindowContextEGL::createContextForEGLVersion(PlatformDisplay& platformDisplay, EGLConfig config, EGLContext sharingContext) {
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

GLWindowContextEGL::GLWindowContextEGL(GLNativeWindowType window, EGLConfig config, PlatformDisplay& platformDisplay, const DisplayParams& params, EGLContext context, EGLSurface surface)
        : INHERITED(params)
        , window_(window)
        , platformDisplay_(platformDisplay)
        , glSurface_(surface)
        , glContext_(context) {

    EGLDisplay display = platformDisplay.eglDisplay();
    if (!getEGLConfig(display, &config, WindowSurface)) {
        RNS_LOG_ERROR("Cannot obtain EGL window context configuration : " << eglErrorString());
        return;
    }

    eglGetConfigAttrib(display, config, EGL_STENCIL_SIZE, &stencilBits_);
    eglGetConfigAttrib(display, config, EGL_SAMPLES, &sampleCount_);
    sampleCount_ = std::max(sampleCount_, 1);
    this->initializeContext();

#if !defined(GOOGLE_STRIP_LOG) || (GOOGLE_STRIP_LOG <= INFO)
    EGLint swapBehaviour = EGL_BUFFER_PRESERVED;
    eglQuerySurface(platformDisplay_.eglDisplay(), glSurface_, EGL_SWAP_BEHAVIOR, &swapBehaviour);
    RNS_LOG_DEBUG("GLWindowContextEGL constructed with WH(" << width_ << " x " << height_ << ") SampleCount & StencilBits : [" << sampleCount_ << "," << stencilBits_ <<
                  "], SWAP_BEHAVIOR : " << ((swapBehaviour==EGL_BUFFER_PRESERVED)?"EGL_BUFFER_PRESERVED":"EGL_BUFFER_DESTROYED"));
#endif
}

std::unique_ptr<GLWindowContextEGL> GLWindowContextEGL::createWindowContext(GLNativeWindowType window, PlatformDisplay& platformDisplay, const DisplayParams& params, EGLContext sharingContext) {
    EGLDisplay display = platformDisplay.eglDisplay();
    EGLConfig config;
    if (!getEGLConfig(display, &config, WindowSurface)) {
        RNS_LOG_ERROR("Cannot obtain EGL window context configuration : " << eglErrorString());
        return nullptr;
    }

    EGLContext context = createContextForEGLVersion(platformDisplay, config, sharingContext);
    if (context == EGL_NO_CONTEXT) {
        RNS_LOG_ERROR("Cannot create EGL window context : " << eglErrorString());
        return nullptr;
    }

    EGLSurface surface = EGL_NO_SURFACE;
    switch (platformDisplay.type()) {
#if PLATFORM(X11)
        case PlatformDisplay::Type::X11:
            surface = createWindowSurfaceX11(display, config, window);
            break;
#endif
#if PLATFORM(LIBWPE) || USE(WPE_RENDERER)
        case PlatformDisplay::Type::WPE:
            surface = createWindowSurfaceWPE(display, config, window);
            break;
#endif // USE(WPE_RENDERER)
        default:
            break;
    }

    if (surface == EGL_NO_SURFACE) {
#if defined(EGL_PLATFORM_BRCM) && EGL_PLATFORM_BRCM
        RNS_LOG_ERROR("Cannot create EGL window surface : " << eglErrorString() << " Retrying with fallback. window : " <<  window);
#else
        RNS_LOG_ERROR("Cannot create EGL window surface : " << eglErrorString() << " Retrying with fallback. window : " << reinterpret_cast<GLNativeWindowType>(window));
#endif
        surface = eglCreateWindowSurface(display, config, static_cast<EGLNativeWindowType>(window), nullptr);
    }

    if (surface == EGL_NO_SURFACE) {
        RNS_LOG_ERROR("Cannot create EGL window surface : " << eglErrorString());
        eglDestroyContext(display, context);
        return nullptr;
    }
    return std::unique_ptr<GLWindowContextEGL>(new GLWindowContextEGL(window, config, platformDisplay, params, context, surface));
}

std::unique_ptr<WindowContext> GLWindowContextEGL::createContext(GLNativeWindowType window, PlatformDisplay& platformDisplay, const DisplayParams& params) {
    if (platformDisplay.eglDisplay() == EGL_NO_DISPLAY) {
        RNS_LOG_ERROR("Cannot create EGL context: invalid display : " << eglErrorString());
        return nullptr;
    }

    if (eglBindAPI(gEGLAPIVersion) == EGL_FALSE) {
        RNS_LOG_ERROR("Cannot create EGL context: error binding " << gEGLAPIName << " API : " << eglErrorString());
        return nullptr;
    }

    EGLContext eglSharingContext = platformDisplay.sharingGLContext() ? platformDisplay.sharingGLContext() : createSharingContext(platformDisplay);
    auto context = window ? createWindowContext(window, platformDisplay, params, eglSharingContext) : nullptr;
    if (!context) {
        RNS_LOG_ERROR("Could not create EGL context");
        return nullptr;
    }
    return std::move(context);
}

EGLContext GLWindowContextEGL::createSharingContext(PlatformDisplay& platformDisplay) {
    EGLDisplay display = platformDisplay.eglDisplay();
    EGLConfig config;

    if (display == EGL_NO_DISPLAY) {
        RNS_LOG_ERROR("Cannot create EGL context: invalid display : " << eglErrorString());
        return EGL_NO_CONTEXT;
    }

    if (eglBindAPI(gEGLAPIVersion) == EGL_FALSE) {
        RNS_LOG_ERROR("Cannot create EGL context: error binding " << gEGLAPIName << " API : " << eglErrorString());
        return EGL_NO_CONTEXT;
    }

    const char* extensions = eglQueryString(display, EGL_EXTENSIONS);
    if (!isExtensionSupported(extensions, "EGL_KHR_surfaceless_context") && !isExtensionSupported(extensions, "EGL_KHR_surfaceless_opengl")) {
        RNS_LOG_ERROR("Cannot create surfaceless EGL context: required extensions missing");
        return EGL_NO_CONTEXT;
    }

    if (!getEGLConfig(display, &config, Surfaceless)) {
        RNS_LOG_ERROR("Cannot obtain EGL surfaceless configuration : " << eglErrorString());
        return EGL_NO_CONTEXT;
    }

    EGLContext context = createContextForEGLVersion(platformDisplay, config, nullptr);
    if (context == EGL_NO_CONTEXT) {
        RNS_LOG_ERROR("Cannot create EGL surfaceless context : " << eglErrorString());
        return EGL_NO_CONTEXT;
    }
    return context;
}

sk_sp<const GrGLInterface> GLWindowContextEGL::onInitializeContext() {
    RNS_LOG_ASSERT(glContext_, "Invalid GL Context");
    sk_sp<const GrGLInterface> interface;
    bool current = false;

    EGLDisplay display = platformDisplay_.eglDisplay();
    if(EGL_NO_DISPLAY == display) {
        RNS_LOG_ERROR("Cannot initialize EGL context: invalid display : " << eglErrorString());
        return nullptr;
    }

    if( glContext_ && true == makeContextCurrent()) {
        // Look to see if RenderDoc is attached. If so, re-create the context with a
        // core profile.
        interface = GrGLMakeNativeInterface();
        current = true;
        if (interface && interface->fExtensions.has("GL_EXT_debug_tool")) {
            RNS_LOG_ERROR("Extension has GL_EXT_debug_tool");
            interface.reset();
            eglMakeCurrent(display, nullptr, nullptr, nullptr);
            eglDestroyContext(display, glContext_);
            glContext_ = nullptr;
            current = false;
        }
    }
    if (!glContext_ || false == current) {
        RNS_LOG_ERROR("No valid EGL context "<< glContext_ << " or No current context");
        return nullptr;
    }

    glClearStencil(0);
    glClearColor(0, 0, 0, 0);
    glStencilMask(0xffffffff);
    glClear(GL_STENCIL_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    if (!eglQuerySurface(display, glSurface_, EGL_WIDTH, &width_)
        || !eglQuerySurface(display, glSurface_, EGL_HEIGHT, &height_)) {
        glViewport(0, 0, width_, height_);
    }

#if USE(RNS_SHELL_PARTIAL_UPDATES) &&  USE(RNS_SHELL_COPY_BUFFERS)
    eglInitializeOffscreenFrameBuffer();
#endif
    swapInterval();
    return interface ? interface : GrGLMakeNativeInterface();
}

GLWindowContextEGL::~GLWindowContextEGL() {
    this->destroyContext();
}

void GLWindowContextEGL::onDestroyContext() {

    if (glContext_) {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        eglMakeCurrent(platformDisplay_.eglDisplay(), nullptr, nullptr, nullptr);
        eglDestroyContext(platformDisplay_.eglDisplay(), glContext_);
        glContext_ = nullptr;
    }
    if (glSurface_) {
        eglDestroySurface(platformDisplay_.eglDisplay(), glSurface_);
        glSurface_ = nullptr;
    }
#if USE(RNS_SHELL_PARTIAL_UPDATES) &&  USE(RNS_SHELL_COPY_BUFFERS)
        eglDeleteOffscreenFrameBuffer();
#endif

#if USE(WPE_RENDERER)
    destroyWPETarget();
#endif

}

bool GLWindowContextEGL::makeContextCurrent() {
    bool res = false;
    if (eglGetCurrentContext() == glContext_)
        return true;

    res = eglMakeCurrent(platformDisplay_.eglDisplay(), glSurface_, glSurface_, glContext_);
    RNS_LOG_ERROR_IF((res == false), "Egl Make Current Error : " << eglErrorString());

    return res;
}

void GLWindowContextEGL::onSwapBuffers(std::vector<SkIRect> &damage) {
    if (glContext_ && glSurface_) {
#if !defined(GOOGLE_STRIP_LOG) || (GOOGLE_STRIP_LOG <= INFO)
        RNS_GET_TIME_STAMP_US(start);
#endif

#if USE(RNS_SHELL_PARTIAL_UPDATES)
        if(eglSwapBuffersWithDamage) {
            auto rects = rectsToInts(platformDisplay_.eglDisplay(), glSurface_, damage);
            eglSwapBuffersWithDamage(platformDisplay_.eglDisplay(), glSurface_, rects.data(), damage.size());
        } else { // Doesnt have swapBufferWithDamage extenstion support
#if USE(RNS_SHELL_COPY_BUFFERS) // Partial updates can still be supported with offscreen buffer drawing, if enabled
            eglBlitAndSwapBuffers();
#else
            eglSwapBuffers(platformDisplay_.eglDisplay(), glSurface_); // Partial update disabled
#endif //RNS_SHELL_COPY_BUFFERS
        }
#else
        RNS_UNUSED(damage);
        eglSwapBuffers(platformDisplay_.eglDisplay(), glSurface_);
#endif // RNS_SHELL_PARTIAL_UPDATES

#if !defined(GOOGLE_STRIP_LOG) || (GOOGLE_STRIP_LOG <= INFO)
        RNS_GET_TIME_STAMP_US(end);
        Performance::takeSamples(end - start);
#endif
    }
}

void GLWindowContextEGL::swapInterval() {
    EGLDisplay display = platformDisplay_.eglDisplay();
    const char* extensions = eglQueryString(display, EGL_EXTENSIONS);

    if (isExtensionSupported(extensions, "EGL_EXT_buffer_age")) {
        RNS_LOG_INFO("EGL_EXT_buffer_age extenstion supported....");

#if defined(EGL_KHR_partial_update) && EGL_KHR_partial_update
        if (isExtensionSupported(extensions, "EGL_KHR_partial_update")) {
            RNS_LOG_INFO("EGL_KHR_partial_update extenstion supported....");
            eglSetDamageRegion = reinterpret_cast<PFNEGLSETDAMAGEREGIONKHRPROC>(eglGetProcAddress("eglSetDamageRegionKHR"));
        }
#endif

#if defined(EGL_EXT_swap_buffers_with_damage) && EGL_EXT_swap_buffers_with_damage
        if (isExtensionSupported(extensions, "EGL_EXT_swap_buffers_with_damage")) {
            RNS_LOG_INFO("EGL_EXT_swap_buffers_with_damage extenstion supported....");
            eglSwapBuffersWithDamage =  reinterpret_cast<PFNEGLSWAPBUFFERSWITHDAMAGEEXTPROC>(eglGetProcAddress("eglSwapBuffersWithDamageEXT"));
        } else if (isExtensionSupported(extensions, "EGL_KHR_swap_buffers_with_damage")) {
            RNS_LOG_INFO("EGL_KHR_swap_buffers_with_damage extenstion supported....");
            eglSwapBuffersWithDamage =  reinterpret_cast<PFNEGLSWAPBUFFERSWITHDAMAGEEXTPROC>(eglGetProcAddress("eglSwapBuffersWithDamageKHR"));
        }
#endif
    }

    eglSwapInterval(platformDisplay_.eglDisplay(), displayParams_.disableVsync_ ? 0 : 1);
}

#if USE(RNS_SHELL_PARTIAL_UPDATES)
std::vector<EGLint> GLWindowContextEGL::rectsToInts(EGLDisplay display, EGLSurface surface, const std::vector<SkIRect>& rects) {
    std::vector<EGLint> res;
    EGLint height;

    eglQuerySurface(platformDisplay_.eglDisplay(), glSurface_, EGL_HEIGHT, &height);
    res.reserve(rects.size() * 4);

    for (const auto& r : rects) {
        res.push_back(r.left());
        res.push_back(height - r.bottom());
        res.push_back(r.width());
        res.push_back(r.height());
    }
    return res;
}

int32_t GLWindowContextEGL::getBufferAge() {
  EGLint bufferAge = 0;

  if(false == eglQuerySurface(platformDisplay_.eglDisplay(), glSurface_, EGL_BUFFER_AGE_EXT, &bufferAge)) {
    RNS_LOG_ERROR("Egl Query Surface(EGL_BUFFER_AGE_EXT) Error : " << eglErrorString());
  } else {
    RNS_LOG_INFO("Buffer Age of Current backBuffer of surface : " << bufferAge);
  }

  return bufferAge;
}

bool GLWindowContextEGL::onHasSwapBuffersWithDamage() {
    return !!eglSwapBuffersWithDamage;
}

bool GLWindowContextEGL::onHasBufferCopy() {
#if USE(RNS_SHELL_COPY_BUFFERS)
    return (offScreenFbo_ > 0);
#else
    return false;
#endif
}

#if USE(RNS_SHELL_COPY_BUFFERS)
void GLWindowContextEGL::eglBlitAndSwapBuffers() {
	static GLint viewport[4];

	glGetIntegerv(GL_VIEWPORT, viewport);

// Copy ReadBuffer from SourceFB and write it to DrawFB's DrawBuffer
#if USE(OPENGL_ES)
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // Default FB
	glBindFramebuffer(GL_READ_FRAMEBUFFER , offScreenFbo_);
	RNS_PROFILE_API_OFF("FB Blit ", glBlitFramebuffer(0, 0, viewport[2], viewport[3], 0, 0, viewport[2], viewport[3], (GL_COLOR_BUFFER_BIT ) , GL_NEAREST));
#else
	RNS_PROFILE_API_OFF("FB Blit ", glBlitNamedFramebuffer(offScreenFbo_, 0,
                                        0, 0, viewport[2], viewport[3],
                                        0, 0, viewport[2], viewport[3],
                                        (GL_COLOR_BUFFER_BIT ) , GL_NEAREST));
#endif

    eglSwapBuffers(platformDisplay_.eglDisplay(), glSurface_);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0); // Must keep default FB as readFrameBuffer

#if USE(OPENGL_ES)
    //Draw FrameBuffers
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, offScreenFbo_);
#endif
}

void GLWindowContextEGL::eglInitializeOffscreenFrameBuffer() {

    // Create FBO
    glGenFramebuffers(1, &offScreenFbo_);
    glBindFramebuffer(GL_FRAMEBUFFER, offScreenFbo_);

    // Create Texture for Color Buffer and attach it to FBO
    glGenTextures(1, &colorTexture_);
    glBindTexture(GL_TEXTURE_2D, colorTexture_);
    glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA, width_, height_, 0,GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture_, 0);

    // Create Texture for Depth and Stencil Buffer and attach it to FBO
    glGenTextures(1, &depthStencilTexture_);
    glBindTexture(GL_TEXTURE_2D, depthStencilTexture_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width_, height_, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depthStencilTexture_, 0);

    // Cleanup in case of error
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        eglDeleteOffscreenFrameBuffer();
    } else {
        glClearStencil(0);
        glClearColor(0, 0, 0, 0);
        glStencilMask(0xffffffff);
        glClear(GL_STENCIL_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    }
}

void GLWindowContextEGL::eglDeleteOffscreenFrameBuffer() {
    if(offScreenFbo_) {
        glDeleteFramebuffers(1, &offScreenFbo_);
        offScreenFbo_ = 0;
    }
    if(colorTexture_) {
        glDeleteTextures(1, &colorTexture_);
        colorTexture_ = 0;
    }
    if(depthStencilTexture_) {
        glDeleteTextures(1, &depthStencilTexture_);
        depthStencilTexture_ = 0;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0); // reset to default FB
}
#endif //RNS_SHELL_COPY_BUFFERS

#endif

}  // namespace RnsShell

#endif // USE(EGL)
