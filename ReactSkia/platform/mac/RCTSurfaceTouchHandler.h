/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * Copyright (C) Kudo Chien.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#import <AppKit/AppKit.h>

namespace facebook {
namespace react {
class RSkSurfaceWindow;
class UIManager;
} // namespace react
} // namespace facebook

NS_ASSUME_NONNULL_BEGIN

@interface RCTSurfaceTouchHandler : NSGestureRecognizer

/*
 * Attaches (and detaches) a view to the touch handler.
 * The receiver does not retain the provided view.
 */
- (void)attachToView:(NSView *)view
         withSurface:(facebook::react::RSkSurfaceWindow *)surface
           uiManager:(facebook::react::UIManager *)uiManager;
- (void)detachFromView:(NSView *)view;

@end

NS_ASSUME_NONNULL_END
