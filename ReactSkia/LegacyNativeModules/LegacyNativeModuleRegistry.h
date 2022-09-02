/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 * Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "cxxreact/MessageQueueThread.h"
#include "cxxreact/ModuleRegistry.h"
#include "ReactSkia/ComponentViewRegistry.h"
#include "ReactSkia/LegacyNativeModules/LegacyNativeModule.h"

namespace facebook {
namespace react {

class Instance;

class LegacyNativeModuleRegistry : public ModuleRegistry {
 public:
  LegacyNativeModuleRegistry(ComponentViewRegistry *componentViewRegistry,
                              std::weak_ptr<react::Instance> rnInstance,
                              std::shared_ptr<MessageQueueThread> moduleMessageQueue);
  bool ModuleNotFound(std::string moduleName, std::weak_ptr<Instance> rnInstance, std::shared_ptr<MessageQueueThread> moduleMessageQueue);
  xplat::module::CxxModule* moduleForName(std::string moduleName);
 private:
  mutable std::mutex registryMutex_;
  std::unordered_map<std::string, xplat::module::CxxModule*> modules_;
};

} // namespace react
} // namespace facebook
