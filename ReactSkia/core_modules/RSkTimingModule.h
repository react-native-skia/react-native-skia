/*
* Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
*
* This source code is licensed under the MIT license found in the
* LICENSE file in the root directory of this source tree.
*/

#pragma once

#include <butter/map.h>

#include "cxxreact/Instance.h"
#include "ReactCommon/TurboModule.h"

#include "ReactSkia/sdk/FollyTimer.h"
#include "ReactSkia/utils/RnsLog.h"
#include "ReactSkia/utils/RnsUtils.h"

namespace facebook {
namespace react {

using namespace std::chrono;
using namespace folly;
using namespace rns::sdk;

class RSkJsTimer;
using SharedJsTimer = std::shared_ptr<RSkJsTimer>;
typedef butter::map <double, SharedJsTimer> JsTimersMap;

class RSkJsTimer {
 public:
  RSkJsTimer(
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

  void timerDidFire();
  void immediatelyCallTimer(double callbackId);

  jsi::Value createTimer(double callbackId, double duration, double jsSchedulingTime, bool repeats);
  jsi::Value deleteTimer(double timerId);
  jsi::Value setSendIdleEvents(bool sendIdleEvents);
  void createTimerForNextFrame(double callbackId, double jsDuration, SysTimePoint jsSchedulingTime, bool repeats);

  bool sendIdleEvents_;
  Instance *bridgeInstance_;
  Timer * timer_{nullptr};
  JsTimersMap jsTimers_;
  std::mutex jsTimerList_;
};

} // namespace react
} // namespace facebook
