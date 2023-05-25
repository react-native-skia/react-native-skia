/*
 * Copyright (C) 1994-2023 OpenTV, Inc. and Nagravision S.A.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */
#pragma once

#include "ReactSkia/pluginfactory/RnsPlugin.h"

namespace facebook {
namespace react {
namespace rnsplugin {

using namespace std;

using DeviceCPUInfo = RNSPlatformManagerInterface::PlatformDevice::DeviceProcessUnitInfo;
using DeviceGPUInfo = RNSPlatformManagerInterface::PlatformDevice::DeviceProcessUnitInfo;
using DeviceOsInfo = RNSPlatformManagerInterface::PlatformDevice::DeviceOsInfo;
using DeviceHardwareInfo = RNSPlatformManagerInterface::PlatformDevice::DeviceHardwareInfo;
using DeviceScreenInfo = RNSPlatformManagerInterface::PlatformDevice::DeviceScreenInfo;

class RNSPlatformManagerInterfaceImpl : public RNSPlatformManagerInterface {
 public:
  RNSPlatformManagerInterfaceImpl(PlatformCallbackClient &client);
  ~RNSPlatformManagerInterfaceImpl()override;

  shared_ptr<RNSPlatformManagerInterface::PlatformDevice> currentDevice() override;
 private:
  class PlatformDeviceImpl : public RNSPlatformManagerInterface::PlatformDevice {
   public:
    PlatformDeviceImpl() = default;
    ~PlatformDeviceImpl() {};
    bool isTesting() override;
    SkRect getSafeAreaInsets() override;
    shared_ptr<DeviceCPUInfo> getCPUInfo() override;
    shared_ptr<DeviceGPUInfo> getGPUInfo() override;
    shared_ptr<DeviceOsInfo> getOsInfo() override;
    shared_ptr<DeviceHardwareInfo> getHardwareInfo() override;
    shared_ptr<DeviceScreenInfo> getScreenInfo() override;
   private:
    shared_ptr<DeviceCPUInfo> cpuInfo_ = nullptr;
    shared_ptr<DeviceGPUInfo> gpuInfo_ = nullptr;
    shared_ptr<DeviceOsInfo> osInfo_ = nullptr;
    shared_ptr<DeviceHardwareInfo> hardwareInfo_ = nullptr;
    shared_ptr<DeviceScreenInfo> screenInfo_ = nullptr;
  };
 
  shared_ptr<RNSPlatformManagerInterface::PlatformDevice> currentDevice_ {nullptr};

  typedef RNSPlatformManagerInterface INHERITED;
};

extern "C" {
  RNSP_EXPORT RNSPlatformManagerInterface * RnsPluginPlatformCreate(PlatformCallbackClient &client);
}

} // rnsplugin
} // react
} // facebook
