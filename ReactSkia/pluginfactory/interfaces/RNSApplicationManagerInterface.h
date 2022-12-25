/*
 * Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/io/async/ScopedEventBaseThread.h>
#include <folly/Format.h>

#include "ReactSkia/pluginfactory/RnsPlugin.h"

namespace facebook {
namespace react {
namespace rnsplugin {

using namespace folly;
using namespace std;
using folly::format;
using std::string;
using std::vector;

class RNSApplicationManagerInterface {
 public:
  class CallbackClient {
   public:
    virtual ~CallbackClient() {}
    // Events
    virtual void onChange(string nextAppState) = 0;
    virtual void onMemoryWarning() = 0;
    virtual void onFocus() = 0;
    virtual void onBlur() = 0;
  };

  RNSApplicationManagerInterface(CallbackClient &client);
  virtual ~RNSApplicationManagerInterface();

  string currentState() { return currentState_; }

 private:
  static uint64_t nextUniqueId();
  string appStateName_{};
  ScopedEventBaseThread appStateThread_;
  CallbackClient &client_;
  string currentState_{"inactive"};

 protected:
  void dispatchOnChange(string nextAppState);
  void dispatchOnMemoryWarning();
  void dispatchOnFocus();
  void dispatchOnBlur();
};

using AppCallbackClient = RNSApplicationManagerInterface::CallbackClient;
using RNSApplicationManagerInterface_t = RNSApplicationManagerInterface *(*)(AppCallbackClient &);

} // rnsplugin
} // react
} // facebook
