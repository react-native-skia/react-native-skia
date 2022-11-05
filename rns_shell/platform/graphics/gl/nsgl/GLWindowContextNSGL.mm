/*
 * Copyright 2016 Google Inc.
 * Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
 * Copyright (C) Kudo Chien.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "rns_shell/platform/graphics/gl/nsgl/GLWindowContextNSGL.h"

#import <AppKit/AppKit.h>

#include "include/gpu/gl/GrGLInterface.h"
#include "rns_shell/common/Performance.h"
#include "rns_shell/platform/mac/PlatformDisplayMac.h"

namespace RnsShell {

// static
std::unique_ptr<WindowContext> GLWindowContextNSGL::createContext(
    GLNativeWindowType window,
    PlatformDisplay &platformDisplay,
    const DisplayParams &params)
{
  return std::make_unique<GLWindowContextNSGL>(window, platformDisplay, params);
}

GLWindowContextNSGL::GLWindowContextNSGL(
    GLNativeWindowType window,
    PlatformDisplay &platformDisplay,
    const DisplayParams &params)
    : GLWindowContext(params),
      display_(&reinterpret_cast<PlatformDisplayMac &>(platformDisplay)),
      mainView_((reinterpret_cast<NSWindow *>(window)).contentView)
{
  this->initializeContext();
}

sk_sp<const GrGLInterface> GLWindowContextNSGL::onInitializeContext()
{
  assert(mainView_);

  if (!glContext_) {
    // set up pixel format
    constexpr int kMaxAttributes = 19;
    NSOpenGLPixelFormatAttribute attributes[kMaxAttributes];
    int numAttributes = 0;
    attributes[numAttributes++] = NSOpenGLPFAAccelerated;
    attributes[numAttributes++] = NSOpenGLPFAClosestPolicy;
    attributes[numAttributes++] = NSOpenGLPFADoubleBuffer;
    attributes[numAttributes++] = NSOpenGLPFAOpenGLProfile;
    attributes[numAttributes++] = NSOpenGLProfileVersion3_2Core;
    attributes[numAttributes++] = NSOpenGLPFAColorSize;
    attributes[numAttributes++] = 24;
    attributes[numAttributes++] = NSOpenGLPFAAlphaSize;
    attributes[numAttributes++] = 8;
    attributes[numAttributes++] = NSOpenGLPFADepthSize;
    attributes[numAttributes++] = 0;
    attributes[numAttributes++] = NSOpenGLPFAStencilSize;
    attributes[numAttributes++] = 8;
    if (displayParams_.msaaSampleCount_ > 1) {
      attributes[numAttributes++] = NSOpenGLPFAMultisample;
      attributes[numAttributes++] = NSOpenGLPFASampleBuffers;
      attributes[numAttributes++] = 1;
      attributes[numAttributes++] = NSOpenGLPFASamples;
      attributes[numAttributes++] = displayParams_.msaaSampleCount_;
    } else {
      attributes[numAttributes++] = NSOpenGLPFASampleBuffers;
      attributes[numAttributes++] = 0;
    }
    attributes[numAttributes++] = 0;
    SkASSERT(numAttributes <= kMaxAttributes);

    pixelFormat_ = [[NSOpenGLPixelFormat alloc] initWithAttributes:attributes];
    if (pixelFormat_ == nil) {
      return nullptr;
    }

    // create context
    glContext_ = [[NSOpenGLContext alloc] initWithFormat:pixelFormat_ shareContext:nil];
    if (glContext_ == nil) {
      [pixelFormat_ release];
      pixelFormat_ = nil;
      return nullptr;
    }

    [mainView_ setWantsBestResolutionOpenGLSurface:YES];
    [glContext_ setView:mainView_];
  }

  GLint swapInterval = displayParams_.disableVsync_ ? 0 : 1;
  [glContext_ setValues:&swapInterval forParameter:NSOpenGLCPSwapInterval];

  // make context current
  [glContext_ makeCurrentContext];

  glClearStencil(0);
  glClearColor(0, 0, 0, 255);
  glStencilMask(0xffffffff);
  glClear(GL_STENCIL_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

  GLint stencilBits;
  [pixelFormat_ getValues:&stencilBits forAttribute:NSOpenGLPFAStencilSize forVirtualScreen:0];
  stencilBits_ = stencilBits;
  GLint sampleCount;
  [pixelFormat_ getValues:&sampleCount forAttribute:NSOpenGLPFASamples forVirtualScreen:0];
  sampleCount_ = sampleCount;
  sampleCount_ = std::max(sampleCount_, 1);

  auto backingScaleFactor = display_->scaleFactor();
  width_ = mainView_.bounds.size.width * backingScaleFactor;
  height_ = mainView_.bounds.size.height * backingScaleFactor;
  glViewport(0, 0, width_, height_);

  return GrGLMakeNativeInterface();
}

GLWindowContextNSGL::~GLWindowContextNSGL()
{
  teardownContext();
  this->destroyContext();
}

void GLWindowContextNSGL::onDestroyContext()
{
  // We only need to tear down the GLContext if we've changed the sample count.
  if (glContext_ && sampleCount_ != displayParams_.msaaSampleCount_) {
    teardownContext();
  }
}

bool GLWindowContextNSGL::makeContextCurrent()
{
  assert(glContext_ && mainView_);
  [glContext_ makeCurrentContext];
  return true;
}

void GLWindowContextNSGL::teardownContext()
{
  [NSOpenGLContext clearCurrentContext];
  [pixelFormat_ release];
  pixelFormat_ = nil;
  [glContext_ release];
  glContext_ = nil;
}

void GLWindowContextNSGL::onSwapBuffers(std::vector<SkIRect> &damage)
{
  RNS_UNUSED(damage);
  if (glContext_) {
#if !defined(GOOGLE_STRIP_LOG) || (GOOGLE_STRIP_LOG <= INFO)
    RNS_GET_TIME_STAMP_US(start);
#endif
    [glContext_ flushBuffer];
#if !defined(GOOGLE_STRIP_LOG) || (GOOGLE_STRIP_LOG <= INFO)
    RNS_GET_TIME_STAMP_US(end);
    Performance::takeSamples(end - start);
#endif
  }
}

int32_t GLWindowContextNSGL::getBufferAge()
{
  RNS_LOG_NOT_IMPL;
  return 0;
}

#if USE(RNS_SHELL_PARTIAL_UPDATES)

bool GLWindowContextNSGL::onHasSwapBuffersWithDamage()
{
  RNS_LOG_NOT_IMPL;
  return false;
}

bool GLWindowContextNSGL::onHasBufferCopy()
{
  RNS_LOG_NOT_IMPL;
  return false;
}

#endif // USE(RNS_SHELL_PARTIAL_UPDATES)

} // namespace RnsShell
