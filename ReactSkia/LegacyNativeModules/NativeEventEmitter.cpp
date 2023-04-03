/*
* Copyright (C) 1994-2023 OpenTV, Inc. and Nagravision S.A.
*
* This source code is licensed uander the MIT license found in the
* LICENSE file in the root directory of this source tree.
*/
#include <folly/dynamic.h>
#include <cxxreact/Instance.h>
#include <cxxreact/JsArgumentHelpers.h>

#include "NativeEventEmitter.h"

namespace facebook {
namespace xplat {

NativeEventEmitterModule::NativeEventEmitterModule(Instance *bridgeInstance)
  :RSkBaseEventEmitter(bridgeInstance){}

NativeEventEmitterModule::~NativeEventEmitterModule() {
}


auto NativeEventEmitterModule::getMethods() -> std::vector<Method> {
  return {
    Method(
        "addListener",
        [&] (folly::dynamic args) {
          addListener(args[0].asString());
        }),// end of addListener lambda

    Method(
        "removeListeners",
        [&] (folly::dynamic args) {
          removeListeners(args[0].asInt());
        }),
  };
}

void NativeEventEmitterModule::sendEventWithName(std::string eventName, folly::dynamic &&params, EmitterCompleteVoidCallback completeCallback) {
  auto instance = getInstance().lock();
  if(instance){
    SetBridgeInstance(instance.get());
    RSkBaseEventEmitter::sendEventWithName(eventName, folly::dynamic(params),completeCallback);
  }
}



}//xplat
}//facebook
