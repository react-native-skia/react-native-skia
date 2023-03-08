/*
* Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
* Copyright (C) Kudo Chien
*
* This source code is licensed under the MIT license found in the
* LICENSE file in the root directory of this source tree.
*/
#include <folly/io/async/ScopedEventBaseThread.h>

#include "build/build_config.h"
#include "cxxreact/Instance.h"
#include "jsi/JSIDynamic.h"

#include "JSITurboModuleManager.h"
#include "version.h"
#include "core_modules/RSkAppStateModule.h"
#include "core_modules/RSkDeviceInfo.h"
#include "core_modules/RSkImageLoader.h"
#include "core_modules/RSkTimingModule.h"
#include "core_modules/RSkKeyboardObserver.h"
#include "modules/platform/libcurl/RSkNetworkingModule.h"
#include "modules/RSkTVNavigationEventEmitter.h"
#include "utils/RnsLog.h"
#include "ReactSkia/buildflags.h"

#if BUILDFLAG(USE_WEBSOCKET_NOPOLL)
#include "modules/platform/nopoll/RSkWebSocketModule.h"
#else
#include "modules/platform/libcurl/RSkWebSocketModule.h"
#endif

#if ENABLE(FEATURE_ALERT)
#include "core_modules/RSkAlertManager.h"
#endif//FEATURE_ALERT

namespace facebook {
namespace react {

namespace {

static jsi::Value NoOp(
    jsi::Runtime &rt,
    TurboModule &turboModule,
    const jsi::Value *args,
    size_t count) {
  return jsi::Value::undefined();
}

class StaticTurboModule : public TurboModule {
 public:
  StaticTurboModule(
      const std::string &name,
      std::shared_ptr<CallInvoker> jsInvoker)
      : TurboModule(name, jsInvoker) {}

  facebook::jsi::Value get(
      facebook::jsi::Runtime &runtime,
      const facebook::jsi::PropNameID &propName) override {
    std::string propNameUtf8 = propName.utf8(runtime);
    if (propNameUtf8 == "getConstants") {
      return jsi::Function::createFromHostFunction(
          runtime,
          propName,
          0,
          [this](
              jsi::Runtime &rt,
              const jsi::Value &thisVal,
              const jsi::Value *args,
              size_t count) { return jsi::valueFromDynamic(rt, constants_); });
    }
    return jsi::Value::undefined();
  }

  void SetConstants(folly::dynamic &&constants) {
    constants_ = std::move(constants);
  }

 private:
  folly::dynamic constants_;
};

class ExceptionsManagerModule : public TurboModule {
 public:
  ExceptionsManagerModule(
      const std::string &name,
      std::shared_ptr<CallInvoker> jsInvoker)
      : TurboModule(name, jsInvoker) {
    methodMap_["reportException"] = MethodMetadata{1, ReportException};

    methodMap_["updateExceptionMessage"] = MethodMetadata{3, NoOp};
  }

 private:
  static jsi::Value ReportException(
      jsi::Runtime &rt,
      TurboModule &turboModule,
      const jsi::Value *args,
      size_t count) {
    if (count == 1 && args[0].isObject()) {
      auto data = args[0].asObject(rt);
      auto message = data.getProperty(rt, "message");
      RNS_LOG_ERROR("[ExceptionManager] message : " << message.asString(rt).utf8(rt));
    }
    return jsi::Value::undefined();
  }
};

class UnimplementedTurboModule : public TurboModule {
 public:
  UnimplementedTurboModule(
      const std::string &name,
      std::shared_ptr<CallInvoker> jsInvoker)
      : TurboModule(name, jsInvoker) {}

  facebook::jsi::Value get(
      facebook::jsi::Runtime &runtime,
      const facebook::jsi::PropNameID &propName) override {
    return jsi::Value::undefined();
  };
};

} // namespace

JSITurboModuleManager::JSITurboModuleManager(Instance *bridgeInstance)
    : bridgeInstance_(bridgeInstance) {
  std::shared_ptr<CallInvoker> jsInvoker = bridgeInstance->getJSCallInvoker();
  auto staticModule =
      std::make_shared<StaticTurboModule>("SourceCode", jsInvoker);
  staticModule->SetConstants(folly::dynamic::object("scriptURL", "foo"));
  modules_["SourceCode"] = std::move(staticModule);

  staticModule =
      std::make_shared<StaticTurboModule>("PlatformConstants", jsInvoker);
  auto rnVersion = folly::dynamic::object("major", RN_MAJOR_VERSION)("minor", RN_MINOR_VERSION)("patch", RN_PATCH_VERSION);
  staticModule->SetConstants(folly::dynamic::object("isTesting", true)(
      "reactNativeVersion", std::move(rnVersion)) ("osVersion",STRINGIFY(RNS_OS_VERSION))
#if defined(TARGET_OS_TV) && TARGET_OS_TV
      ("interfaceIdiom", STRINGIFY(tv))
#else
      ("interfaceIdiom", STRINGIFY(unknown))
#endif //TARGET_OS_TV
    );
  modules_["PlatformConstants"] = std::move(staticModule);

  modules_["ExceptionsManager"] =
      std::make_shared<ExceptionsManagerModule>("ExceptionsManager", jsInvoker);

  modules_["Timing"] =
      std::make_shared<RSkTimingModule>("Timing", jsInvoker, bridgeInstance);
  modules_["AppState"] =
      std::make_shared<RSkAppStateModule>("AppState", jsInvoker, bridgeInstance);
  modules_["Networking"] =
      std::make_shared<RSkNetworkingModule>("Networking", jsInvoker, bridgeInstance );
  modules_["WebSocketModule"] =
      std::make_shared<RSkWebSocketModule>("WebSocketModule", jsInvoker, bridgeInstance);
  modules_["KeyboardObserver"] =
      std::make_shared<RSkKeyboardObserver>("KeyboardObserver", jsInvoker, bridgeInstance);
  modules_["DeviceInfo"] =
      std::make_shared<RSkDeviceInfoModule>("DeviceInfo", jsInvoker, bridgeInstance);
  modules_["ImageLoader"] =
      std::make_shared<RSkImageLoader>("ImageLoader", jsInvoker);
#if ENABLE(FEATURE_ALERT)
  modules_["AlertManager"] =
      std::make_shared<RSkAlertManager>("AlertManager", jsInvoker, bridgeInstance);
#endif//FEATURE_ALERT
#if defined(TARGET_OS_TV) && TARGET_OS_TV
  modules_["TVNavigationEventEmitter"] =
      std::make_shared<RSkTVNavigationEventEmitter>("TVNavigationEventEmitter",jsInvoker, bridgeInstance);
#endif //TARGET_OS_TV

  modules_["DevSettings"] =
      std::make_shared<UnimplementedTurboModule>("DevSettings", jsInvoker);
  modules_["StatusBarManager"] =
      std::make_shared<UnimplementedTurboModule>("StatusBarManager", jsInvoker);
  modules_["Appearance"] =
      std::make_shared<UnimplementedTurboModule>("Appearance", jsInvoker);
  modules_["NativeAnimatedModule"] = std::make_shared<UnimplementedTurboModule>(
      "NativeAnimatedModule", jsInvoker);
}

TurboModuleProviderFunctionType JSITurboModuleManager::GetProvider() {
  return [this](const std::string &name) -> std::shared_ptr<TurboModule> {
    if (modules_.find(name) != modules_.end()) {
      return modules_[name];
    }
    RNS_LOG_WARN("!!!!! Turbo Module " << name << " Not found !!!!!");
    return nullptr;
  };
}

} // namespace react
} // namespace facebook
