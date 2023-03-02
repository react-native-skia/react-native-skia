/*
* Copyright (C) 1994-2023 OpenTV, Inc. and Nagravision S.A.
*
* This source code is licensed under the MIT license found in the
* LICENSE file in the root directory of this source tree.
*/

#include "jsi/JSIDynamic.h"

#include "ReactCommon/TurboModule.h"

#include "ReactSkia/JSITurboModuleManager.h"
#include "RSkPlatform.h"

#include "version.h"

namespace facebook {
namespace react {

RSkPlatformModule::RSkPlatformModule(
  const std::string &name,
  std::shared_ptr<CallInvoker> jsInvoker,
  Instance *bridgeInstance)
    : TurboModule(name, jsInvoker), bridgeInstance_(bridgeInstance) {
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

folly::dynamic RSkPlatformModule::getConstants() {
  auto rnVersion = folly::dynamic::object("major", RN_MAJOR_VERSION)("minor", RN_MINOR_VERSION)("patch", RN_PATCH_VERSION)("prerelease", RN_PRERELEASE_VERSION);  
  auto platformConstants = folly::dynamic::object("forceTouchAvailable", false)
    ("reactNativeVersion", std::move(rnVersion)) ("osVersion", STRINGIFY(RNS_OS_VERSION)) ("systemName", STRINGIFY(Ubuntu/Linux))
#if defined(TARGET_OS_TV) && TARGET_OS_TV
    ("interfaceIdiom", STRINGIFY(tv))
#else
    ("interfaceIdiom", STRINGIFY(unknown))
#endif //TARGET_OS_TV
    ("isTesting", true);

  return platformConstants;
}

}// namespace react
}//namespace facebook
