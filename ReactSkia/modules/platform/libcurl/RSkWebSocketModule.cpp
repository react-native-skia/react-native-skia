/*
 * Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
 * Copyright (C) Kudo Chien.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "cxxreact/Instance.h"
#include "jsi/JSIDynamic.h"

#include <ReactCommon/TurboModuleUtils.h>
#include "ReactCommon/TurboModule.h"

#include "RSkWebSocketModule.h"
#include "ReactSkia/JSITurboModuleManager.h"
#include "ReactSkia/utils/RnsLog.h"

namespace facebook {
namespace react {

RSkWebSocketModule::RSkWebSocketModule(
    const std::string &name,
    std::shared_ptr<CallInvoker> jsInvoker,
    Instance *bridgeInstance)
    : RSkWebSocketModuleBase(name, jsInvoker, bridgeInstance) {}

RSkWebSocketModule::~RSkWebSocketModule() {}

jsi::Value RSkWebSocketModule::getConnect(
    std::string url,
    folly::dynamic protocols,
    folly::dynamic options,
    int socketID) {
  return jsi::Value();
}

jsi::Value
RSkWebSocketModule::getClose(int code, std::string reason, int socketID) {
  return jsi::Value();
}

jsi::Value RSkWebSocketModule::send(std::string message, int socketID) {
  return jsi::Value();
}

jsi::Value RSkWebSocketModule::sendBinary(
    std::string base64String,
    int socketID) {
  return jsi::Value();
}

jsi::Value RSkWebSocketModule::ping(int socketID) {
  return jsi::Value();
}

} // namespace react
} // namespace facebook
