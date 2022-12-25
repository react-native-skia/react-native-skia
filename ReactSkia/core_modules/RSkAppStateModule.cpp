/*
 * Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "ReactSkia/core_modules/RSkAppStateModule.h"
#include "ReactSkia/utils/RnsLog.h"

namespace facebook {
namespace react {

RNSP_EXPORT RSkAppStateModule::AppStateCallBackClient::AppStateCallBackClient(RSkAppStateModule& appStateModule)
  : appStateModule_(appStateModule){}

RSkAppStateModule::RSkAppStateModule(
  const std::string &name,
  std::shared_ptr<react::CallInvoker> jsInvoker,
  react::Instance *bridgeInstance)
  : RSkEventEmitter(name, jsInvoker, bridgeInstance),
    pluginFactory_(new RnsPluginFactory),
    appStateCallBackClient_(*this) {
  methodMap_["getConstants"] = MethodMetadata{0, getConstantsWrapper};
  methodMap_["getCurrentAppState"] = MethodMetadata{2, getCurrentAppStateWrapper};
}

jsi::Value RSkAppStateModule::getConstantsWrapper(
  jsi::Runtime &rt,
  TurboModule &turboModule,
  const jsi::Value *args,
  size_t count) {

  if (count != 0) {
    return jsi::Value::undefined();
  }
  auto &self = static_cast<RSkAppStateModule &>(turboModule);
  return self.getConstants(rt);
}

jsi::Value RSkAppStateModule::getCurrentAppStateWrapper(
  jsi::Runtime &rt,
  TurboModule &turboModule,
  const jsi::Value *args,
  size_t count) {

  if (count != 2) {
    return jsi::Value::undefined();
  }
  auto &self = static_cast<RSkAppStateModule &>(turboModule);

  return self.getCurrentAppState(rt, args[0].getObject(rt), args[1].getObject(rt));
}

void RSkAppStateModule::lazyInit() {
  if(appManagerHandle_ == nullptr) {
    RNS_LOG_INFO("Creating app handle from Plugin Factory");
    appManagerHandle_ = pluginFactory_->createAppManagerHandle(appStateCallBackClient_);
    if(appManagerHandle_ == nullptr) {
      RNS_LOG_ERROR("Could not get AppState handle from RNS platform Plugin");
    } else {
      RNS_LOG_DEBUG(this << " : RNS AppState Plugin Loaded with AppState interface : " << appManagerHandle_.get() << " : Thread : " << std::this_thread::get_id());
    }
  }
}

jsi::Value RSkAppStateModule::getConstants(jsi::Runtime &rt) {
  lazyInit();
  return jsi::valueFromDynamic(rt, folly::dynamic::array("initialAppState", "active"));
}

jsi::Value RSkAppStateModule::getCurrentAppState(jsi::Runtime &rt, const jsi::Object &successObj, const jsi::Object &errorObj) {
  lazyInit();
  if(successObj.isFunction(rt)) {
    jsi::Function callback = successObj.getFunction(rt);
    folly::dynamic appState = folly::dynamic::object();
    appState["app_state"] = (appManagerHandle_) ? appManagerHandle_->currentState() : "unknown";
    RNS_LOG_INFO("getCurrentAppState : " << appState["app_state"].asString());
    callback.call(rt, jsi::valueFromDynamic(rt, appState));
  }
  return jsi::Value::undefined();
}

void RSkAppStateModule::startObserving() {}
void RSkAppStateModule::stopObserving() {}

void RSkAppStateModule::AppStateCallBackClient::onChange(string newAppState) {
  folly::dynamic appState = folly::dynamic::object();
  appState["app_state"] = newAppState;
  RNS_LOG_INFO("onChangeEventReceived newAppState : " << appState["app_state"]);
  appStateModule_.sendEventWithName("appStateDidChange", folly::dynamic(appState));
}

void RSkAppStateModule::AppStateCallBackClient::onMemoryWarning() {
  RNS_LOG_INFO("onMemoryWarningEventReceived");
  appStateModule_.sendEventWithName("memoryWarning", folly::dynamic());
}

void RSkAppStateModule::AppStateCallBackClient::onFocus() {
  RNS_LOG_INFO("onFocusEventReceived");
  appStateModule_.sendEventWithName("appStateFocusChange", folly::dynamic(true));
}

void RSkAppStateModule::AppStateCallBackClient::onBlur() {
  RNS_LOG_INFO("onBlurEventReceived");
  appStateModule_.sendEventWithName("appStateFocusChange", folly::dynamic(false));
}

} // react
} // facebook

