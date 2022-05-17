/*
* Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
*
* This source code is licensed under the MIT license found in the
* LICENSE file in the root directory of this source tree.
*/

#include <algorithm>

#include "FollyTimer.h"

namespace rns {
namespace sdk {

Timer::Timer(double duration,
           bool repeats,
           std::function<void()> cb,
           bool autostart)
           : targetDuration_(duration),
             repeats_(repeats),
             cb_(cb),
             timerThread_("FollyTimerThread"){

  timerThread_.getEventBase()->waitUntilRunning();

  RNS_LOG_DEBUG("[" << this << "] Created timer with duration:" << targetDuration_ << "ms ,repeats:" << repeats_ << " ,autostart:" << autostart);
  if(autostart) start();

}

void Timer::start() {
  RNS_LOG_DEBUG("["<< this << "] Schedule timer for duration:" << targetDuration_ << " ms");

  if(cb_ == nullptr) {
    RNS_LOG_ERROR("No callback registered with timer, ignore scheduling of timer");
    return;
  }

  if(timerCallback_.isScheduled()) {
    RNS_LOG_WARN("Timer is already been scheduled for duration:" << targetDuration_ << " ms");
    return;
  }

  auto timeoutCb = std::bind(&Timer::scheduleTimerTimeout, this);
  // For super fast, on-off timers, just enqueue them immediately rather than waiting
  if(targetDuration_ < 1) {
    timerThread_.getEventBase()->runInEventBaseThread(timeoutCb);
    return;
  }

  timerCallback_.cb = timeoutCb;
  timerThread_.getEventBase()->runInEventBaseThread(
     [this]() {
       std::scoped_lock lock(timerCallback_.cbLock);
       if(timerCallback_.cb) {
         HHWheelTimer& wheelTimer = timerThread_.getEventBase()->timer();
         wheelTimer.scheduleTimeout(&timerCallback_, std::chrono::milliseconds(static_cast<unsigned long long>(targetDuration_)));
       }
     }
  );
  return;

}

void Timer::reschedule(double duration, bool repeats) {
  RNS_LOG_DEBUG("["<< this << "] Reschedule timer for duration:" << duration << "ms ,repeats:" << repeats);

  if(cb_ == nullptr) {
    RNS_LOG_ERROR("No callback registered with timer, ignore scheduling of timer");
    return;
  }

  repeats_ = repeats;

  auto timeoutCb = std::bind(&Timer::scheduleTimerTimeout, this);
  // For super fast, on-off timers, just enqueue them immediately rather than waiting
  // If any previous timers scheduled,abort it.
  if(duration < 1) {
    if(timerCallback_.isScheduled())
        abort();
    timerThread_.getEventBase()->runInEventBaseThread(timeoutCb);
    return;
  }

  if(timerCallback_.cb == nullptr)
     timerCallback_.cb = timeoutCb;

  timerThread_.getEventBase()->runInEventBaseThread(
     [this,duration]() {
        std::scoped_lock lock(timerCallback_.cbLock);
        if(timerCallback_.cb) {
            HHWheelTimer& wheelTimer = timerThread_.getEventBase()->timer();
            if(timerCallback_.isScheduled() == false || duration < getTimeRemaining()) {
              wheelTimer.scheduleTimeout(&timerCallback_, std::chrono::milliseconds(static_cast<unsigned long long>(duration)));
              targetDuration_ = duration;
            }
        }
     }
  );

  return;
}

void Timer::scheduleTimerTimeout() {
  RNS_LOG_DEBUG("[" << this << "] scheduleTimerTimeout fired");
  SysTimePoint timeoutFiredTime = system_clock::now(); //Take this as base clock for all calculations here
  if(cb_)
    cb_();

  if(repeats_) {
     duration<double, std::milli> elapsed = system_clock::now() - timeoutFiredTime;
     double schedulingOverhead = std::max(elapsed.count(), 0.0);
     double targetDuration = std::max((targetDuration_ - schedulingOverhead), 0.0);

     RNS_LOG_INFO("["<< this <<"] Reschedule repeat timer for duration:" << targetDuration);
     if(targetDuration < 1) {
       timerThread_.getEventBase()->runInEventBaseThread(std::bind(&Timer::scheduleTimerTimeout,this));
     } else {
       timerThread_.getEventBase()->runInEventBaseThread(
          [this,targetDuration]() {
             std::scoped_lock lock(timerCallback_.cbLock);
             if(timerCallback_.cb) {
                HHWheelTimer& wheelTimer = timerThread_.getEventBase()->timer();
                wheelTimer.scheduleTimeout(&timerCallback_, std::chrono::milliseconds(static_cast<unsigned long long>(targetDuration)));
             }
          }
       );
     }
  }

}

void Timer::abort() {
  RNS_LOG_DEBUG("[" << this << "] Abort timer remainingDuration:" << getTimeRemaining() << "ms");
  timerCallback_.cbLock.lock();
  timerCallback_.cb = nullptr;
  timerCallback_.cbLock.unlock();

  if(!timerCallback_.isScheduled()) {
    RNS_LOG_DEBUG("Timer is idle,nothing to do!!");
    return;
  }

  timerThread_.getEventBase()->runInEventBaseThread(
    [this]() {
      HHWheelTimer& wheelTimer = timerThread_.getEventBase()->timer();
      wheelTimer.cancelAll();
    }
  );
  return;
}

double Timer::getTimeRemaining() {
  double remainingTime = timerCallback_.isScheduled() ? timerCallback_.getTimeRemaining().count()/1000 : 0;
  RNS_LOG_DEBUG("[" << this << "] getTimeRemaining [" << remainingTime << "]");
  return remainingTime;
}

SysTimePoint Timer::getFutureTime() {
  return system_clock::now() + milliseconds(static_cast<unsigned long long>(31536000000)); // Set 1 year ahead time from now
}

void Timer::scheduleImmediate(std::function<void()> cb) {
  timerThread_.getEventBase()->runInEventBaseThread(cb);
}

} // namespace rns
} // namespace sdk
