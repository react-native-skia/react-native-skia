/*
* Copyright (C) 1994-2023 OpenTV, Inc. and Nagravision S.A.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#pragma once
#include "cxxreact/Instance.h"

namespace facebook {
namespace react {

using EmitterCompleteVoidCallback = std::function<void()>;

class RSkBaseEventEmitter{
    public:
      RSkBaseEventEmitter(Instance *bridgeInstance);
      virtual void sendEventWithName(std::string eventName, folly::dynamic &&params, EmitterCompleteVoidCallback completeCallback=nullptr);
      virtual void startObserving() = 0;
      virtual void stopObserving() = 0;
      void addListener(std::string);
      void removeListeners(int removeCount);
      virtual void SetBridgeInstance(Instance *bridgeInstance);
      Instance *bridgeInstance_;
      int listenerCount_;
};
} // namespace react
} // namespace facebook
