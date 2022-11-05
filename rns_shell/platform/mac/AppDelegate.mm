/*
 * Copyright 2016 Google Inc.
 * Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
 * Copyright (C) Kudo Chien.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "rns_shell/platform/mac/AppDelegate.h"

@implementation AppDelegate : NSObject

@synthesize done = _done;

- (id)init
{
  self = [super init];
  return self;
}

- (void)applicationDidFinishLaunching:(NSNotification *)notification
{
}

@end
