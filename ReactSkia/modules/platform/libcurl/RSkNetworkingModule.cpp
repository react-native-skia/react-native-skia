/*
* Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
*
* This source code is licensed under the MIT license found in the
* LICENSE file in the root directory of this source tree.
*/
#include <curl/curl.h>
#include <semaphore.h>

#include "cxxreact/Instance.h"
#include "jsi/JSIDynamic.h"

#include <ReactCommon/TurboModuleUtils.h>
#include "ReactCommon/TurboModule.h"

#include "ReactSkia/utils/RnsLog.h"
#include "ReactSkia/JSITurboModuleManager.h"
#include "RSkNetworkingModule.h"
#ifndef CA_CERTIFICATE
#define CA_CERTIFICATE       "/etc/ssl/certs/ca-certificates.crt"      /**< The certificate of the CA to establish https connection to the server*/
#endif
#define MAX_PARALLEL_CONNECTION 5
enum curlStatus {CURL_RETURN_FAILURE=-1,CURL_RETURN_SUCESS};
using namespace std;
namespace facebook {
namespace react {

RSkNetworkingModule::RSkNetworkingModule(
    const std::string &name,
    std::shared_ptr<CallInvoker> jsInvoker,
    Instance *bridgeInstance)
    :  RSkNetworkingModuleBase(name, jsInvoker, bridgeInstance) {

  curl_global_init(CURL_GLOBAL_ALL);
  sem_init(&networkRequestSem_, 0, 0);
  curlMultiHandle_ = curl_multi_init();
 /* Limit the amount of simultaneous connections curl should allow: */
  curl_multi_setopt(curlMultiHandle_, CURLMOPT_MAX_TOTAL_CONNECTIONS, (long)MAX_PARALLEL_CONNECTION);
  multiNetworkThread_ = std::thread([this]() {
    while(!exitLoop_){
      if(curlMultiHandle_) {
        sem_wait(&networkRequestSem_);
        processNetworkRequest(curlMultiHandle_);
      }  else {
          std::this_thread::sleep_for(1000ms);
      }
    }

  });


}

RSkNetworkingModule::~RSkNetworkingModule() {
  exitLoop_ = true;
  multiNetworkThread_.join();
  if(curlMultiHandle_){
    curl_multi_cleanup(curlMultiHandle_);
    curl_global_cleanup();
  }
};

size_t RSkNetworkingModule::progressCallback(void *userdata, double dltotal, double dlnow, double ultotal, double ulnow) {
  struct NetworkRequest *networkRequest = ((struct NetworkRequest *)userdata);
  networkRequest->self_->sendProgressEventwrapper(networkRequest, dltotal, dlnow, ultotal, ulnow);
  return 0;
}

size_t RSkNetworkingModule::writeMemoryCallback(void *contents, size_t size, size_t nmemb, void *userdata) {
  size_t realsize = size * nmemb;
  struct NetworkRequest *networkRequest = ((struct NetworkRequest *)userdata);
  networkRequest->self_->writeMemoryCallbackWrapper(networkRequest, (char*)contents, realsize);
  return realsize;
}

size_t RSkNetworkingModule::headerCallback(void *contents, size_t size,size_t nitems, void *userdata)
{
  struct NetworkRequest *networkRequest = ((struct NetworkRequest *)userdata);
  networkRequest->self_->headerCallbackWrapper(networkRequest, (char*)contents, nitems);
  return nitems * size;
}
 
uint64_t RSkNetworkingModule::nextUniqueId() {
  static std::atomic<uint64_t> nextId(1);
  uint64_t id;
  do {
    id = nextId.fetch_add(1);
  } while (id == 0);  // 0 invalid id.
  return id;
}

void RSkNetworkingModule::processNetworkRequest(CURLM *curlMultiHandle) {
  CURLMcode res = CURLM_OK;
  CURLMsg *msg;
  int stillAlive = 0;
  int msgsLeft = 0;
  do {
    res = curl_multi_perform(curlMultiHandle, &stillAlive);
    while((msg = curl_multi_info_read(curlMultiHandle, &msgsLeft))) {
      if(msg->msg == CURLMSG_DONE) {
        int requestId = 0;
        CURL *curlHandle = msg->easy_handle;
        curl_easy_getinfo(curlHandle, CURLINFO_PRIVATE, &requestId);
        std::scoped_lock lock(connectionListLock_);
        struct NetworkRequest *networkRequest = connectionList_[requestId];
        // completionBlock
        if(networkRequest) {
          if(networkRequest->curl_ != NULL) {
            if(!(networkRequest->useIncrementalUpdates_ && (networkRequest->responseType_ == "text") ))
              sendData(networkRequest);
            sendEventWithName("didCompleteNetworkResponse", folly::dynamic::array(networkRequest->requestId_ , (res != CURLM_OK) ? curl_easy_strerror(msg->data.result):"" , ((msg->data.result) == CURLE_OPERATION_TIMEDOUT ? "true" : "false") ));
            connectionList_.erase(networkRequest->requestId_);
          }
          curl_multi_remove_handle(curlMultiHandle, curlHandle);
          delete networkRequest;
        } else {
           curl_easy_cleanup(curlHandle);
        }
      }
      else {
        RNS_LOG_ERROR("Unknown critical error: CURLMsg" << msg->msg);
      }

    }
    if(stillAlive)
      curl_multi_wait(curlMultiHandle, NULL, 0, 1000, NULL);

  } while(stillAlive);
}

bool RSkNetworkingModule::preparePostRequest(void* request, folly::dynamic headers, folly::dynamic data ) {
  struct NetworkRequest *networkRequest = (struct NetworkRequest*)request;  
  struct curl_slist *curlListRequestHeader = NULL;
  const char *dataPtr = NULL;
  size_t dataSize = 0;
  bool status = false;

  if(data["string"].c_str()) {
    dataPtr = data["string"].c_str();
    dataSize = data["string"].getString().length();
  } else if(data["formData"].c_str()) {
    RNS_LOG_NOT_IMPL;
    return status;
  } else if(data["blob"].c_str()) {
    RNS_LOG_NOT_IMPL;
    return status;
  } else if(data["uri"].c_str()) {
    RNS_LOG_NOT_IMPL;
    return status;
  } else if(data["base64"].c_str()) {
    RNS_LOG_NOT_IMPL;
    return status;
  } else {
    RNS_LOG_ERROR ("Unknown Data for Post Request\n") ;
    return status;
  }
  
  curl_easy_setopt(networkRequest->curl_, CURLOPT_POST, 1L);
  for (auto& pair : headers.items()) {
    /* Key is pair.first, value is pair.second */
    string headerKeyValue = pair.first.c_str();
    const char* parse= ": ";
    headerKeyValue += parse;
    headerKeyValue += pair.second.c_str();
    curlListRequestHeader = curl_slist_append(curlListRequestHeader, headerKeyValue.c_str());
  }
  curl_easy_setopt(networkRequest->curl_, CURLOPT_HTTPHEADER, curlListRequestHeader);
  /* get verbose debug output please */
  curl_easy_setopt(networkRequest->curl_, CURLOPT_POSTFIELDSIZE, (long)dataSize);
  curl_easy_setopt(networkRequest->curl_, CURLOPT_COPYPOSTFIELDS, dataPtr);
  status = true;
  return status;
}

jsi::Value RSkNetworkingModule::sendRequest(
  folly::dynamic query, const jsi::Object &callbackObj, jsi::Runtime &rt) {
  std::string method = query["method"].getString();
  auto url = query["url"];
  auto headers = query["headers"];
  auto data = query["data"];
  std::string responseType  = query["responseType"].getString();
  bool incrementalUpdates  = query["incrementalUpdates"].asInt();
  long timeout = query["timeout"].asDouble();
  auto  withCredentials  = query["withCredentials"];
  CURL *curl = nullptr;
  jsi::Value status = jsi::Value((int)CURL_RETURN_FAILURE);
  CURLcode res = CURLE_FAILED_INIT;
  NetworkRequest *networkRequest =  new NetworkRequest(incrementalUpdates, responseType, this);
  /* get a curl handle */
  curl = curl_easy_init();
  if(curl == NULL) {
    RNS_LOG_ERROR (stderr << "curl_easy_init() failed \n");
    goto safe_return;
  } 
  networkRequest->curl_ = curl;
  networkRequest->requestId_ = nextUniqueId();
  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl, CURLOPT_PRIVATE, networkRequest->requestId_);
  
