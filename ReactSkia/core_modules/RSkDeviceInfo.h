/*
* Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
*
* This source code is licensed under the MIT license found in the
* LICENSE file in the root directory of this source tree.
*/

#include "cxxreact/Instance.h"
#include "ReactCommon/TurboModule.h"
#include "core_modules/RSkEventEmitter.h"
#include "rns_shell/common/Window.h"

namespace facebook {
namespace react {

class RSkDeviceInfoModule : public RSkEventEmitter {
    public:
      RSkDeviceInfoModule(
             const std::string &name,
             std::shared_ptr<CallInvoker> jsInvoker,
             Instance *bridgeInstance);
    private:
        static jsi::Value getConstants(
            jsi::Runtime &rt,
            TurboModule &turboModule,
            const jsi::Value *args,
            size_t count);

        void startObserving() {};

        void stopObserving() {};

};

}//namespace react
}//namespace facebook
