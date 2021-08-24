/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 * Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */
#include <folly/io/async/ScopedEventBaseThread.h>

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
 private:
  void startObserving() const noexcept override;
  void stopObserving() const noexcept override;
  void beat();
  folly::ScopedEventBaseThread baetThread_;
  RunLoopObserver::Activity activities_;
};

} // namespace react
} // namespace facebook