  //The following code gets executed for a https connection.
  if(strstr(url.c_str(),"https") != NULL) {
       curl_easy_setopt(curl, CURLOPT_SSLENGINE_DEFAULT, 1L);
       curl_easy_setopt(curl, CURLOPT_CAINFO, CA_CERTIFICATE);
       curl_easy_setopt(curl,CURLOPT_SSL_VERIFYPEER,0);
  }

  if(timeout)
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);
  
  curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);//Enable curl logs

  // Progress callback and user data
  if (incrementalUpdates) {
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L); // Enable progress callback
    curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, networkRequest );
    curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, progressCallback);
  }

  //Header callback and user data
  curl_easy_setopt(curl, CURLOPT_WRITEHEADER, networkRequest);
  curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, headerCallback);

  if(method.compare("POST") == 0) {
    if(preparePostRequest(networkRequest, headers, data) == false)
      goto safe_return;
  } else if(method.compare("GET") == 0) {
      // ResponseWrite callback and user data
      curl_easy_setopt(curl, CURLOPT_WRITEDATA, networkRequest);
      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeMemoryCallback);
  } else {
      RNS_LOG_ERROR ("Not supported method\n" << method) ;
      goto safe_return;
  }

  // Add the request object to list and notify app using callback function. 
  connectionList_[networkRequest->requestId_] = networkRequest;
  if(callbackObj.isFunction(rt)) {
    jsi::Function callback = callbackObj.getFunction(rt);
    callback.call(rt, (int) networkRequest->requestId_, 1);
  }
  curl_multi_add_handle(curlMultiHandle_, curl);
  sem_post(&networkRequestSem_);
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
  curl_easy_cleanup(networkRequest->curl_);
  networkRequest->curl_  = NULL;
  connectionList_.erase(requestId.asInt());
  return jsi::Value((int)CURL_RETURN_SUCESS);
}

