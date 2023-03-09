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

// Platform Plugin Object Creator
extern "C" {
RNSP_EXPORT RNSPlatformManagerInterface* RnsPluginPlatformCreate(PlatformCallbackClient &client) {
  return new RNSPlatformManagerInterfaceImpl(client);
}
} // extern C

} // rnsplugin
} // react
} // facebook
