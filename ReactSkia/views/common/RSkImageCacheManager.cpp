 /*
 * Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */
#include <better/map.h>

#include "include/core/SkData.h"

#include "ReactSkia/RSkSurfaceWindow.h"
#include "ReactSkia/views/common/RSkImageCacheManager.h"
#include "ReactSkia/utils/RnsLog.h"

using namespace std;

#define SKIA_CPU_IMAGE_CACHE_HWM_LIMIT  SKIA_CPU_IMAGE_CACHE_LIMIT *.95 //95% as High Water mark level
#define SKIA_GPU_IMAGE_CACHE_HWM_LIMIT  SKIA_CPU_IMAGE_CACHE_LIMIT *.95 //95% as High Water mark level
#define EVICT_COUNT  2 // Max No. of entries to be evicted in single run

namespace facebook {
namespace react {

namespace RSkImageCacheManager{
   
namespace {

  typedef better::map <string, sk_sp<SkImage>> ImageCacheMap;
  ImageCacheMap imageCache;
  #define CPU_MEM_ARR_INDEX 0
  #define GPU_MEM_ARR_INDEX 1
  std::mutex mutexLock;

  inline sk_sp<SkImage> makeImageData(const char *path) {
    sk_sp<SkData> data = SkData::MakeFromFileName(path);
    if (!data) {
      RNS_LOG_ERROR("Unable to make SkData for path :" << path);
      return nullptr;
    }
    return( SkImage::MakeFromEncoded(data ));
  }
  
  inline void getCacheUsage(size_t usageArr[]) {
    int fOldCount{0};
    usageArr[CPU_MEM_ARR_INDEX] = SkGraphics::GetResourceCacheTotalBytesUsed();
  #ifdef RNS_SHELL_HAS_GPU_SUPPORT
    GrDirectContext* gpuContext =RSkSurfaceWindow::getDirectContext();
    if(gpuContext)
      gpuContext->getResourceCacheUsage(&fOldCount, &usageArr[GPU_MEM_ARR_INDEX]);
    else
      usageArr[GPU_MEM_ARR_INDEX]=0;
  #endif
    RNS_LOG_DEBUG("CPU CACHE consumed bytes: "<<usageArr[CPU_MEM_ARR_INDEX]<< ", GPU CACHE consumed bytes: "<<usageArr[GPU_MEM_ARR_INDEX]);
  }

  bool evictAsNeeded() {

    int evictCount{0};
    size_t usageArr[2]={0,0};

    getCacheUsage(usageArr);
    if ((usageArr[CPU_MEM_ARR_INDEX] < SKIA_CPU_IMAGE_CACHE_HWM_LIMIT) && ( usageArr[GPU_MEM_ARR_INDEX] < SKIA_GPU_IMAGE_CACHE_HWM_LIMIT))
      return true;

    ImageCacheMap::iterator it=imageCache.begin();
    while( it != imageCache.end()) {
      if( evictCount >= EVICT_COUNT)
        break;
      if((it->second)->unique()) {
          it=imageCache.erase(it);
          evictCount++;
      } else {
          ++it;
      }
    }
    /*As eviction from Skia's cache & RNS cache system  were asynchronous,Ensuring cache memory drained below the Limit is not feasible at this point.
      So just let to add further, if eviction occured at RNS Level */
    return (evictCount == EVICT_COUNT);
  }

}//namespace

void configure() {
  SkGraphics::SetResourceCacheTotalByteLimit(SKIA_CPU_IMAGE_CACHE_LIMIT);
#ifdef RNS_SHELL_HAS_GPU_SUPPORT
  GrDirectContext* gpuContext = RSkSurfaceWindow::getDirectContext();
    if(gpuContext)
      gpuContext->setResourceCacheLimit(SKIA_GPU_IMAGE_CACHE_LIMIT);
#endif /*RNS_SHELL_HAS_GPU_SUPPORT*/
}

sk_sp<SkImage> getImageData(const char *path) {

  sk_sp<SkImage> imageData{nullptr};

  if(!path) {
    RNS_LOG_ERROR("Invalid File");
    return nullptr;
  }

  std::scoped_lock lock(mutexLock);

 /*First to check file entry presence . If not exist, generate imageData*/
  ImageCacheMap::iterator it = imageCache.find(path);
  imageData= ((it != imageCache.end()) ? it->second : nullptr);
  if(!imageData) {
    imageData = makeImageData(path);
    /*Add entry to hash map only if the cache mem usage is with in the limit*/
    /* TO DO: Eviction mechanism to be improved by decouple it from getImageData,
              through worker thread ....*/
    if(evictAsNeeded() && imageData) {
      imageCache.insert(pair<string, sk_sp<SkImage>>(path,imageData));
      RNS_LOG_DEBUG("New Entry in Map..."<<" file :"<<path);
    } else {
        RNS_LOG_DEBUG("Cache Memory limit reached or Couldn't create imageData, So file not cached ...");
    }

  }
  return imageData;
}

#ifdef RNS_IMAGE_CACHE_USAGE_DEBUG
void printCacheUsage() {
  static size_t prevCpuUsedMem{0},prevGpuUsedMem{0};
  size_t usageArr[2]={0,0};

  getCacheUsage(usageArr);
  RNS_LOG_INFO("Memory consumed for this run in CPU CACHE :"<<(usageArr[CPU_MEM_ARR_INDEX] - prevCpuUsedMem));
  prevCpuUsedMem = usageArr[CPU_MEM_ARR_INDEX];
#ifdef RNS_SHELL_HAS_GPU_SUPPORT
    RNS_LOG_INFO("Memory consumed for this run in GPU CACHE:"<<(usageArr[GPU_MEM_ARR_INDEX] - prevGpuUsedMem));
    prevGpuUsedMem = usageArr[GPU_MEM_ARR_INDEX];
#endif
}
#endif/*RNS_IMAGE_CACHE_USAGE_DEBUG*/

} //RSkImageCacheManager

} // namespace react
} // namespace facebook
