/*
* Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/
#include "RSkInputEventManager.h"
#include "ReactSkia/components/RSkComponent.h"

namespace facebook{
namespace react {
static unsigned int eventId_;
RSkInputEventManager* RSkInputEventManager::sharedInputEventManager_{nullptr};
RSkInputEventManager::RSkInputEventManager(){
  std::function<void(rnsKey, rnsKeyAction)> handler = std::bind(&RSkInputEventManager::keyHandler, this,
                                                              std::placeholders::_1, // rnsKey
                                                              std::placeholders::_2);
  eventId_ = NotificationCenter::defaultCenter().addListener("onHWKeyEvent", handler);
  spatialNavigator_ =  SpatialNavigator::RSkSpatialNavigator::sharedSpatialNavigator();
}

void RSkInputEventManager::keyHandler(rnsKey eventKeyType, rnsKeyAction eventKeyAction){
  bool stopPropagate = false;
  if(eventKeyAction != RNS_KEY_Press)
    return;
  auto currentFocused = spatialNavigator_->getCurrentFocusElement();
  if(currentFocused){ // send key to Focused component.
    currentFocused->onHandleKey(eventKeyType,&stopPropagate);
    if(stopPropagate){
      return;//don't propagate key further
    }
  }
  sendNotificationWithEventType(
      RNSKeyMap[eventKeyType],
      currentFocused ? currentFocused->getComponentData().tag : -1,
      eventKeyAction);
  spatialNavigator_->handleKeyEvent(eventKeyType,eventKeyAction);
}

RSkInputEventManager* RSkInputEventManager::getInputKeyEventManager(){
  if (sharedInputEventManager_ == nullptr) {
    sharedInputEventManager_ = new RSkInputEventManager();
  }
  return sharedInputEventManager_;
}

void RSkInputEventManager::sendNotificationWithEventType(std::string eventType, int tag, rnsKeyAction keyAction) {
  if(eventType.c_str() == nullptr)
    return;
  RNS_LOG_DEBUG("Send : " << eventType  << " To ComponentTag : " << tag );
  NotificationCenter::defaultCenter().emit("RCTTVNavigationEventNotification",
      folly::dynamic(folly::dynamic::object("eventType", eventType.c_str())
      ("eventKeyAction", (int)keyAction)
      ("tag", tag)
      ("target", tag)
      ));
}

}//react
}//facebook
