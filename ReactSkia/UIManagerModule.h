#pragma once

#include "ReactCommon/TurboModule.h"

namespace facebook {
namespace react {

class UIManagerModule : public TurboModule {
 public:
  UIManagerModule(
      const std::string &name,
      std::shared_ptr<CallInvoker> jsInvoker);

 private:
  // NativeUIManager.Spec
  static jsi::Value GetConstants(
      jsi::Runtime &rt,
      TurboModule &turboModule,
      const jsi::Value *args,
      size_t count);

  static jsi::Value GetConstantsForViewManager(
      jsi::Runtime &rt,
      TurboModule &turboModule,
      const jsi::Value *args,
      size_t count);

  // UIManagerJSInterface spec
  static jsi::Value GetViewManagerConfig(
      jsi::Runtime &rt,
      TurboModule &turboModule,
      const jsi::Value *args,
      size_t count);
};

} // namespace react
} // namespace facebook
