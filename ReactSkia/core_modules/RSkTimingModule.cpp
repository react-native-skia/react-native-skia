/*
* Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
*
* This source code is licensed under the MIT license found in the
* LICENSE file in the root directory of this source tree.
*/

#include <algorithm>

#include "RSkTimingModule.h"

namespace facebook {
namespace react {

RSkTimingModule::RSkTimingModule(
    const std::string &name,
    std::shared_ptr<CallInvoker> jsInvoker,
    Instance *bridgeInstance)
    : TurboModule(name, jsInvoker),
      sendIdleEvents_(false),
      bridgeInstance_(bridgeInstance) {
  methodMap_["createTimer"] = MethodMetadata{4, createTimerWrapper};
  methodMap_["deleteTimer"] = MethodMetadata{1, deleteTimerWrapper};
  methodMap_["setSendIdleEvents"] = MethodMetadata{1, setSendIdleEventsWrapper};

}

jsi::Value RSkTimingModule::createTimerWrapper(
    jsi::Runtime &rt,
    TurboModule &turboModule,
    const jsi::Value *args,
    size_t count) {

  if (count != 4) {
    return jsi::Value::undefined();
  }
  auto &self = static_cast<RSkTimingModule &>(turboModule);
  double callbackId = args[0].getNumber();
  double duration = args[1].getNumber();
  double jsSchedulingTime = args[2].getNumber();
  bool repeats = args[3].getBool();

  // Call specific Event listener in Class object
  return self.createTimer(callbackId, duration, jsSchedulingTime, repeats);
}

jsi::Value RSkTimingModule::createTimer(
    double callbackId,
    double duration,
    double jsSchedulingTime,
    bool repeats) {

  RNS_LOG_DEBUG("Create Timer for callbackId : " << callbackId << ", jsSchedulingTime : " << jsSchedulingTime << ", Duration : " << duration);
  SysTimePoint schedulingTime{std::chrono::milliseconds(static_cast<unsigned long long>(jsSchedulingTime))};

  if(timer_ == nullptr) {
      timer_ = new Timer(duration,false, [this](){timerDidFire();},false);
  }

  if(duration == 0 && repeats == false) {
    timer_->scheduleImmediate(std::bind(&RSkTimingModule::immediatelyCallTimer, this, callbackId));
  } else {
    createTimerForNextFrame(callbackId, duration, schedulingTime, repeats);
  }
  return jsi::Value::undefined();
}

void RSkTimingModule::createTimerForNextFrame(
    double callbackId,
    double jsDuration,
    SysTimePoint jsSchedulingTime,
    bool repeats) {

  // Correcting scheduling overhead and finding actual targetDuration
  duration<double, std::milli> elapsed = system_clock::now() - jsSchedulingTime;
  double jsSchedulingOverhead = std::max(elapsed.count(), 0.0);
  double targetDuration = std::max((jsDuration - jsSchedulingOverhead), 0.0);

  SharedJsTimer jstimer = std::make_shared<RSkJsTimer>(callbackId, jsDuration, targetDuration, repeats);
  timer_->reschedule(targetDuration,false);

  jsTimerList_.lock();
  jsTimers_[callbackId] = jstimer;
  jsTimerList_.unlock();
}

void RSkTimingModule::timerDidFire() {
  std::vector<RSkJsTimer *> jsTimersToCall;
  SysTimePoint now = system_clock::now(); //Take this as base clock for all calculations here
  SysTimePoint nextScheduledTarget = Timer::getFutureTime();
  // Loop timer list and list down all expired timer into a vector
  jsTimerList_.lock();
  for (auto& timer : jsTimers_) {
    if( timer.second->target_ <= now ) { // Expired timer
      // Insert in sorted order
      auto it = lower_bound(jsTimersToCall.begin(), jsTimersToCall.end(), timer.second,
      [](auto& inList, auto& newItem ) { return (inList->target_ < newItem->target_);});

      jsTimersToCall.insert(it, timer.second.get());
      RNS_LOG_TRACE("Expired TimerID=" << timer.second->callbackId_ << ", repeat=" << timer.second->repeats_ << ", duration=" << timer.second->duration_);
    } else {
      RNS_LOG_TRACE("Pending TimerID=" << timer.second->callbackId_ << ", repeat=" << timer.second->repeats_ << ", duration=" << timer.second->duration_);
      if(timer.second->target_ < nextScheduledTarget)
        nextScheduledTarget = timer.second->target_;
    }
  }
  jsTimerList_.unlock();

  // Call expired callbacks
  if((jsTimersToCall.size() > 0 ) && bridgeInstance_) {
    dynamic sortedTimers = folly::dynamic::array;
    for (auto& timer : jsTimersToCall) {
      RNS_LOG_DEBUG("TimersToCall ID : " << timer->callbackId_ << " Duration : " << timer->duration_ );
      sortedTimers.push_back(timer->callbackId_);
    }
    bridgeInstance_->callJSFunction("JSTimers", "callTimers", folly::dynamic::array(sortedTimers));
  }

  // Go through the expired timers and reschedule repeating callbacks
  for (auto& timer : jsTimersToCall) {
    if (timer->repeats_) {
      timer->reschedule(now); // First update target_ of this timer
      if(timer->target_ < nextScheduledTarget)
        nextScheduledTarget = timer->target_;
    } else {
      jsTimerList_.lock();
      jsTimers_.erase(timer->callbackId_); // Remove expired callbacks which is already fired and doesnt repeat
      jsTimerList_.unlock();
    }
  }

  // Send Idle events
  if(sendIdleEvents_ && bridgeInstance_) {
    RNS_LOG_TODO("!!!!!!!!!! Send callIdleCallbacks with proper data");
    double absoluteFrameStartMS = 0; // TODO Refer RCTTiming.mm : The amount of time left in the frame, in ms.
    bridgeInstance_->callJSFunction("JSTimers", "callIdleCallbacks", folly::dynamic::array(absoluteFrameStartMS));
  }

  // Reschedule timer with nextScheduledTarget
  if(jsTimers_.size() > 0) {
    duration<double, std::milli> remaining = nextScheduledTarget - system_clock::now(); // Remining duration to target from this point in time.
    double targetDuration = std::max(remaining.count(), 0.0);
    timer_->reschedule(targetDuration,false);
    RNS_LOG_DEBUG("Rescheduled timer with shortest duration : " << targetDuration);
  }
}

void RSkTimingModule::immediatelyCallTimer(double callbackId) {
  if(bridgeInstance_) {
    RNS_LOG_DEBUG("--> immediatelyCallTimer - callbackId=" << callbackId << ", duration=0");
    bridgeInstance_->callJSFunction("JSTimers", "callTimers", folly::dynamic::array(folly::dynamic::array(callbackId)));
  }
}

jsi::Value RSkTimingModule::deleteTimerWrapper(
    jsi::Runtime &rt,
    TurboModule &turboModule,
    const jsi::Value *args,
    size_t count) {
  if (count != 1) {
    return jsi::Value::undefined();
  }
  auto &self = static_cast<RSkTimingModule &>(turboModule);
  double timerId = args[0].getNumber();

  // Call the specific non-static Class object
  return self.deleteTimer(timerId);
}

jsi::Value RSkTimingModule::deleteTimer(double timerId) {
  RNS_LOG_DEBUG("Delete Timer for callbackId : " << timerId);
  jsTimerList_.lock();
  jsTimers_.erase(timerId);
  jsTimerList_.unlock();
  if(jsTimers_.empty() && timer_) {
     timer_->abort();
     delete timer_;
     timer_ = nullptr;
  }
  return jsi::Value::undefined();
}

jsi::Value RSkTimingModule::setSendIdleEventsWrapper(
    jsi::Runtime &rt,
    TurboModule &turboModule,
    const jsi::Value *args,
    size_t count) {
  if (count != 1) {
    return jsi::Value::undefined();
  }
  auto &self = static_cast<RSkTimingModule &>(turboModule);
  bool sendIdleEvents = args[0].getBool();

  // Call the specific non-static Class object
  return self.setSendIdleEvents(sendIdleEvents);
}

jsi::Value RSkTimingModule::setSendIdleEvents(bool sendIdleEvents) {
  sendIdleEvents_ = sendIdleEvents;
  RNS_LOG_DEBUG("Set SendIdleEvents : " << sendIdleEvents);

  //TODO Refer RCTTiming.mm
#if 0
  if(sendIdleEvents) {
    startTimers();
  } else if(hasPendingTimers()) {
    stopTimer();
  }
#endif
  return jsi::Value::undefined();
}

} // namespace react
} // namespace facebook
