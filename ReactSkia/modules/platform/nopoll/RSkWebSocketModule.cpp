/*  * Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.  
*  * This source code is licensed under the MIT license found in the  
* LICENSE file in the root directory of this source tree.  */

#include "cxxreact/Instance.h"
#include "jsi/JSIDynamic.h"

#include <ReactCommon/TurboModuleUtils.h>
#include "ReactCommon/TurboModule.h"

#include "ReactSkia/utils/RnsLog.h"
#include "ReactSkia/JSITurboModuleManager.h"
#include "RSkWebSocketModule.h"

#define WEBSOCKET_RETURN_SUCESS 0
#define WEBSOCKET_RETURN_FAILURE -1

namespace facebook {
namespace react {

RSkWebSocketModule::RSkWebSocketModule(
    const std::string &name,
    std::shared_ptr<CallInvoker> jsInvoker,
    Instance *bridgeInstance) :  RSkWebSocketModuleBase(name, jsInvoker, bridgeInstance) {

  sharedNopollWebsocket_ = NopollWebsocket::sharedNopollWebsocket();
}

RSkWebSocketModule::~RSkWebSocketModule() {

};

jsi::Value RSkWebSocketModule::getConnect(
  std::string url,
  folly::dynamic protocols,
  folly::dynamic options,
  int socketID)  {
  WebsocketRequest* websocketRequest = new WebsocketRequest;
  websocketRequest->self_ = this;
  websocketRequest->socketID = socketID;
  NopollRequest* nopollRequest =new NopollRequest(strdup(url.c_str()));

  auto MessageHandlerCallback = [&](std::string type, const char* data, void* userData) -> void {
    folly::dynamic parameters = folly::dynamic::object();
    WebsocketRequest * websocketRequest = (WebsocketRequest *)userData;
    parameters["id"]= websocketRequest->socketID;
    parameters["data"]= data;
    parameters["type"]= type;
    sendEventWithName(events_[2], folly::dynamic(parameters) );
  };

  auto connectCallback = [&](void* userData) -> void {
    WebsocketRequest* websocketRequest = (WebsocketRequest*)userData;
    folly::dynamic parameters = folly::dynamic::object();
    parameters["id"] = websocketRequest->socketID;
    connectionListLock_.lock();
    connectionList_[websocketRequest->socketID] = websocketRequest;
    connectionListLock_.unlock();
    sendEventWithName(events_[0], folly::dynamic(parameters)); 
  };

  auto failureCallback = [&](char* message, void* userData) -> void {
    folly::dynamic parameters = folly::dynamic::object();
    WebsocketRequest * websocketRequest = (WebsocketRequest *)userData;
    parameters["id"] = websocketRequest->socketID;
    parameters["message"] = message;
    RNS_LOG_ERROR("error message" << message);
    sendEventWithName(events_[3], folly::dynamic(parameters));
  };

  nopollRequest->nopolldelegator.NOPOLLMessageHandlerCallback = MessageHandlerCallback;
  nopollRequest->nopolldelegator.NOPOLLFailureCallback = failureCallback;
  nopollRequest->nopolldelegator.NOPOLLConnectCallback = connectCallback;
  nopollRequest->nopolldelegator.delegatorData = websocketRequest;
  websocketRequest->nopollRequest_ = nopollRequest;
  sharedNopollWebsocket_->getConnect(websocketRequest->nopollRequest_);
  return jsi::Value();
}

jsi::Value RSkWebSocketModule::getClose(
  int code,
  std::string reason,
  int socketID)  {
  connectionListLock_.lock();
  WebsocketRequest*  websocketRequest =  connectionList_[socketID];
  connectionListLock_.unlock();
  if(websocketRequest == NULL ) {
    RNS_LOG_ERROR ("websocketRequest is not valid \n");
    return jsi::Value();
  }
  websocketRequest->nopollRequest_->closeRequestCode = code;
  websocketRequest->nopollRequest_->closeReason = reason;

  auto disconnectCallback = [&](void* userData) -> void {
      folly::dynamic parameters = folly::dynamic::object();
      WebsocketRequest * websocketRequest = (WebsocketRequest *)userData;
      parameters["id"] = websocketRequest->socketID;
      parameters["code"] = websocketRequest->nopollRequest_->closeRequestCode;
      parameters["reason"] = websocketRequest->nopollRequest_->closeReason;
      connectionListLock_.lock();
      connectionList_.erase(websocketRequest->socketID);
      connectionListLock_.unlock();
      sendEventWithName(events_[1], folly::dynamic(parameters));
  };
  websocketRequest->nopollRequest_->nopolldelegator.NOPOLLDisconnectCallback = disconnectCallback;
  sharedNopollWebsocket_->close(websocketRequest->nopollRequest_);
  return jsi::Value();
}

jsi::Value RSkWebSocketModule::send(
  std::string message,
  int socketID)  {
  connectionListLock_.lock();
  WebsocketRequest*  websocketRequest =  connectionList_[socketID];
  connectionListLock_.unlock();
  if(websocketRequest == NULL ) {
    RNS_LOG_ERROR ("websocketRequest is not valid \n");
    return jsi::Value();
  }
  websocketRequest->nopollRequest_->sendMessageData = message;
  sharedNopollWebsocket_->send(websocketRequest->nopollRequest_);
  return jsi::Value();
}

jsi::Value RSkWebSocketModule::sendBinary(
  std::string base64String,
  int socketID)  {
  connectionListLock_.lock();
  WebsocketRequest*  websocketRequest =  connectionList_[socketID];
  connectionListLock_.unlock();
  if(websocketRequest == NULL ) {
    RNS_LOG_ERROR ("websocketRequest is not valid \n");
    return jsi::Value();
  }
  websocketRequest->nopollRequest_->sendMessageBase64Data = base64String;
  sharedNopollWebsocket_->sendBinary(websocketRequest->nopollRequest_);
  return jsi::Value();
}

jsi::Value RSkWebSocketModule::ping(
  int socketID)  {
  connectionListLock_.lock();
  WebsocketRequest*  websocketRequest =  connectionList_[socketID];
  connectionListLock_.unlock();
  if(websocketRequest == NULL ) {
    RNS_LOG_ERROR ("websocketRequest is not valid \n");
    return jsi::Value();
  }
  sharedNopollWebsocket_->ping(websocketRequest->nopollRequest_);
  return jsi::Value();
}

} // namespace react
} // namespace facebook


