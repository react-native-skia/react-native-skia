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

namespace RnsShell {

class WindowMac;

} // namespace RnsShell

@interface RootView : NSView

- (RootView *)initWithWindow:(RnsShell::WindowMac *)initWindow;

@end
