/*  * Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.  
*  * This source code is licensed under the MIT license found in the  
* LICENSE file in the root directory of this source tree.  */
#include <nopoll.h>
#include "ReactSkia/modules/RSkWebSocketModuleBase.h"

namespace facebook {
namespace react {
class RSkWebSocketModule:  public RSkWebSocketModuleBase {
  public:
	RSkWebSocketModule(
            const std::string &name,
            std::shared_ptr<CallInvoker> jsInvoker,
            Instance *bridgeInstance);
	~ RSkWebSocketModule();

  	jsi::Value getConnect(
	    std::string,
            folly::dynamic,
            folly::dynamic,
            int) override;

    jsi::Value getClose(
            int,
            std::string,
            int) override;

    jsi::Value send(
            std::string,
            int) override;

    jsi::Value sendBinary(
            std::string,
            int) override;

    jsi::Value ping(
            int) override;

    std::thread wsMessageThread_;
    better::map <int , noPollConn*> connectionList_;
    std::vector<std::string> events_ = {"websocketOpen","websocketClosed",
                                    "websocketMessage","websocketFailed"};
  private:
    std::string* parseUrl(std::string&);
    noPollCtx* ctx_;

};
}
}
