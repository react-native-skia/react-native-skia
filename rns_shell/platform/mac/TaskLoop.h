/*
 * Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
 * Copyright (C) Kudo Chien.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <functional>

#include "folly/io/async/EventBase.h"

namespace RnsShell {

class TaskLoop {
 public:
  TaskLoop();
  virtual ~TaskLoop() = default;
  TaskLoop(const TaskLoop &) = delete;
  TaskLoop &operator=(const TaskLoop &) = delete;
  TaskLoop(TaskLoop &&other) = default;

  virtual void run();
  virtual bool running();
  virtual void stop();
  virtual void waitUntilRunning();

  using Func = std::function<void()>;
  virtual void dispatch(Func func);

 protected:
  TaskLoop(bool isMain);

 public:
  static void initializeMain();
  static TaskLoop &main();

 private:
  std::unique_ptr<folly::EventBase> eventBase_;
};

class MainTaskLoop : public TaskLoop {
 public:
  MainTaskLoop();

 public:
  void run() override;
  bool running() override;
  void stop() override;
  void waitUntilRunning() override;
  void dispatch(Func func) override;
};

} // namespace RnsShell
