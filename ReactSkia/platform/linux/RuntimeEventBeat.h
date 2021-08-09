/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <react/utils/RunLoopObserver.h>

namespace facebook {
namespace react {

/*
 * RuntimeEventBeat to flush Asynchronous Native Module Events into JS world .
 * using ReactCommon 'RunLoopObserver'
 */
class RuntimeEventBeat : public RunLoopObserver{
 public:
  RuntimeEventBeat(RunLoopObserver::WeakOwner const &owner);
  ~RuntimeEventBeat();
  virtual bool isOnRunLoopThread() const noexcept override;
  static bool keepBeating;
  void triggerBeat(RunLoopObserver::Activity activities); /*to transmit beat call to Beat Thread*/

 private:
  void startObserving() const noexcept override;
  void stopObserving() const noexcept override;
};

} // namespace react
} // namespace facebook
