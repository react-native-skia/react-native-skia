/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "ReactSkia/utils/RnsLog.h"
#include "MainRunLoopEventBeat.h"

#include "RCTUtils.h"
#include <mutex>

namespace facebook {
namespace react {

MainRunLoopEventBeat::MainRunLoopEventBeat(EventBeat::SharedOwnerBox const &ownerBox, RuntimeExecutor runtimeExecutor)
    : EventBeat(ownerBox), runtimeExecutor_(std::move(runtimeExecutor)) {
    RNS_LOG_NOT_IMPL;
}

MainRunLoopEventBeat::~MainRunLoopEventBeat() {
    RNS_LOG_NOT_IMPL;
}

void MainRunLoopEventBeat::induce() const {
    RNS_LOG_NOT_IMPL;
}

void MainRunLoopEventBeat::lockExecutorAndBeat() const {
    RNS_LOG_NOT_IMPL;
}

} // namespace react
} // namespace facebook
