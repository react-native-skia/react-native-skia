/*
* Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
*
* This source code is licensed under the MIT license found in the
* LICENSE file in the root directory of this source tree.
*/
#pragma once

#include "jsi/JSIDynamic.h"
#include "ReactCommon/TurboModule.h"

#include "ReactSkia/JSITurboModuleManager.h"

#include "ReactSkia/sdk/Alert.h"

namespace facebook {
namespace react {

class RSkAlertManager : public TurboModule {
  public:

    RSkAlertManager(
    	const std::string &name,
        const std::shared_ptr<facebook::react::CallInvoker> jsInvoker,
        facebook::react::Instance *bridgeInstance );

    ~RSkAlertManager() = default;

    static facebook::jsi::Value alertWithArgsWrapper(
        facebook::jsi::Runtime &rt,
        TurboModule &turboModule,
        const facebook::jsi::Value *args,
        size_t count);

  private:

    jsi::Value processAlertMessages(rns::sdk::Alert::alertInfo &alertProps);

};
}//react
}//facebook
