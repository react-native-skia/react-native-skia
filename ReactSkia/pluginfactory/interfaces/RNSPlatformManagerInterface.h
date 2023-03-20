/*
 * Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/io/async/ScopedEventBaseThread.h>
#include <folly/Format.h>

#include "ReactSkia/pluginfactory/RnsPlugin.h"

namespace facebook {
namespace react {
namespace rnsplugin {

using namespace folly;
using namespace std;
using folly::format;
using std::string;
using std::vector;


class RNSPlatformManagerInterface {
 public:
  class CallbackClient {
   public:
    virtual ~CallbackClient() {}
    // Events
    virtual void onStubEvent() = 0;
  };

  class PlatformDevice {
   public:
    PlatformDevice() = default;
    virtual ~PlatformDevice() {};

    struct DeviceProcessUintInfo{
      DeviceProcessUintInfo() = delete;
      DeviceProcessUintInfo(string lname, string lcores, string ltechnology, string lfrequency);

      string name;
      string cores;
      string technology;
      string frequency;
    };

    struct DeviceOsInfo{
      DeviceOsInfo() = delete;
      DeviceOsInfo(string provider, string name, string version);

      string provider;
      string name;
      string version;
    };

    struct DeviceHardwareInfo{
      DeviceHardwareInfo() = delete;
      DeviceHardwareInfo(string hwManufacturer, string hwModel, string hwType);

      string manufacturer;
      string model;
      string type;
    };

    struct DeviceScreenInfo{
      DeviceScreenInfo() = delete;
      DeviceScreenInfo(string screenDepth,  int screenWidth,int screenHeight, string screenRefresh);

      string depth;
      int width;
      int height;
      string refresh;
    };

    using DeviceCPUInfo = DeviceProcessUintInfo;
    using DeviceGPUInfo = DeviceProcessUintInfo;

    string interfaceIdiom {"tv"};
    bool forceTouchAvailable{false};
    virtual bool isTesting() = 0;
    virtual shared_ptr<DeviceCPUInfo> getCPUInfo()=0;
    virtual shared_ptr<DeviceGPUInfo> getGPUInfo()=0;
    virtual shared_ptr<DeviceOsInfo> getOsInfo()=0;
    virtual shared_ptr<DeviceHardwareInfo> getHardwareInfo()=0;
    virtual shared_ptr<DeviceScreenInfo> getScreenInfo()=0;
  };

  RNSPlatformManagerInterface(CallbackClient &client);
  virtual ~RNSPlatformManagerInterface();

  virtual shared_ptr<RNSPlatformManagerInterface::PlatformDevice> currentDevice() = 0;

 private:

  static uint64_t nextUniqueId();
  string platformManagerName_{};
  ScopedEventBaseThread platformThread_;
  CallbackClient &client_;

 protected:
  void dispatchOnStubEvent();
};

using PlatformCallbackClient = RNSPlatformManagerInterface::CallbackClient;
using RNSPlatformManagerInterface_t = RNSPlatformManagerInterface *(*)(PlatformCallbackClient &);

} // rnsplugin
} // react
} // facebook
