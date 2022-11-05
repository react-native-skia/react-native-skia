/*
 * Copyright 2016 Google Inc.
 * Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
 * Copyright (C) Kudo Chien.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "rns_shell/platform/mac/WindowDelegate.h"

#include "rns_shell/platform/mac/WindowMac.h"

@implementation WindowDelegate {
  RnsShell::WindowMac *window_;
}

- (WindowDelegate *)initWithWindow:(RnsShell::WindowMac *)initWindow
{
  self = [super init];
  if (self) {
    window_ = initWindow;
  }
  return self;
}

- (void)windowDidResize:(NSNotification *)notification
{
  NSView *view = window_->rootView();
  auto scaleFactor = window_->scaleFactor();
  window_->onResize(view.bounds.size.width * scaleFactor, view.bounds.size.height * scaleFactor);
  // window_->inval();
}

- (BOOL)windowShouldClose:(NSWindow *)sender
{
  window_->closeWindow();
  return FALSE;
}

@end
