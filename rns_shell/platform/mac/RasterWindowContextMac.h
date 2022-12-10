/*
 * Copyright 2016 Google Inc.
 * Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
 * Copyright (C) Kudo Chien.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

#include <OpenGL/gl.h>

#include "rns_shell/common/RasterWindowContext.h"
#include "rns_shell/platform/graphics/WindowContextFactory.h"

#include "include/core/SkSurface.h"

#ifdef __OBJC__
@class NSView;
@class NSOpenGLContext;
@class NSOpenGLPixelFormat;
#else
struct NSView;
struct NSOpenGLContext;
struct NSOpenGLPixelFormat;
#endif // __OBJC__

namespace RnsShell {

class PlatformDisplayMac;

class RasterWindowContextMac : public RasterWindowContext {
 public:
  static std::unique_ptr<WindowContext> createContext(
      GLNativeWindowType window,
      PlatformDisplay *platformDisplay,
      const DisplayParams &params);
  RasterWindowContextMac(
      GLNativeWindowType,
      PlatformDisplay *,
      const DisplayParams &);

  sk_sp<SkSurface> getBackbufferSurface() override;
  void swapBuffers(std::vector<SkIRect> &damage) override;
  bool makeContextCurrent() override {
    return true;
  }
#if USE(RNS_SHELL_PARTIAL_UPDATES)
  bool hasSwapBuffersWithDamage() override {
    return false;
  }
  bool hasBufferCopy() override;
#endif
  bool isValid() override {
    return SkToBool(mainView_);
  }
  void initializeContext();
  void setDisplayParams(const DisplayParams &params) override;

 protected:
  PlatformDisplayMac *display_;
  NSView *mainView_;
  NSOpenGLContext *glContext_;
  NSOpenGLPixelFormat *pixelFormat_;
  sk_sp<SkSurface> backbufferSurface_;
};

} // namespace RnsShell
