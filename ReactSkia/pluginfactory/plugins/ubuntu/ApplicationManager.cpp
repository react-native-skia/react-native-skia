/*
 * Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "ReactSkia/pluginfactory/plugins/ubuntu/ApplicationManager.h"

namespace facebook {
namespace react {
namespace rnsplugin {

RNSApplicationManagerInterfaceImpl::RNSApplicationManagerInterfaceImpl(AppCallbackClient& client)
    : INHERITED(client),
      cgMemHandle_(initializeCgroupMemory(std::bind(&RNSApplicationManagerInterfaceImpl::handleMemoryNotification, this, std::placeholders::_1))) {
  // FIXME This needs to be removed once we add proper application manager code
  dispatchOnChange("active");
  dispatchOnFocus();
}

RNSApplicationManagerInterfaceImpl::~RNSApplicationManagerInterfaceImpl(){}

void RNSApplicationManagerInterfaceImpl::handleMemoryNotification(int efd) {
  dispatchOnMemoryWarning();
}

RNSApplicationManagerInterfaceImpl::CgroupMemoryData&  RNSApplicationManagerInterfaceImpl::initializeCgroupMemory(CgroupMemoryNotificationCallback cb) {
  static std::once_flag onceFlag;
  static std::unique_ptr<CgroupMemoryData> cgMemHandle;
  std::call_once(onceFlag, [cb]{
    cgMemHandle =  std::unique_ptr<CgroupMemoryData>(new CgroupMemoryData(cb));
  });
  return *cgMemHandle;
}

// AppState Plugin Object Creator
extern "C" {
RNSP_EXPORT RNSApplicationManagerInterface* RnsPluginAppManagerCreate(AppCallbackClient &client) {
  return new RNSApplicationManagerInterfaceImpl(client);
}
} // extern C

} // rnsplugin
} // react
} // facebook
