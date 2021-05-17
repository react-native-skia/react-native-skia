/*  * Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.  
*  * This source code is licensed under the MIT license found in the  
* LICENSE file in the root directory of this source tree.  */
#include <nopoll.h>
#include <better/map.h>

#include "cxxreact/Instance.h"
#include "ReactCommon/TurboModule.h"

namespace facebook {
namespace react {

class RSkWebSocketModuleBase: public TurboModule { 
   public :
	RSkWebSocketModuleBase(
            const std::string &name,
            std::shared_ptr<CallInvoker> jsInvoker,
            Instance *bridgeInstance);

        static jsi::Value NoOp(
            jsi::Runtime &rt,
            TurboModule &turboModule,
            const jsi::Value *args,
            size_t count) {
            return jsi::Value::undefined();
        }
    private:
        static jsi::Value getConnectWrapper(
            jsi::Runtime &rt,
            TurboModule &turboModule,
            const jsi::Value *args,
            size_t count);

	static jsi::Value getCloseWrapper(
            jsi::Runtime &rt,
            TurboModule &turboModule,
            const jsi::Value *args,
            size_t count);

	virtual jsi::Value getConnect(
            std::string,
            folly::dynamic,
            folly::dynamic,
            int) = 0;

	virtual jsi::Value getClose(
            int,
            std::string,
            int) = 0;

  
};
}//namespace react
}//namespace facebook
