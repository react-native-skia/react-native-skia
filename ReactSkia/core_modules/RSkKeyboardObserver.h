/*
* Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "core_modules/RSkEventEmitter.h"
#include "utils/RnsLog.h"

namespace facebook {
namespace react {

class RSkKeyboardObserver final: public RSkEventEmitter {
 public:
  RSkKeyboardObserver(
      const std::string &name,
      std::shared_ptr<CallInvoker> jsInvoker,
      Instance *bridgeInstance)
      : RSkEventEmitter(name, jsInvoker, bridgeInstance){}

 private:
  void startObserving() { RNS_LOG_NOT_IMPL;};

  void stopObserving() { RNS_LOG_NOT_IMPL;};

};

} //namespace react
} //namespace facebook