/*
 * Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <sstream>

#include "ReactSkia/RNInstance.h"

#include "ReactSkia/utils/RnsJsRaf.h"
#include "ReactSkia/utils/RnsLog.h"

namespace facebook {
namespace react {

#define RNSJSRAF_CALLBACK_PREFIX "__rnsOnAnimationFrameCallback"

RnsJsRequestAnimation::RnsJsRequestAnimation(const std::function<void(double)>& callback) {

  std::ostringstream ss;
  ss << RNSJSRAF_CALLBACK_PREFIX << nextUniqueId();
  callbackName_ = ss.str();

  std::cout<<" RnsJsRequestAnimation" <<std::endl;

  auto runtime = RNInstance::RskJsRuntime();

  runtime->global().setProperty(
    *runtime,
    callbackName_.c_str(),
    jsi::Function::createFromHostFunction(
      *runtime,
      jsi::PropNameID::forAscii(*runtime, callbackName_.c_str()),
      1,
      [this, callback](jsi::Runtime &runtime,
        jsi::Value const &thisValue,
        const jsi::Value *args,
        size_t count) {
          if(count >= 1) {
            static double previousValue = args[0].asNumber();
            if(((args[0].asNumber()) - previousValue) > (RNS_ANIMATION_FRAME_RATE_THROTTLE * 1000)){
              callback(args[0].asNumber());
              previousValue = args[0].asNumber();
            }
            rafId_ = RnsRequestAnimationFrame();
          } else {
            RNS_LOG_WARN("Invalid number of argumentd for RAF callback");
          }
          return jsi::Value();
        }));
}

uint64_t RnsJsRequestAnimation::nextUniqueId() {
  static std::atomic<uint64_t> nextId(1);
  uint64_t id;
  do {
    id = nextId.fetch_add(1);
  } while (id == 0);  // 0 invalid id.
  return id;
}

Value RnsJsRequestAnimation::RnsRequestAnimationFrame() {
  if(!isActive_) {
    return jsi::Value(0);
  }
  auto runtime = RNInstance::RskJsRuntime();
  return runtime->global()
    .getPropertyAsFunction(*runtime, "requestAnimationFrame")
    .call(*runtime, runtime->global().getPropertyAsFunction(*runtime, callbackName_.c_str()));
}

void RnsJsRequestAnimation::RnsCancelAnimationFrame() {
  auto runtime = RNInstance::RskJsRuntime();
  if(rafId_.asNumber()) {
    runtime->global()
      .getPropertyAsFunction(*runtime, "cancelAnimationFrame")
      .call(*runtime, rafId_);
  }
}

void RnsJsRequestAnimation::start() {
  if(isActive_ == false){
    isActive_ = true;
    rafId_ = RnsRequestAnimationFrame();
  }
}

void RnsJsRequestAnimation::stop() {
  if(isActive_ == true){
    isActive_ = false;
    RnsCancelAnimationFrame();
  }
}

} // namespace react
} // namespace facebook
