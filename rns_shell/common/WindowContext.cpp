/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifdef RNS_SHELL_HAS_GPU_SUPPORT
#include "include/gpu/GrDirectContext.h"
#include "gl/GLWindowContext.h"
#endif
#include "WindowContext.h"


namespace RnsShell {

WindowContext::WindowContext(const DisplayParams& params)
        : displayParams_(params)
        , sampleCount_(1)
        , stencilBits_(0) {}

WindowContext::~WindowContext() {}

}   //namespace RnsShell
