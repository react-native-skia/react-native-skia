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
  if(platformManagerHandle_== nullptr){
    return folly::dynamic::object();
  }
  shared_ptr<RNSPlatformManagerInterface::PlatformDevice>  device = platformManagerHandle_->currentDevice();
  shared_ptr<RNSPlatformManagerInterface::PlatformDevice::DeviceOsInfo> osInfo = device->getOsInfo();

  auto rnVersion = folly::dynamic::object("major", RN_MAJOR_VERSION)("minor", RN_MINOR_VERSION)("patch", RN_PATCH_VERSION)("prerelease", RN_PRERELEASE_VERSION);  
  auto platformConstants = folly::dynamic::object("forceTouchAvailable", device->forceTouchAvailable)
    ("reactNativeVersion", std::move(rnVersion))
    ("osVersion", osInfo->version)
    ("systemName", osInfo->name)
    ("interfaceIdiom", device->interfaceIdiom)
    ("isTesting", device->isTesting());

  return platformConstants;
}

void RSkPlatformModule::PlatformCallBackClient::onStubEvent() {
  RNS_LOG_DEBUG("onStubEventReceived");
  RNS_UNUSED(platformModule_);
}

}// namespace react
}//namespace facebook
