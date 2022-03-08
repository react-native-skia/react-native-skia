/*
* Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
*
* This source code is licensed under the MIT license found in the
* LICENSE file in the root directory of this source tree.
*/
#include "jsi/JSIDynamic.h"

#include "ReactCommon/TurboModule.h"

#include "ReactSkia/utils/RnsLog.h"
#include "ReactSkia/JSITurboModuleManager.h"
#include "RSkDeviceInfo.h"

namespace facebook {
namespace react {

RSkDeviceInfoModule::RSkDeviceInfoModule(
    const std::string &name,
    std::shared_ptr<CallInvoker> jsInvoker,
    Instance *bridgeInstance)
    : TurboModule(name, jsInvoker), bridgeInstance_(bridgeInstance) {
    methodMap_["getConstants"] = MethodMetadata{0, getConstants};
    std::function<void ()> dimensionHandler = std::bind(&RSkDeviceInfoModule::handlewindowDimensionEventNotification, this);  // folly::dynamic
    navEventId_ = NotificationCenter::defaultCenter().addListener("dimensionEventNotification", dimensionHandler);
}

RSkDeviceInfoModule::~RSkDeviceInfoModule(){
    NotificationCenter::defaultCenter().removeListener(navEventId_);
}

jsi::Value RSkDeviceInfoModule::getConstants(
    jsi::Runtime &rt,
    TurboModule &turboModule,
    const jsi::Value *args,
    size_t count)  {

    if (count != 0) {
        return jsi::Value::undefined();
    }

    auto dimension = getDimension();

    return jsi::valueFromDynamic(rt, folly::dynamic::object("Dimensions", std::move(dimension)));
}

void RSkDeviceInfoModule::handlewindowDimensionEventNotification() {

    sendDeviceEventWithName("didUpdateDimensions", getDimension());
}

folly::dynamic RSkDeviceInfoModule::getDimension() {

    SkSize screenSize = RnsShell::PlatformDisplay::sharedDisplay().screenSize();
    SkSize mainWindowSize = RnsShell::Window::getMainWindowSize();
    auto windowMetrics = folly::dynamic::object("width", mainWindowSize.width())("height", mainWindowSize.height())(
      "scale", 1)("fontScale", 1);
    auto screenMetrics = folly::dynamic::object("width", screenSize.width())("height", screenSize.height())(
      "scale", 1)("fontScale", 1);

    return folly::dynamic::object("window", std::move(windowMetrics))(
      "screen", std::move(screenMetrics));

}

void RSkDeviceInfoModule::sendDeviceEventWithName(std::string eventName, folly::dynamic &&params) {
    if (bridgeInstance_ == NULL) {
        LOG(ERROR) << "Turbomodule not initialized with Bridge instance";
    }
    bridgeInstance_->callJSFunction(
        "RCTDeviceEventEmitter", "emit",
        params != NULL ? folly::dynamic::array(folly::dynamic::array(eventName),
        params)
         : folly::dynamic::array(eventName));
}

}// namespace react
}//namespace facebook
