/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 * Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <memory>
#include <folly/dynamic.h>

#include "ReactSkia/RSkThirdPartyNativeModuleProvider.h"
#include "ReactSkia/LegacyNativeModules/uimanager/UiManagerModule.h"
#include "ReactSkia/utils/RnsLog.h"

#include "LegacyNativeModuleRegistry.h"

namespace facebook {
namespace react {

using namespace xplat::uimanager;

LegacyNativeModuleRegistry::LegacyNativeModuleRegistry(ComponentViewRegistry *componentViewRegistry,
                                                      std::weak_ptr<Instance> rnInstance,
                                                      std::shared_ptr<MessageQueueThread> moduleMessageQueue)
    : ModuleRegistry({},
                    [=](std::string moduleName) -> bool {
                      RNS_LOG_WARN(this << " !!!!! Native Module " << moduleName.c_str() << " Not found !!!!!");
                      return ModuleNotFound(moduleName, rnInstance, moduleMessageQueue);
                    }) {

  std::vector<std::unique_ptr<NativeModule>> modules;
  auto uiMgr = std::make_unique<LegacyNativeModule>("UIManager",
                                                    rnInstance,
                                                    [this, componentViewRegistry]() -> std::unique_ptr<xplat::module::CxxModule> {
                                                      auto module = UimanagerModule::createModule(componentViewRegistry);
                                                      std::lock_guard<std::mutex> lock(registryMutex_);
                                                      modules_["UIManager"] = module.get();
                                                      return module;
                                                    },
                                                    moduleMessageQueue);
  modules.emplace_back(std::move(uiMgr));
  registerModules(std::move(modules));
}

bool LegacyNativeModuleRegistry::ModuleNotFound(std::string moduleName, std::weak_ptr<Instance> rnInstance, std::shared_ptr<MessageQueueThread> moduleMessageQueue) {

  auto createModule = RSkThirdparyNativeModuleCreatorClassWithName(moduleName);
  if(createModule) {
    std::vector<std::unique_ptr<NativeModule>> modules;
    auto nativeModule = std::make_unique<LegacyNativeModule>(moduleName,
                                                            rnInstance,
                                                            [=]() -> std::unique_ptr<xplat::module::CxxModule> {
                                                              std::unique_ptr<xplat::module::CxxModule> module(createModule());
                                                              std::lock_guard<std::mutex> lock(registryMutex_);
                                                              modules_[moduleName.c_str()] = module.get();
                                                              return module;
                                                            },
                                                            moduleMessageQueue);
    modules.emplace_back(std::move(nativeModule));
    registerModules(std::move(modules));
    return true;
  }
  return false;
}

xplat::module::CxxModule* LegacyNativeModuleRegistry::moduleForName(std::string moduleName) {
  std::lock_guard<std::mutex> lock(registryMutex_);
  if (modules_.find(moduleName) != modules_.end()) {
    return modules_[moduleName];
  }
  return nullptr;
}

} // namespace react
} // namespace facebook
