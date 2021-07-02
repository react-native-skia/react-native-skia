/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "src/core/SkMathPriv.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/gl/GrGLDefines.h"
#include "src/gpu/gl/GrGLUtil.h"
#include "src/image/SkImage_Base.h"

#include "GLWindowContext.h"

namespace RnsShell {

GLWindowContext::GLWindowContext(const DisplayParams& params)
        : WindowContext(params)
        , backendContext_(nullptr)
        , surface_(nullptr) {
    displayParams_.msaaSampleCount_ = GrNextPow2(displayParams_.msaaSampleCount_);
}

void GLWindowContext::initializeContext() {
    SkASSERT(!context_);
    backendContext_ = this->onInitializeContext();

    context_ = GrDirectContext::MakeGL(backendContext_, displayParams_.grContextOptions_);
    if (!context_ && displayParams_.msaaSampleCount_ > 1) {
        displayParams_.msaaSampleCount_ /= 2;
        this->initializeContext();
        return;
    }
}

void GLWindowContext::destroyContext() {
    surface_.reset(nullptr);

    if (context_) {
        // in case we have outstanding refs to this (lua?)
        context_->abandonContext();
        context_.reset();
    }
    backendContext_.reset(nullptr);

    this->onDestroyContext();
}

sk_sp<SkSurface> GLWindowContext::getBackbufferSurface() {
    if (nullptr == surface_) {
        if (context_) {
            GrGLint buffer;
            GR_GL_CALL(backendContext_.get(), GetIntegerv(GR_GL_FRAMEBUFFER_BINDING, &buffer));

            GrGLFramebufferInfo fbInfo;
            fbInfo.fFBOID = buffer;
            fbInfo.fFormat = GR_GL_RGBA8;

            RNS_LOG_INFO("Create backbuffer surface(" << width_ << "x" << height_ <<
                         "), SampleCount & StencilBits(" << sampleCount_ << "," << stencilBits_ << ")");
            GrBackendRenderTarget backendRT(width_,
                                            height_,
                                            sampleCount_,
                                            stencilBits_,
                                            fbInfo);

            surface_ = SkSurface::MakeFromBackendRenderTarget(context_.get(), backendRT,
                                                              kBottomLeft_GrSurfaceOrigin,
                                                              kRGBA_8888_SkColorType,
                                                              displayParams_.colorSpace_,
                                                              &displayParams_.surfaceProps_);
        }
    }

    return surface_;
}

<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> Munez graphics (#20)
void GLWindowContext::swapBuffers(std::vector<SkIRect> &damage) {
    this->onSwapBuffers(damage);
}

<<<<<<< HEAD
#if USE(RNS_SHELL_PARTIAL_UPDATES)
bool GLWindowContext::hasSwapBuffersWithDamage() {
    return this->onHasSwapBuffersWithDamage();
}

bool GLWindowContext::hasBufferCopy() {
    return this->onHasBufferCopy();
}
#endif

=======
void GLWindowContext::swapBuffers() {
    this->onSwapBuffers();
=======
bool GLWindowContext::hasSwapBuffersWithDamage() {
    return this->onHasSwapBuffersWithDamage();
>>>>>>> Munez graphics (#20)
}

>>>>>>> RNS Shell Implementation  (#8)
void GLWindowContext::setDisplayParams(const DisplayParams& params) {
    displayParams_ = params;
    this->destroyContext();
    this->initializeContext();
}

}   //namespace RnsShell
