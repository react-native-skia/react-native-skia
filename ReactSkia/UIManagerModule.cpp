#include "ReactSkia/UIManagerModule.h"

#include "jsi/JSIDynamic.h"

#include <glog/logging.h>

namespace facebook {
namespace react {

UIManagerModule::UIManagerModule(
    const std::string &name,
    std::shared_ptr<CallInvoker> jsInvoker)
    : TurboModule(name, jsInvoker) {
  // NativeUIManager.Spec
  methodMap_["getConstants"] = MethodMetadata{0, GetConstants};
  methodMap_["getConstantsForViewManager"] =
      MethodMetadata{1, GetConstantsForViewManager};

  // UIManagerJSInterface spec
  // methodMap_["getViewManagerConfig"] = MethodMetadata{1,
  // GetViewManagerConfig};
}

// static
jsi::Value UIManagerModule::GetConstants(
    jsi::Runtime &rt,
    TurboModule &turboModule,
    const jsi::Value *args,
    size_t count) {
  return jsi::Object(rt);
}

// static
jsi::Value UIManagerModule::GetConstantsForViewManager(
    jsi::Runtime &rt,
    TurboModule &turboModule,
    const jsi::Value *args,
    size_t count) {
  if (count != 1) {
    return jsi::Object(rt);
  }
  auto viewManagerName = args[0].asString(rt).utf8(rt);
  if (viewManagerName == "RCTView") {
    // NOTE(kudo): These are stubs to pass JS checkings for non bridge-less mode.
    // The real implementations should be from fabric uimanager.
    auto nativeProps = folly::dynamic::object("onLayout", true);
    auto registry =
        folly::dynamic::object("NativeProps", std::move(nativeProps));
    return jsi::valueFromDynamic(rt, std::move(registry));
  }
  return jsi::Object(rt);
}

// static
jsi::Value UIManagerModule::GetViewManagerConfig(
    jsi::Runtime &rt,
    TurboModule &turboModule,
    const jsi::Value *args,
    size_t count) {
  return jsi::Object(rt);
}

} // namespace react
} // namespace facebook
