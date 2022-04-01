/*
 * Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once
#include "ReactSkia/core_modules/RSkSpatialNavigator.h"

namespace facebook {
namespace react {
class RSkInputEventManager{
 private:
  static RSkInputEventManager *sharedInputEventManager_;
  RSkInputEventManager();
  SpatialNavigator::RSkSpatialNavigator* spatialNavigator_;
 public:
  static RSkInputEventManager* getInputKeyEventManager();
  void keyHandler(rnsKey key, rnsKeyAction eventKeyAction);
#if defined(TARGET_OS_TV) && TARGET_OS_TV
  void sendNotificationWithEventType(std::string eventType, int tag, rnsKeyAction keyAction = RNS_KEY_UnknownAction);
#endif //TARGET_OS_TV
};

} // namespace react
} // namespace facebook
