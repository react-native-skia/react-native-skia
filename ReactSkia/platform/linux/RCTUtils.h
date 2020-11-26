/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

//#import <CoreGraphics/CoreGraphics.h>
//#import <Foundation/Foundation.h>

//NS_ASSUME_NONNULL_BEGIN

// Check if we are currently on the main queue (not to be confused with
// the main thread, which is not necessarily the same thing)
// https://twitter.com/olebegemann/status/738656134731599872
bool RCTIsMainQueue(void);

// Execute the specified block on the main queue. Unlike dispatch_async()
// this will execute immediately if we're already on the main queue.
void RCTExecuteOnMainQueue(void);

//NS_ASSUME_NONNULL_END
