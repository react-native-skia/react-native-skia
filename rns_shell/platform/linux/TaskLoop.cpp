/*
* Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "rns_shell/platform/linux/TaskLoop.h"

using namespace folly;

namespace RnsShell {

static std::unique_ptr<TaskLoop> mainTaskRunner_;

TaskLoop::TaskLoop() {
}

TaskLoop::~TaskLoop() {
}

bool TaskLoop::running() {
    return eventBase_.isRunning();
}

void TaskLoop::run() {
    eventBase_.loopForever();
}

void TaskLoop::waitUntilRunning() {
    eventBase_.waitUntilRunning();
}

void TaskLoop::stop(){
    eventBase_.terminateLoopSoon();
}

TaskLoop& TaskLoop::main() {
    return *mainTaskRunner_;
}

void TaskLoop::dispatch(folly::Func fun) {
    if(eventBase_.isRunning())
        eventBase_.runInEventBaseThread(std::move(fun));
}

void TaskLoop::scheduleDispatch(Func fun, long long timeoutMs) {
    eventBase_.scheduleAt(std::move(fun),
                          std::chrono::steady_clock::now() + std::chrono::milliseconds(timeoutMs));
}

void TaskLoop::initializeMain() {
    if(mainTaskRunner_.get() == nullptr)
        mainTaskRunner_ = std::make_unique<TaskLoop>();
}

}   // namespace RnsShell
