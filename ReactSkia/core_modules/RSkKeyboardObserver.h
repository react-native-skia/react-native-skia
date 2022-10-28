/*
* Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "core_modules/RSkEventEmitter.h"
#include "utils/RnsLog.h"
#include "utils/RnsUtils.h"

namespace facebook {
namespace react {

class RSkKeyboardObserver final: public RSkEventEmitter {
 public:
  RSkKeyboardObserver(
      const std::string &name,
      std::shared_ptr<CallInvoker> jsInvoker,
      Instance *bridgeInstance);
  void handleOnScreenKeyboardEventNotification(std::string eventType);
 private:
  void startObserving();
  void stopObserving();
#if ENABLE(FEATURE_ONSCREEN_KEYBOARD)
  unsigned int onscreenKeyboardEventId_ = UINT_MAX;
#endif
};

} //namespace react
} //namespace facebook
