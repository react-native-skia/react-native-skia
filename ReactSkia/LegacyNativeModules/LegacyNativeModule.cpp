/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 * Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "ReactSkia/utils/RnsLog.h"

#include "LegacyNativeModule.h"

// Refer : react-native/ReactCommon/cxxreact/CxxNativeModule.cpp

namespace facebook {
namespace react {

std::string LegacyNativeModule::getName() {
  return name_;
}

std::string LegacyNativeModule::getSyncMethodName(unsigned int reactMethodId) {
  if (reactMethodId >= methods_.size()) {
    throw std::invalid_argument(folly::to<std::string>(
        "methodId ",
        reactMethodId,
        " out of range [0..",
        methods_.size(),
        "]"));
  }
  return methods_[reactMethodId].name;
}

std::vector<MethodDescriptor> LegacyNativeModule::getMethods() {
  lazyInit();

  std::vector<MethodDescriptor> descs;
  for (auto &method : methods_) {
    descs.emplace_back(method.name, method.getType());
  }
  return descs;
}

folly::dynamic LegacyNativeModule::getConstants() {
  lazyInit();

  if (!module_) {
    return nullptr;
  }

  folly::dynamic constants = folly::dynamic::object();
  for (auto &pair : module_->getConstants()) {
    constants.insert(std::move(pair.first), std::move(pair.second));
  }
  return constants;
}

void LegacyNativeModule::invoke(
    unsigned int reactMethodId,
    folly::dynamic &&params,
    int callId) {
  RNS_LOG_NOT_IMPL;
}

MethodCallResult LegacyNativeModule::callSerializableNativeHook(
    unsigned int hookId,
    folly::dynamic &&args) {
  if (hookId >= methods_.size()) {
    throw std::invalid_argument(folly::to<std::string>(
        "methodId ", hookId, " out of range [0..", methods_.size(), "]"));
  }

  const auto &method = methods_[hookId];

  if (!method.syncFunc) {
    throw std::runtime_error(folly::to<std::string>(
        "Method ", method.name, " is asynchronous but invoked synchronously"));
  }

  return method.syncFunc(std::move(args));
}

void LegacyNativeModule::lazyInit() {
  if (module_ || !provider_) {
    return;
  }

  module_ = provider_(); // Should never return null
  provider_ = nullptr;
  if (module_) {
    methods_ = module_->getMethods();
  }
}

} // namespace react
} // namespace facebook
