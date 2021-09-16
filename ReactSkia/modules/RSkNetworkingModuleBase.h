/*
* Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
*
* This source code is licensed under the MIT license found in the
* LICENSE file in the root directory of this source tree.
*/

#include "cxxreact/Instance.h"
#include "ReactCommon/TurboModule.h"
#include "core_modules/RSkEventEmitter.h"

namespace facebook {
namespace react {

class RSkNetworkingModuleBase: public RSkEventEmitter { 
   public :
     RSkNetworkingModuleBase(
            const std::string &name,
            std::shared_ptr<CallInvoker> jsInvoker,
            Instance *bridgeInstance);

    private:
        static jsi::Value sendRequestWrapper(
            jsi::Runtime &rt,
            TurboModule &turboModule,
            const jsi::Value *args,
            size_t count);

        virtual jsi::Value sendRequest(
            folly::dynamic,
            const jsi::Object&,
            jsi::Runtime&) = 0;

        static jsi::Value abortRequestWrapper(
            jsi::Runtime &rt,
            TurboModule &turboModule,
            const jsi::Value *args,
            size_t count);

        virtual jsi::Value abortRequest(
            folly::dynamic) = 0;

        void startObserving() {};

        void stopObserving() {};



};
}//namespace react
}//namespace facebook
