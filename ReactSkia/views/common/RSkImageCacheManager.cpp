 /*
 * Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */
#include <string>
#include <map>
#include <iterator>

#include "include/core/SkData.h"

#include "ReactSkia/RSkSurfaceWindow.h"
#include "ReactSkia/views/common/RSkImageCacheManager.h"
#include "ReactSkia/utils/RnsLog.h"

using namespace std;

#define SKIA_CPU_IMAGE_CACHE_HWM_LIMIT  SKIA_CPU_IMAGE_CACHE_LIMIT *.95 //95% as High Water mark level
#define SKIA_GPU_IMAGE_CACHE_HWM_LIMIT  SKIA_CPU_IMAGE_CACHE_LIMIT *.95 //95% as High Water mark level
#define EVICT_COUNT  2 // Number of entries to be evicted on evictAsNeeded

namespace facebook {
namespace react {

namespace RSkImageCacheManager{
   
namespace {
  
  map<size_t, sk_sp<SkImage>> ImageCacheMap;
  size_t cpuCacheLimit_{0},gpuCacheLimit_{0};

  void setCpuImageCacheLimit(size_t cacheLimit) {
    SkGraphics::SetResourceCacheTotalByteLimit(cacheLimit);
    cpuCacheLimit_ = SkGraphics::GetResourceCacheTotalByteLimit();
  }

#ifdef RNS_SHELL_HAS_GPU_SUPPORT
  void setGpuImageCacheLimit(size_t cacheLimit) {
    GrDirectContext* gpuContext = RSkSurfaceWindow::getDirectContext();
    if(gpuContext) {
      gpuContext->setResourceCacheLimit(cacheLimit);  
      gpuCacheLimit_ = gpuContext->getResourceCacheLimit();
    }
  }
#endif

  sk_sp<SkImage> makeImageData(const char *path) {
    sk_sp<SkData> data = SkData::MakeFromFileName(path);
    if (!data) {
      RNS_LOG_ERROR("Unable to make SkData for path :" << path);
      return nullptr;
    }
    return( SkImage::MakeFromEncoded(data ));
  }
  
  bool evictAsNeeded() {

  	size_t cpuCacheUsed{0} , gpuCacheUsed{0};
    int fOldCount{0}, evictCount{0};
    map<size_t, sk_sp<SkImage>>::iterator it;

/*Read memory usage on CPU/GU Caches*/
  #ifdef RNS_SHELL_HAS_GPU_SUPPORT
    GrDirectContext* gpuContext =RSkSurfaceWindow::getDirectContext();
    if(gpuContext) {
      gpuContext->getResourceCacheUsage(&fOldCount, &gpuCacheUsed);       
    }
  #endif
    cpuCacheUsed = SkGraphics::GetResourceCacheTotalBytesUsed();
    RNS_LOG_DEBUG("CPU CACHE consumed bytes: "<<cpuCacheUsed<< ",, GPU CACHE consumed bytes: "<<gpuCacheUsed);

    /*Evict entry on reaching HWM. This logic to be enchanced based on growing need and clarity.*/
    for(it = ImageCacheMap.begin();(it != ImageCacheMap.end()) && (evictCount < EVICT_COUNT);) {
      if((cpuCacheUsed < SKIA_CPU_IMAGE_CACHE_HWM_LIMIT) && ( gpuCacheUsed < SKIA_GPU_IMAGE_CACHE_HWM_LIMIT))
        break;
      if((it->second)->unique()) {
          ImageCacheMap.erase(it++);
          evictCount++;
          RNS_LOG_DEBUG("Evicting Entry from RNS Image Hash Map ...");
      } else {
          ++it;
      }
    }
    return ((cpuCacheUsed < SKIA_CPU_IMAGE_CACHE_LIMIT) && ( gpuCacheUsed < SKIA_GPU_IMAGE_CACHE_LIMIT));
  }

}//namespace

sk_sp<SkImage> getImageData(const char *path) {

  hash<string> hashedKey; 
  sk_sp<SkImage> imageData{nullptr};
  /*Set Cache limit, if not set before*/
  if(cpuCacheLimit_ != SKIA_CPU_IMAGE_CACHE_LIMIT) {
    setCpuImageCacheLimit(SKIA_CPU_IMAGE_CACHE_LIMIT);
  }
#ifdef RNS_SHELL_HAS_GPU_SUPPORT
  if(gpuCacheLimit_ != SKIA_GPU_IMAGE_CACHE_LIMIT) {
    setGpuImageCacheLimit(SKIA_GPU_IMAGE_CACHE_LIMIT);
  }
#endif
  if(!path) {
    RNS_LOG_ERROR("Invalid File");
    return nullptr;
  }
 /*first check file entry in hash map, if entry not exist, create imageData*/
  map<size_t, sk_sp<SkImage>>::iterator it = ImageCacheMap.find(hashedKey(path));
  imageData= ((it != ImageCacheMap.end()) ? it->second : nullptr);
  if(!imageData) {
    imageData = makeImageData(path);
    /*Add entry to hash map only if the cache mem usage is with in the limit*/
    if(evictAsNeeded() && imageData) {
      ImageCacheMap.insert(pair<size_t, sk_sp<SkImage>>(hashedKey(path),imageData));
      RNS_LOG_DEBUG("New Entry in Map..."<< ImageCacheMap.size()<<" for file :"<<path);
    } else {
        RNS_LOG_DEBUG("Mem limit reached or Couldn't create imageData, file is not cached ...");
    }
  }
  return imageData;
}

#ifdef RNS_IMAGE_CACHE_USAGE_DEBUG
void printCacheUsage() {
  size_t cpuUsedMem{0},gpuUsedMem{0};
  static size_t prevCpuUsedMem{0},prevGpuUsedMem{0};
  int fOldCount{0};

  cpuUsedMem = SkGraphics::GetResourceCacheTotalBytesUsed(); 
#ifdef RNS_SHELL_HAS_GPU_SUPPORT
  GrDirectContext* gpuContext =RSkSurfaceWindow::getDirectContext();
  if(gpuContext) {
    gpuContext->getResourceCacheUsage(&fOldCount, &gpuUsedMem);
    RNS_LOG_INFO("Memory consumed for this run in GPU CACHE:"<<(gpuUsedMem - prevGpuUsedMem));
    prevGpuUsedMem = gpuUsedMem;
  }
#endif
  RNS_LOG_INFO("Memory consumed for this run in CPU CACHE :"<<(cpuUsedMem - prevCpuUsedMem));
  prevCpuUsedMem = cpuUsedMem;
}
#endif/*RNS_IMAGE_CACHE_USAGE_DEBUG*/

} //RSkImageCacheManager

} // namespace react
} // namespace facebook
