/*
 * Copyright (C) 1994-2023 OpenTV, Inc. and Nagravision S.A.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "ReactSkia/pluginfactory/RnsPlugin.h"

namespace facebook {
namespace react {
namespace rnsplugin {

RNSPlatformManagerInterface::RNSPlatformManagerInterface(PlatformCallbackClient &client)
    :platformManagerName_(format("RNSPlatformManager-{}",nextUniqueId()).str()),
    platformThread_(!platformManagerName_.empty()? platformManagerName_.c_str(): "RNSPlatformManager-UnknownID"),      //not used for now
    client_(client) {}
RNSPlatformManagerInterface::~RNSPlatformManagerInterface(){
  // platformThread_ will  flush the all theee pending tasks in thread before exiting.
}
RNSPlatformManagerInterface::PlatformDevice::DeviceProcessUintInfo::DeviceProcessUintInfo(string lname, string lcores, string ltechnology, string lfrequency)
    :name(lname),
  cores(lcores),
  technology(ltechnology),
  frequency(lfrequency){}

RNSPlatformManagerInterface::PlatformDevice::DeviceOsInfo::DeviceOsInfo(string osProvider, string osName, string osVersion)
    : provider(osProvider),
  name(osName),
  version(osVersion){}

RNSPlatformManagerInterface::PlatformDevice::DeviceHardwareInfo::DeviceHardwareInfo(string hwManufacturer, string hwModel, string hwType)
    :manufacturer(hwManufacturer),
  model(hwModel),
  type(hwType){}

RNSPlatformManagerInterface::PlatformDevice:: DeviceScreenInfo::DeviceScreenInfo(string screenDepth, int screenWidth, int screenHeight, string screenRefresh)
    :depth(screenDepth),
  width(screenWidth),
  height(screenHeight),
  refresh(screenRefresh){}


uint64_t RNSPlatformManagerInterface::nextUniqueId() {
  static std::atomic<uint64_t> nextId(1);
  uint64_t id;
  do {
    id = nextId.fetch_add(1);
    RNS_LOG_DEBUG("nextUniqueId() : id = " << id);
  } while(id ==0);

  return id;
}

void RNSPlatformManagerInterface::dispatchOnStubEvent() {
  platformThread_.getEventBase()->runInEventBaseThread([&] {
    client_.onStubEvent();
  });
}

} // rnsplugin
} // react
} // facebook
