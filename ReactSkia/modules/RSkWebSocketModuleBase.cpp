/*  * Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.  
*  * This source code is licensed under the MIT license found in the  
* LICENSE file in the root directory of this source tree.  */
#include <glog/logging.h>

#include "cxxreact/Instance.h"
#include "jsi/JSIDynamic.h"

#include <ReactCommon/TurboModuleUtils.h>
#include "ReactCommon/TurboModule.h"

#include "ReactSkia/JSITurboModuleManager.h"
#include "RSkWebSocketModuleBase.h"

namespace facebook {
namespace react {

RSkWebSocketModuleBase::RSkWebSocketModuleBase(
    const std::string &name,
    std::shared_ptr<CallInvoker> jsInvoker,
    Instance *bridgeInstance)
    : TurboModule(name, jsInvoker) {

    methodMap_["connect"] = MethodMetadata{4, getConnectWrapper};
    methodMap_["send"] = MethodMetadata{2, NoOp};
    methodMap_["sendBinary"] = MethodMetadata{2, NoOp};
    methodMap_["ping"] = MethodMetadata{1, NoOp};
    methodMap_["close"] = MethodMetadata{3, getCloseWrapper};
    methodMap_["addListener"] = MethodMetadata{1, NoOp};
    methodMap_["removeListeners"] = MethodMetadata{1, NoOp};

}

jsi::Value RSkWebSocketModuleBase::getConnectWrapper(
     jsi::Runtime &rt,
     TurboModule &turboModule,
     const jsi::Value *args,
     size_t count)  {

  if(count != 4) {
      return jsi::Value::undefined();
  }
  auto &self = static_cast<RSkWebSocketModuleBase &>(turboModule);
  std::string url = args[0].getString(rt).utf8(rt);
  auto protocols = jsi::dynamicFromValue(rt, args[1]);
  auto options = jsi::dynamicFromValue(rt, args[2]);
  int socketID = args[3].getNumber();
 
  return self.getConnect(url, protocols, options, socketID);
}

jsi::Value RSkWebSocketModuleBase::getCloseWrapper(
     jsi::Runtime &rt,
     TurboModule &turboModule,
     const jsi::Value *args,
     size_t count)  {
  if(count != 3) {
      return jsi::Value::undefined();
  }
  auto &self = static_cast<RSkWebSocketModuleBase &>(turboModule);
  int code = args[0].getNumber();
  std::string reason = args[1].getString(rt).utf8(rt);
  int socketID = args[2].getNumber();

  return self.getClose(code, reason.c_str(), socketID);
}

}// namespace react
}// namespace facebook
