/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#pragma once

#include "include/core/SkImageInfo.h"
#include "include/core/SkSurfaceProps.h"
#include "include/gpu/GrContextOptions.h"

namespace RnsShell {

struct DisplayParams {
    DisplayParams()
        : colorType_(kN32_SkColorType)
        , colorSpace_(nullptr)
        , msaaSampleCount_(1)
        , surfaceProps_(SkSurfaceProps::kLegacyFontHost_InitType)
        , disableVsync_(false)
    {}

    SkColorType         colorType_;
    sk_sp<SkColorSpace> colorSpace_;
    int                 msaaSampleCount_;
    GrContextOptions    grContextOptions_;
    SkSurfaceProps      surfaceProps_;
    bool                disableVsync_;
};

}   // namespace RnsShell
