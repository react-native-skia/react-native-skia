/*
 * Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include<iostream>

#include <jsi/JSIDynamic.h>
#include <jsi/instrumentation.h>

#include "ReactSkia/utils/RnsLog.h"

#ifndef RNS_ANIMATION_FRAME_RATE
#define RNS_ANIMATION_FRAME_RATE 60
#endif
#define RNS_ANIMATION_FRAME_RATE_THROTTLE     ((1/RNS_ANIMATION_FRAME_RATE)) //Milliseconds

using namespace facebook::jsi;

namespace facebook {
namespace react {

class RnsJsRequestAnimation {
  public:
    RnsJsRequestAnimation(const std::function<void(double)>& callback);
    virtual ~RnsJsRequestAnimation() { };

    void start();
    void stop();

    static uint64_t nextUniqueId();

  private :
    std::atomic<bool> isActive_{false};
    Value rafId_;
    std::string callbackName_;
    //HostFunctionType rafNativeCallback_;

    facebook::jsi::Value RnsRequestAnimationFrame();
    void RnsCancelAnimationFrame();
};

} // namespace react
} // namespace facebook