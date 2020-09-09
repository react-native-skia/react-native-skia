
/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef WindowContextFactory_mac_DEFINED
#define WindowContextFactory_mac_DEFINED

#include "WindowContext.h"

#include <Cocoa/Cocoa.h>

#include <memory>

namespace sk_app {

struct DisplayParams;

namespace window_context_factory {

struct MacWindowInfo {
    NSView*   fMainView;
};

std::unique_ptr<WindowContext> MakeGLForMac(const MacWindowInfo&, const DisplayParams&);

std::unique_ptr<WindowContext> MakeRasterForMac(const MacWindowInfo&, const DisplayParams&);

}  // namespace window_context_factory

}  // namespace sk_app

#endif
