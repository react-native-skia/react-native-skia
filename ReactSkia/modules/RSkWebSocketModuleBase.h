/*  * Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.  
*  * This source code is licensed under the MIT license found in the  
* LICENSE file in the root directory of this source tree.  */
#include <butter/map.h>

#include "cxxreact/Instance.h"
#include "ReactCommon/TurboModule.h"
#include "core_modules/RSkEventEmitter.h"

namespace facebook {
namespace react {

class RSkWebSocketModuleBase: public RSkEventEmitter { 
   public :
	RSkWebSocketModuleBase(
            const std::string &name,
            std::shared_ptr<CallInvoker> jsInvoker,
            Instance *bridgeInstance);

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

        static jsi::Value sendWrapper(
            jsi::Runtime &rt,
            TurboModule &turboModule,
            const jsi::Value *args,
            size_t count);

        static jsi::Value sendBinaryWrapper(
            jsi::Runtime &rt,
            TurboModule &turboModule,
            const jsi::Value *args,
            size_t count);

        static jsi::Value pingWrapper(
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

        virtual jsi::Value send(
            std::string,
            int) = 0;

        virtual jsi::Value sendBinary(
            std::string,
            int) = 0;

        virtual jsi::Value ping(
            int) = 0;

        void startObserving() {};

        void stopObserving() {};
  
};
}//namespace react
}//namespace facebook
