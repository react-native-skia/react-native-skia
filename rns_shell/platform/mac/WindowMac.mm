/*
 * Copyright 2016 Google Inc.
 * Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
 * Copyright (C) Kudo Chien.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "rns_shell/platform/mac/WindowMac.h"

#include "src/utils/SkUTF.h"

#ifdef RNS_SHELL_HAS_GPU_SUPPORT
#include "rns_shell/platform/graphics/gl/GLWindowContext.h"
#endif
#include "rns_shell/platform/graphics/WindowContextFactory.h"

#include "rns_shell/platform/mac/PlatformDisplayMac.h"
#include "rns_shell/platform/mac/RootView.h"
#include "rns_shell/platform/mac/TaskLoop.h"
#include "rns_shell/platform/mac/WindowDelegate.h"

namespace RnsShell {

// static
std::map<NSInteger, WindowMac *> WindowMac::windowMap_;

// static
Window *Window::mainWindow_;

// static
Window *Window::createNativeWindow(void *platformData, SkSize dimension, WindowType type)
{
  PlatformDisplay *pDisplay = (PlatformDisplay *)platformData;

  RNS_LOG_ASSERT(pDisplay, "Invalid Platform Display");

  WindowMac *window = new WindowMac();
  if (!window->initWindow(pDisplay)) {
    delete window;
    return nullptr;
  }
  if (!mainWindow_) {
    mainWindow_ = window;
  }
  return window;
}

// static
void Window::createEventLoop(Application *app)
{
  // TODO:
  // TaskLoop::main().stop();
}

WindowMac::WindowMac() : Window() {}

WindowMac::~WindowMac()
{
  if (this == mainWindow_) {
    mainWindow_ = nullptr;
    this->closeWindow();
  }
}

bool WindowMac::initWindow(PlatformDisplay *platformDisplay)
{
  // we already have a window
  if (window_) {
    return true;
  }

  display_ = reinterpret_cast<PlatformDisplayMac *>(platformDisplay);

  // Create a delegate to track certain events
  WindowDelegate *delegate = [[WindowDelegate alloc] initWithWindow:this];
  if (delegate == nil) {
    return false;
  }

  // Create window
  constexpr int initialWidth = 1280;
  constexpr int initialHeight = 960;
  NSRect windowRect = NSMakeRect(100, 100, initialWidth, initialHeight);

  NSUInteger windowStyle =
      (NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable |
       NSWindowStyleMaskMiniaturizable);

  window_ = [[NSWindow alloc] initWithContentRect:windowRect
                                        styleMask:windowStyle
                                          backing:NSBackingStoreBuffered
                                            defer:NO];
  if (window_ == nil) {
    [delegate release];
    return false;
  }

  // create view
  RootView *view = [[RootView alloc] initWithWindow:this];
  if (view == nil) {
    [window_ release];
    [delegate release];
    return false;
  }

  [window_ setContentView:view];
  [window_ makeFirstResponder:view];
  [window_ setDelegate:delegate];
  [window_ setAcceptsMouseMovedEvents:YES];
  [window_ setRestorable:NO];

  // Should be retained by window now
  [view release];

  windowMap_.insert({window_.windowNumber, this});

  SkString title("React Native Skia : ");
  title.append("OpenGL");
  setTitle(title.c_str());
  show();

  return true;
}

void WindowMac::setTitle(const char *title)
{
  if (NSString *titleStr = [NSString stringWithUTF8String:title]) {
    [window_ setTitle:titleStr];
  }
}

void WindowMac::show()
{
  [window_ orderFront:nil];

  [NSApp activateIgnoringOtherApps:YES];
  [window_ makeKeyAndOrderFront:NSApp];
}

void WindowMac::closeWindow()
{
  if (window_ != nil) {
    auto it = windowMap_.find(window_.windowNumber);
    if (it != windowMap_.end()) {
      windowMap_.erase(it);
    }
    if (windowMap_.size() < 1) {
      [NSApp terminate:window_];
    }
    [window_ close];
    window_ = nil;
  }
}

SkSize WindowMac::getWindowSize()
{
  return SkSize::Make(window_.frame.size.width, window_.frame.size.height);
}

float WindowMac::scaleFactor()
{
  return display_->scaleFactor();
}

void WindowMac::setRequestedDisplayParams(const DisplayParams &params, bool allowReattach)
{
  RNS_LOG_NOT_IMPL;
  // INHERITED::setRequestedDisplayParams(params, allowReattach);
}

// void WindowMac::onKey(rnsKey eventKeyType, rnsKeyAction eventKeyAction){
//     NotificationCenter::defaultCenter().emit("onHWKeyEvent", eventKeyType, eventKeyAction);
//     return;
// }

NSView *WindowMac::rootView()
{
  return window_.contentView;
}

} // namespace RnsShell
