#pragma once

#include "cxxreact/ModuleRegistry.h"

namespace facebook {
namespace react {

class Instance;

class LegacyNativeModuleRegistry : public ModuleRegistry {
 public:
  LegacyNativeModuleRegistry();
};

} // namespace react
} // namespace facebook
