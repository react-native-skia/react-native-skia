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
        auto nativeProps = folly::dynamic::object("onLayout", true)
          // NOTE(kudo): current react-native still partially relies on legacy UIManager to get supported view props.
          // That's why we have to add the props here.
          // In newer react-native should overcome this problem.
          ("isSkiaProp", true);
        auto directEventTypes = folly::dynamic::object(
            "topLayout",
            folly::dynamic::object("registrationName", "onLayout"));
        auto registry = folly::dynamic::object(
            "NativeProps", std::move(nativeProps))(
            "bubblingEventTypes", folly::dynamic::object())(
            "directEventTypes", std::move(directEventTypes));
        return {std::move(registry)};
      } else if (args[0] == "RCTImageView") {
        // NOTE(kudo): The ImageView config setup has two cases
        // 1. From JS typing (only happens in bridgeless mode)
        // 2. From native UIManager (the traditional way)
        // Since we don't use `global.RN$Bridgeless` right now, we should use
        // the second one to setup config. See ImageViewNativeComponent.js for
        // more details.
        auto nativeProps = folly::dynamic::object("blurRadius", true)(
            "defaultSrc", true)("fadeDuration", true)("headers", true)(
            "loadingIndicatorSrc", true)("onError", true)("onLoad", true)(
            "onLoadEnd", true)("onLoadStart", true)("onPartialLoad", true)(
            "onProgress", true)("progressiveRenderingEnabled", true)(
            "resizeMethod", true)("resizeMode", true)(
            "shouldNotifyLoadEvents", true)("source", true)("src", true)
            // NOTE(kudo): Trick to leverage ios type transform for UIEdgeInsets
            // to insetsDiffer
            ("capInsets", "UIEdgeInsets")
            // NOTE(kudo): Trick to leverage ios type transform for UIColor to
            // processColor
            ("overlayColor", "UIColor")("tintColor", "UIColor")
            // NOTE(kudo): Trick to leverage ios type transform for UIImage to
            // resolveAssetSource
            ("defaultSource", "UIImage");

        auto directEventTypes = folly::dynamic::object(
            "topLoadStart",
            folly::dynamic::object("registrationName", "onLoadStart"))(
            "topProgress",
            folly::dynamic::object("registrationName", "onProgress"))(
            "topError", folly::dynamic::object("registrationName", "onError"))(
            "topPartialLoad",
            folly::dynamic::object("registrationName", "onPartialLoad"))(
            "topLoad", folly::dynamic::object("registrationName", "onLoad"))(
            "topLoadEnd",
            folly::dynamic::object("registrationName", "onLoadEnd"));

        auto registry = folly::dynamic::object(
            "NativeProps", std::move(nativeProps))("baseModuleName", "RCTView")(
            "bubblingEventTypes", folly::dynamic::object())(
            "directEventTypes", std::move(directEventTypes));
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
