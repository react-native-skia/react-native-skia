/*
* Copyright 2016 Google Inc.
* Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/
#include "rns_shell/common/Application.h"

#include "include/core/SkGraphics.h"

namespace RnsShell {


Application::Application()
    : INHERITED(*this) {
  SkGraphics::Init();
  windowScreenChanged(layerTreeHost_->displayID());
}

void Application::windowScreenChanged(PlatformDisplayID displayId) {
  if(displayId == displayID_)
    return;
  displayID_ = displayId;
}

uint32_t Application::identifier() {
  RNS_LOG_TODO("Identify App with unique ID");
  return 7;
}

void Application::sizeChanged(int width, int height) {
  SkSize newSize = SkSize::Make(width, height);
  onResize(newSize);
  layerTreeHost_->sizeDidChange(newSize);
}

}   // namespace RnsShell
