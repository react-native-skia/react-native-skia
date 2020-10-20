#include "ReactSkia/LegacyNativeModuleRegistry.h"

#include <folly/dynamic.h>
#include <glog/logging.h>
#include <memory>

namespace facebook {
namespace react {

class LegacyUIManagerModule : public NativeModule {
 public:
  LegacyUIManagerModule() = default;

  std::string getName() override {
    return "UIManager";
  }
  std::string getSyncMethodName(unsigned int methodId) override {
    if (methodId == 0) {
      return "getConstantsForViewManager";
    }
    throw std::invalid_argument("Invalid methodId");
  }

  std::vector<MethodDescriptor> getMethods() override {
    return {{"getConstantsForViewManager", "sync"}};
  }

  folly::dynamic getConstants() override {
    return folly::dynamic::object();
  }

  void invoke(unsigned int reactMethodId, folly::dynamic &&params, int callId)
      override {}

  MethodCallResult callSerializableNativeHook(
      unsigned int reactMethodId,
      folly::dynamic &&args) override {
    if (reactMethodId == 0) {
      if (args[0] == "RCTView") {
        auto nativeProps = folly::dynamic::object("onLayout", true);
        auto registry =
            folly::dynamic::object("NativeProps", std::move(nativeProps));
        return {std::move(registry)};
      }
      throw std::invalid_argument(
          "Invalid getConstantsForViewManager viewManagerName");
    }
    throw std::invalid_argument("Invalid methodId");
  }
};

LegacyNativeModuleRegistry::LegacyNativeModuleRegistry()
    : ModuleRegistry({}, nullptr) {
  auto u = std::make_unique<LegacyUIManagerModule>();
  std::vector<std::unique_ptr<NativeModule>> modules;
  modules.emplace_back(std::move(u));
  registerModules(std::move(modules));
}

} // namespace react
} // namespace facebook
