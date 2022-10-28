/*
* Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
*
* This source code is licensed under the MIT license found in the
* LICENSE file in the root directory of this source tree.
*/
#include <nopoll.h>

#include "ReactSkia/utils/RnsLog.h"
#include "NopollWebsocket.h"

using namespace std;
namespace facebook {
namespace react {

NopollWebsocket* NopollWebsocket::sharedNopollWebsocket_{nullptr};
std::mutex NopollWebsocket::requestLock_;

NopollWebsocket::NopollWebsocket() {
  ctx_ = nopoll_ctx_new ();
  sem_init(&startDownloadingThread_, 0, 0);
  wsMessageThread_ = std::thread([this]() {
    std::thread t1(&NopollWebsocket::pollThread,this);
    int error_code;
    while (ctx_) {
      // wait for ever
      error_code = nopoll_loop_wait (ctx_, 0);
      if (error_code == -4)
        RNS_LOG_ERROR(" io waiting mechanism, errno=" << errno);
      // recover by just calling io wait engine
      // try to limit recoveries to avoid infinite loop
    }
  });
}

NopollWebsocket* NopollWebsocket::sharedNopollWebsocket() {
  std::lock_guard<std::mutex> lock(requestLock_);
  if(sharedNopollWebsocket_ == nullptr)
    sharedNopollWebsocket_ = new NopollWebsocket();
  return sharedNopollWebsocket_;
}

NopollWebsocket::~NopollWebsocket() {
  nopoll_ctx_unref(ctx_);
  std::lock_guard<std::mutex> lock(requestLock_);
  if(this == sharedNopollWebsocket_)
    sharedNopollWebsocket_ = nullptr;
}

void NopollWebsocket::multiConn(){
  NopollRequest* nopollRequest = nullptr;
  while(true) { // forever
    requestQueueLock_.lock();
    if(RequestQueue_.size() !=0) {
      nopollRequest =RequestQueue_.front();
      if(!nopollRequest) {
        requestQueueLock_.unlock();
        continue;
      }
      RequestQueue_.pop();
    } else {
      threadCountLock_.lock();
      currentWorkerThreadCount_--;
      threadCountLock_.unlock();
      requestQueueLock_.unlock();
      break;
    }
    requestQueueLock_.unlock();
    switch(nopollRequest->messageType){
      case REQUEST_MESSAGE_TYPE_OPEN:
        getConnectNopoll(nopollRequest);
        break;
      case REQUEST_MESSAGE_TYPE_SEND:
        sendNopoll(nopollRequest);
        break;
      case REQUEST_MESSAGE_TYPE_SENDBINARY:
        sendBinaryNopoll(nopollRequest);
        break;
      case REQUEST_MESSAGE_TYPE_PING:
        pingNopoll(nopollRequest);
        break;
      case REQUEST_MESSAGE_TYPE_CLOSE:
        closeNopoll(nopollRequest);
        break;
      default :
        RNS_LOG_DEBUG("messageType undefined");
    }
  }      
}

void  NopollWebsocket::pollThread() {
  while(true) { //forever
    requestQueueLock_.lock();
    if(RequestQueue_.empty()){
      requestQueueLock_.unlock();
      sem_wait(&startDownloadingThread_);
    } else {
      threadCountLock_.lock();
      if(currentWorkerThreadCount_ < MAX_WORKER_THEAD_COUNT) {
        std::thread connectionThreads(&NopollWebsocket::multiConn,this);
        connectionThreads.detach();
        currentWorkerThreadCount_++;
      }
      threadCountLock_.unlock();
      requestQueueLock_.unlock();
      usleep(100); //to avoid cpu utilization
    }
  }
}

void onMessageHandler(noPollCtx * ctx, noPollConn * conn, 
    noPollMsg * msg, noPollPtr user_data) {
  const char * content = (const char *) nopoll_msg_get_payload (msg);
  std::string type;
  noPollOpCode  nopollType = nopoll_msg_opcode(msg);
  switch(nopollType) {
    case NOPOLL_TEXT_FRAME:
      type = "blob";
    case NOPOLL_BINARY_FRAME:
      type = "binary";
    default :
      type = "unKnown";
  }
  NopollRequest* nopollRequest =  (NopollRequest*)user_data;
  if(nopollRequest->nopolldelegator.NOPOLLMessageHandlerCallback)
    nopollRequest->nopolldelegator.NOPOLLMessageHandlerCallback(type, content, nopollRequest->nopolldelegator.delegatorData);
}

std::string * NopollWebsocket::parseUrl(std::string& url) {
// finding the "s://" substring in the url ex: wss://echo.websocket.org:80
  if(url.find("s://") == -1) {
      RNS_LOG_ERROR("websocket url is invalid");
      return NULL;
  }

  std::string* webSocketUrl = new std::string[2];
  std::string delimiter = ":";
  std::string webSocketEndPoint = url.substr(url.find(delimiter)+3,url.size());
  webSocketUrl[WEBSOCKET_URL]  = webSocketEndPoint.substr(0,webSocketEndPoint.find(delimiter));

  webSocketUrl[WEBSOCKET_PORTNO] = webSocketEndPoint.substr(
      webSocketEndPoint.find(delimiter)+1,
      webSocketEndPoint.size());

  return webSocketUrl;
}

void NopollWebsocket::getConnect(NopollRequest* nopollRequest) {
  nopollRequest->messageType = REQUEST_MESSAGE_TYPE_OPEN;
  requestQueueLock_.lock();
  RequestQueue_.push(nopollRequest);
  requestQueueLock_.unlock();
  sem_post(&startDownloadingThread_);
}

void NopollWebsocket::getConnectNopoll(NopollRequest* nopollRequest) {
  if(!nopollRequest)
    return;  
  std::string url = nopollRequest->url;
  std::string* parsedUrl = NopollWebsocket::parseUrl(url);
  if(parsedUrl == NULL) {
    RNS_LOG_ERROR(" parsedUrl is NULL ");
    return;
  }

  if(!ctx_) {
    RNS_LOG_ERROR("nopoll_ctx is NULL ");
    return;
  }

  /* creating a connection 
   * TO DO: NULL/optional arguments has to be verified */
  noPollConn* conn = nopoll_conn_new(ctx_,  parsedUrl[0].c_str() , parsedUrl[1].c_str() ,  NULL , "/", NULL , NULL);
  delete []parsedUrl;
  if(!conn) {
      RNS_LOG_ERROR("websocket connection is NULL");
      return;
  }
  RNS_LOG_DEBUG("waiting until connection is ok");
  if(!nopoll_conn_wait_until_connection_ready (conn, 5)) {
      RNS_LOG_ERROR("websocket connection is not ready");
      nopoll_conn_unref(conn);
      return;
  }
  nopollRequest->conn = conn;
  RNS_LOG_DEBUG("websocket connection sucessss");
  nopoll_conn_set_on_msg(conn,onMessageHandler,nopollRequest);
  if(nopollRequest->nopolldelegator.NOPOLLConnectCallback)
    nopollRequest->nopolldelegator.NOPOLLConnectCallback(nopollRequest->nopolldelegator.delegatorData);
  return;
}

void NopollWebsocket::send(NopollRequest* nopollRequest) {
  nopollRequest->messageType = REQUEST_MESSAGE_TYPE_SEND;
  requestQueueLock_.lock();
  RequestQueue_.push(nopollRequest);
  requestQueueLock_.unlock();
  sem_post(&startDownloadingThread_);
}

void NopollWebsocket::sendNopoll(NopollRequest* nopollRequest) {
  if(!nopollRequest)
    return;
  int result = 0;
  if(nopollRequest->conn != NULL ) {
      result = nopoll_conn_send_text(nopollRequest->conn,  nopollRequest->sendMessageData.c_str(), nopollRequest->sendMessageData.length());
      if (result== nopollRequest->sendMessageData.length()) {
          RNS_LOG_DEBUG("sending data sucessfully");
          return;
      } 
  }
  if(nopollRequest->nopolldelegator.NOPOLLFailureCallback)
    nopollRequest->nopolldelegator.NOPOLLFailureCallback((char*)"sending data is failed", nopollRequest->nopolldelegator.delegatorData);
  return;
}

void NopollWebsocket::close(NopollRequest* nopollRequest) {
  nopollRequest->messageType = REQUEST_MESSAGE_TYPE_CLOSE;
  requestQueueLock_.lock();
  RequestQueue_.push(nopollRequest);
  requestQueueLock_.unlock();
  sem_post(&startDownloadingThread_);
}

void NopollWebsocket::closeNopoll(NopollRequest* nopollRequest) {
  if(!nopollRequest)
    return;
  if(nopollRequest->conn != NULL ) {
    nopoll_conn_close_ext(nopollRequest->conn, nopollRequest->closeRequestCode, nopollRequest->closeReason.c_str(), nopollRequest->closeReason.size());
    if(nopollRequest->nopolldelegator.NOPOLLDisconnectCallback)
      nopollRequest->nopolldelegator.NOPOLLDisconnectCallback(nopollRequest->nopolldelegator.delegatorData);
    return;  
  }
  if(nopollRequest->nopolldelegator.NOPOLLFailureCallback)
    nopollRequest->nopolldelegator.NOPOLLFailureCallback((char*)"close connection is failed", nopollRequest->nopolldelegator.delegatorData);
  return;
}

void NopollWebsocket::ping(NopollRequest* nopollRequest) {
  requestQueueLock_.lock();
  nopollRequest->messageType = REQUEST_MESSAGE_TYPE_PING;
  RequestQueue_.push(nopollRequest);
  requestQueueLock_.unlock();
  sem_post(&startDownloadingThread_);
}

void NopollWebsocket::pingNopoll(NopollRequest* nopollRequest) {
  if(!nopollRequest)
    return;
  if(nopollRequest->conn != NULL ) {
    nopoll_conn_send_ping(nopollRequest->conn);
    return;  
  }
  if(nopollRequest->nopolldelegator.NOPOLLFailureCallback)
    nopollRequest->nopolldelegator.NOPOLLFailureCallback((char*)"ping operation failed", nopollRequest->nopolldelegator.delegatorData);
  return;
}

void NopollWebsocket::sendBinary(NopollRequest* nopollRequest) {
  nopollRequest->messageType = REQUEST_MESSAGE_TYPE_SENDBINARY;
  requestQueueLock_.lock();
  RequestQueue_.push(nopollRequest);
  requestQueueLock_.unlock();
  sem_post(&startDownloadingThread_);
}

void NopollWebsocket::sendBinaryNopoll(NopollRequest* nopollRequest) {
  if(!nopollRequest)
    return;
  int result = 0;
  char webSocketBuffer[B64DECODE_OUT_SAFESIZE(nopollRequest->sendMessageBase64Data.length())];
  int size = B64DECODE_OUT_SAFESIZE(nopollRequest->sendMessageBase64Data.length()); 
  int wsBufferSize = B64DECODE_OUT_SAFESIZE(nopollRequest->sendMessageBase64Data.length());
  if(!nopoll_base64_decode(nopollRequest->sendMessageBase64Data.c_str(), nopollRequest->sendMessageBase64Data.length(),
      webSocketBuffer, &wsBufferSize)) {
      if(nopollRequest->nopolldelegator.NOPOLLFailureCallback)
        nopollRequest->nopolldelegator.NOPOLLFailureCallback((char*)"base64 string error", nopollRequest->nopolldelegator.delegatorData);
    return;
  }
  else if(nopollRequest->conn != NULL) {
    result = nopoll_conn_send_binary(nopollRequest->conn, webSocketBuffer,strlen(webSocketBuffer) );
    if (result == strlen(webSocketBuffer)) {
      RNS_LOG_DEBUG("sending binary data sucessfully");
      return;
    } 
  }
  
  if(nopollRequest->nopolldelegator.NOPOLLFailureCallback)
    nopollRequest->nopolldelegator.NOPOLLFailureCallback((char*)"sending binary data is failed", nopollRequest->nopolldelegator.delegatorData);
  return;
}

}
}