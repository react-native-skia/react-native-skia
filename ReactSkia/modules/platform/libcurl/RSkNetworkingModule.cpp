/*
* Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
*
* This source code is licensed under the MIT license found in the
* LICENSE file in the root directory of this source tree.
*/
#include <curl/curl.h>

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
enum curlStatus {CURL_RETURN_FAILURE=-1,CURL_RETURN_SUCESS};
static bool curlInit = false;
using namespace std;
namespace facebook {
namespace react {

RSkNetworkingModule::RSkNetworkingModule(
            const std::string &name,
            std::shared_ptr<CallInvoker> jsInvoker,
            Instance *bridgeInstance) :  RSkNetworkingModuleBase(name, jsInvoker, bridgeInstance) {

}

RSkNetworkingModule::~RSkNetworkingModule() {

};


struct MemoryStruct {
  char *memory;
  size_t size;
};
 
static size_t
WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
  size_t realsize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)userp; 
  char *ptr = (char *)realloc(mem->memory, mem->size + realsize + 1);
  if(!ptr) {
    /* out of memory! */
    printf("not enough memory (realloc returned NULL)\n");
    return 0;
  }
  mem->memory = ptr;
  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;
  return realsize;
}
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
  std::string method = query["method"].getString();
  auto url = query["url"];
  auto headers = query["headers"];
  auto data = query["data"];
  auto responseType  = query["responseType"];
  auto useIncrementalUpdates  = query["useIncrementalUpdates"];
  long timeout = query["timeout"].asDouble();
  auto  withCredentials  = query["withCredentials"];
  CURL *curl;
  CURLcode res = CURLE_FAILED_INIT;
  struct curl_slist *curlListRequestHeader = NULL;
  const char *readptr;
  size_t sizeleft;
  struct MemoryStruct chunk;
  chunk.memory = (char *)malloc(1);  /* will be grown as needed by the realloc above */
  chunk.size = 0;    /* no data at this point */
  uint64_t requestId =0;
  if(data["string"].c_str()) {
      readptr = data["string"].c_str();
      sizeleft = data["string"].getString().length();    
  } else if(data["formData"].c_str()) {
      readptr = data["formData"].c_str();
      sizeleft = data["formData"].getString().length();
  } else {
      readptr = data.c_str();
      sizeleft = data.getString().length();  
  }

  if(!curlInit) {
       res = curl_global_init(CURL_GLOBAL_DEFAULT);
       curlInit = true;
       /* Check for errors */
       if(res != CURLE_OK) {
          RNS_LOG_ERROR (stderr << "curl_global_init() failed: %s\n" <<curl_easy_strerror(res));
          return jsi::Value((int)CURL_RETURN_FAILURE);
       }
  }

  /* get a curl handle */
  curl = curl_easy_init();

  if(curl != NULL) {
        requestId = nextUniqueId();
        if(callbackObj.isFunction(rt)){
            jsi::Function callback = callbackObj.getFunction(rt);
            callback.call(rt, (int) requestId, 1);
        }
        connectionList_[requestId] = curl;
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        /*The following code gets executed for a https connection.*/

        if(strstr(url.c_str(),"https") != NULL) {
             curl_easy_setopt(curl, CURLOPT_SSLENGINE_DEFAULT, 1L);
             curl_easy_setopt(curl, CURLOPT_CAINFO, CA_CERTIFICATE);
             curl_easy_setopt(curl,CURLOPT_SSL_VERIFYPEER,0);
        }
        if(timeout == 0) {
            /* if timeout is not specified, setting default time as 10L */
            curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
        }
        else
            curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);


        if(method.compare("POST") == 0) { 
            curl_easy_setopt(curl, CURLOPT_POST, 1L);
            for (auto& pair : headers.items()) {
                /* Key is pair.first, value is pair.second */
                string headerKeyValue = pair.first.c_str();
                const char* parse= ": ";
                headerKeyValue += parse;
                headerKeyValue += pair.second.c_str();
                curlListRequestHeader = curl_slist_append(curlListRequestHeader, headerKeyValue.c_str());
            }
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, curlListRequestHeader);
            /* get verbose debug output please */
            curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)sizeleft);
            curl_easy_setopt(curl, CURLOPT_COPYPOSTFIELDS, readptr);
        }

        if(method.compare("GET") == 0) {
            /* send all data to this function  */
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

            /* we pass our 'chunk' struct to the callback function */
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
        }
        res = curl_easy_perform(curl);
        /* Check for errors */
        if(res != CURLE_OK) {
            if(res == CURLE_OPERATION_TIMEDOUT) {
                sendEventWithName("didCompleteNetworkResponse", folly::dynamic::array(requestId ,curl_easy_strerror(res), true));
            }else {
                sendEventWithName("didCompleteNetworkResponse", folly::dynamic::array(requestId ,curl_easy_strerror(res), false));
            }
            RNS_LOG_ERROR (stderr << "curl_easy_perform() failed: %s\n" <<curl_easy_strerror(res));
        }else {
            sendEventWithName("didReceiveNetworkData", folly::dynamic::array(requestId ,chunk.memory));
        }
        free(chunk.memory);
  }
 
  return jsi::Value((int)CURL_RETURN_SUCESS);

}

jsi::Value RSkNetworkingModule::abortRequest(
  folly::dynamic requestId) {

 CURL* curl = connectionList_[requestId.asInt()];
 if(curl != NULL) {
     curl_easy_cleanup(curl);
     connectionList_.erase(requestId.asInt());
     curl = NULL;
     sendEventWithName("didCompleteNetworkResponse", folly::dynamic::array(requestId.asInt() ,"Abort connection request succeeded.", false));
     return jsi::Value((int)CURL_RETURN_SUCESS);
 }

 sendEventWithName("didCompleteNetworkResponse", folly::dynamic::array(requestId.asInt() ,"Abort connection request failed.", false));

 return jsi::Value((int)CURL_RETURN_FAILURE);

}
}// namespace react
}//namespace facebook
