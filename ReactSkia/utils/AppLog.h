/*
 * Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "jsireact/JSIExecutor.h"
#include "ReactSkia/utils/RnsLog.h"

namespace facebook {
namespace react {

// Logging message displays filename as AppLog.h followed by message
Logger rnsLoggingBinder = [](const std::string &message,unsigned int logLevel) {
   switch(logLevel) {
      case 3:RNS_LOG_ERROR("ReactNativeJS: " << message.c_str());break;
      case 2:RNS_LOG_WARN("ReactNativeJS: " << message.c_str());break;
      case 1:RNS_LOG_INFO("ReactNativeJS: " << message.c_str());break;
      case 0:
      default:
             RNS_LOG_TRACE("ReactNativeJS: " << message.c_str());
   }
};

}
}
