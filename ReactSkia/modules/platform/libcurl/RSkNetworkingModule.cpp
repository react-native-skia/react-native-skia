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
// TODO: this data response will be sent in load event
  return realsize;
}



jsi::Value RSkNetworkingModule::sendRequest(
  folly::dynamic query) {
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
          return 1;
       }
  }

  /* get a curl handle */
  curl = curl_easy_init();

    if(curl != NULL) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        /*The following code gets executed for a https connection.*/

        if(strstr(url.c_str(),"https") != NULL) {
             curl_easy_setopt(curl, CURLOPT_SSLENGINE_DEFAULT, 1L);
             curl_easy_setopt(curl, CURLOPT_CAINFO, CA_CERTIFICATE);
             curl_easy_setopt(curl,CURLOPT_SSL_VERIFYPEER,1);
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
        if(res != CURLE_OK)
            RNS_LOG_ERROR (stderr << "curl_easy_perform() failed: %s\n" <<curl_easy_strerror(res));
        // TODO:: clean to be done in abort, once implemented
        curl_easy_cleanup(curl);
        free(chunk.memory);

  }
 
  return jsi::Value::undefined();


}
}// namespace react
}//namespace facebook
