#pragma once

#include "ReactCommon/TurboModule.h"
#include "cxxreact/Instance.h"

namespace facebook {
namespace react {
class RSkEventEmitter: public TurboModule {
    public:
      RSkEventEmitter(
        const std::string &name, 
        std::shared_ptr<CallInvoker> jsInvoker,
        Instance *bridgeInstance);

      virtual std::vector<std::string> supportedEvents() {
        // Return empty vector by default
        return {};
      }

      void sendEventWithName(std::string eventName, folly::dynamic &&params);

      virtual void startObserving() = 0;

      virtual void stopObserving() = 0;
    
    private:
      static jsi::Value addListenerWrapper(
        jsi::Runtime &rt,
        TurboModule &turboModule,
        const jsi::Value *args,
        size_t count);
      
      static jsi::Value removeListenersWrapper(
        jsi::Runtime &rt,
        TurboModule &turboModule,
        const jsi::Value *args,
        size_t count);

      jsi::Value addListener(std::string);
      
      jsi::Value removeListeners(
        int removeCount);

      Instance *bridgeInstance_;
      int listenerCount_;
};
} // namespace react
} // namespace facebook