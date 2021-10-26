/*
* Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#pragma once

#include <better/map.h>

#include <folly/portability/SysTypes.h>
#include <folly/executors/ScheduledExecutor.h>
#include <folly/io/async/ScopedEventBaseThread.h>
#include <folly/io/async/HHWheelTimer.h>

#include "cxxreact/Instance.h"
#include "ReactCommon/TurboModule.h"

#include "ReactSkia/utils/RnsLog.h"
#include "ReactSkia/utils/RnsUtils.h"

namespace facebook {
namespace react {

using namespace std::chrono;
using namespace folly;
typedef std::chrono::system_clock::time_point SysTimePoint;
typedef std::chrono::microseconds DurationUs;

class RSkTimer;
using SharedTimer = std::shared_ptr<RSkTimer>;
typedef better::map <double, SharedTimer> TimersMap;

class TimingCallback : public HHWheelTimer::Callback {
 public:
  TimingCallback() {}

  void timeoutExpired() noexcept override {
    if (cb)
      cb();
  }
  std::function<void()> cb;
};

class RSkTimer {
 public:
  RSkTimer(
      double callbackId,
      double duration,
      double targetDuration,
      bool repeats)
      : target_(system_clock::now() + milliseconds(static_cast<unsigned long long>(targetDuration))),
      callbackId_(callbackId),
      repeats_(repeats),
      duration_(duration) {
  }

  friend class RSkTimingModule;
 private:
  void reschedule(SysTimePoint baseNow) { // Update repeating timer target
    target_ = baseNow + milliseconds(static_cast<unsigned long long>(duration_));
  }
  SysTimePoint target_;
  double callbackId_;
  bool repeats_;
  double duration_;
};

class RSkTimingModule: public TurboModule {
 public:
  RSkTimingModule(
      const std::string &name,
      std::shared_ptr<CallInvoker> jsInvoker,
      Instance *bridgeInstance);

 private:
  static jsi::Value createTimerWrapper(
      jsi::Runtime &rt,
      TurboModule &turboModule,
      const jsi::Value *args,
      size_t count);

  static jsi::Value deleteTimerWrapper(
      jsi::Runtime &rt,
      TurboModule &turboModule,
      const jsi::Value *args,
      size_t count);

  static jsi::Value setSendIdleEventsWrapper(
      jsi::Runtime &rt,
      TurboModule &turboModule,
      const jsi::Value *args,
      size_t count);

  void immediatelyCallTimer(double callbackId);
  void timerDidFire();

  jsi::Value createTimer(double callbackId, double duration, double jsSchedulingTime, bool repeats);
  jsi::Value deleteTimer(double timerId);
  jsi::Value setSendIdleEvents(bool sendIdleEvents);
  void createTimerForNextFrame(double callbackId, double jsDuration, SysTimePoint jsSchedulingTime, bool repeats);

  bool sendIdleEvents_;
  Instance *bridgeInstance_;
  ScopedEventBaseThread timerThread_;
  TimersMap timers_;
  TimingCallback timerCallback_;
};

} // namespace react
} // namespace facebook
