#pragma once

#include "ReactCommon/TurboModule.h"

#include <unordered_map>

namespace facebook {
namespace react {

class Instance;

class JSITurboModuleManager {
 public:
  JSITurboModuleManager(Instance *bridgeInstance);
  JSITurboModuleManager(JSITurboModuleManager &&) = default;

  TurboModuleProviderFunctionType GetProvider();
  Instance *GetBridge() {
    return bridgeInstance_;
  }

 private:
  Instance *bridgeInstance_;
  std::unordered_map<std::string, std::shared_ptr<TurboModule>> modules_;
};

} // namespace react
} // namespace facebook
