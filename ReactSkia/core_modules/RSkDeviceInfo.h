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
#include "sdk/NotificationCenter.h"

namespace facebook {
namespace react {

class RSkDeviceInfoModule : public TurboModule {
    public:
      RSkDeviceInfoModule(
             const std::string &name,
             std::shared_ptr<CallInvoker> jsInvoker,
             Instance *bridgeInstance);
      ~RSkDeviceInfoModule();

    private:

        unsigned int navEventId_ = UINT_MAX;

        void handlewindowDimensionEventNotification();
        void sendDeviceEventWithName(std::string eventName, folly::dynamic &&params);
        static folly::dynamic getDimension();

        static jsi::Value getConstants(
            jsi::Runtime &rt,
            TurboModule &turboModule,
            const jsi::Value *args,
            size_t count);
        Instance *bridgeInstance_;

        void startObserving() {};

        void stopObserving() {};

};

}//namespace react
}//namespace facebook
