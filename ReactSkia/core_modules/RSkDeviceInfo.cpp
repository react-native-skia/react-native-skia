/*
* Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
*
* This source code is licensed under the MIT license found in the
* LICENSE file in the root directory of this source tree.
*/
#include "jsi/JSIDynamic.h"

#include "ReactCommon/TurboModule.h"

#include "ReactSkia/utils/RnsLog.h"
#include "ReactSkia/JSITurboModuleManager.h"
#include "RSkDeviceInfo.h"

namespace facebook {
namespace react {

RSkDeviceInfoModule::RSkDeviceInfoModule(
  const std::string &name,
  std::shared_ptr<CallInvoker> jsInvoker,
  Instance *bridgeInstance)
  : RSkEventEmitter(name, jsInvoker, bridgeInstance) {
  methodMap_["getConstants"] = MethodMetadata{0, getConstants};

}

jsi::Value RSkDeviceInfoModule::getConstants(
  jsi::Runtime &rt,
  TurboModule &turboModule,
  const jsi::Value *args,
  size_t count)  {

  if (count != 0) {
      return jsi::Value::undefined();
  }

  SkSize screenSize = RnsShell::PlatformDisplay::sharedDisplay().screenSize();
  SkSize mainWindowSize = RnsShell::Window::getMainWindowSize();
  auto windowMetrics = folly::dynamic::object("width", mainWindowSize.width())("height", mainWindowSize.height())(
      "scale", 1)("fontScale", 1);
  auto screenMetrics = folly::dynamic::object("width", screenSize.width())("height", screenSize.height())(
      "scale", 1)("fontScale", 1);
  auto dimension = folly::dynamic::object("window", std::move(windowMetrics))(
      "screen", std::move(screenMetrics));
  return jsi::valueFromDynamic(rt, folly::dynamic::object("Dimensions", std::move(dimension)));
}

}// namespace react
}//namespace facebook
