/*
* Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
*
* This source code is licensed under the MIT license found in the
* LICENSE file in the root directory of this source tree.
*/
#include <curl/curl.h>
#include <butter/map.h>
#include <semaphore.h>

#include "ReactSkia/modules/RSkNetworkingModuleBase.h"
 #include "ReactSkia/sdk/CurlNetworking.h"

namespace facebook {
namespace react {
class RSkNetworkingModule;
struct NetworkRequest {
  NetworkRequest(bool useIncrementalUpdates,  std::string responseType, RSkNetworkingModule *self)
    :self_(self)
    ,useIncrementalUpdates_(useIncrementalUpdates)
    ,responseType_(responseType)
    ,uploadComplete_(false)
    ,downloadComplete_(false)
    {}
  RSkNetworkingModule *self_;
  int requestId_;
  bool useIncrementalUpdates_;
  std::string responseType_;
  bool uploadComplete_;
  bool downloadComplete_;
  std::shared_ptr<CurlRequest> curlRequest_;
};

  
class RSkNetworkingModule:  public RSkNetworkingModuleBase {
 public:
  RSkNetworkingModule(
      const std::string &name,
      std::shared_ptr<CallInvoker> jsInvoker,
      Instance *bridgeInstance);
  ~ RSkNetworkingModule();

   jsi::Value sendRequest(
       folly::dynamic,
       const jsi::Object&,
       jsi::Runtime&) override;

   jsi::Value abortRequest(
       folly::dynamic) override;

  void sendData(CurlResponse*, NetworkRequest*);
  void sendProgressEventwrapper(double, double, double, double, NetworkRequest*);
  void headerCallbackWrapper(void*, NetworkRequest*);
  void writeMemoryCallbackWrapper(void*, char*, size_t);
  butter::map <int , NetworkRequest*> connectionList_;
  void processNetworkRequest(CURLM *cm);
  static size_t writeMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp);
  static size_t progressCallback(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow);
  static size_t headerCallback(void *contents, size_t size, size_t nitems, void *userdata);

 private:
  std::mutex connectionListLock_;
  uint64_t nextUniqueId();
  CurlNetworking* sharedCurlNetworking_;

};

}// namespace react
}// namespace facebook

