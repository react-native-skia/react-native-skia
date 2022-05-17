/*
* Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
*
* This source code is licensed under the MIT license found in the
* LICENSE file in the root directory of this source tree.
*/

#include <semaphore.h>

#include "cxxreact/Instance.h"
#include "jsi/JSIDynamic.h"

#include <ReactCommon/TurboModuleUtils.h>
#include "ReactCommon/TurboModule.h"

#include "ReactSkia/utils/RnsLog.h"
#include "ReactSkia/JSITurboModuleManager.h"
#include "RSkNetworkingModule.h"

enum curlStatus {CURL_RETURN_FAILURE=-1,CURL_RETURN_SUCESS};
using namespace std;
namespace facebook {
namespace react {

RSkNetworkingModule::RSkNetworkingModule(
    const std::string &name,
    std::shared_ptr<CallInvoker> jsInvoker,
    Instance *bridgeInstance)
    :  RSkNetworkingModuleBase(name, jsInvoker, bridgeInstance) {

  sharedCurlNetworking_ = CurlNetworking::sharedCurlNetworking();
}

RSkNetworkingModule::~RSkNetworkingModule() {
 
};
 
uint64_t RSkNetworkingModule::nextUniqueId() {
  static std::atomic<uint64_t> nextId(1);
  uint64_t id;
  do {
    id = nextId.fetch_add(1);
  } while (id == 0);  // 0 invalid id.
  return id;
}

jsi::Value RSkNetworkingModule::sendRequest(
  folly::dynamic query, const jsi::Object &callbackObj, jsi::Runtime &rt) {
  jsi::Value status = jsi::Value((int)CURL_RETURN_FAILURE);
  std::string method = query["method"].getString();
  std::string  url = query["url"].getString();
  std::string responseType  = query["responseType"].getString();
  bool incrementalUpdates  = query["incrementalUpdates"].asInt();
  long timeout = query["timeout"].asDouble();
  NetworkRequest *networkRequest =  new NetworkRequest(incrementalUpdates, responseType, this);
  networkRequest->requestId_ = nextUniqueId();

  // Add the request object to list and notify app using callback function.

  CurlRequest* curlRequest = new CurlRequest(nullptr,url.c_str(),timeout,method.c_str());

  auto headerCallback = [&](void* curlResponse,void *userdata) -> size_t {
    struct NetworkRequest *networkRequest = ((struct NetworkRequest *)userdata);
    networkRequest->self_->headerCallbackWrapper(curlResponse,networkRequest);
    return 0;
  };

  auto progressCallback = [&](double dltotal, double dlnow, double ultotal, double ulnow,void *userdata) -> size_t {
    struct NetworkRequest *networkRequest = ((struct NetworkRequest *)userdata);
    networkRequest->self_->sendProgressEventwrapper(dltotal, dlnow, ultotal, ulnow, networkRequest);
    return 0;
  };

  auto completionCallback =[&](void* curlResponse, void* userdata) -> bool {
    std::scoped_lock lock(connectionListLock_);
    struct CurlResponse *responseData =  (struct CurlResponse *)curlResponse;
    struct NetworkRequest *networkRequest = ((struct NetworkRequest *)userdata);
    if(networkRequest) {
      networkRequest->downloadComplete_ = true;
      if(!(networkRequest->useIncrementalUpdates_ && (networkRequest->responseType_ == "text") ))
        sendData(responseData, networkRequest);
      sendEventWithName("didCompleteNetworkResponse", folly::dynamic::array(networkRequest->requestId_, responseData->errorResult , responseData->responseTimeout ));
      connectionList_.erase(networkRequest->requestId_);
      delete networkRequest;
      return true; //network object is deleted, so curl_cleanup is not required
    }
    return false;
  };
  curlRequest->curldelegator.CURLNetworkingProgressCallback = progressCallback;
  curlRequest->curldelegator.CURLNetworkingHeaderCallback = headerCallback;
  curlRequest->curldelegator.CURLNetworkingCompletionCallback = completionCallback;
  curlRequest->curldelegator.delegatorData = networkRequest;
  networkRequest->curlRequest_ = curlRequest;
  if(sharedCurlNetworking_->sendRequest(curlRequest,query) == false)
    goto safe_return;

  connectionList_[networkRequest->requestId_] = networkRequest;
  if(callbackObj.isFunction(rt)) {
    jsi::Function callback = callbackObj.getFunction(rt);
    callback.call(rt, (int) networkRequest->requestId_, 1);
  }
  status = jsi::Value((int)CURL_RETURN_SUCESS);
safe_return :
  if(status.getNumber() == CURL_RETURN_FAILURE && networkRequest )
    delete networkRequest;
  return status;
}

jsi::Value RSkNetworkingModule::abortRequest(folly::dynamic requestId) {
  std::scoped_lock lock(connectionListLock_);
  struct NetworkRequest *networkRequest = connectionList_[requestId.asInt()];
  if(networkRequest == NULL ) {
    RNS_LOG_ERROR ("networkRequest is not valid \n");
    return jsi::Value((int)CURL_RETURN_FAILURE);
  }
  if(sharedCurlNetworking_->abortRequest(networkRequest->curlRequest_)) {
    RNS_LOG_DEBUG (" aborting Curl is success \n");
    connectionList_.erase(networkRequest->requestId_);
  }
  return jsi::Value((int)CURL_RETURN_SUCESS);
}

void RSkNetworkingModule::sendProgressEventwrapper(double dltotal,double dlnow, double ultotal, double ulnow, NetworkRequest *networkRequest) {
  struct CurlResponse *responseData =  (struct CurlResponse *)&networkRequest->curlRequest_->curlResponse;
  if(networkRequest->uploadComplete_ == false && ultotal != 0.0) {
    sendEventWithName("didSendNetworkData", folly::dynamic::array(networkRequest->requestId_, ulnow,ultotal ));
    if(ulnow >= ultotal)
      networkRequest->uploadComplete_ = true;
  }
  if(networkRequest->downloadComplete_ == false && dlnow != 0.0) {
    if(networkRequest->useIncrementalUpdates_) {
      if(networkRequest->responseType_ == "text" && responseData->responseBufferOffset !=0) {
        sendEventWithName("didReceiveNetworkIncrementalData", folly::dynamic::array(networkRequest->requestId_ ,responseData->responseBuffer, dlnow,dltotal ));
        responseData->responseBufferOffset = 0;
      }else 
        sendEventWithName("didReceiveNetworkDataProgress", folly::dynamic::array(networkRequest->requestId_ , dlnow,dltotal ));
    }

  }
}

void RSkNetworkingModule::headerCallbackWrapper(void* curlResponse, NetworkRequest *networkRequest) {
  struct CurlResponse *responseData =  (struct CurlResponse *)curlResponse;
  sendEventWithName("didReceiveNetworkResponse", folly::dynamic::array(networkRequest->requestId_  , responseData->statusCode, responseData->headerBuffer ,responseData->responseurl));
}

void RSkNetworkingModule::sendData(CurlResponse *responseData, NetworkRequest *networkRequest) {
  char* responseBuffer= NULL;
  if(!(responseData->responseBuffer) || responseData->contentSize == 0)
    return;
  if(networkRequest->responseType_ == "text")
    responseBuffer = responseData->responseBuffer;
  else if(networkRequest->responseType_ == "base64")
    RNS_LOG_NOT_IMPL;
  else {
    RNS_LOG_ERROR ( "Invalid responseType: \n");
    return;
  }
  sendEventWithName("didReceiveNetworkData", folly::dynamic::array(networkRequest->requestId_ , responseBuffer));
}

}// namespace react
}//namespace facebook

