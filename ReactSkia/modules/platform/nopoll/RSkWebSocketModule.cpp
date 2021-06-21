/*  * Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.  
*  * This source code is licensed under the MIT license found in the  
* LICENSE file in the root directory of this source tree.  */
#include <nopoll.h>

#include "cxxreact/Instance.h"
#include "jsi/JSIDynamic.h"

#include <ReactCommon/TurboModuleUtils.h>
#include "ReactCommon/TurboModule.h"

#include "ReactSkia/utils/RnsLog.h"
#include "ReactSkia/JSITurboModuleManager.h"
#include "RSkWebSocketModule.h"

#define WEBSOCKET_URL 0
#define WEBSOCKET_PORTNO 1
#define WEBSOCKET_RETURN_SUCESS 0
#define WEBSOCKET_RETURN_FAILURE -1
#define B64DECODE_OUT_SAFESIZE(x) (((x)*3)/4)

namespace facebook {
namespace react {

RSkWebSocketModule::RSkWebSocketModule(
            const std::string &name,
            std::shared_ptr<CallInvoker> jsInvoker,
            Instance *bridgeInstance) :  RSkWebSocketModuleBase(name, jsInvoker, bridgeInstance) {

  ctx_ = nopoll_ctx_new ();
  wsMessageThread_ = std::thread([this]() {
    while (ctx_) {
      // wait for ever
      int error_code = nopoll_loop_wait (ctx_, 0);
      if (error_code == -4)
        RNS_LOG_ERROR(" io waiting mechanism, errno=" << errno);
      // recover by just calling io wait engine
      // try to limit recoveries to avoid infinite loop
      continue;
    }
  });

}

RSkWebSocketModule::~RSkWebSocketModule() {

  nopoll_ctx_unref(ctx_);

};
void onMessageHandler(noPollCtx * ctx, noPollConn * conn, 
		noPollMsg * msg, noPollPtr user_data) {

  int socketID = 0;
  const char * content = (const char *) nopoll_msg_get_payload (msg);
  int type = nopoll_msg_opcode(msg);
  RSkWebSocketModule * wsObject = (RSkWebSocketModule *)user_data;
  folly::dynamic parameters = folly::dynamic::object();
  for (const auto &connList : wsObject->connectionList_) {
    if(connList.second == conn)
      socketID = connList.first;
  }
  parameters["id"]=socketID;
  parameters["data"]= content;
  parameters["type"]= type;
  wsObject->sendEventWithName(wsObject->events_[2], folly::dynamic(parameters) );
}


std::string * RSkWebSocketModule::parseUrl(std::string& url) {
// finding the "s://" substring in the url ex: wss://echo.websocket.org:80
  if(url.find("s://") == -1) {
      RNS_LOG_ERROR("websocket url is invalid");
      return NULL;
  }

  std::string* webSocketUrl = new std::string[2];
  std::string delimiter = ":";
  std::string webSocketEndPoint = url.substr(url.find(delimiter)+3,url.size());
  webSocketUrl[WEBSOCKET_URL]  = webSocketEndPoint.substr(0,
		 		   webSocketEndPoint.find(delimiter));

  webSocketUrl[WEBSOCKET_PORTNO] = webSocketEndPoint.substr(
		                     webSocketEndPoint.find(delimiter)+1,
				     webSocketEndPoint.size());

  return webSocketUrl;
}

jsi::Value RSkWebSocketModule::getConnect(
  std::string url,
  folly::dynamic protocols,
  folly::dynamic options,
  int socketID)  {
  folly::dynamic parameters = folly::dynamic::object();
  std::string* parsedUrl = RSkWebSocketModule::parseUrl(url);
  if(parsedUrl == NULL) {
      RNS_LOG_ERROR(" parsedUrl is NULL ");
      return jsi::Value(WEBSOCKET_RETURN_FAILURE);
  }

  if(!ctx_) {
      RNS_LOG_ERROR("nopoll_ctx is NULL ");
      return jsi::Value(WEBSOCKET_RETURN_FAILURE);
  }

  /* creating a connection 
   * TO DO: NULL/optional arguments has to be verified */
  noPollConn* conn = nopoll_conn_new(ctx_,  parsedUrl[0].c_str() , parsedUrl[1].c_str() ,  NULL , "/", NULL , NULL);
  delete []parsedUrl;
  parameters["id"] = socketID;
  if(conn == NULL) {
      RNS_LOG_ERROR("websocket connection is NULL");
      return jsi::Value(WEBSOCKET_RETURN_FAILURE);
  }

  RNS_LOG_INFO("waiting until connection is ok");
  if(!nopoll_conn_wait_until_connection_ready (conn, 5)) {
      RNS_LOG_ERROR("websocket connection is not ready");
      nopoll_conn_unref(conn);
      return jsi::Value(WEBSOCKET_RETURN_FAILURE);
  }

  RNS_LOG_INFO("websocket connection sucess");
  nopoll_conn_set_on_msg(conn,onMessageHandler,this);

  connectionList_[socketID] = conn;

  sendEventWithName(events_[0], folly::dynamic(parameters)); 
  
  return jsi::Value(WEBSOCKET_RETURN_SUCESS);
}



jsi::Value RSkWebSocketModule::getClose(
  int code,
  std::string reason,
  int socketID)  {
  noPollConn* conn =  connectionList_[socketID];
	folly::dynamic parameters = folly::dynamic::object();
  connectionList_[socketID] = conn;
  int getCode = nopoll_conn_get_close_status(conn);
	if(conn != NULL) {
	    parameters["id"] = socketID;
	    parameters["code"] = getCode;
	    parameters["reason"] = reason; 	    
	    nopoll_conn_close_ext(conn, code, reason.c_str(), reason.size());
	    connectionList_.erase(socketID);
	    sendEventWithName(events_[1], folly::dynamic(parameters));
	    return jsi::Value(WEBSOCKET_RETURN_SUCESS);
	}
  parameters = folly::dynamic::object();
  parameters["id"] = socketID;
  parameters["message"] = "close connection is failed";
  sendEventWithName(events_[3], folly::dynamic(parameters));
  RNS_LOG_ERROR("close connection is failed");
  return jsi::Value(WEBSOCKET_RETURN_FAILURE);
}

jsi::Value RSkWebSocketModule::send(
  std::string message,
  int socketID)  {
	int result = 0;
	folly::dynamic parameters = folly::dynamic::object();
	noPollConn* conn =  connectionList_[socketID];
  if(conn != NULL ) {
      result = nopoll_conn_send_text(conn,  message.c_str(), message.length());
      if (result== message.length()) {
          RNS_LOG_INFO("sending data sucessfully");
          return jsi::Value(WEBSOCKET_RETURN_SUCESS);
      } 
    }
	        
  parameters["id"] = socketID;
  parameters["message"] = "sending data is failed";
  sendEventWithName(events_[3], folly::dynamic(parameters));
  RNS_LOG_ERROR("sending data is failed");
  return jsi::Value(WEBSOCKET_RETURN_FAILURE);

}

jsi::Value RSkWebSocketModule::sendBinary(
  std::string base64String,
  int socketID)  {
	int result = 0;
	folly::dynamic parameters = folly::dynamic::object();
	char webSocketBuffer[B64DECODE_OUT_SAFESIZE(base64String.length())];
	int size = B64DECODE_OUT_SAFESIZE(base64String.length()); 
	int wsBufferSize = B64DECODE_OUT_SAFESIZE(base64String.length());
	noPollConn* conn =  connectionList_[socketID];
	if(!nopoll_base64_decode(base64String.c_str(), base64String.length(),
		       	webSocketBuffer, &wsBufferSize)) {
	    parameters = folly::dynamic::object();
	    parameters["id"] = socketID;
	    parameters["message"] = "base64 string error";
	    sendEventWithName(events_[3], folly::dynamic(parameters));
	    RNS_LOG_ERROR("base64 string error");
	    return jsi::Value(WEBSOCKET_RETURN_FAILURE);

	}
	else if(conn != NULL ) {
	    result = nopoll_conn_send_binary(conn, webSocketBuffer,strlen(webSocketBuffer) );
	    if (result == strlen(webSocketBuffer)) {
                RNS_LOG_INFO("sending binary data sucessfully");
		return jsi::Value(WEBSOCKET_RETURN_SUCESS);
	    } 
	        
	}
  parameters = folly::dynamic::object();
  parameters["id"] = socketID;
  parameters["message"] = "sending binary data is failed";
  sendEventWithName(events_[3], folly::dynamic(parameters));
  RNS_LOG_ERROR("sending binary data is failed");
  return jsi::Value(WEBSOCKET_RETURN_FAILURE);


}

jsi::Value RSkWebSocketModule::ping(
  int socketID)  {
  noPollConn* conn =  connectionList_[socketID];
  folly::dynamic parameters = folly::dynamic::object();
  if(conn != NULL ) {
      if(nopoll_conn_send_ping(conn)) {
          RNS_LOG_INFO("ping operation sucess");
	        return jsi::Value(WEBSOCKET_RETURN_SUCESS);
	    }

  }

  parameters["id"] = socketID;
  parameters["message"] = "conn is null";
  sendEventWithName(events_[3], folly::dynamic(parameters));
  RNS_LOG_ERROR("ping operation failed");
  return jsi::Value(WEBSOCKET_RETURN_FAILURE);

}

} // namespace react
} // namespace facebook


