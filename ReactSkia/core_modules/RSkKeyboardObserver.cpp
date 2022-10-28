 /*
 * Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "RSkKeyboardObserver.h"
#include "ReactSkia/sdk/NotificationCenter.h"
namespace facebook {
namespace react {
using namespace std;
RSkKeyboardObserver::RSkKeyboardObserver(
      const std::string &name,
      std::shared_ptr<CallInvoker> jsInvoker,
      Instance *bridgeInstance)
      : RSkEventEmitter(name, jsInvoker, bridgeInstance){
}

void RSkKeyboardObserver::handleOnScreenKeyboardEventNotification(string eventType){
  sendEventWithName(eventType,folly::dynamic::object());
}

void RSkKeyboardObserver::startObserving(){
    //Add listerners
#if ENABLE(FEATURE_ONSCREEN_KEYBOARD)
  std::function<void (std::string)> onScreenKeyboardNotification  = std::bind(&RSkKeyboardObserver::handleOnScreenKeyboardEventNotification, this,std::placeholders::_1);
  onscreenKeyboardEventId_ = NotificationCenter::subWindowCenter().addListener("onScreenKeyboardEvent", onScreenKeyboardNotification);
#endif 
}

void RSkKeyboardObserver::stopObserving(){
#if ENABLE(FEATURE_ONSCREEN_KEYBOARD)
  NotificationCenter::subWindowCenter().removeListener(onscreenKeyboardEventId_);
#endif 
}

} //namespace react
} //namespace facebook
