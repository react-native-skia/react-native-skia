/*
 * Copyright 2016 Google Inc.
 * Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
 * Copyright (C) Kudo Chien.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "rns_shell/platform/mac/RasterWindowContextMac.h"

#import <AppKit/AppKit.h>

#include "include/core/SkCanvas.h"
#include "include/gpu/gl/GrGLInterface.h"

#include "rns_shell/common/Performance.h"
#include "rns_shell/platform/mac/PlatformDisplayMac.h"

using RnsShell::DisplayParams;
using RnsShell::RasterWindowContext;

namespace RnsShell {

std::unique_ptr<WindowContext> RasterWindowContextMac::createContext(
    GLNativeWindowType window,
    PlatformDisplay *platformDisplay,
    const DisplayParams &params)
{
  return std::unique_ptr<RasterWindowContextMac>(new RasterWindowContextMac(window, platformDisplay, params));
}

RasterWindowContextMac::RasterWindowContextMac(
    GLNativeWindowType window,
    PlatformDisplay *platformDisplay,
    const DisplayParams &params)
    : RasterWindowContext(params),
      display_(reinterpret_cast<PlatformDisplayMac *>(platformDisplay)),
      mainView_((reinterpret_cast<NSWindow *>(window)).contentView)
{
  initializeContext();
}

void RasterWindowContextMac::setDisplayParams(const DisplayParams &params)
{
  displayParams_ = params;
  RNS_LOG_NOT_IMPL;
}

void RasterWindowContextMac::initializeContext()
{
  assert(mainView_);

  if (!glContext_) {
    // set up pixel format
    constexpr int kMaxAttributes = 18;
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
      return;
    }

    // create context
    glContext_ = [[NSOpenGLContext alloc] initWithFormat:pixelFormat_ shareContext:nil];
    if (glContext_ == nil) {
      [pixelFormat_ release];
      pixelFormat_ = nil;
      return;
    }

    [mainView_ setWantsBestResolutionOpenGLSurface:YES];
    [glContext_ setView:mainView_];

    GLint swapInterval = displayParams_.disableVsync_ ? 0 : 1;
    [glContext_ setValues:&swapInterval forParameter:NSOpenGLCPSwapInterval];
  }

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

  // make the offscreen image
  SkImageInfo info =
      SkImageInfo::Make(width_, height_, displayParams_.colorType_, kPremul_SkAlphaType, displayParams_.colorSpace_);
  backbufferSurface_ = SkSurface::MakeRaster(info);
}

sk_sp<SkSurface> RasterWindowContextMac::getBackbufferSurface()
{
  return backbufferSurface_;
}

void RasterWindowContextMac::swapBuffers(std::vector<SkIRect> &damage)
{
  if (backbufferSurface_) {
    // We made/have an off-screen surface. Get the contents as an SkImage:
    sk_sp<SkImage> snapshot = backbufferSurface_->makeImageSnapshot();

    sk_sp<SkSurface> gpuSurface = getBackbufferSurface();
    SkCanvas *gpuCanvas = gpuSurface->getCanvas();
    gpuCanvas->drawImage(snapshot, 0, 0);
    gpuCanvas->flush();

    [glContext_ flushBuffer];
  }
}

#if USE(RNS_SHELL_PARTIAL_UPDATES)
bool RasterWindowContextMac::hasBufferCopy()
{
  // With current implementation We are using offscreen bitmap to draw and then copying this bitmap to window.
  // This means both bitmap and window has same data after frame display ( swapbuffer )
  return true;
}
#endif

} // namespace RnsShell
