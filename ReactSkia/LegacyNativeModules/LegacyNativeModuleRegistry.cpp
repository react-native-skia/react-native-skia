/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 * Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <memory>
#include <folly/dynamic.h>

#include "ReactSkia/utils/RnsLog.h"

#include "ReactSkia/LegacyNativeModules/LegacyNativeModule.h"
#include "ReactSkia/LegacyNativeModules/uimanager/UiManagerModule.h"

#include "LegacyNativeModuleRegistry.h"

namespace facebook {
namespace react {

using namespace xplat::uimanager;

LegacyNativeModuleRegistry::LegacyNativeModuleRegistry(ComponentViewRegistry *componentViewRegistry)
    : ModuleRegistry({},
                    [](std::string moduleName) -> bool {
                      RNS_LOG_WARN("!!!!! Native Module " << moduleName.c_str() << " Not found !!!!!");
                      return false;
                    }) {

  std::vector<std::unique_ptr<NativeModule>> modules;
  auto uiMgr = std::make_unique<LegacyNativeModule>("UIManager",
                                                  [componentViewRegistry]() -> std::unique_ptr<xplat::module::CxxModule> {
                                                    return UimanagerModule::createModule(componentViewRegistry);
                                                  });
  modules.emplace_back(std::move(uiMgr));
  registerModules(std::move(modules));
}

} // namespace react
} // namespace facebook
