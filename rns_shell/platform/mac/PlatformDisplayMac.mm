/*
 * Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
 * Copyright (C) Kudo Chien.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "rns_shell/platform/mac/PlatformDisplayMac.h"

#import <AppKit/AppKit.h>

#include "ReactSkia/utils/RnsUtils.h"

namespace RnsShell {

std::unique_ptr<PlatformDisplay> PlatformDisplayMac::create()
{
  NSScreen *mainScreen = [NSScreen mainScreen];
  if (mainScreen == nil) {
    return nullptr;
  }

  return std::unique_ptr<PlatformDisplayMac>(new PlatformDisplayMac());
}

PlatformDisplayMac::PlatformDisplayMac() : PlatformDisplay(true) {}

PlatformDisplayMac::~PlatformDisplayMac() {}

SkSize PlatformDisplayMac::screenSize()
{
  NSScreen *mainScreen = [NSScreen mainScreen];
  return SkSize::Make(mainScreen.frame.size.width, mainScreen.frame.size.height);
}

float PlatformDisplayMac::scaleFactor()
{
  NSScreen *mainScreen = [NSScreen mainScreen];
  return mainScreen.backingScaleFactor;
}

} // namespace RnsShell
