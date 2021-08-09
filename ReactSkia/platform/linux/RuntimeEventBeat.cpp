/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <thread>

#include "RuntimeEventBeat.h"
#include "ReactSkia/utils/RnsLog.h"

#define BEAT_INTERVAL  1 /*Temp:1 sec*/

namespace facebook {
namespace react {

namespace {

/*
EventBeatThread : to push the dispatched asynchronous event to the JS world
TODO: BEAT_INTERVAL to be refined to the need.
*/
void BeatThread(RuntimeEventBeat *runtimeEventBeat,RunLoopObserver::Activity activities) {
  while(RuntimeEventBeat::keepBeating) {
    sleep(BEAT_INTERVAL);
    runtimeEventBeat->triggerBeat(activities);
  }
}

}//namespace

bool RuntimeEventBeat::keepBeating{true};
/*Activity is dummy param to fit in React common implementation, It is not needed as
  RNS implementation is not based on runloop*/
RunLoopObserver::Activity activities{RunLoopObserver::Activity::BeforeWaiting};
RuntimeEventBeat::RuntimeEventBeat(RunLoopObserver::WeakOwner const &owner)
  : RunLoopObserver(activities, owner) {
  /*Spawning  Beat Thread*/
  std::thread thread(BeatThread,this,activities);
  thread.detach();
}

RuntimeEventBeat::~RuntimeEventBeat() {
  RuntimeEventBeat::keepBeating=false; /*Signal BeatThread to stop Beating*/
}

void RuntimeEventBeat::triggerBeat(RunLoopObserver::Activity activities){
  this->activityDidChange(activities);
}
void RuntimeEventBeat::startObserving() const noexcept
{
  RNS_LOG_NOT_IMPL;
  /*Start Beating once the Beat thread created*/
}

void RuntimeEventBeat::stopObserving() const noexcept
{
  RNS_LOG_NOT_IMPL;
  /*TODO:Need to check the usecase for stop & start beating
         Based on the Beat thread to be redesigned*/
}

bool RuntimeEventBeat::isOnRunLoopThread() const noexcept
{
  /*As Beat thread is not based on runLoop, having dummy implementation to fit in to
    React common runlooponsrever design need*/
  return true;
}

} // namespace react
} // namespace facebook
