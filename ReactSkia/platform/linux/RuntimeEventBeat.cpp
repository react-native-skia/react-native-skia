/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "RuntimeEventBeat.h"
#include "ReactSkia/utils/RnsLog.h"

namespace facebook {
namespace react {

RuntimeEventBeat::RuntimeEventBeat(EventBeat::SharedOwnerBox const &ownerBox, RuntimeExecutor runtimeExecutor)
    : EventBeat(ownerBox), runtimeExecutor_(std::move(runtimeExecutor)) {
    RNS_LOG_NOT_IMPL;
}

RuntimeEventBeat::~RuntimeEventBeat() {
    RNS_LOG_NOT_IMPL;
}

void RuntimeEventBeat::induce() const {
    RNS_LOG_NOT_IMPL;
}

} // namespace react
} // namespace facebook
