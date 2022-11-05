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

#include "rns_shell/platform/graphics/PlatformDisplay.h"
#include "rns_shell/platform/graphics/WindowContextFactory.h"
#include "rns_shell/platform/graphics/gl/GLWindowContext.h"

#include "include/gpu/gl/GrGLInterface.h"

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

class GLWindowContextNSGL : public GLWindowContext {
 public:
  static std::unique_ptr<WindowContext> createContext(
      GLNativeWindowType window,
      PlatformDisplay &platformDisplay,
      const DisplayParams &params);

  GLWindowContextNSGL(
      GLNativeWindowType,
      PlatformDisplay &,
      const DisplayParams &);
  ~GLWindowContextNSGL() override;

 protected:
  sk_sp<const GrGLInterface> onInitializeContext() override;
  void onDestroyContext() override;
  void onSwapBuffers(std::vector<SkIRect> &damage) override;
  int32_t getBufferAge() override;

#if USE(RNS_SHELL_PARTIAL_UPDATES)
  bool onHasSwapBuffersWithDamage() override;
  bool onHasBufferCopy() override;
#endif

 private:
  bool makeContextCurrent() override;
  void teardownContext();

 private:
  PlatformDisplayMac *display_;
  NSView *mainView_;
  NSOpenGLContext *glContext_;
  NSOpenGLPixelFormat *pixelFormat_;
};

} // namespace RnsShell
