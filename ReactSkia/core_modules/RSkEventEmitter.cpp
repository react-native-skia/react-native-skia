/*
* Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include <algorithm>
#include "ReactSkia/utils/RnsLog.h"

#include "RSkEventEmitter.h"

namespace facebook {
namespace react {
RSkEventEmitter::RSkEventEmitter(
    const std::string &name, 
    std::shared_ptr<CallInvoker> jsInvoker,
    Instance *bridgeInstance)
    : TurboModule(name, jsInvoker),
    bridgeInstance_(bridgeInstance), 
    listenerCount_(0) {

        methodMap_["addListener"] = MethodMetadata{1, addListenerWrapper};
        methodMap_["removeListeners"] = MethodMetadata{1, removeListenersWrapper};
}

jsi::Value RSkEventEmitter::addListenerWrapper(
      jsi::Runtime &rt,
      TurboModule &turboModule,
      const jsi::Value *args,
      size_t count) {
    if (count != 1) {
      return jsi::Value::undefined();
    }
    auto &self = static_cast<RSkEventEmitter &>(turboModule);
    auto nameValue = args[0].getString(rt);
    auto eventName = nameValue.utf8(rt);
    
    // Call specific Event listener in Class object 
    return self.addListener(eventName.data());
}

jsi::Value RSkEventEmitter::addListener(std::string eventName) {
    listenerCount_++;
    if (listenerCount_ == 1) {
        // TODO: It would be beneficial to pass on the eventName as a parameter as a Derived class may
        // be responsible to observing different types of events 
        startObserving();
    }
    return jsi::Value::undefined();
}

void RSkEventEmitter::sendEventWithName(std::string eventName, folly::dynamic &&params, EmitterCompleteVoidCallback completeCallback) {
    if (bridgeInstance_ == NULL) {
        RNS_LOG_ERROR("EventEmitter not initialized with Bridge instance");
    }

    // TODO: check if the eventName is in supportedEvents()
    if (listenerCount_ >= 1) {
        bridgeInstance_->callJSFunction(
            "RCTDeviceEventEmitter", "emit", 
            params != NULL ? folly::dynamic::array(folly::dynamic::array(eventName), 
                params)
            : folly::dynamic::array(eventName));
            if(completeCallback)
              bridgeInstance_->getJSCallInvoker()->invokeAsync(std::move(completeCallback));
    }
}

jsi::Value RSkEventEmitter::removeListenersWrapper(
      jsi::Runtime &rt,
      TurboModule &turboModule,
      const jsi::Value *args,
      size_t count) {
    if (count != 1) {
      return jsi::Value::undefined();
    }
    auto &self = static_cast<RSkEventEmitter &>(turboModule);
    int removeCount = args[0].getNumber();

    // Call the specific non-static Class object
    return self.removeListeners(removeCount);
}

jsi::Value RSkEventEmitter::removeListeners(int removeCount) {

    listenerCount_ = std::max(listenerCount_ - removeCount, 0);

    if (listenerCount_ == 0) {
        // TODO: It should be beneficial to pass on the eventName as a Derived class may
        // be responsible to observing different types of events 
        stopObserving();
    }
    return jsi::Value::undefined();
}

} // namespace react
} // namespace facebook
