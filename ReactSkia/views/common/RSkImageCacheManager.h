/*
 * Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */
#pragma once

#include <better/map.h>
#include "include/core/SkImage.h"
#include "include/core/SkGraphics.h"
#include "include/gpu/GrDirectContext.h"

#include "ReactSkia/sdk/FollyTimer.h"

#define SKIA_CPU_IMAGE_CACHE_LIMIT  50*1024*1024 // 52,428,800 bytes
#define SKIA_GPU_IMAGE_CACHE_LIMIT  50*1024*1024 // 52,428,800 bytes

using namespace std;
using namespace rns::sdk;
namespace facebook {
namespace react {

typedef struct decodedimageCacheData {
  sk_sp<SkImage> imageData;
  double expiryTime;
}decodedimageCacheData;

typedef better::map <string,decodedimageCacheData> ImageCacheMap;

class RSkImageCacheManager {
 public:
  ~RSkImageCacheManager();
  static RSkImageCacheManager* getImageCacheManagerInstance();
  static void init();
  sk_sp<SkImage> findImageDataInCache(const char* path);
  bool imageDataInsertInCache(const char* path,decodedimageCacheData imageCacheData);
  bool clearMemory();
  bool clearDisk();
 private:
  static std::mutex mutex_;
  static RSkImageCacheManager *imageCacheManagerInstance_;
  ImageCacheMap imageCache_;
  double scheduleTimeExpiry_;
  RSkImageCacheManager();
  Timer * timer_{nullptr};
  void getCacheUsage(size_t usageArr[]);
  bool evictAsNeeded();
  void expiryTimeCallback();
#ifdef RNS_IMAGE_CACHE_USAGE_DEBUG
  void printCacheUsage();
#endif
};

} // namespace react
} // namespace facebook