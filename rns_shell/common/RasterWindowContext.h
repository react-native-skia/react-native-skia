/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once
#include "rns_shell/common/WindowContext.h"

namespace RnsShell {

class RasterWindowContext : public WindowContext {
public:
    RasterWindowContext(const DisplayParams& params) : WindowContext(params) {}

protected:
    bool isGpuContext() override { return false; }
};

}   // namespace RnsShell
