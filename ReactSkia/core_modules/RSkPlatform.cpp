/*
* Copyright (C) 1994-2023 OpenTV, Inc. and Nagravision S.A.
*
* This source code is licensed under the MIT license found in the
* LICENSE file in the root directory of this source tree.
*/


#include "ReactCommon/TurboModule.h"


#include "ReactSkia/core_modules/RSkPlatform.h"
#include "ReactSkia/utils/RnsLog.h"
#include "ReactSkia/utils/RnsUtils.h"

#include "version.h"

namespace facebook {
namespace react {


RNSP_EXPORT RSkPlatformModule::PlatformCallBackClient::PlatformCallBackClient(RSkPlatformModule& platformModule)
  : platformModule_(platformModule){}

RSkPlatformModule::RSkPlatformModule(
  const std::string &name,
  std::shared_ptr<CallInvoker> jsInvoker,
  Instance *bridgeInstance)
    : TurboModule(name, jsInvoker),
      bridgeInstance_(bridgeInstance),
      pluginFactory_(new RnsPluginFactory),
			platformCallBackClient_(*this) {
  RNS_UNUSED(bridgeInstance_);
  methodMap_["getConstants"] = MethodMetadata{0, getConstants};
}

RSkPlatformModule::~RSkPlatformModule(){}

jsi::Value RSkPlatformModule::getConstants(
  jsi::Runtime &rt,
  TurboModule &turboModule,
  const jsi::Value *args,
  size_t count)  {

  if (count != 0) {
    return jsi::Value::undefined();
  }
  auto &self = static_cast<RSkPlatformModule &>(turboModule);
  return jsi::valueFromDynamic(rt, self.getConstants());
}

void RSkPlatformModule::lazyInit() {
  if(platformManagerHandle_ == nullptr) {
    RNS_LOG_DEBUG("Creating platform handle from Plugin Factory");
    platformManagerHandle_ = pluginFactory_->createPlatformManagerHandle(platformCallBackClient_);
    if(platformManagerHandle_ == nullptr) {
      RNS_LOG_ERROR("Could not get Platform handle from RNS platform Plugin");
    } else {
      RNS_LOG_DEBUG(this << " : RNS Platform Plugin Loaded with Platform interface : " << platformManagerHandle_.get() << " : Thread : " << std::this_thread::get_id());
    }
  }
}

folly::dynamic RSkPlatformModule::getConstants() {
  lazyInit();
  auto rnVersion = folly::dynamic::object("major", RN_MAJOR_VERSION)("minor", RN_MINOR_VERSION)("patch", RN_PATCH_VERSION)("prerelease", RN_PRERELEASE_VERSION);  
  shared_ptr<RNSPlatformManagerInterface::PlatformDevice> device = nullptr;
  dynamic platformConstants = folly::dynamic::object("forceTouchAvailable", false)
    ("reactNativeVersion", std::move(rnVersion))
    ("osVersion", "0.0")
    ("systemName", "Unknown")
    ("interfaceIdiom", "tv")
    ("isTesting", false);

  if(platformManagerHandle_ &&
    (device = platformManagerHandle_->currentDevice())) {
    shared_ptr<RNSPlatformManagerInterface::PlatformDevice::DeviceOsInfo> osInfo = device->getOsInfo();
    platformConstants["forceTouchAvailable"] = device->forceTouchAvailable;
    platformConstants["interfaceIdiom"] = device->interfaceIdiom;
    platformConstants["isTesting"] = device->isTesting();
    if(osInfo) {
      platformConstants["osVersion"] = osInfo->version;
      platformConstants["systemName"] = osInfo->name;
    }
  }

  return platformConstants;
}

void RSkPlatformModule::PlatformCallBackClient::onSafeAreaInsetsDidChange() {
  RNS_LOG_DEBUG("onSafeAreaInsetsDidChange Received");
  RNS_UNUSED(platformModule_);
}

}// namespace react
}//namespace facebook
