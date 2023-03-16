/*
 * Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
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
