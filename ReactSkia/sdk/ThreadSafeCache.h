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
  static  int overallCurrentSize_;
  static  int overallMaxSize_;
  std::mutex cacheLock_;
  int currentSize_;
  int maxSize_;
  KeyT key_;
  better::map <KeyT,ValueT> cacheMap_;
 public:
  bool isAvailableInCache(const KeyT &key){
    auto it = cacheMap_.find(key);
    if(it!= cacheMap_.end())
      return true;
    return false;
  };
  better::optional<ValueT> getCacheData(const KeyT &key) {
    std::scoped_lock lock(cacheLock_);
    auto it = cacheMap_.find(key);
    if(it == cacheMap_.end()) {
       return{};
    }
    return it->second;
  };
  ThreadSafeCache(int currentSize, int maxsize):currentSize_(currentSize),maxSize_(maxsize){
      
  };
  void setCache(const KeyT &key ,  const ValueT &value) {
    std::scoped_lock lock(cacheLock_);
    cacheMap_[key] = value;
  };
  void evict() {
    RNS_LOG_NOT_IMPL;
  };
    
};

}// namespace sdk
}// namespace rns
