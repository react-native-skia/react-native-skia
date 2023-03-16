/*
* Copyright (C) 1994-2023 OpenTV, Inc. and Nagravision S.A.
*
* This source code is licensed under the MIT license found in the
* LICENSE file in the root directory of this source tree.
*/

#include "jsi/JSIDynamic.h"
#include "core_modules/RSkEventEmitter.h"
#include "pluginfactory/RnsPlugin.h"
#include "pluginfactory/RnsPluginFactory.h"

namespace facebook {
namespace react {

using namespace rnsplugin;

class RSkPlatformModule : public TurboModule {
 public:
  RSkPlatformModule(
      const std::string &name,
      std::shared_ptr<CallInvoker> jsInvoker,
      Instance *bridgeInstance);
  ~RSkPlatformModule();

  private:
   static jsi::Value getConstants(
        jsi::Runtime &rt,
        TurboModule &turboModule,
        const jsi::Value *args,
        size_t count);
   Instance *bridgeInstance_;
  // RNS Plugin Factgory and interfaces
  RnsPluginFactory *pluginFactory_; // TODO make shared singleton object
  std::unique_ptr<RNSPlatformManagerInterface> platformManagerHandle_;

  void lazyInit();
  folly::dynamic getConstants();

   void startObserving() {};
   void stopObserving() {};

  //CallBackClient for Events
  class PlatformCallBackClient : public RNSPlatformManagerInterface::CallbackClient {
   public:
    PlatformCallBackClient(RSkPlatformModule& platformModule);
    ~PlatformCallBackClient(){}

    void onStubEvent() override;

   private:
    RSkPlatformModule& platformModule_;
  };
  PlatformCallBackClient platformCallBackClient_;
};

}//namespace react
}//namespace facebook
