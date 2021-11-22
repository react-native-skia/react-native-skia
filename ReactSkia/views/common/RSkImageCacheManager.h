/*
 * Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */
#pragma once

#include "include/core/SkImage.h"
#include "include/core/SkGraphics.h"
#include "include/gpu/GrDirectContext.h"

#define SKIA_CPU_IMAGE_CACHE_LIMIT  50*1024*1024 // 52,428,800 bytes
#define SKIA_GPU_IMAGE_CACHE_LIMIT  50*1024*1024 // 52,428,800 bytes

namespace facebook {
namespace react {

namespace RSkImageCacheManager{
  void configure();
  sk_sp<SkImage> getImageData(const char *path);
#ifdef RNS_IMAGE_CACHE_USAGE_DEBUG
  void printCacheUsage();
#endif

} //namespace RSkImageCacheManager

} // namespace react
} // namespace facebook



