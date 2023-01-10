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
#include "rns_shell/common/WindowContext.h"
using namespace std;

#define SKIA_CPU_IMAGE_CACHE_HWM_LIMIT  SKIA_CPU_IMAGE_CACHE_LIMIT *.95 //95% as High Water mark level
#define SKIA_GPU_IMAGE_CACHE_HWM_LIMIT  SKIA_CPU_IMAGE_CACHE_LIMIT *.95 //95% as High Water mark level
#define EVICT_COUNT  2 // Max No. of entries to be evicted in single run

#ifdef RNS_SHELL_HAS_GPU_SUPPORT
std::mutex RnsShell::WindowContext::grTransactionMutex_;
#endif
namespace facebook {
namespace react {

#define CPU_MEM_ARR_INDEX 0
#define GPU_MEM_ARR_INDEX 1
std::mutex imageCacheLock;
std::mutex RSkImageCacheManager::mutex_;

RSkImageCacheManager* RSkImageCacheManager::imageCacheManagerInstance_{nullptr};

RSkImageCacheManager::RSkImageCacheManager() { };
RSkImageCacheManager::~RSkImageCacheManager() {
  std::lock_guard<std::mutex> lock(mutex_);
  if(this == imageCacheManagerInstance_)
    imageCacheManagerInstance_ = nullptr;
};

RSkImageCacheManager* RSkImageCacheManager::getImageCacheManagerInstance() {
  std::lock_guard<std::mutex> lock(mutex_);
  if (!imageCacheManagerInstance_) {
    imageCacheManagerInstance_ = new RSkImageCacheManager();
  }
  return imageCacheManagerInstance_;
}

void RSkImageCacheManager::getCacheUsage(size_t usageArr[]) {
  int fOldCount{0};
  usageArr[CPU_MEM_ARR_INDEX] = SkGraphics::GetResourceCacheTotalBytesUsed();
#ifdef RNS_SHELL_HAS_GPU_SUPPORT
  RnsShell::WindowContext::grTransactionBegin();
  GrDirectContext* gpuContext =RSkSurfaceWindow::getDirectContext();
  if(gpuContext)
    gpuContext->getResourceCacheUsage(&fOldCount, &usageArr[GPU_MEM_ARR_INDEX]);
  else
    usageArr[GPU_MEM_ARR_INDEX]=0;
  RnsShell::WindowContext::grTransactionEnd();
#endif
  RNS_LOG_DEBUG("CPU CACHE consumed bytes: "<<usageArr[CPU_MEM_ARR_INDEX]<< ", GPU CACHE consumed bytes: "<<usageArr[GPU_MEM_ARR_INDEX]);
}

bool RSkImageCacheManager::evictAsNeeded() {
  int evictCount{0};
  size_t usageArr[2]={0,0};
  RSkImageCacheManager::getCacheUsage(usageArr);
  if ((usageArr[CPU_MEM_ARR_INDEX] < SKIA_CPU_IMAGE_CACHE_HWM_LIMIT) &&
    ( usageArr[GPU_MEM_ARR_INDEX] < SKIA_GPU_IMAGE_CACHE_HWM_LIMIT))
    return true;

  ImageCacheMap::iterator it=imageCache_.begin();
  while( it != imageCache_.end()) {
    if( evictCount >= EVICT_COUNT){
      break;
    }
    if((it->second.imageData)->unique()) {
      it=imageCache_.erase(it);
      evictCount++;
    } else {
      ++it;
    }
  }
  //As eviction from Skia's cache & RNS cache system  were
  //asynchronous,Ensuring cache memory drained below the
  //Limit is not feasible at this point.
  //So just let to add further, if eviction occured at RNS Level
  return (evictCount == EVICT_COUNT);
}

void RSkImageCacheManager::init() {
  RSkImageCacheManager::getImageCacheManagerInstance();
  SkGraphics::SetResourceCacheTotalByteLimit(SKIA_CPU_IMAGE_CACHE_LIMIT);
#ifdef RNS_SHELL_HAS_GPU_SUPPORT
  RnsShell::WindowContext::grTransactionBegin();
  GrDirectContext* gpuContext = RSkSurfaceWindow::getDirectContext();
  if(gpuContext)
    gpuContext->setResourceCacheLimit(SKIA_GPU_IMAGE_CACHE_LIMIT);
  RnsShell::WindowContext::grTransactionEnd();
#endif //RNS_SHELL_HAS_GPU_SUPPORT
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
#endif//RNS_IMAGE_CACHE_USAGE_DEBUG

void RSkImageCacheManager::expiryTimeCallback() {
  ImageCacheMap::iterator it =imageCache_.begin();
  double currentTime = SkTime::GetMSecs();
  std::chrono::duration<double, std::milli> milliseconds = Timer::getFutureTime().time_since_epoch();
  double scheduleTimeExpiry = milliseconds.count();
  while(it != imageCache_.end()) {
    if(it->second.expiryTime <= currentTime){
     RNS_LOG_DEBUG("erase imageData :"<<it->first<<std::endl);
     it = imageCache_.erase(it);
    } else{
      if (scheduleTimeExpiry > it->second.expiryTime)
        scheduleTimeExpiry = it->second.expiryTime;
      it++;
    }
  }
  if(imageCache_.size()){
    scheduleTimeExpiry_ = scheduleTimeExpiry;
    timer_->reschedule((scheduleTimeExpiry_ - SkTime::GetMSecs()),0);
  } else {
    scheduleTimeExpiry_ = 0;
  }
}

sk_sp<SkImage> RSkImageCacheManager::findImageDataInCache(const char* path) {
  std::scoped_lock lock(imageCacheLock);
  sk_sp<SkImage> imageData{nullptr};
  ImageCacheMap::iterator it = imageCache_.find(path);
  imageData = ((it != imageCache_.end()) ? it->second.imageData : nullptr);
  return imageData;
}

bool RSkImageCacheManager::imageDataInsertInCache(const char* path,decodedimageCacheData imageCacheData) {
  std::scoped_lock lock(imageCacheLock);
  double currentTime = SkTime::GetMSecs();
  if(imageCacheData.imageData && evictAsNeeded()) {
    imageCache_.insert(std::pair<std::string, decodedimageCacheData>(path,imageCacheData));
    RNS_LOG_INFO("New Entry in Map..."<<" file :"<<path<< "  expiryTime :"<<imageCacheData.expiryTime);
    if(imageCache_.size() == 1) {
      scheduleTimeExpiry_ = imageCacheData.expiryTime;
      if(timer_ == nullptr) {
        auto callback = std::bind(&RSkImageCacheManager::expiryTimeCallback,this);
        timer_ = new Timer(scheduleTimeExpiry_ - currentTime,0,callback,true);
      }else {
        timer_->reschedule( scheduleTimeExpiry_ - currentTime,0);
      }
    } else if (imageCacheData.expiryTime < scheduleTimeExpiry_) {
      scheduleTimeExpiry_ = imageCacheData.expiryTime;
      double duration = scheduleTimeExpiry_ - currentTime;
      timer_->reschedule(duration,0);
    }
    return true;
  } else {
    RNS_LOG_ERROR("Insert image data to cache failed... :"<<" file :" << path);
    return false;
  }
}

bool RSkImageCacheManager::clearMemory() {
  std::scoped_lock lock(imageCacheLock);
  size_t usageArr[2]={0,0};
  ImageCacheMap::iterator it=imageCache_.begin();
  while( it != imageCache_.end()) {
    if((it->second.imageData)->unique()) {
      it=imageCache_.erase(it);
    } else {
      ++it;
    }
  }
#ifdef RNS_SHELL_HAS_GPU_SUPPORT
  RnsShell::WindowContext::grTransactionBegin();
  GrDirectContext* gpuContext = RSkSurfaceWindow::getDirectContext();
  if(gpuContext) {
    gpuContext->purgeUnlockedResources(false);
  }
  RnsShell::WindowContext::grTransactionEnd();
#endif
#ifdef RNS_IMAGE_CACHE_USAGE_DEBUG
    printCacheUsage(); // for print memory size for CPU and GPU cache
#endif //RNS_IMAGECACHING_DEBUG
  SkGraphics::PurgeResourceCache(); // purge for CPU memory cache
  if(imageCache_.empty()) {
    timer_->abort();
  }
  return true;
}

bool RSkImageCacheManager::clearDisk() {
  RNS_LOG_NOT_IMPL;
  return true;
}

} // namespace react
} // namespace facebook
