#include "ReactSkia/JSITurboModuleManager.h"

#include "ReactSkia/utils/RnsLog.h"

#include "cxxreact/Instance.h"
#include "jsi/JSIDynamic.h"

#include <folly/io/async/ScopedEventBaseThread.h>
#include "core_modules/RSkTimingModule.h"

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

class AppStateModule : public TurboModule {
 public:
  AppStateModule(
      const std::string &name,
      std::shared_ptr<CallInvoker> jsInvoker)
      : TurboModule(name, jsInvoker) {
    methodMap_["getConstants"] = MethodMetadata{0, GetConstants};

    methodMap_["getCurrentAppState"] = MethodMetadata{2, NoOp};
    methodMap_["addListener"] = MethodMetadata{1, NoOp};
    methodMap_["removeListener"] = MethodMetadata{1, NoOp};
  }

 private:
  static jsi::Value GetConstants(
      jsi::Runtime &rt,
      TurboModule &turboModule,
      const jsi::Value *args,
      size_t count) {
    return jsi::valueFromDynamic(
        rt, folly::dynamic::array("initialAppState", "active"));
  }
};

class WebSocketModule : public TurboModule {
 public:
  WebSocketModule(
      const std::string &name,
      std::shared_ptr<CallInvoker> jsInvoker)
      : TurboModule(name, jsInvoker) {
    methodMap_["connect"] = MethodMetadata{4, NoOp};
    methodMap_["send"] = MethodMetadata{2, NoOp};
    methodMap_["sendBinary"] = MethodMetadata{2, NoOp};
    methodMap_["ping"] = MethodMetadata{1, NoOp};
    methodMap_["close"] = MethodMetadata{3, NoOp};
    methodMap_["addListener"] = MethodMetadata{1, NoOp};
    methodMap_["removeListeners"] = MethodMetadata{1, NoOp};
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
  auto rnVersion = folly::dynamic::object("major", 0)("minor", 0)("patch", 0);
  staticModule->SetConstants(folly::dynamic::object("isTesting", true)(
      "reactNativeVersion", std::move(rnVersion)));
  modules_["PlatformConstants"] = std::move(staticModule);

  modules_["ExceptionsManager"] =
      std::make_shared<ExceptionsManagerModule>("ExceptionsManager", jsInvoker);

  modules_["Timing"] =
      std::make_shared<RSkTimingModule>("Timing", jsInvoker, bridgeInstance);

  modules_["AppState"] =
      std::make_shared<AppStateModule>("AppState", jsInvoker);

  modules_["WebSocketModule"] =
      std::make_shared<WebSocketModule>("WebSocketModule", jsInvoker);

  modules_["Networking"] =
      std::make_shared<UnimplementedTurboModule>("Networking", jsInvoker);
  modules_["DevSettings"] =
      std::make_shared<UnimplementedTurboModule>("DevSettings", jsInvoker);
  modules_["ImageLoader"] =
      std::make_shared<UnimplementedTurboModule>("ImageLoader", jsInvoker);
  modules_["StatusBarManager"] =
      std::make_shared<UnimplementedTurboModule>("StatusBarManager", jsInvoker);
  modules_["Appearance"] =
      std::make_shared<UnimplementedTurboModule>("Appearance", jsInvoker);
  modules_["KeyboardObserver"] =
      std::make_shared<UnimplementedTurboModule>("KeyboardObserver", jsInvoker);
  modules_["NativeAnimatedModule"] = std::make_shared<UnimplementedTurboModule>(
      "NativeAnimatedModule", jsInvoker);

  staticModule = std::make_shared<StaticTurboModule>("DeviceInfo", jsInvoker);
  auto windowMetrics = folly::dynamic::object("width", 1024)("height", 768)(
      "scale", 1)("fontScale", 1);
  auto screenMetrics = folly::dynamic::object("width", 1024)("height", 768)(
      "scale", 1)("fontScale", 1);
  auto dimension = folly::dynamic::object("window", std::move(windowMetrics))(
      "screen", std::move(screenMetrics));
  staticModule->SetConstants(
      folly::dynamic::object("Dimensions", std::move(dimension)));
  modules_["DeviceInfo"] = std::move(staticModule);
}

TurboModuleProviderFunctionType JSITurboModuleManager::GetProvider() {
  return [this](
             const std::string &name,
             const jsi::Value *schema) -> std::shared_ptr<TurboModule> {
    if (modules_.find(name) != modules_.end()) {
      return modules_[name];
    }
    return nullptr;
  };
}

} // namespace react
} // namespace facebook
