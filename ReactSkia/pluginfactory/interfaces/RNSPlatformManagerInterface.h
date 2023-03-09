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

class PlatformDevice;

class RNSPlatformManagerInterface {
 public:
  class CallbackClient {
   public:
    virtual ~CallbackClient() {}
    // Events
    virtual void onStubEvent() = 0;
  };

  class PlatformDevice {
   public:
    PlatformDevice() = default;
   virtual ~PlatformDevice() {};

    string systemName {"Linux"};
    string systemVersion {"Ubuntu-20.0"};
    string interfaceIdiom {"tv"};
    bool forceTouchAvailable{true};
    virtual bool isTesting() = 0;
   private:
  };

  RNSPlatformManagerInterface(CallbackClient &client);
  virtual ~RNSPlatformManagerInterface();

  virtual shared_ptr<RNSPlatformManagerInterface::PlatformDevice> currentDevice() = 0;

 private:

  static uint64_t nextUniqueId();
  string platformManagerName_{};
  ScopedEventBaseThread platformThread_;
  CallbackClient &client_;

 protected:
  void dispatchOnStubEvent();
};

using PlatformCallbackClient = RNSPlatformManagerInterface::CallbackClient;
using RNSPlatformManagerInterface_t = RNSPlatformManagerInterface *(*)(PlatformCallbackClient &);

} // rnsplugin
} // react
} // facebook
