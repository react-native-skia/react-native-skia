/*
* Copyright (C) 1994-2023 OpenTV, Inc. and Nagravision S.A.
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
#if ENABLE(FEATURE_KEY_THROTTLING)
  keyQueue_ =  std::make_unique<ThreadSafeQueue<RSkKeyInput>>();
#endif
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
#if ENABLE(FEATURE_KEY_THROTTLING)
  inputWorkerThread_ = std::thread(&RSkInputEventManager::inputWorkerThreadFunction, this);
  sem_init(&keyEventPost_, 0, 1);
  completeCallback_ = std::bind(&RSkInputEventManager::onEventComplete, this);
#else
  completeCallback_ = nullptr;
#endif
}

RSkInputEventManager::~RSkInputEventManager(){
  if(eventId_ > 0) {
    NotificationCenter::defaultCenter().removeListener(eventId_);
    eventId_ = 0;
  }
  if(subWindowEventId_ > 0) {
    NotificationCenter::subWindowCenter().removeListener(subWindowEventId_);
    subWindowEventId_ = 0;
  }
#if ENABLE(FEATURE_KEY_THROTTLING)
  sem_destroy(&keyEventPost_);
  if(keyQueue_)
    keyQueue_ = nullptr;
  if(inputWorkerThread_.joinable()) {
    inputWorkerThread_.join();
  }
#endif
}

#if ENABLE(FEATURE_KEY_THROTTLING)
void RSkInputEventManager::inputWorkerThreadFunction() {
  RSkKeyInput keyInput;
  while(true) {
    while(activeInputClients_ > 0) // If there are clients who are still processing previous key then wait..
      sem_wait(&keyEventPost_);
    keyQueue_->pop(keyInput); // Blocks if empty. TODO better to use timed pop to avoid unforseen block issues ??
    RNS_LOG_DEBUG("Process input from queue,  Key : " << keyInput.key_ << " Action : " <<keyInput.action_ << ", Repeat : " <<keyInput.repeat_);
    processKey(keyInput);
  }
}

void RSkInputEventManager::onEventEmit() {
  activeInputClients_++;
  RNS_LOG_DEBUG("++++++++++ On Key Event Emit ++++++++++ : Pending Events : " << activeInputClients_.load());
}

void RSkInputEventManager::onEventComplete() {
  (activeInputClients_ > 0) ? ((--activeInputClients_ == 0) ? sem_post(&keyEventPost_) : false) : false;
  RNS_LOG_DEBUG("---------- On Key Event Emit Complete ---------- : Pending Events : " << activeInputClients_.load());
}
#endif

void RSkInputEventManager::keyHandler(rnsKey eventKeyType, rnsKeyAction eventKeyAction){
  RNS_LOG_DEBUG("[keyHandler] Key Repeat" << keyRepeat<<"  eventKeyType  " <<eventKeyType << " previousKeyType " <<previousKeyType <<"  eventKeyAction  " << eventKeyAction);

  if(previousKeyType == eventKeyType  && eventKeyAction == RNS_KEY_Press){
    keyRepeat = true;
  }

  if(eventKeyAction == RNS_KEY_Release) {
    previousKeyType = RNS_KEY_UnKnown;
    if(keyRepeat == true) {
      keyRepeat = false;
#if ENABLE(FEATURE_KEY_THROTTLING)
      if(!keyQueue_->isEmpty())
        keyQueue_->clear(); // flush the queue
#endif
    }
  } 
  RSkKeyInput keyInput(eventKeyType, eventKeyAction, keyRepeat);
  previousKeyType = eventKeyType;
#if ENABLE(FEATURE_KEY_THROTTLING)
    keyQueue_->push(keyInput);
#else
    processKey(keyInput);
#endif
}

void RSkInputEventManager::processKey(RSkKeyInput &keyInput) {
  bool stopPropagate = false;
  RNS_LOG_DEBUG("[Process Key] Key Repeat " << keyInput.repeat_ << " eventKeyType  " << keyInput.key_ << " previousKeyType " << previousKeyType);
  
  auto currentFocused = spatialNavigator_->getCurrentFocusElement();
  if(currentFocused){ // send key to Focused component.
    currentFocused->onHandleKey(keyInput.key_, keyInput.repeat_,keyInput.action_, &stopPropagate);
    if(stopPropagate){
      return;//don't propagate key further
    }
  }
  if(keyInput.action_ == RNS_KEY_Press) {
#if defined(TARGET_OS_TV) && TARGET_OS_TV
    sendNotificationWithEventType(
        RNSKeyMap[keyInput.key_],
        currentFocused ? currentFocused->getComponentData().tag : -1,
        keyInput.action_, nullptr);
#endif //TARGET_OS_TV
    spatialNavigator_->handleKeyEvent(keyInput.key_, keyInput.action_);
  }
  /*Sending Events to the registered callback*/
  eventCallbackMutex_.lock();
  for (auto pair : eventCallbackMap_){
    RNS_LOG_DEBUG("calling clients");
    auto clientCallback = pair.second;
    clientCallback(keyInput);
  }
  eventCallbackMutex_.unlock();
}

RSkInputEventManager* RSkInputEventManager::getInputKeyEventManager(){
  if (sharedInputEventManager_ == nullptr) {
    sharedInputEventManager_ = new RSkInputEventManager();
  }
  return sharedInputEventManager_;
}

#if defined(TARGET_OS_TV) && TARGET_OS_TV
void RSkInputEventManager::sendNotificationWithEventType(std::string eventType, int tag, rnsKeyAction keyAction, NotificationCompleteVoidCallback completeCallback) {
  if(eventType.c_str() == nullptr)
    return;
  RNS_LOG_DEBUG("Send : " << eventType  << " To ComponentTag : " << tag );
  NotificationCenter::defaultCenter().emit("RCTTVNavigationEventNotification",
      folly::dynamic(folly::dynamic::object("eventType", eventType.c_str())
      ("eventKeyAction", (int)keyAction)
      ("tag", tag)
      ("target", tag)
      ), completeCallback);
}
#endif //TARGET_OS_TV

int RSkInputEventManager::addKeyEventCallback(inputEventClientCallback clientCallback){
  RNS_LOG_DEBUG("[addKeyEventCallback] ");
  std::scoped_lock lock(eventCallbackMutex_);
  callbackId_++;
  eventCallbackMap_.insert({callbackId_,clientCallback});
  return callbackId_;
}

void RSkInputEventManager::removeKeyEventCallback(int callbackId){
  RNS_LOG_DEBUG("[removeListener]");
  std::scoped_lock lock(eventCallbackMutex_);
  eventCallbackMap_.erase(callbackId);
}


}//react
}//facebook
