/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#import "RCTUtils.h"

BOOL RCTIsMainQueue()
{
  static void *mainQueueKey = &mainQueueKey;
  static dispatch_once_t onceToken;
  dispatch_once(&onceToken, ^{
    dispatch_queue_set_specific(dispatch_get_main_queue(), mainQueueKey, mainQueueKey, NULL);
  });
  return dispatch_get_specific(mainQueueKey) == mainQueueKey;
}

void RCTExecuteOnMainQueue(dispatch_block_t block)
{
  if (RCTIsMainQueue()) {
    block();
  } else {
    dispatch_async(dispatch_get_main_queue(), ^{
      block();
    });
  }
}
