/*
* Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
*
* This source code is licensed under the MIT license found in the
* LICENSE file in the root directory of this source tree.
*/
#include <curl/curl.h>
#include <semaphore.h>

#include "ReactSkia/utils/RnsLog.h"
#include "CurlNetworking.h"

using namespace std;
namespace facebook {
namespace react {
CurlNetworking* CurlNetworking::sharedCurlNetworking_{nullptr};
std::mutex CurlNetworking::mutex_;

CurlNetworking::CurlNetworking() {
  curl_global_init(CURL_GLOBAL_ALL);
  sem_init(&networkRequestSem_, 0, 0);
  curlMultihandle_ = curl_multi_init();
  /* Limit the amount of simultaneous connections curl should allow: */
  curl_multi_setopt(curlMultihandle_, CURLMOPT_MAX_TOTAL_CONNECTIONS, (long)MAX_PARALLEL_CONNECTION);
  multiNetworkThread_ = std::thread([this]() {
    while(!exitLoop_){
      if(curlMultihandle_) {
        sem_wait(&networkRequestSem_);
        CurlNetworking::processNetworkRequest(curlMultihandle_);
      } else {
          std::this_thread::sleep_for(1000ms);
      }
    }
  });

}
CurlNetworking* CurlNetworking::sharedCurlNetworking() {
  std::lock_guard<std::mutex> lock(mutex_);
  if(sharedCurlNetworking_ == nullptr) {
    sharedCurlNetworking_ = new CurlNetworking();
  }
    return sharedCurlNetworking_;
}
CurlRequest::CurlRequest(CURL *lhandle, const char* lURL,size_t ltimeout,const char* lmethod):
  handle(lhandle),
  URL(lURL),
  timeout(ltimeout),
  method(lmethod)
  {}

CurlRequest::~CurlRequest() {
  if(curlResponse.headerBuffer){
    free(curlResponse.headerBuffer);
    curlResponse.headerBuffer = NULL;
  }
  if(curlResponse.responseBuffer){
    free(curlResponse.responseBuffer);
    curlResponse.responseBuffer = NULL;
  }
}    

CurlNetworking::~CurlNetworking() {
  exitLoop_ = true;
  multiNetworkThread_.join();
  if(curlMultihandle_){
    curl_multi_cleanup(curlMultihandle_);
    curl_global_cleanup();
  }
  std::lock_guard<std::mutex> lock(mutex_);
  if(this == sharedCurlNetworking_)
    sharedCurlNetworking_ = nullptr;
};

void CurlNetworking::processNetworkRequest(CURLM *curlMultiHandle) {
  CURLMcode res = CURLM_OK;
  CURLMsg *msg;
  int stillAlive = 0;
  int msgsLeft = 0;

  do {
    res = curl_multi_perform(curlMultiHandle, &stillAlive);
    while((msg = curl_multi_info_read(curlMultiHandle, &msgsLeft))) {
      if(msg->msg == CURLMSG_DONE) {
        CURL *curlHandle = msg->easy_handle;
        CurlRequest *curlRequest = nullptr;
        curl_easy_getinfo(curlHandle, CURLINFO_PRIVATE, &curlRequest);
        res == CURLM_OK ?
            (curlRequest->curlResponse.errorResult = "")
            :(curlRequest->curlResponse.errorResult= curl_easy_strerror(msg->data.result));
        (msg->data.result) == CURLE_OPERATION_TIMEDOUT ?
            (curlRequest->curlResponse.responseTimeout = true)
            :(curlRequest->curlResponse.responseTimeout = false);
        curl_multi_remove_handle(curlMultiHandle, curlHandle);
        if(curlRequest->curldelegator.CURLNetworkingCompletionCallback)
          curlRequest->curldelegator.CURLNetworkingCompletionCallback(&curlRequest->curlResponse,curlRequest->curldelegator.delegatorData);
        curl_easy_cleanup(curlHandle);
      }
      else {
        RNS_LOG_ERROR("Unknown critical error: CURLMsg" << msg->msg);
      }

    }
    if(stillAlive)
      curl_multi_wait(curlMultiHandle, NULL, 0, 1000, NULL);

  } while(stillAlive);

}

bool CurlNetworking::preparePostRequest(CurlRequest *curlRequest, folly::dynamic headers, folly::dynamic data ) { 
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
  
  curl_easy_setopt(curlRequest->handle, CURLOPT_POST, 1L);
  for (auto& pair : headers.items()) {
    /* Key is pair.first, value is pair.second */
    string headerKeyValue = pair.first.c_str();
    const char* parse= ": ";
    headerKeyValue += parse;
    headerKeyValue += pair.second.c_str();
    curlListRequestHeader = curl_slist_append(curlListRequestHeader, headerKeyValue.c_str());
  }
  curl_easy_setopt(curlRequest->handle, CURLOPT_HTTPHEADER, curlListRequestHeader);
  /* get verbose debug output please */
  curl_easy_setopt(curlRequest->handle, CURLOPT_POSTFIELDSIZE, (long)dataSize);
  curl_easy_setopt(curlRequest->handle, CURLOPT_COPYPOSTFIELDS, dataPtr);
  status = true;
  return status;
}
size_t CurlNetworking::writeCallbackCurlWrapper(void* buffer, size_t size, size_t nitems, void* userData) {
  CurlRequest *curlRequest = (CurlRequest *)userData;
  std::scoped_lock lock(curlRequest->bufferLock);
  curlRequest->curlResponse.responseBuffer  = (char *) realloc(curlRequest->curlResponse.responseBuffer , curlRequest->curlResponse.responseBufferOffset+nitems+1);
  RNS_LOG_ASSERT((curlRequest->curlResponse.responseBuffer), "responseBuffer cannot be null ");
  memcpy(&(curlRequest->curlResponse.responseBuffer [curlRequest->curlResponse.responseBufferOffset]), (char *)buffer, nitems);
  curlRequest->curlResponse.responseBufferOffset += nitems;
  curlRequest->curlResponse.responseBuffer[curlRequest->curlResponse.responseBufferOffset] = 0;
  curlRequest->curlResponse.contentSize = curlRequest->curlResponse.responseBufferOffset;
  if(!curlRequest->curlResponse.responseurl) {
    char *url = NULL;
    curl_easy_getinfo(curlRequest->handle, CURLINFO_EFFECTIVE_URL, &url);
    curlRequest->curlResponse.responseurl= url;
  }
  return size*nitems;
}

size_t CurlNetworking::progressCallbackCurlWrapper(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow) {
  CurlRequest *curlRequest = (CurlRequest *)clientp;
  if(curlRequest){
    std::scoped_lock lock(curlRequest->bufferLock);
    curlRequest->curldelegator.CURLNetworkingProgressCallback(dltotal, dlnow, ultotal, ulnow, curlRequest->curldelegator.delegatorData);
  }
  return 0;
} 
size_t CurlNetworking::headerCallbackCurlWrapper(char* buffer, size_t size, size_t nitems, void* userData) {
  CurlRequest *curlRequest = (CurlRequest *)userData;
  curlRequest->curlResponse.headerBuffer  = (char *) realloc(curlRequest->curlResponse.headerBuffer , curlRequest->curlResponse.headerBufferOffset+nitems+1); 
  RNS_LOG_ASSERT(curlRequest->curlResponse.headerBuffer  , "headerBuffer cannot be null");
  // Each headerInfo line comes as a seperate callback, so append all here
  memcpy(&(curlRequest->curlResponse.headerBuffer[curlRequest->curlResponse.headerBufferOffset]), buffer, nitems);
  curlRequest->curlResponse.headerBufferOffset+= nitems;
  // headerInfo ends with \r and \n
  if(nitems == 2 && buffer[0] == 13 && buffer[1] == 10  ) {
    long response_code = 0;
    char *url = NULL;
    curl_easy_getinfo(curlRequest->handle, CURLINFO_EFFECTIVE_URL, &url);
    curl_easy_getinfo(curlRequest->handle, CURLINFO_RESPONSE_CODE, &response_code);
    curlRequest->curlResponse.headerBuffer[curlRequest->curlResponse.headerBufferOffset] = 0;
    curlRequest->curlResponse.responseurl= url;
    curlRequest->curlResponse.statusCode = response_code;
    curlRequest->curldelegator.CURLNetworkingHeaderCallback(&curlRequest->curlResponse,curlRequest->curldelegator.delegatorData);
  }
  return size*nitems;
}
bool CurlNetworking::sendRequest(CurlRequest *curlRequest, folly::dynamic query) {
  auto headers = query["headers"];
  auto data = query["data"];
  bool status = false;
  CURL *curl = nullptr;
  CURLcode res = CURLE_FAILED_INIT;

  /* get a curl handle */
  curl = curl_easy_init();
  if(curl == NULL) {
    RNS_LOG_ERROR (stderr << "curl_easy_init() failed \n");
    goto safe_return;
  }
  curlRequest->handle = curl;
  curl_easy_setopt(curl, CURLOPT_URL, curlRequest->URL);
  curl_easy_setopt(curl, CURLOPT_PRIVATE,  curlRequest);
  //The following code gets executed for a https connection.
  if(strstr(curlRequest->URL,"https") != NULL) {
    curl_easy_setopt(curl, CURLOPT_SSLENGINE_DEFAULT, 1L);
    curl_easy_setopt(curl, CURLOPT_CAINFO, CA_CERTIFICATE);
    curl_easy_setopt(curl,CURLOPT_SSL_VERIFYPEER,0);
  }

  if(curlRequest->timeout)
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, curlRequest->timeout);

  curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);//Enable curl logs

  // ResponseWrite callback and user data
  if(curlRequest->curldelegator.CURLNetworkingHeaderCallback) {
    curl_easy_setopt(curl, CURLOPT_WRITEHEADER, curlRequest);
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, headerCallbackCurlWrapper);
  }
  if(curlRequest->curldelegator.CURLNetworkingProgressCallback) {
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L); // Enable progress callback
    curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, curlRequest );
    curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, progressCallbackCurlWrapper);
   }
  if(!(strcmp(curlRequest->method, "POST"))) {
    if(preparePostRequest(curlRequest, headers, data) == false)
      goto safe_return;
  } else if(!(strcmp(curlRequest->method,"GET"))) {
      // ResponseWrite callback and user data
      curl_easy_setopt(curl, CURLOPT_WRITEDATA, curlRequest);
      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallbackCurlWrapper);
  } else {
      RNS_LOG_ERROR ("Not supported method\n" << curlRequest->method) ;
      goto safe_return;
  }
  curl_multi_add_handle(curlMultihandle_, curl);
  sem_post(&networkRequestSem_);
  status = true;
  safe_return :
   if(status == false && curlRequest )
     delete curlRequest;
  return status;
} 
bool CurlNetworking::abortRequest(CurlRequest* curlRequest) {
  if(curlRequest->handle) {
    curl_easy_cleanup(curlRequest->handle);
    curlRequest->handle = NULL;
    return true;
  }
  return false;
}

}// namespace react
}//namespace facebook


