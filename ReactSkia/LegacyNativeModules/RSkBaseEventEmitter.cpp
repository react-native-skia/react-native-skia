/*
* Copyright (C) 1994-2023 OpenTV, Inc. and Nagravision S.A.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include <algorithm>
#include "ReactSkia/utils/RnsLog.h"

#include "RSkBaseEventEmitter.h"

namespace facebook {
namespace react {
RSkBaseEventEmitter::RSkBaseEventEmitter(
    Instance *bridgeInstance)
    :bridgeInstance_(bridgeInstance), 
    listenerCount_(0) { }



void RSkBaseEventEmitter::addListener(std::string eventName) {
    listenerCount_++;
    if (listenerCount_ == 1) {
        // TODO: It would be beneficial to pass on the eventName as a parameter as a Derived class may
        // be responsible to observing different types of events 
        startObserving();
    }
}

void RSkBaseEventEmitter::sendEventWithName(std::string eventName, folly::dynamic &&params, EmitterCompleteVoidCallback completeCallback) {
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



void RSkBaseEventEmitter::removeListeners(int removeCount) {

    listenerCount_ = std::max(listenerCount_ - removeCount, 0);

    if (listenerCount_ == 0) {
        // TODO: It should be beneficial to pass on the eventName as a Derived class may
        // be responsible to observing different types of events 
        stopObserving();
    }
    
}
void RSkBaseEventEmitter::SetBridgeInstance(Instance *bridgeInstance){
    bridgeInstance_=bridgeInstance;
}

} // namespace react
} // namespace facebook
