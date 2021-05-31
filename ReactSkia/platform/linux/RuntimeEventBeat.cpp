/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <thread>

#include "RuntimeEventBeat.h"
#include "ReactSkia/utils/RnsLog.h"
<<<<<<< HEAD

#define BEAT_INTERVAL  1000 /*unit ms. Beat interval Set to:1 sec as temp fix*/
=======
>>>>>>> RNS Shell Implementation  (#8)

namespace facebook {
namespace react {

<<<<<<< HEAD

/*Activity is dummy param to fit in React common implementation, It is not needed as
  RNS implementation is not based on runloop*/
RunLoopObserver::Activity activities{RunLoopObserver::Activity::BeforeWaiting};
RuntimeEventBeat::RuntimeEventBeat(RunLoopObserver::WeakOwner const &owner)
  : RunLoopObserver(activities, owner),baetThread_("BeatThread"),activities_{activities} {
    baetThread_.getEventBase()->runInEventBaseThread(std::bind(&RuntimeEventBeat::beat,this));
}

RuntimeEventBeat::~RuntimeEventBeat() {
  baetThread_.getEventBase()->terminateLoopSoon();
}

void RuntimeEventBeat::startObserving() const noexcept
{
  RNS_LOG_NOT_IMPL;
/* Start Beating once the Beat thread created*/
}

void RuntimeEventBeat::beat(){
  this->activityDidChange(activities_);
  baetThread_.getEventBase()->scheduleAt(std::bind(&RuntimeEventBeat::beat,this), \
           std::chrono::steady_clock::now() + std::chrono::milliseconds(BEAT_INTERVAL));
}

void RuntimeEventBeat::stopObserving() const noexcept
{
  RNS_LOG_NOT_IMPL;
/* TODO:Need to check the usecase for stop & start beating
       Based on the Beat thread to be redesigned*/
}

bool RuntimeEventBeat::isOnRunLoopThread() const noexcept
{
  return (baetThread_.getThreadId() == std::thread::id());
=======
RuntimeEventBeat::RuntimeEventBeat(EventBeat::SharedOwnerBox const &ownerBox, RuntimeExecutor runtimeExecutor)
    : EventBeat(ownerBox), runtimeExecutor_(std::move(runtimeExecutor)) {
    RNS_LOG_NOT_IMPL;
}

RuntimeEventBeat::~RuntimeEventBeat() {
    RNS_LOG_NOT_IMPL;
}

void RuntimeEventBeat::induce() const {
    RNS_LOG_NOT_IMPL;
>>>>>>> RNS Shell Implementation  (#8)
}

} // namespace react
} // namespace facebook
