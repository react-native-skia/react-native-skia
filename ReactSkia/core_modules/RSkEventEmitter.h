/*
* Copyright (C) 1994-2023 OpenTV, Inc. and Nagravision S.A.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#pragma once

#include "ReactCommon/TurboModule.h"
#include "ReactSkia/LegacyNativeModules/RSkBaseEventEmitter.h"

namespace facebook {
namespace react {

using EmitterCompleteVoidCallback = std::function<void()>;

class RSkEventEmitter: public TurboModule, public RSkBaseEventEmitter {
    public:
      RSkEventEmitter(
        const std::string &name, 
        std::shared_ptr<CallInvoker> jsInvoker,
        Instance *bridgeInstance);

      virtual std::vector<std::string> supportedEvents() {
        // Return empty vector by default
        return {};
      }

    
    private:
      static jsi::Value addListenerWrapper(
        jsi::Runtime &rt,
        TurboModule &turboModule,
        const jsi::Value *args,
        size_t count);
      
      static jsi::Value removeListenersWrapper(
        jsi::Runtime &rt,
        TurboModule &turboModule,
        const jsi::Value *args,
        size_t count);

};
} // namespace react
} // namespace facebook
