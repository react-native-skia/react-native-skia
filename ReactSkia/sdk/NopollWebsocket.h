/*  * Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.  
*  * This source code is licensed under the MIT license found in the  
* LICENSE file in the root directory of this source tree.  */
#include <nopoll.h>
#include <thread> 
#include <mutex>
#include <semaphore.h>
#include <queue>

#define WEBSOCKET_URL 0
#define WEBSOCKET_PORTNO 1
#define WEBSOCKET_RETURN_SUCESS 0
#define WEBSOCKET_RETURN_FAILURE -1
#define B64DECODE_OUT_SAFESIZE(x) (((x)*3)/4)
#define MAX_WORKER_THEAD_COUNT 5

namespace facebook {
namespace react {
typedef struct Nopolldelegator {
  std::function<void(std::string& , const char* , void*)> NOPOLLMessageHandlerCallback;
  std::function<void(char*, void*)> NOPOLLFailureCallback;
  std::function<void(void*)> NOPOLLConnectCallback;
  std::function<void(void*)> NOPOLLDisconnectCallback;
  void *delegatorData;
}Nopolldelegator;

typedef enum MessageType {
  REQUEST_MESSAGE_TYPE_OPEN =1,
  REQUEST_MESSAGE_TYPE_SEND,
  REQUEST_MESSAGE_TYPE_SENDBINARY,
  REQUEST_MESSAGE_TYPE_PING,
  REQUEST_MESSAGE_TYPE_CLOSE
}MessageType;

class NopollRequest {
 public :
  char* url;
  int closeRequestCode;
  std::string sendMessageData; 
  std::string closeReason;
  std::string sendMessageBase64Data;
  noPollConn* conn;
  MessageType messageType;
  Nopolldelegator nopolldelegator;
  NopollRequest(char* lurl):url(lurl),conn(NULL){};
};

class NopollWebsocket {

public:
 NopollWebsocket();
 ~NopollWebsocket();
 void getConnect(NopollRequest*);
 void send(NopollRequest*);
 void sendBinary(NopollRequest*);
 void ping(NopollRequest*);
 void close(NopollRequest*);
 void getConnectNopoll(NopollRequest*);
 void sendNopoll(NopollRequest*);
 void sendBinaryNopoll(NopollRequest*);
 void pingNopoll(NopollRequest*);
 void closeNopoll(NopollRequest*);
 std::thread wsMessageThread_;
 std::string * parseUrl(std::string& url);
 static NopollWebsocket* sharedNopollWebsocket();
 void multiConn();
 void pollThread();
private:
 noPollCtx* ctx_;
 static NopollWebsocket *sharedNopollWebsocket_;
 static std::mutex requestLock_;
 std::queue<NopollRequest*> RequestQueue_;
 int currentWorkerThreadCount_ = 0;
 std::mutex threadCountLock_;
 std::mutex requestQueueLock_;
 sem_t startDownloadingThread_;  /*semaphore to signal downloading thread to start, once there is something in the list*/

};
}
}
