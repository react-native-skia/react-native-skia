/*
* Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
*
* This source code is licensed under the MIT license found in the
* LICENSE file in the root directory of this source tree.
*/
#pragma once
#include <curl/curl.h>
#include <better/map.h>
#include <semaphore.h>
#include <thread>  
#include "jsi/JSIDynamic.h"
#include "ThreadSafeCache.h"
#define DEFAULT_MAX_CACHE_EXPIRY_TIME 1800000 // 30mins in seconds 1800000
#define CURRENT_CACHE_SIZE 10 // in megabyte
#define TOTAL_MAX_CACHE_SIZE 15 // in megabyte
#define MAX_URL_REDIRECT 10L // maximum number of redirects allowed

#ifndef CA_CERTIFICATE
#define CA_CERTIFICATE       "/etc/ssl/certs/ca-certificates.crt"      /**< The certificate of the CA to establish https connection to the server*/
#endif

#define MAX_PARALLEL_CONNECTION 5

namespace facebook {
namespace react {
using namespace rns::sdk;
typedef struct Curldelegator {
  std::function<size_t(double, double, double, double, void*)> CURLNetworkingProgressCallback;
  std::function<size_t(void*,void*)> CURLNetworkingHeaderCallback;
  std::function<bool(void*,void*)> CURLNetworkingCompletionCallback;
  std::function<bool(void*,void*)> CURLNetworkingWriteCallback;
  void *delegatorData;
}Curldelegator;

typedef struct CurlResponse {
  CurlResponse()
   :responseBuffer(nullptr),
    responseBufferOffset(0),
    contentSize(0),
    responseurl(nullptr){
    headerBuffer = folly::dynamic::object();
  }
  ~CurlResponse() {
    if(responseBuffer) {
      free(responseBuffer);
      responseBuffer = NULL;
    }
  }
  folly::dynamic headerBuffer;
  char* responseBuffer;
  int responseBufferOffset;
  int contentSize;
  const char* responseurl;
  int statusCode;
  std::string errorResult;
  bool responseTimeout;
  double cacheExpiryTime{DEFAULT_MAX_CACHE_EXPIRY_TIME};
}CurlResponse;

class CurlRequest {
 public:
  CURL *handle;
  std::string URL;
  size_t timeout;
  std::string method;
  Curldelegator curldelegator;
  shared_ptr<CurlResponse> curlResponse;
  std::mutex bufferLock;
  inline bool shouldCacheData();
  CurlRequest(CURL *lhandle, std::string lURL, size_t ltimeout, std::string lmethod);
};



class CurlNetworking {
 public:
  CurlNetworking();
  ~CurlNetworking();
  bool sendRequest(shared_ptr<CurlRequest> curlRequest, folly::dynamic query);
  bool abortRequest(shared_ptr<CurlRequest> curlRequest);
  void processNetworkRequest(CURLM *cm);
  bool preparePostRequest(shared_ptr<CurlRequest> curlRequest, folly::dynamic data);
  static CurlNetworking* sharedCurlNetworking();
  static size_t writeCallbackCurlWrapper(void* buffer, size_t size, size_t nitems, void* userData) ;
  static size_t progressCallbackCurlWrapper(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow);
  static size_t headerCallbackCurlWrapper(char* buffer, size_t size, size_t nitems, void* userData);
  void sendResponseCacheData(shared_ptr<CurlRequest> curlRequest);
  void setHeaders(shared_ptr<CurlRequest> curlRequest, folly::dynamic headers);

 private:
  static CurlNetworking *sharedCurlNetworking_;
  ThreadSafeCache<std::string, shared_ptr<CurlResponse> >*  networkCache_;
  sem_t networkRequestSem_; 
  CURLM* curlMultihandle_ = nullptr;
  bool exitLoop_ = false;
  std::thread multiNetworkThread_;
  static std::mutex mutex_;
};
}//namespace react
}//namespace facebook