void RSkNetworkingModule::sendProgressEventwrapper(NetworkRequest *networkRequest, double dltotal,double dlnow, double ultotal, double ulnow) {
  if(networkRequest->uploadComplete_ == false && ultotal != 0) {
    sendEventWithName("didSendNetworkData", folly::dynamic::array(networkRequest->requestId_ , ulnow,ultotal ));
    if(ulnow >= ultotal)
      networkRequest->uploadComplete_ = true;
  }
  if(networkRequest->downloadComplete_ == false && dltotal != 0) {
    if(networkRequest->useIncrementalUpdates_) {
      if(networkRequest->responseType_ == "text") {
        // TODO: responseText property needs to get and pass the value in the didReceiveNetworkIncrementalData event
        sendEventWithName("didReceiveNetworkIncrementalData", folly::dynamic::array(networkRequest->requestId_ , "", dlnow,dltotal ));
      }else 
        sendEventWithName("didReceiveNetworkDataProgress", folly::dynamic::array(networkRequest->requestId_ , dlnow,dltotal ));
    }
    if(dlnow >= dltotal)
      networkRequest->downloadComplete_ = true;
  }
}

void RSkNetworkingModule::headerCallbackWrapper(NetworkRequest *networkRequest, char* headerInfoBuffer, size_t size) {
  long response_code = 0;
  char *url = NULL;
  curl_easy_getinfo(networkRequest->curl_, CURLINFO_EFFECTIVE_URL, &url);
  curl_easy_getinfo(networkRequest->curl_, CURLINFO_RESPONSE_CODE, &response_code);
  networkRequest->headerBuffer_  = (char *) realloc(networkRequest->headerBuffer_ , networkRequest->headerBufferOffset_+size+1);	
  RNS_LOG_ASSERT(networkRequest->headerBuffer_ , "headerBuffer cannot be null");
  // Each headerInfo line comes as a seperate callback, so append all here
  memcpy(&(networkRequest->headerBuffer_[networkRequest->headerBufferOffset_]), headerInfoBuffer, size);
  networkRequest->headerBufferOffset_ += size;
  // headerInfo ends with \r and \n
  if(size == 2 && headerInfoBuffer[0] == 13 && headerInfoBuffer[1] == 10  ) {
     networkRequest->headerBuffer_[networkRequest->headerBufferOffset_] = 0;
     sendEventWithName("didReceiveNetworkResponse", folly::dynamic::array(networkRequest->requestId_  , response_code, networkRequest->headerBuffer_ ,url));
  }
}

void RSkNetworkingModule::writeMemoryCallbackWrapper(NetworkRequest *networkRequest, char* writeMemoryBuffer, size_t realSize) {              
  networkRequest->responseBuffer_  = (char *) realloc(networkRequest->responseBuffer_ , networkRequest->responseBufferOffset_+realSize+1);
  RNS_LOG_ASSERT((networkRequest->responseBuffer_), "responseBuffer cannot be null");
  memcpy(&(networkRequest->responseBuffer_ [networkRequest->responseBufferOffset_]), writeMemoryBuffer, realSize);
  networkRequest->responseBufferOffset_ += realSize;
  networkRequest->responseBuffer_[networkRequest->responseBufferOffset_] = 0;
  networkRequest->contentSize_ = networkRequest->responseBufferOffset_;
}

void RSkNetworkingModule::sendData(NetworkRequest *networkRequest) {
  char* responseBuffer= NULL;
  if(!(networkRequest->responseBuffer_) || networkRequest->contentSize_ == 0)
    return;
  if(networkRequest->responseType_ == "text")
    responseBuffer = networkRequest->responseBuffer_;
  else if(networkRequest->responseType_ == "base64")
    RNS_LOG_NOT_IMPL;
  else {
    RNS_LOG_ERROR ( "Invalid responseType: \n");
    return;
  }
  sendEventWithName("didReceiveNetworkData", folly::dynamic::array(networkRequest->requestId_  , responseBuffer));
}

}// namespace react
}//namespace facebook

