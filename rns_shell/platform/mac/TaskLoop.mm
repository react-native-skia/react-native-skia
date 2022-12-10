/*
 * Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
 * Copyright (C) Kudo Chien.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "rns_shell/platform/mac/TaskLoop.h"

#include <dispatch/dispatch.h>

namespace RnsShell {

static std::unique_ptr<TaskLoop> mainTaskRunner_;

TaskLoop::TaskLoop() : TaskLoop(false) {}

TaskLoop::TaskLoop(bool isMain)
{
  if (!isMain) {
    eventBase_ = std::make_unique<folly::EventBase>();
  }
}

bool TaskLoop::running()
{
  return eventBase_->isRunning();
}

void TaskLoop::run()
{
  eventBase_->loopForever();
}

void TaskLoop::waitUntilRunning()
{
  eventBase_->waitUntilRunning();
}

void TaskLoop::stop()
{
  eventBase_->terminateLoopSoon();
}

void TaskLoop::dispatch(Func func)
{
  if (eventBase_->isRunning()) {
    eventBase_->runInEventBaseThread(std::move(func));
  }
}

MainTaskLoop::MainTaskLoop() : TaskLoop(true) {}

bool MainTaskLoop::running()
{
  return true;
}

void MainTaskLoop::run()
{
  // no-op
}

void MainTaskLoop::waitUntilRunning()
{
  assert(false);
}

void MainTaskLoop::stop()
{
  assert(false);
}

void MainTaskLoop::dispatch(Func func)
{
  dispatch_async(dispatch_get_main_queue(), ^{
    func();
  });
}

// static
TaskLoop &TaskLoop::main()
{
  return *mainTaskRunner_;
}

// static
void TaskLoop::initializeMain()
{
  if (mainTaskRunner_.get() == nullptr) {
    mainTaskRunner_ = std::make_unique<MainTaskLoop>();
  }
}

} // namespace RnsShell
