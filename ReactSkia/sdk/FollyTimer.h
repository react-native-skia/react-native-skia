/*
* Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
*
* This source code is licensed under the MIT license found in the
* LICENSE file in the root directory of this source tree.
*/

#pragma once


#include <folly/portability/SysTypes.h>
#include <folly/executors/ScheduledExecutor.h>
#include <folly/io/async/ScopedEventBaseThread.h>
#include <folly/io/async/HHWheelTimer.h>


#include "ReactSkia/utils/RnsLog.h"
#include "ReactSkia/utils/RnsUtils.h"

namespace rns {
namespace sdk {

using namespace std::chrono;
using namespace folly;
typedef std::chrono::system_clock::time_point SysTimePoint;
typedef std::chrono::microseconds DurationUs;

class TimingCallback : public HHWheelTimer::Callback {
 public:
  TimingCallback() {}

  void timeoutExpired() noexcept override {
    if (cb)
      cb();
  }
  std::function<void()> cb;
  std::mutex cbLock;
};

class Timer {
 public:
  Timer(double duration,bool repeats,std::function<void()> cb,bool autostart=false);
  ~Timer() { abort(); }
  void start();
  void reschedule(double duration,bool repeats);
  void abort();
  void scheduleTimerTimeout();
  double getTimeRemaining();
  static SysTimePoint getFutureTime();

 private:

  double targetDuration_{0};
  bool repeats_{false};
  std::function<void()> cb_{nullptr};

  ScopedEventBaseThread timerThread_;
  TimingCallback timerCallback_;
};

} // namespace sdk
} // namespace rns
