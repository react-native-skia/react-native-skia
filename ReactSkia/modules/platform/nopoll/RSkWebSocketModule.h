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
            Instance *bridgeInstance) : RSkWebSocketModuleBase(name, 
		         jsInvoker, bridgeInstance){}

  	jsi::Value getConnect(
	    std::string,
            folly::dynamic,
            folly::dynamic,
            int) override;

        jsi::Value getClose(
            int,
            std::string,
            int) override;


  private:
        std::string* parseUrl(std::string&);
        better::map <int , noPollConn*> connectionList_;
        noPollCtx* ctx_;

};
}
}
