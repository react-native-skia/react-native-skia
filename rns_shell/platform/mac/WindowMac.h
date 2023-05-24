/*
 * Copyright 2016 Google Inc.
 * Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
 * Copyright (C) Kudo Chien.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

#import <AppKit/AppKit.h>
#include <map>

#include "include/private/SkChecksum.h"
#include "src/core/SkTDynamicHash.h"

#include "rns_shell/common/Window.h"
#include "rns_shell/input/InputEventDelegate.h"
#include "rns_shell/platform/graphics/PlatformDisplay.h"
#include "ReactSkia/sdk/NotificationCenter.h"
#include "ReactSkia/sdk/RNSKeyCodeMapping.h"
#include "ReactSkia/utils/RnsUtils.h"

namespace RnsShell {

class PlatformDisplay;

class WindowMac : public Window {
 public:
  WindowMac();
  ~WindowMac() override;

  bool initWindow(PlatformDisplay *display);

  void setTitle(const char *) override;
  void show() override;
  void closeWindow() override;
  uint64_t nativeWindowHandle() override
  {
    return reinterpret_cast<uint64_t>(window_);
  }
  SkSize getWindowSize() override;
  float scaleFactor();

  void setRequestedDisplayParams(const DisplayParams &, bool allowReattach) override;

  NSView *rootView();

  void BindInputEventDelegate(InputEventDelegate *delegate) override {
    inputEventDelegate_ = delegate;
  }
  InputEventDelegate *GetInputEventDelegate() {
    return inputEventDelegate_;
  }

 private:
  static std::map<NSInteger, WindowMac *> windowMap_;

  NSWindow *window_;
  PlatformDisplay *display_;
  InputEventDelegate *inputEventDelegate_;
};

} // namespace RnsShell
