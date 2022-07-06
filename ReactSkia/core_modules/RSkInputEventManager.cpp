/*
* Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/
#include "RSkInputEventManager.h"
#include "ReactSkia/components/RSkComponent.h"

static bool keyRepeat;
static rnsKey previousKeyType;

namespace facebook{
namespace react {
static unsigned int eventId_;
static unsigned int subWindowEventId_;
RSkInputEventManager* RSkInputEventManager::sharedInputEventManager_{nullptr};
RSkInputEventManager::RSkInputEventManager(){
  std::function<void(rnsKey, rnsKeyAction)> handler = std::bind(&RSkInputEventManager::keyHandler, this,
                                                              std::placeholders::_1, // rnsKey
                                                              std::placeholders::_2);
  eventId_ = NotificationCenter::defaultCenter().addListener("onHWKeyEvent", handler);
#if ENABLE(FEATURE_ONSCREEN_KEYBOARD)
  subWindowEventId_ = NotificationCenter::subWindowCenter().addListener("onOSKKeyEvent", handler);
#endif/*FEATURE_ONSCREEN_KEYBOARD*/
  spatialNavigator_ =  SpatialNavigator::RSkSpatialNavigator::sharedSpatialNavigator();
  keyRepeat=false;
  previousKeyType=RNS_KEY_UnKnown;
}

void RSkInputEventManager::keyHandler(rnsKey eventKeyType, rnsKeyAction eventKeyAction){
  bool stopPropagate = false;
  RNS_LOG_DEBUG("[keyHandler] Key Repeat" << keyRepeat<<"  eventKeyType  " <<eventKeyType << " previousKeyType " <<previousKeyType <<"  eventKeyAction  " << eventKeyAction);
  if(previousKeyType == eventKeyType  && eventKeyAction == RNS_KEY_Press){
    keyRepeat = true;
  }
  if(eventKeyAction == RNS_KEY_Release ){
    previousKeyType = RNS_KEY_UnKnown;
    if(keyRepeat == true){
      keyRepeat = false;
    }
    else{
      return;// ignore key release 
    }
  }else
    previousKeyType = eventKeyType; 

  RNS_LOG_DEBUG(" [keyHandler] Key Repeat" << keyRepeat<<"  eventKeyType  " <<eventKeyType << " previousKeyType " <<previousKeyType);
  auto currentFocused = spatialNavigator_->getCurrentFocusElement();
  if(currentFocused){ // send key to Focused component.
    currentFocused->onHandleKey(eventKeyType, keyRepeat, &stopPropagate);
    if(stopPropagate){
      return;//don't propagate key further
    }
  }
#if defined(TARGET_OS_TV) && TARGET_OS_TV
  sendNotificationWithEventType(
      RNSKeyMap[eventKeyType],
      currentFocused ? currentFocused->getComponentData().tag : -1,
      eventKeyAction);
#endif //TARGET_OS_TV
  spatialNavigator_->handleKeyEvent(eventKeyType,eventKeyAction);
}

RSkInputEventManager* RSkInputEventManager::getInputKeyEventManager(){
  if (sharedInputEventManager_ == nullptr) {
    sharedInputEventManager_ = new RSkInputEventManager();
  }
  return sharedInputEventManager_;
}

#if defined(TARGET_OS_TV) && TARGET_OS_TV
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
#endif //TARGET_OS_TV

}//react
}//facebook
