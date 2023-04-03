/*
* Copyright (C) 1994-2023 OpenTV, Inc. and Nagravision S.A.
*
* This source code is licensed under the MIT license found in the
* LICENSE file in the root directory of this source tree.
*/
#pragma once
#include <cxxreact/CxxModule.h>
#include "ReactSkia/LegacyNativeModules/RSkBaseEventEmitter.h"
#include "ReactSkia/utils/RnsUtils.h"


namespace facebook {
namespace xplat {
using namespace react;
class NativeEventEmitterModule : public module::CxxModule, public react::RSkBaseEventEmitter{
 public:
  NativeEventEmitterModule(Instance *bridgeInstance);
  ~NativeEventEmitterModule();
  void sendEventWithName(std::string eventName, folly::dynamic &&params, EmitterCompleteVoidCallback completeCallback=nullptr);
  auto getMethods() -> std::vector<Method>;
};
}//xplat
}//facebook
