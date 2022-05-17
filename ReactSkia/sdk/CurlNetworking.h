/*
* Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
*
* This source code is licensed under the MIT license found in the
* LICENSE file in the root directory of this source tree.
*/
#include <curl/curl.h>
#include <better/map.h>
#include <semaphore.h>
#include <thread>  
#include "jsi/JSIDynamic.h"

#ifndef CA_CERTIFICATE
#define CA_CERTIFICATE       "/etc/ssl/certs/ca-certificates.crt"      /**< The certificate of the CA to establish https connection to the server*/
#endif

#define MAX_PARALLEL_CONNECTION 5

namespace facebook {
namespace react {
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
  folly::dynamic headerBuffer;
  char* responseBuffer;
  int responseBufferOffset;
  int contentSize;
  const char* responseurl;
  int statusCode;
  std::string errorResult;
  bool responseTimeout;
}CurlResponse;

class CurlRequest {
 public:
  CURL *handle;
  const char *URL;
  size_t timeout;
  const char* method;
  Curldelegator curldelegator;
  CurlResponse curlResponse;
  std::mutex bufferLock;
  CurlRequest(CURL *lhandle, const char* lURL,size_t ltimeout,const char* lmethod);
  ~CurlRequest();
};



class CurlNetworking {
 public:
  CurlNetworking();
  ~CurlNetworking();
  bool sendRequest(CurlRequest *curlRequest, folly::dynamic query);
  bool abortRequest(CurlRequest *);
  static void processNetworkRequest(CURLM *cm);
  bool preparePostRequest(CurlRequest *, folly::dynamic, folly::dynamic);
  static CurlNetworking* sharedCurlNetworking();
  static size_t writeCallbackCurlWrapper(void* buffer, size_t size, size_t nitems, void* userData) ;
  static size_t progressCallbackCurlWrapper(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow);
  static size_t headerCallbackCurlWrapper(char* buffer, size_t size, size_t nitems, void* userData);

 private:
  static CurlNetworking *sharedCurlNetworking_;
  sem_t networkRequestSem_; 
  CURLM* curlMultihandle_ = nullptr;
  bool exitLoop_ = false;
  std::thread multiNetworkThread_;
  static std::mutex mutex_;
};
}//namespace react
}//namespace facebook

