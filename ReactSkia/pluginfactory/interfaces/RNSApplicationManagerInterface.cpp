/*
 * Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "ReactSkia/pluginfactory/RnsPlugin.h"

namespace facebook {
namespace react {
namespace rnsplugin {

RNSApplicationManagerInterface::RNSApplicationManagerInterface(AppCallbackClient &client)
    :appStateName_(format("RNSAppState-{}",nextUniqueId()).str()),
    appStateThread_(!appStateName_.empty()? appStateName_.c_str(): "RNSAppState-UnknownID"),      //not used for now
    client_(client) {}

RNSApplicationManagerInterface::~RNSApplicationManagerInterface(){
  // appStateThread_ will  flush the all theee pending tasks in thread before exiting.
}

uint64_t RNSApplicationManagerInterface::nextUniqueId() {
  static std::atomic<uint64_t> nextId(1);
  uint64_t id;
  do {
    id = nextId.fetch_add(1);
    RNS_LOG_DEBUG("nextUniqueId() : id = " << id);
  } while(id ==0);

  return id;
}

void RNSApplicationManagerInterface::dispatchOnChange(std::string nextAppState) {
  if (currentState_ == nextAppState)
    return;
  currentState_ = nextAppState;
  appStateThread_.getEventBase()->runInEventBaseThread([&] {
    client_.onChange(currentState_);
  });
}

void RNSApplicationManagerInterface::dispatchOnMemoryWarning() {
  appStateThread_.getEventBase()->runInEventBaseThread([&] {
    client_.onMemoryWarning();
  });
}

void RNSApplicationManagerInterface::dispatchOnFocus() {
  appStateThread_.getEventBase()->runInEventBaseThread([&] {
    client_.onFocus();
  });
}

void RNSApplicationManagerInterface::dispatchOnBlur() {
  appStateThread_.getEventBase()->runInEventBaseThread([&] {
    client_.onBlur();
  });
}

} // rnsplugin
} // react
} // facebook
