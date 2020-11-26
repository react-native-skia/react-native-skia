/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#import "MainRunLoopEventBeat.h"

#import "RCTUtils.h"
#import <mutex>

namespace facebook {
namespace react {

MainRunLoopEventBeat::MainRunLoopEventBeat(EventBeat::SharedOwnerBox const &ownerBox, RuntimeExecutor runtimeExecutor)
    : EventBeat(ownerBox), runtimeExecutor_(std::move(runtimeExecutor))
{
  printf("\033[22;35m [WARN  ] %s %d: %s() !!!!!!!!!! NOT IMPLEMENTED !!!!!!!!!! \033[22;0m \n", __FILE__, __LINE__, __func__);
}

MainRunLoopEventBeat::~MainRunLoopEventBeat()
{
  printf("\033[22;35m [WARN  ] %s %d: %s() !!!!!!!!!! NOT IMPLEMENTED !!!!!!!!!! \033[22;0m \n", __FILE__, __LINE__, __func__);
}

void MainRunLoopEventBeat::induce() const
{
  printf("\033[22;35m [WARN  ] %s %d: %s() !!!!!!!!!! NOT IMPLEMENTED !!!!!!!!!! \033[22;0m \n", __FILE__, __LINE__, __func__);
}

void MainRunLoopEventBeat::lockExecutorAndBeat() const
{
  printf("\033[22;35m [WARN  ] %s %d: %s() !!!!!!!!!! NOT IMPLEMENTED !!!!!!!!!! \033[22;0m \n", __FILE__, __LINE__, __func__);
}

} // namespace react
} // namespace facebook
