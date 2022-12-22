/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 * Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>
#include <vector>

#include <cxxreact/CxxModule.h>
#include "ReactSkia/ComponentViewRegistry.h"

#define BUBBLING_EVENTS_KEY "bubblingEventTypes"
#define DIRECT_EVENTS_KEY   "directEventTypes"
#define CONSTANTS_KEY       "Constants"
#define COMMANDS_KEY        "Commands"
#define NATIVE_PROPS_KEY    "NativeProps"
#define BASE_MODULE_NAME_KEY    "baseModuleName"

namespace facebook {
namespace xplat {
namespace uimanager {

class Uimanager {
 public:
  Uimanager(ComponentViewRegistry *componentViewRegistry)
    : componentViewRegistry_(componentViewRegistry) {}
  folly::dynamic getConstantsForViewManager(std::string viewManagerName);
  void updateView(int tag, std::string viewManagerName, folly::dynamic props);

  // To be Generated using codeGen ??
  folly::dynamic getConstantsForThirdpartyViewManager(std::string viewManagerName);

 private:
  ComponentViewRegistry *componentViewRegistry_{nullptr};
};

class UimanagerModule : public module::CxxModule {
 public:
  UimanagerModule(std::unique_ptr<Uimanager> uimanager);
  static std::unique_ptr<xplat::module::CxxModule> createModule(ComponentViewRegistry *componentViewRegistry);

  virtual auto getConstants() -> std::map<std::string, folly::dynamic>;
  virtual auto getMethods() -> std::vector<Method>;

  std::string getName();

  void updateViewForReactTag(int viewTag, folly::dynamic newViewProps);
 private:
  std::unique_ptr<Uimanager> uimanager_;
};

} // namespace uimanager
} // namespace xplat
} // namespace facebook
