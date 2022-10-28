/*
 * Copyright 2016 Google Inc.
 * Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

#include <memory>

#include "ReactSkia/utils/RnsUtils.h"

#if PLATFORM(X11)
#include <X11/Xlib.h>
#endif

#if USE(EGL)
#include <EGL/eglplatform.h>
#elif USE(GLX)
#include <GL/glx.h>
#endif

// webgpu_cpp.h and X.h don't get along. Include this first, before X11 defines None, Success etc.
#ifdef SK_DAWN
#include "dawn/webgpu_cpp.h"
#endif

#include "ReactSkia/utils/RnsLog.h"
#include "PlatformDisplay.h"
#include "WindowContext.h"
#if PLATFORM(X11)
#include "x11/PlatformDisplayX11.h"
#endif

namespace RnsShell {

class WindowContext;
struct DisplayParams;

namespace WCF {

std::unique_ptr<WindowContext> createContextForWindow(GLNativeWindowType, PlatformDisplay*, const DisplayParams&);

}  // namespace WCF

}  // namespace RnsShell
