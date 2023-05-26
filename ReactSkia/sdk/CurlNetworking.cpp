/*
* Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
*
* This source code is licensed under the MIT license found in the
* LICENSE file in the root directory of this source tree.
*/
#include <curl/curl.h>
#include <semaphore.h>
#include "ReactSkia/utils/RnsLog.h"
#include "ReactSkia/utils/RnsUtils.h"
#include "CurlNetworking.h"

using namespace std;
namespace facebook {
namespace react {
CurlNetworking* CurlNetworking::sharedCurlNetworking_{nullptr};
std::mutex CurlNetworking::curlInstanceMutex_;
CurlNetworking::CurlNetworking() {
  networkCache_ = new ThreadSafeCache<string,shared_ptr<CurlResponse>>();
  curl_global_init(CURL_GLOBAL_ALL);
  sem_init(&networkRequestSem_, 0, 0);
  curlMultihandle_ = curl_multi_init();
  // we are limiting the number of max number of connection to MAX_TOTAL_CONNECTIONS.
  curl_multi_setopt(curlMultihandle_, CURLMOPT_MAX_TOTAL_CONNECTIONS, (long)MAX_TOTAL_CONNECTIONS);
  // we are limiting the number of connection per host(sever) to MAX_PARALLEL_CONNECTIONS_PER_HOST.
  curl_multi_setopt(curlMultihandle_, CURLMOPT_MAX_HOST_CONNECTIONS, (long)MAX_PARALLEL_CONNECTIONS_PER_HOST);
  multiNetworkThread_ = std::thread([this]() {
    while(!exitLoop_){
      if(curlMultihandle_) {
        sem_wait(&networkRequestSem_);
        processNetworkRequest(curlMultihandle_);
      } else {
          std::this_thread::sleep_for(1000ms);
      }
    }
  });

}

CurlNetworking* CurlNetworking::sharedCurlNetworking() {
  std::lock_guard<std::mutex> lock(curlInstanceMutex_);
  if(sharedCurlNetworking_ == nullptr) {
    sharedCurlNetworking_ = new CurlNetworking();
  }
    return sharedCurlNetworking_;
}

CurlRequest::CurlRequest(CURL *lhandle, std::string lURL, size_t ltimeout, std::string lmethod):
  handle(lhandle),
  URL(lURL),
  timeout(ltimeout),
  method(lmethod),
  curlResponse(make_shared<CurlResponse>())
  {}

CurlRequest::~CurlRequest() {
  if(uploadDataPtr!=NULL) {
    free(uploadDataPtr);
    uploadDataPtr = NULL;
  }
}

CurlNetworking::~CurlNetworking() {
  exitLoop_ = true;
  multiNetworkThread_.join();
  if(curlMultihandle_){
    curl_multi_cleanup(curlMultihandle_);
    curl_global_cleanup();
  }
  std::lock_guard<std::mutex> lock(curlInstanceMutex_);
  if(this == sharedCurlNetworking_)
    sharedCurlNetworking_ = nullptr;
};

bool CurlRequest::shouldCacheData() {
  curlResponse->cacheExpiryTime = DEFAULT_MAX_CACHE_EXPIRY_TIME;
  double responseMaxAgeTime = DEFAULT_MAX_CACHE_EXPIRY_TIME;
  double requestMaxAgeTime = DEFAULT_MAX_CACHE_EXPIRY_TIME;
  // Parse server response headers and retrieve caching details
  auto responseCacheControlData = curlResponse->headerBuffer.find("Cache-Control");
  if(responseCacheControlData != curlResponse->headerBuffer.items().end()) {
    std::string responseCacheControlString = responseCacheControlData->second.asString();
    size_t maxAgePos = responseCacheControlString.find("max-age");
    if(responseCacheControlString.find("no-store") != std::string::npos) return false;
    else if(responseCacheControlString.find("no-cache") != std::string::npos) return false;
    else if(maxAgePos != std::string::npos) {
      char tempStr[responseCacheControlString.length()+1];
      strcpy(tempStr,responseCacheControlString.c_str());
      char *token;
      char *rest = tempStr;
      while ((token = strtok_r(rest, ",", &rest))) {
        std::string tokenStr = token;
        if(tokenStr.find("max-age") != std::string::npos) {
          strtok_r(token, "=", &token);
          responseMaxAgeTime = (double)atoi(token);
          if(responseMaxAgeTime == 0) {
            return false;
          }
          curlResponse->cacheExpiryTime = Timer::getCurrentTimeMSecs() + std::min(std::min(RNS_SECONDS_TO_MILLISECONDS(responseMaxAgeTime),requestMaxAgeTime),static_cast<double>(DEFAULT_MAX_CACHE_EXPIRY_TIME));
          return true;
        }
      }

    }
  }
  return true;
}

void CurlNetworking::processNetworkRequest(CURLM *curlMultiHandle) {
  CURLMcode res = CURLM_OK;
  CURLMsg *msg;
  int stillAlive = 0;
  int msgsLeft = 0;
  RNS_LOG_INFO("calling processNetworkRequest");
  do {
    {
      std::lock_guard<std::mutex> lock(curlInstanceMutex_);
      res = curl_multi_perform(curlMultiHandle, &stillAlive);
    }
    while((msg = curl_multi_info_read(curlMultiHandle, &msgsLeft))) {
      if(msg->msg == CURLMSG_DONE) {
        CURL *curlHandle = msg->easy_handle;
        if(!curlHandle) {// check valid curl handle or not. if invalid break the loop.
          break;
        }
        CurlRequest *curlRequest = nullptr;
        curl_easy_getinfo(curlHandle, CURLINFO_PRIVATE, &curlRequest);
        res == CURLM_OK ?
            (curlRequest->curlResponse->errorResult = "")
            :(curlRequest->curlResponse->errorResult= curl_easy_strerror(msg->data.result));
        (msg->data.result) == CURLE_OPERATION_TIMEDOUT ?
            (curlRequest->curlResponse->responseTimeout = true)
            :(curlRequest->curlResponse->responseTimeout = false);
        curl_multi_remove_handle(curlMultiHandle, curlHandle);
        if(msg->data.result == CURLE_OK && curlRequest->shouldCacheData()) {
          if(networkCache_->isAvailableInCache(curlRequest->URL)) {
            RNS_LOG_DEBUG("Data is already in cache");
          } else {
            double downloadedSize = curlRequest->curlResponse->contentSize+curlRequest->curlResponse->headerBufferSize;
            if(!networkCache_->needEvict(downloadedSize)) {
              networkCache_->setCache(curlRequest->URL, curlRequest->curlResponse, curlRequest->curlResponse->cacheExpiryTime);
            }else {
              // TODO:Need to implement LRU cache
              RNS_LOG_ERROR("Insert data to cache failed... :"<<" file :" << curlRequest->URL);
            }
          }
        }
        if(curlRequest->curldelegator.CURLNetworkingCompletionCallback){
          curlRequest->curldelegator.CURLNetworkingCompletionCallback(curlRequest->curlResponse.get(),curlRequest->curldelegator.delegatorData);
        }
        curl_easy_cleanup(curlHandle);
        curlRequest->handle=NULL;
      } else {
        RNS_LOG_ERROR("Unknown critical error: CURLMsg" << msg->msg);
      }

    }
    if(stillAlive)
      curl_multi_wait(curlMultiHandle, NULL, 0, 1000, NULL);

  } while(stillAlive);

}

size_t CurlNetworking::readCallback(void* ptr, size_t size, size_t nitems, void* userdata) {
  CurlRequest *curlRequest = static_cast<CurlRequest *>(userdata);
  if(!curlRequest) {
    return 0;
  }
  size_t readSize = size * nitems;
  // Calculate the remaining data size to send
  size_t remainingSize = curlRequest->uploadDataLength - curlRequest->uploadBufferOffset;
  if(remainingSize == 0) { // no more data left to copy,stop the current transfer
    return remainingSize;
  }
  // Determine the size to be copied
  size_t copySize = (remainingSize < readSize) ? remainingSize : readSize;
  // Copy the data into the buffer
  memcpy(ptr, curlRequest->uploadDataPtr + curlRequest->uploadBufferOffset, copySize);
  // Update the position for the next read
  curlRequest->uploadBufferOffset += copySize;
  return copySize;
}


bool CurlNetworking::prepareRequest(shared_ptr<CurlRequest> curlRequest, folly::dynamic data, string methodName ) {
  size_t dataSize = 0;
  bool status = false;

  if(methodName.compare("DELETE")) {
    if(!data["string"].empty()) {
      dataSize = data["string"].getString().length();
      curlRequest->uploadDataPtr =(char *) malloc(dataSize);
      strcpy(curlRequest->uploadDataPtr,data["string"].c_str());
    } else if(!data["formData"].empty()) {
      RNS_LOG_NOT_IMPL_MSG("formData");
      return status;
    } else if(!data["blob"].empty()) {
      RNS_LOG_NOT_IMPL_MSG("blob");
      return status;
    } else if(!data["uri"].empty()) {
      RNS_LOG_NOT_IMPL_MSG("uri");
      return status;
    } else if(!data["base64"].empty()) {
      RNS_LOG_NOT_IMPL_MSG("base64");
      return status;
    } else {
      RNS_LOG_ERROR("Unknown Data for Post Request");
      return status;
    }
  }
  if(!(methodName.compare("POST"))) {
    curl_easy_setopt(curlRequest->handle, CURLOPT_POST, 1L);
    /* get verbose debug output please */
    curl_easy_setopt(curlRequest->handle, CURLOPT_POSTFIELDSIZE, (long)dataSize);
    curl_easy_setopt(curlRequest->handle, CURLOPT_COPYPOSTFIELDS, curlRequest->uploadDataPtr);
      // ResponseWrite callback and user data
    curl_easy_setopt(curlRequest->handle, CURLOPT_WRITEDATA, curlRequest.get());
    curl_easy_setopt(curlRequest->handle, CURLOPT_WRITEFUNCTION, writeCallbackCurlWrapper);
  } else if(!(methodName.compare("PUT"))) {
    curlRequest->uploadBufferOffset = 0;
    curlRequest->uploadDataLength = dataSize;
    curl_easy_setopt(curlRequest->handle, CURLOPT_READFUNCTION,readCallback);
    curl_easy_setopt(curlRequest->handle, CURLOPT_PUT, 1L);
    curl_easy_setopt(curlRequest->handle, CURLOPT_URL, curlRequest->URL.c_str());
    curl_easy_setopt(curlRequest->handle, CURLOPT_READDATA, curlRequest.get());
    curl_easy_setopt(curlRequest->handle, CURLOPT_INFILESIZE_LARGE, (curl_off_t)dataSize);

  } else if(!(methodName.compare("PATCH"))) {
    curl_easy_setopt(curlRequest->handle, CURLOPT_URL, curlRequest->URL.c_str());
    curl_easy_setopt(curlRequest->handle, CURLOPT_CUSTOMREQUEST, "PATCH");
      // set the request body data
    curl_easy_setopt(curlRequest->handle, CURLOPT_POSTFIELDS, curlRequest->uploadDataPtr);
    curl_easy_setopt(curlRequest->handle, CURLOPT_WRITEDATA, curlRequest.get());
    curl_easy_setopt(curlRequest->handle, CURLOPT_WRITEFUNCTION, writeCallbackCurlWrapper);

  } else if (!(methodName.compare("DELETE"))) {
    curl_easy_setopt(curlRequest->handle, CURLOPT_CUSTOMREQUEST, "DELETE");
    curl_easy_setopt(curlRequest->handle, CURLOPT_URL,curlRequest->URL.c_str());
  }

  status = true;
  return status;
}

void CurlNetworking::setHeaders(shared_ptr<CurlRequest> curlRequest, folly::dynamic headers) {
  struct curl_slist *curlListRequestHeader = NULL;
  for (auto& pair : headers.items()) {
    /* Key is pair.first, value is pair.second */
    string headerKeyValue = pair.first.c_str();
    const char* parse= ": ";
    headerKeyValue += parse;
    headerKeyValue += pair.second.c_str();
    curlListRequestHeader = curl_slist_append(curlListRequestHeader, headerKeyValue.c_str());
  }
  if(curlListRequestHeader != NULL)
    curl_easy_setopt(curlRequest->handle, CURLOPT_HTTPHEADER, curlListRequestHeader);
}

size_t CurlNetworking::writeCallbackCurlWrapper(void* buffer, size_t size, size_t nitems, void* userData) {
  CurlRequest *curlRequest = (CurlRequest *)userData;
  std::scoped_lock lock(curlRequest->bufferLock);
  curlRequest->curlResponse->responseBuffer  = (char *) realloc(curlRequest->curlResponse->responseBuffer , curlRequest->curlResponse->responseBufferOffset+nitems+1);
  RNS_LOG_ASSERT((curlRequest->curlResponse->responseBuffer), "responseBuffer cannot be null ");
  memcpy(&(curlRequest->curlResponse->responseBuffer [curlRequest->curlResponse->responseBufferOffset]), (char *)buffer, nitems);
  curlRequest->curlResponse->responseBufferOffset += (size*nitems);
  curlRequest->curlResponse->responseBuffer[curlRequest->curlResponse->responseBufferOffset] = 0;
  curlRequest->curlResponse->contentSize = curlRequest->curlResponse->responseBufferOffset;
  if(!curlRequest->curlResponse->responseurl) {
    char *url = NULL;
    curl_easy_getinfo(curlRequest->handle, CURLINFO_EFFECTIVE_URL, &url);
    curlRequest->curlResponse->responseurl= url;
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
  // Each headerInfo line comes as a seperate callback
  std::string str((unsigned char*)buffer, (unsigned char*)buffer + nitems);
  size_t keyEndpos = str.find(": ");
  // Each headerInfo line ends with \r\n character,so we dont consider these characters in our headerData
  size_t valueEndpos = str.find('\r',keyEndpos);
  if (keyEndpos != std::string::npos) {
    curlRequest->curlResponse->headerBuffer[str.substr(0, keyEndpos)] = str.substr(keyEndpos + 2 , valueEndpos-keyEndpos-2);
  }
  // headerInfo ends with \r and \n
  if(nitems == 2 && buffer[0] == 13 && buffer[1] == 10  ) {
    long response_code = 0;
    char *url = NULL;
    curl_easy_getinfo(curlRequest->handle, CURLINFO_EFFECTIVE_URL, &url);
    curl_easy_getinfo(curlRequest->handle, CURLINFO_RESPONSE_CODE, &response_code);
    curlRequest->curlResponse->responseurl= url;
    curlRequest->curlResponse->statusCode = response_code;

#if !defined(GOOGLE_STRIP_LOG) || (GOOGLE_STRIP_LOG <= INFO)
    RNS_LOG_DEBUG("Header buffer content size:" << curlRequest->curlResponse->headerBuffer.size());
    for( auto const &header : curlRequest->curlResponse->headerBuffer.items())
       RNS_LOG_DEBUG("KEY[" << header.first << "] Value["<< header.second << "]");
#endif 
    curlRequest->curldelegator.CURLNetworkingHeaderCallback(curlRequest->curlResponse.get(),curlRequest->curldelegator.delegatorData);
  }
  curlRequest->curlResponse->headerBufferSize += (size*nitems);
  return size*nitems;
}

void CurlNetworking::sendResponseCacheData(shared_ptr<CurlRequest> curlRequest) {
  if(curlRequest->curldelegator.CURLNetworkingHeaderCallback) {
    curlRequest->curldelegator.CURLNetworkingHeaderCallback(curlRequest->curlResponse.get(),curlRequest->curldelegator.delegatorData);
  } else {
    RNS_LOG_DEBUG("CurlNetworking CURLNetworkingHeaderCallback is not available");
  }
  if(curlRequest->curldelegator.CURLNetworkingCompletionCallback) {
    curlRequest->curldelegator.CURLNetworkingCompletionCallback(curlRequest->curlResponse.get(),curlRequest->curldelegator.delegatorData);
  } else {
    RNS_LOG_WARN("CurlNetworking CURLNetworkingCompletionCallback is null");
  }
}

bool CurlNetworking::sendRequest(shared_ptr<CurlRequest> curlRequest, folly::dynamic query) {
  auto headers = query["headers"];
  auto data = query["data"];
  bool status = false;
  int semCount = 0XFFFFFFFF;
  CURL *curl = nullptr;
  CURLcode res = CURLE_FAILED_INIT;
  string methodName= curlRequest->method.c_str();
  auto cacheData = networkCache_->getCacheData(curlRequest->URL);
  if(cacheData.has_value()) {
    curlRequest->curlResponse = cacheData.value();
    if(curlRequest->curlResponse->headerBuffer != nullptr && curlRequest->curlResponse->responseBuffer != nullptr) {
      std::thread responseCacheProviderThread(&CurlNetworking::sendResponseCacheData, this, curlRequest);
      responseCacheProviderThread.detach();
    }
    return true;
  }
  /* get a curl handle */
  curl = curl_easy_init();
  if(curl == NULL) {
    RNS_LOG_ERROR (stderr << "curl_easy_init() failed \n");
    goto safe_return;
  }
  curlRequest->handle = curl;
  curl_easy_setopt(curl, CURLOPT_URL, curlRequest->URL.c_str());
  curl_easy_setopt(curl, CURLOPT_PRIVATE,  curlRequest.get());
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);//Enable HTTP(S) redirects
  curl_easy_setopt(curl, CURLOPT_MAXREDIRS, MAX_URL_REDIRECT);
  //The following code gets executed for a https connection.
  if(strstr(curlRequest->URL.c_str(),"https") != NULL) {
    curl_easy_setopt(curl, CURLOPT_SSLENGINE_DEFAULT, 1L);
    curl_easy_setopt(curl, CURLOPT_CAINFO, CA_CERTIFICATE);
    curl_easy_setopt(curl,CURLOPT_SSL_VERIFYPEER,0);
  }

  if(curlRequest->timeout)
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, curlRequest->timeout);

  curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);//Enable curl logs

  // ResponseWrite callback and user data
  if(curlRequest->curldelegator.CURLNetworkingHeaderCallback) {
    curl_easy_setopt(curl, CURLOPT_WRITEHEADER, curlRequest.get());
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, headerCallbackCurlWrapper);
  }
  if(curlRequest->curldelegator.CURLNetworkingProgressCallback) {
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L); // Enable progress callback
    curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, curlRequest.get() );
    curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, progressCallbackCurlWrapper);
  }
  if(headers != nullptr) {
    setHeaders(curlRequest, headers);
  }
  if((!(strcmp(methodName.c_str(), "POST")))||
     (!(strcmp(methodName.c_str(), "PUT")))||
     (!(strcmp(methodName.c_str(), "PATCH")))||
     (!(strcmp(methodName.c_str(), "DELETE")))) {

    if((data != nullptr) && (prepareRequest(curlRequest, data,methodName) == false))
      goto safe_return;
  } else if(!(strcmp(curlRequest->method.c_str(),"GET"))) {
      // ResponseWrite callback and user data
      curl_easy_setopt(curl, CURLOPT_WRITEDATA, curlRequest.get());
      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallbackCurlWrapper);
  } else {
      RNS_LOG_ERROR ("Not supported method\n" << curlRequest->method) ;
      goto safe_return;
  }
  {
    std::scoped_lock lock(curlInstanceMutex_);
    curl_multi_add_handle(curlMultihandle_, curl);
  }
  sem_getvalue(&networkRequestSem_, &semCount);
  if(!semCount){ //We don't have to send signal if count is already 1. Using this as binary sem
    sem_post(&networkRequestSem_);
  }

  status = true;
  safe_return :
  return status;
} 

bool CurlNetworking::abortRequest(shared_ptr<CurlRequest> curlRequest) {
  //Fix Me abort is called by main thread, Main thread will block untill abort completes. 
  //schedule in different thread. 
  if(curlRequest->handle) {
    std::scoped_lock lock(curlInstanceMutex_);
    // remove the handle from the multihandle and cleanup the curl handle.
    curl_multi_remove_handle(curlMultihandle_, curlRequest->handle);
    curl_easy_cleanup(curlRequest->handle);
    curlRequest->handle = NULL;
    return true;
  }
  return false;
}

}// namespace react
}//namespace facebook
