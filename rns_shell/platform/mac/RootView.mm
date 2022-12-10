/*
 * Copyright 2016 Google Inc.
 * Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
 * Copyright (C) Kudo Chien.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "rns_shell/platform/mac/RootView.h"

#include "rns_shell/platform/mac/WindowMac.h"

@implementation RootView {
  RnsShell::WindowMac *window_;
  // A TrackingArea prevents us from capturing events outside the view
  NSTrackingArea *trackingArea_;
}

- (RootView *)initWithWindow:(RnsShell::WindowMac *)initWindow
{
  self = [super init];
  if (self) {
    window_ = initWindow;
    [self updateTrackingAreas];
  }
  return self;
}

- (void)dealloc
{
  [trackingArea_ release];
  [super dealloc];
}

- (BOOL)isOpaque
{
  return YES;
}

- (BOOL)canBecomeKeyView
{
  return YES;
}

- (BOOL)acceptsFirstResponder
{
  return YES;
}

- (void)updateTrackingAreas
{
  if (trackingArea_ != nil) {
    [self removeTrackingArea:trackingArea_];
    [trackingArea_ release];
  }

  const NSTrackingAreaOptions options = NSTrackingMouseEnteredAndExited | NSTrackingActiveInKeyWindow |
      NSTrackingEnabledDuringMouseDrag | NSTrackingCursorUpdate | NSTrackingInVisibleRect | NSTrackingAssumeInside;

  trackingArea_ = [[NSTrackingArea alloc] initWithRect:[self bounds] options:options owner:self userInfo:nil];

  [self addTrackingArea:trackingArea_];
  [super updateTrackingAreas];
}

#if 0
- (void)mouseDown:(NSEvent *)event {
  [super mouseDown:event];
  auto backingScaleFactor = window_->scaleFactor();

  // skui::ModifierKey modifiers = [self updateModifierKeys:event];

  const NSPoint pos = [event locationInWindow];
  const NSRect rect = [self frame];
  LOG(INFO) << "mouseDown x=" << pos.x * backingScaleFactor << ", y=" << (rect.size.height - pos.y) *
  backingScaleFactor;
  // fWindow->onMouse(pos.x * backingScaleFactor, (rect.size.height - pos.y) * backingScaleFactor,
  //                  skui::InputState::kDown, modifiers);
}

- (void)mouseUp:(NSEvent *)event {
  [super mouseUp:event];
  auto backingScaleFactor = window_->scaleFactor();

  // skui::ModifierKey modifiers = [self updateModifierKeys:event];

  const NSPoint pos = [event locationInWindow];
  const NSRect rect = [self frame];
  LOG(INFO) << "mouseUp x=" << pos.x * backingScaleFactor << ", y=" << (rect.size.height - pos.y) *
  backingScaleFactor;
  // fWindow->onMouse(pos.x * backingScaleFactor, (rect.size.height - pos.y) * backingScaleFactor,
  //                  skui::InputState::kUp, modifiers);
}

- (void)mouseDragged:(NSEvent *)event {
  [super mouseDragged:event];
  // [self updateModifierKeys:event];
  [self mouseMoved:event];
}

- (void)mouseMoved:(NSEvent *)event {
  [super mouseMoved:event];
  auto backingScaleFactor = window_->scaleFactor();

  // skui::ModifierKey modifiers = [self updateModifierKeys:event];

  const NSPoint pos = [event locationInWindow];
  const NSRect rect = [self frame];
  LOG(INFO) << "mouseMove x=" << pos.x * backingScaleFactor << ", y=" << (rect.size.height - pos.y) *
  backingScaleFactor;
  // fWindow->onMouse(pos.x * backingScaleFactor, (rect.size.height - pos.y) * backingScaleFactor,
  //                  skui::InputState::kMove, modifiers);
}

- (void)scrollWheel:(NSEvent *)event {
  [super scrollWheel:event];
  // skui::ModifierKey modifiers = [self updateModifierKeys:event];

  LOG(INFO) << "mouseWheel scrollingDeltaY=" << [event scrollingDeltaY];
  // TODO: support hasPreciseScrollingDeltas?
  // fWindow->onMouseWheel([event scrollingDeltaY], modifiers);
}
#endif

@end
