/*
* Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
*
* This source code is licensed under the MIT license found in the
* LICENSE file in the root directory of this source tree.
*/
#include <mutex>
#include <better/map.h>
#include <better/optional.h>
#include "ReactSkia/utils/RnsLog.h"
#include "ReactSkia/sdk/FollyTimer.h"
#define THREADSAFE_DEFAULT_MAX_CACHE_LIMIT 10*1024*1024 // 1MB
#define THREADSAFE_MAX_CACHE_HWM_LIMIT  THREADSAFE_DEFAULT_MAX_CACHE_LIMIT*.95 //95% as High Water mark level
using namespace std;
namespace rns {
namespace sdk {
using namespace facebook;
// KeyT any type and ValueT any type
// ThreadSafeCache<<key>,<value>>
//Example: ThreadSafeCache<string,shared_ptr<CurlResponse>>
template <typename KeyT, typename ValueT>
class ThreadSafeCache {
 private:
  struct cacheData{
    ValueT value;
    double cacheExpiryTime;
  };
  double overallCurrentSize_ = 0;
  std::mutex cacheLock_;
  KeyT key_;
  better::map <KeyT,std::unique_ptr<cacheData>> cacheMap_;
  Timer * timer_{nullptr};
  double scheduleTimeExpiry_;
 public:
  bool isAvailableInCache(const KeyT &key){
    auto it = cacheMap_.find(key);
    if(it!= cacheMap_.end())
      return true;
    return false;
  }

  better::optional<ValueT> getCacheData(const KeyT &key) {
    std::scoped_lock lock(cacheLock_);
    auto it = cacheMap_.find(key);
    if(it == cacheMap_.end()) {
       return{};
    }
    return it->second->value;
  }

  void expiryTimerCallback() {
    auto it =cacheMap_.begin();
    double currentTime = Timer::getCurrentTimeMSecs();
    std::chrono::duration<double, std::milli> milliseconds = Timer::getFutureTime().time_since_epoch();
    double scheduleTimeExpiry = milliseconds.count();
    while(it != cacheMap_.end()) {
      if(it->second->cacheExpiryTime <= currentTime) {
        RNS_LOG_DEBUG("erase imageData :"<<it->first<<std::endl);
        it = cacheMap_.erase(it);
      } else{
        if (scheduleTimeExpiry > it->second->cacheExpiryTime) {
          scheduleTimeExpiry = it->second->cacheExpiryTime;
        }
        it++;
      }
    }
    if(cacheMap_.size()) {
      scheduleTimeExpiry_ = scheduleTimeExpiry;
      timer_->reschedule((scheduleTimeExpiry_ - currentTime),0);
    } else {
      scheduleTimeExpiry_ = 0;
    }
  }

  void setCache(const KeyT &key ,  const ValueT &value, double cacheExpiryTime) {
    std::scoped_lock lock(cacheLock_);
    std::unique_ptr<cacheData> cacheObj =  make_unique<cacheData>();;
    cacheObj->cacheExpiryTime = cacheExpiryTime;
    cacheObj->value = value;
    cacheMap_[key] = std::move(cacheObj);
    double currentTime = Timer::getCurrentTimeMSecs();
    if(cacheMap_.size() == 1) {
      scheduleTimeExpiry_ = cacheExpiryTime;
      if(timer_ == nullptr) {
        auto callback = std::bind(&ThreadSafeCache::expiryTimerCallback,this);
        timer_ = new Timer(scheduleTimeExpiry_ - currentTime,0,callback,true);
      }else{
        timer_->reschedule( scheduleTimeExpiry_ - currentTime,0);
      }
    } else if(cacheExpiryTime < scheduleTimeExpiry_) {
      scheduleTimeExpiry_ = cacheExpiryTime;
      double duration = scheduleTimeExpiry_ - currentTime;
      timer_->reschedule(duration,0);
    }
  }

  bool needEvict(double requiredSize) {
    if(overallCurrentSize_+requiredSize < THREADSAFE_MAX_CACHE_HWM_LIMIT)
      return false;
    return true;
  }

};

}// namespace sdk
}// namespace rns
