/*
 * Copyright (C) 1994-2023 OpenTV, Inc. and Nagravision S.A.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once
#include <map>
#include <mutex>
#include <semaphore.h>
#include "ReactSkia/sdk/ThreadSafeQueue.h"
#include "ReactSkia/core_modules/RSkSpatialNavigator.h"

using namespace rns::sdk;

namespace facebook {
namespace react {

using namespace SpatialNavigator;
struct RSkKeyInput {
  RSkKeyInput() = default;
  ~RSkKeyInput() = default;
  RSkKeyInput(rnsKey key, rnsKeyAction keyAction, bool keyRepeat)
      : key_(key),
        action_(keyAction),
        repeat_(keyRepeat) { }
  rnsKey key_;
  rnsKeyAction action_ {RNS_KEY_UnknownAction};
  bool repeat_ {false};
};
typedef std::function< void (RSkKeyInput)> inputEventClientCallback;

class RSkInputEventManager {
 private:
  std::mutex eventCallbackMutex_;
  int callbackId_ = 0;
  std::map <int, inputEventClientCallback > eventCallbackMap_;
  static RSkInputEventManager *sharedInputEventManager_;
  RSkInputEventManager();
#if ENABLE(FEATURE_KEY_THROTTLING)
  void inputWorkerThreadFunction();
  std::unique_ptr<ThreadSafeQueue<RSkKeyInput>> keyQueue_;
  sem_t keyEventPost_;
  std::thread inputWorkerThread_;
  std::atomic<int> activeInputClients_ {0};
#endif
  void processKey(RSkKeyInput &keyInput);
  RSkSpatialNavigator* spatialNavigator_ {nullptr};

 public:
  ~RSkInputEventManager();
  int addKeyEventCallback(inputEventClientCallback clientCallback);
  void removeKeyEventCallback(int callbackId);
  static RSkInputEventManager* getInputKeyEventManager();
  void keyHandler(rnsKey key, rnsKeyAction eventKeyAction);
#if ENABLE(FEATURE_KEY_THROTTLING)
  void onEventEmit();
  void onEventComplete();
#endif

#if defined(TARGET_OS_TV) && TARGET_OS_TV
  void sendNotificationWithEventType(std::string eventType, int tag, rnsKeyAction keyAction = RNS_KEY_UnknownAction, NotificationCompleteVoidCallback completeCallback = nullptr);
#endif //TARGET_OS_TV

  NotificationCompleteVoidCallback completeCallback_ {nullptr};
};

} // namespace react
} // namespace facebook
