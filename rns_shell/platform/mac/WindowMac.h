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

#include "ReactSkia/utils/RnsUtils.h"

#include "include/private/SkChecksum.h"
#include "src/core/SkTDynamicHash.h"

#include "PlatformDisplay.h"
#include "Window.h"

#include "ReactSkia/sdk/NotificationCenter.h"
#include "ReactSkia/sdk/RNSKeyCodeMapping.h"

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

 private:
  static std::map<NSInteger, WindowMac *> windowMap_;

  NSWindow *window_;
  PlatformDisplay *display_;
};

} // namespace RnsShell
