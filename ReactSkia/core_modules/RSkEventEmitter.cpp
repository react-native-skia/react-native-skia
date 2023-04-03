/*
* Copyright (C) 1994-2023 OpenTV, Inc. and Nagravision S.A.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "RSkEventEmitter.h"

namespace facebook {
namespace react {
RSkEventEmitter::RSkEventEmitter(
    const std::string &name, 
    std::shared_ptr<CallInvoker> jsInvoker,
    Instance *bridgeInstance)
    : TurboModule(name, jsInvoker),
    RSkBaseEventEmitter(bridgeInstance) {

        methodMap_["addListener"] = MethodMetadata{1, addListenerWrapper};
        methodMap_["removeListeners"] = MethodMetadata{1, removeListenersWrapper};
}

jsi::Value RSkEventEmitter::addListenerWrapper(
      jsi::Runtime &rt,
      TurboModule &turboModule,
      const jsi::Value *args,
      size_t count) {
    if (count != 1) {
      return jsi::Value::undefined();
    }
    auto &self = static_cast<RSkEventEmitter &>(turboModule);
    auto nameValue = args[0].getString(rt);
    auto eventName = nameValue.utf8(rt);
    
    // Call specific Event listener in Class object 
    self.addListener(eventName.data());
    return jsi::Value::undefined();
}



jsi::Value RSkEventEmitter::removeListenersWrapper(
      jsi::Runtime &rt,
      TurboModule &turboModule,
      const jsi::Value *args,
      size_t count) {
    if (count != 1) {
      return jsi::Value::undefined();
    }
    auto &self = static_cast<RSkEventEmitter &>(turboModule);
    int removeCount = args[0].getNumber();

    // Call the specific non-static Class object
    self.removeListeners(removeCount);
    return jsi::Value::undefined();
}

} // namespace react
} // namespace facebook
