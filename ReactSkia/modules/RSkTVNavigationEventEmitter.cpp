/*
* Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "RSkTVNavigationEventEmitter.h"
#include "ReactSkia/utils/RnsLog.h"
#include "ReactSkia/sdk/RNSKeyCodeMapping.h"

using namespace std;

namespace facebook {
namespace react {

RSkTVNavigationEventEmitter::RSkTVNavigationEventEmitter(
    const std::string &name, 
    std::shared_ptr<CallInvoker> jsInvoker,
    Instance *bridgeInstance)
    : RSkEventEmitter(name, jsInvoker, bridgeInstance){}

void RSkTVNavigationEventEmitter::handleTVNavigationEventNotification(folly::dynamic paramObject, NotificationCompleteVoidCallback completeCallback) {
#if !defined(GOOGLE_STRIP_LOG) || (GOOGLE_STRIP_LOG <= INFO)
    for (auto& pair : paramObject.items()) {
      RNS_LOG_INFO("Notify RCTTVNavigationEventNotification : { " << pair.first << " : " << pair.second << " }");
    }
#endif
    sendEventWithName(events_[0], folly::dynamic(paramObject), completeCallback);
}

void RSkTVNavigationEventEmitter::startObserving() {
    // Start observing RCTTVNavigationEventNotification using default NotificationManager
    RNS_LOG_INFO("Start observing RCTTVNavigationEventNotification using default notification center");
    std::function<void(folly::dynamic, NotificationCompleteVoidCallback)> tvNavigationHandler = std::bind(&RSkTVNavigationEventEmitter::handleTVNavigationEventNotification, this,
                                                                          std::placeholders::_1,  // folly::dynamic
                                                                          std::placeholders::_2);  // CompleteCallback
    navEventId_ = NotificationCenter::defaultCenter().addListener(tvEventName_, tvNavigationHandler);
}

void RSkTVNavigationEventEmitter::stopObserving() {
    NotificationCenter::defaultCenter().removeListener(navEventId_);
}

} //namespace react
} //namespace facebook
