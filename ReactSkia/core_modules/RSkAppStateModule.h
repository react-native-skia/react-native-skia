/*
* Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
*
* This source code is licensed under the MIT license found in the
* LICENSE file in the root directory of this source tree.
*/
#pragma once

#include "jsi/JSIDynamic.h"
#include "ReactSkia/core_modules/RSkEventEmitter.h"
#include "ReactSkia/JSITurboModuleManager.h"
#include "ReactSkia/pluginfactory/RnsPlugin.h"
#include "ReactSkia/pluginfactory/RnsPluginFactory.h"

namespace facebook {
namespace react {

using namespace std;
using namespace rnsplugin;

class RSkAppStateModule : public react::RSkEventEmitter {
 public:
  RSkAppStateModule(
    const std::string &name,
    std::shared_ptr<react::CallInvoker> jsInvoker,
    react::Instance *bridgeInstance);
  ~RSkAppStateModule(){};

  static jsi::Value getConstantsWrapper(
    jsi::Runtime &rt,
    TurboModule &turboModule,
    const jsi::Value *args,
    size_t count);
  static jsi::Value getCurrentAppStateWrapper(
    jsi::Runtime &rt,
    TurboModule &turboModule,
    const jsi::Value *args,
    size_t count);

  void startObserving()override;
  void stopObserving()override;

  std::vector<std::string> supportedEvents() override {
    return {"appStateDidChange", "memoryWarning"};
  }

 private:
  // RNS Plugin Factgory and interface
  RnsPluginFactory *pluginFactory_;
  std::unique_ptr<RNSApplicationManagerInterface> appManagerHandle_;

  void lazyInit();
  jsi::Value getConstants(jsi::Runtime &rt);
  jsi::Value getCurrentAppState(jsi::Runtime &rt, const jsi::Object &successObj, const jsi::Object &errorObj);

  //CallBackClient for Events
  class AppStateCallBackClient : public RNSApplicationManagerInterface::CallbackClient {
   public:
    AppStateCallBackClient(RSkAppStateModule& appStateModule);
    ~AppStateCallBackClient(){}

    void onChange(string appState) override;
    void onMemoryWarning() override;
    void onFocus() override;
    void onBlur() override;

   private:
    RSkAppStateModule& appStateModule_;
  };
  AppStateCallBackClient appStateCallBackClient_;
};

} // react
} // facebook
