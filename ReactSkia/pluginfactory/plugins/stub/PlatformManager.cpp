/*
 * Copyright (C) 1994-2023 OpenTV, Inc. and Nagravision S.A.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "ReactSkia/pluginfactory/plugins/stub/PlatformManager.h"

namespace facebook {
namespace react {
namespace rnsplugin {

RNSPlatformManagerInterfaceImpl::RNSPlatformManagerInterfaceImpl(PlatformCallbackClient& client)
    : INHERITED(client) {
  dispatchOnStubEvent();
}

RNSPlatformManagerInterfaceImpl::~RNSPlatformManagerInterfaceImpl(){}

shared_ptr<RNSPlatformManagerInterface::PlatformDevice> RNSPlatformManagerInterfaceImpl::currentDevice() {
  RNS_LOG_TODO("This is a stub implementation. use proper singleton object for current device");
  if(currentDevice_ == nullptr) {
    currentDevice_ = make_shared<PlatformDeviceImpl>();
  }
  return currentDevice_;
}

bool RNSPlatformManagerInterfaceImpl::PlatformDeviceImpl::isTesting() {
  return true;
}
shared_ptr<DeviceCPUInfo> RNSPlatformManagerInterfaceImpl::PlatformDeviceImpl::getCPUInfo() {
  if(cpuInfo_ == nullptr) {
    cpuInfo_ = make_shared<DeviceCPUInfo>("Intel","4","5nm","5Ghz");
  }
  return cpuInfo_;
}

shared_ptr<DeviceGPUInfo> RNSPlatformManagerInterfaceImpl::PlatformDeviceImpl::getGPUInfo() {
  if(gpuInfo_ == nullptr) {
    gpuInfo_ = make_shared<DeviceGPUInfo>("Nvidia","8","6nm","2.4Ghz");
  }
  return gpuInfo_;
}

shared_ptr<DeviceOsInfo> RNSPlatformManagerInterfaceImpl::PlatformDeviceImpl::getOsInfo() {
  if(osInfo_ == nullptr) {
    osInfo_ = make_shared<DeviceOsInfo>("UbuntuDev","GNU/Linux","20.2");
  }
  return osInfo_;
}

shared_ptr<DeviceHardwareInfo> RNSPlatformManagerInterfaceImpl::PlatformDeviceImpl::getHardwareInfo() {
  if(hardwareInfo_ == nullptr) {
    hardwareInfo_ = make_shared<DeviceHardwareInfo>("SAGEM","7250","ARM");
  }
  return hardwareInfo_;
}

shared_ptr<DeviceScreenInfo> RNSPlatformManagerInterfaceImpl::PlatformDeviceImpl::getScreenInfo() {
  if(screenInfo_ == nullptr) {
    screenInfo_ = make_shared<DeviceScreenInfo>("420dpi", 1920, 1080, "60Hz");
  }
  return screenInfo_;
}

// Platform Plugin Object Creator
extern "C" {
RNSP_EXPORT RNSPlatformManagerInterface* RnsPluginPlatformCreate(PlatformCallbackClient &client) {
  return new RNSPlatformManagerInterfaceImpl(client);
}
} // extern C

} // rnsplugin
} // react
} // facebook
