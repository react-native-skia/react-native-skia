/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * Copyright (C) Kudo Chien.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#import "ReactSkia/platform/mac/RCTSurfaceTouchHandler.h"

#include "ReactSkia/RSkSurfaceWindow.h"
#include "ReactSkia/platform/mac/RCTUtils.h"
#include "ReactSkia/utils/RnsLog.h"
#include "react-native/ReactCommon/react/renderer/components/root/RootShadowNode.h"
#include "react-native/ReactCommon/react/renderer/components/view/Touch.h"
#include "react-native/ReactCommon/react/renderer/components/view/TouchEventEmitter.h"
#include "react-native/ReactCommon/react/renderer/core/LayoutableShadowNode.h"
#include "react-native/ReactCommon/react/renderer/uimanager/UIManager.h"

using namespace facebook::react;

inline facebook::react::Point RCTPointFromCGPoint(const CGPoint &point)
{
  return {point.x, point.y};
}
BOOL RCTForceTouchAvailable(void)
{
  return NO;
}

template <size_t size>
class IdentifierPool {
 public:
  void enqueue(int index)
  {
    usage[index] = false;
  }

  int dequeue()
  {
    while (true) {
      if (!usage[lastIndex]) {
        usage[lastIndex] = true;
        return lastIndex;
      }
      lastIndex = (lastIndex + 1) % size;
    }
  }

  void reset()
  {
    for (int i = 0; i < size; i++) {
      usage[i] = false;
    }
  }

 private:
  bool usage[size];
  int lastIndex;
};

typedef NS_ENUM(NSInteger, RCTTouchEventType) {
  RCTTouchEventTypeTouchStart,
  RCTTouchEventTypeTouchMove,
  RCTTouchEventTypeTouchEnd,
  RCTTouchEventTypeTouchCancel,
};

struct ActiveTouch {
  Touch touch;
  SharedTouchEventEmitter eventEmitter;

  /*
   * A component view on which the touch was begun.
   */
  // __strong NSView *componentView = nil;

  struct Hasher {
    size_t operator()(const ActiveTouch &activeTouch) const
    {
      return std::hash<decltype(activeTouch.touch.identifier)>()(activeTouch.touch.identifier);
    }
  };

  struct Comparator {
    bool operator()(const ActiveTouch &lhs, const ActiveTouch &rhs) const
    {
      return lhs.touch.identifier == rhs.touch.identifier;
    }
  };
};

static void UpdateActiveTouch(
    ActiveTouch &activeTouch,
    NSEvent *touchEvent,
    facebook::react::RSkSurfaceWindow *surface,
    facebook::react::UIManager *uiManager,
    NSView *rootComponentView,
    SharedShadowNode targetShadowNode)
{
  CGPoint location = touchEvent.locationInWindow;
  if (!CGRectContainsPoint(rootComponentView.window.contentLayoutRect, location)) {
    // When mouse dragged out of window bounds, do not update the active touch
    return;
  }
  CGPoint rootViewLocation =
      CGPointMake(location.x, CGRectGetHeight(rootComponentView.window.contentLayoutRect) - location.y);

  if (targetShadowNode == nullptr) {
    auto currentRootShadowNode = facebook::react::RootShadowNode::Shared{};
    uiManager->getShadowTreeRegistry().visit(surface->surfaceId, [&](const ShadowTree &shadowTree) {
      currentRootShadowNode = shadowTree.getCurrentRevision().rootShadowNode;
    });
    targetShadowNode = LayoutableShadowNode::findNodeAtPoint(
        currentRootShadowNode, facebook::react::Point{rootViewLocation.x, rootViewLocation.y});
  }
  assert(std::dynamic_pointer_cast<const LayoutableShadowNode>(targetShadowNode));
  auto targetLayoutableShadowNode = std::static_pointer_cast<const LayoutableShadowNode>(targetShadowNode);
  auto targetLayoutMetric = targetLayoutableShadowNode->getLayoutMetrics();
  CGPoint offsetPoint =
      CGPointMake(location.x - targetLayoutMetric.frame.origin.x, location.y - targetLayoutMetric.frame.origin.y);

  activeTouch.touch.offsetPoint = RCTPointFromCGPoint(offsetPoint);
  activeTouch.touch.screenPoint = RCTPointFromCGPoint(location);
  activeTouch.touch.pagePoint = RCTPointFromCGPoint(rootViewLocation);

  activeTouch.touch.timestamp = touchEvent.timestamp;

  // if (RCTForceTouchAvailable()) {
  //   activeTouch.touch.force = nsEvent.force / nsEvent.maximumPossibleForce;
  // }
}

static ActiveTouch CreateTouch(
    NSEvent *nsEvent,
    facebook::react::RSkSurfaceWindow *surface,
    facebook::react::UIManager *uiManager,
    NSView *rootComponentView)
{
  ActiveTouch activeTouch = {};

  CGPoint location = nsEvent.locationInWindow;
  CGPoint rootViewLocation =
      CGPointMake(location.x, CGRectGetHeight(rootComponentView.window.contentLayoutRect) - location.y);

  auto currentRootShadowNode = facebook::react::RootShadowNode::Shared{};
  uiManager->getShadowTreeRegistry().visit(surface->surfaceId, [&](const ShadowTree &shadowTree) {
    currentRootShadowNode = shadowTree.getCurrentRevision().rootShadowNode;
  });
  auto targetShadowNode = LayoutableShadowNode::findNodeAtPoint(
      currentRootShadowNode, facebook::react::Point{rootViewLocation.x, rootViewLocation.y});

  assert(std::dynamic_pointer_cast<const TouchEventEmitter>(targetShadowNode->getEventEmitter()));
  activeTouch.eventEmitter = std::static_pointer_cast<const TouchEventEmitter>(targetShadowNode->getEventEmitter());
  activeTouch.touch.target = targetShadowNode->getTag();
  // activeTouch.componentView = componentView;

  UpdateActiveTouch(activeTouch, nsEvent, surface, uiManager, rootComponentView, targetShadowNode);
  return activeTouch;
}

// static BOOL AllTouchesAreCancelledOrEnded(NSSet<NSEvent *> *touches)
// {
//   for (NSEvent *touch in touches) {
//     if (touch.phase == NSEventPhaseBegan || touch.phase == NSEventPhaseMoved || touch.phase ==
//     NSEventPhaseStationary) {
//       return NO;
//     }
//   }
//   return YES;
// }
//
// static BOOL AnyTouchesChanged(NSSet<NSEvent *> *touches)
// {
//   for (NSEvent *touch in touches) {
//     if (touch.phase == NSEventPhaseBegan || touch.phase == NSEventPhaseMoved) {
//       return YES;
//     }
//   }
//   return NO;
// }

@interface RCTSurfaceTouchHandler () <NSGestureRecognizerDelegate>
@end

@implementation RCTSurfaceTouchHandler {
  std::unordered_map<NSInteger, ActiveTouch> _activeTouches;

  facebook::react::RSkSurfaceWindow *_surface;
  facebook::react::UIManager *_uiManager;

  /*
   * We hold the view weakly to prevent a retain cycle.
   */
  __weak NSView *_rootComponentView;
  IdentifierPool<11> _identifierPool;
}

- (instancetype)init
{
  if (self = [super initWithTarget:nil action:nil]) {
    self.delaysPrimaryMouseButtonEvents = NO; // default is NO.
    self.delaysSecondaryMouseButtonEvents = NO; // default is NO.
    self.delaysOtherMouseButtonEvents = NO; // default is NO.

    self.delegate = self;
  }

  return self;
}

- (instancetype)initWithTarget:(id)target action:(SEL)action
{
  if (self = [super initWithTarget:target action:action]) {
    @throw [NSException exceptionWithName:@"RCTNotImplementedException" reason:@"Unsupported" userInfo:nil];
  }
  return self;
}

- (void)attachToView:(NSView *)view
         withSurface:(facebook::react::RSkSurfaceWindow *)surface
           uiManager:(facebook::react::UIManager *)uiManager
{
  RNS_LOG_ASSERT(self.view == nil, "RCTTouchHandler already has attached view.");

  [view addGestureRecognizer:self];
  _rootComponentView = view;
  _surface = surface;
  _uiManager = uiManager;
}

- (void)detachFromView:(NSView *)view
{
  RNS_LOG_ASSERT(view != nil, "Missing view parameter");
  RNS_LOG_ASSERT(self.view == view, "RCTTouchHandler attached to another view.");

  [view removeGestureRecognizer:self];
  _rootComponentView = nil;
}

- (void)_registerTouches:(NSSet<NSEvent *> *)touches
{
  for (NSEvent *touch in touches) {
    auto activeTouch = CreateTouch(touch, _surface, _uiManager, _rootComponentView);
    activeTouch.touch.identifier = _identifierPool.dequeue();
    _activeTouches.emplace(touch.eventNumber, activeTouch);
  }
}

- (void)_updateTouches:(NSSet<NSEvent *> *)touches
{
  for (NSEvent *touch in touches) {
    auto iterator = _activeTouches.find(touch.eventNumber);
    assert(iterator != _activeTouches.end() && "Inconsistency between local and NSKit touch registries");
    if (iterator == _activeTouches.end()) {
      continue;
    }

    UpdateActiveTouch(iterator->second, touch, _surface, _uiManager, _rootComponentView, nullptr);
  }
}

- (void)_unregisterTouches:(NSSet<NSEvent *> *)touches
{
  for (NSEvent *touch in touches) {
    auto iterator = _activeTouches.find(touch.eventNumber);
    assert(iterator != _activeTouches.end() && "Inconsistency between local and NSKit touch registries");
    if (iterator == _activeTouches.end()) {
      continue;
    }
    auto &activeTouch = iterator->second;
    _identifierPool.enqueue(activeTouch.touch.identifier);
    _activeTouches.erase(touch.eventNumber);
  }
}

- (std::vector<ActiveTouch>)_activeTouchesFromTouches:(NSSet<NSEvent *> *)touches
{
  std::vector<ActiveTouch> activeTouches;
  activeTouches.reserve(touches.count);

  for (NSEvent *touch in touches) {
    auto iterator = _activeTouches.find(touch.eventNumber);
    assert(iterator != _activeTouches.end() && "Inconsistency between local and NSKit touch registries");
    if (iterator == _activeTouches.end()) {
      continue;
    }
    activeTouches.push_back(iterator->second);
  }

  return activeTouches;
}

- (void)_dispatchActiveTouches:(std::vector<ActiveTouch>)activeTouches eventType:(RCTTouchEventType)eventType
{
  TouchEvent event = {};
  std::unordered_set<ActiveTouch, ActiveTouch::Hasher, ActiveTouch::Comparator> changedActiveTouches = {};
  std::unordered_set<SharedTouchEventEmitter> uniqueEventEmitters = {};
  BOOL isEndishEventType = eventType == RCTTouchEventTypeTouchEnd || eventType == RCTTouchEventTypeTouchCancel;

  for (const auto &activeTouch : activeTouches) {
    if (!activeTouch.eventEmitter) {
      continue;
    }

    changedActiveTouches.insert(activeTouch);
    event.changedTouches.insert(activeTouch.touch);
    uniqueEventEmitters.insert(activeTouch.eventEmitter);
  }

  for (const auto &pair : _activeTouches) {
    if (!pair.second.eventEmitter) {
      continue;
    }

    if (isEndishEventType && event.changedTouches.find(pair.second.touch) != event.changedTouches.end()) {
      continue;
    }

    event.touches.insert(pair.second.touch);
  }

  for (const auto &eventEmitter : uniqueEventEmitters) {
    event.targetTouches.clear();

    for (const auto &pair : _activeTouches) {
      if (pair.second.eventEmitter == eventEmitter) {
        event.targetTouches.insert(pair.second.touch);
      }
    }

    switch (eventType) {
      case RCTTouchEventTypeTouchStart:
        eventEmitter->onTouchStart(event);
        break;
      case RCTTouchEventTypeTouchMove:
        eventEmitter->onTouchMove(event);
        break;
      case RCTTouchEventTypeTouchEnd:
        eventEmitter->onTouchEnd(event);
        break;
      case RCTTouchEventTypeTouchCancel:
        eventEmitter->onTouchCancel(event);
        break;
    }
  }
}

#pragma mark - `NSResponder`-ish touch-delivery methods

- (void)mouseDown:(NSEvent *)event
{
  [super mouseDown:event];
  NSSet<NSEvent *> *touches = [NSSet setWithObject:event];
  [self _registerTouches:touches];
  [self _dispatchActiveTouches:[self _activeTouchesFromTouches:touches] eventType:RCTTouchEventTypeTouchStart];

  if (self.state == NSGestureRecognizerStatePossible) {
    self.state = NSGestureRecognizerStateBegan;
  } else if (self.state == NSGestureRecognizerStateBegan) {
    self.state = NSGestureRecognizerStateChanged;
  }
}

- (void)mouseDragged:(NSEvent *)event
{
  [super mouseDragged:event];
  NSSet<NSEvent *> *touches = [NSSet setWithObject:event];
  [self _updateTouches:touches];
  [self _dispatchActiveTouches:[self _activeTouchesFromTouches:touches] eventType:RCTTouchEventTypeTouchMove];

  self.state = NSGestureRecognizerStateChanged;
}

- (void)mouseUp:(NSEvent *)event
{
  [super mouseUp:event];
  NSSet<NSEvent *> *touches = [NSSet setWithObject:event];
  [self _updateTouches:touches];
  [self _dispatchActiveTouches:[self _activeTouchesFromTouches:touches] eventType:RCTTouchEventTypeTouchEnd];
  [self _unregisterTouches:touches];

  self.state = NSGestureRecognizerStateEnded;
}

- (void)reset
{
  [super reset];

  if (!_activeTouches.empty()) {
    std::vector<ActiveTouch> activeTouches;
    activeTouches.reserve(_activeTouches.size());

    for (auto const &pair : _activeTouches) {
      activeTouches.push_back(pair.second);
    }

    [self _dispatchActiveTouches:activeTouches eventType:RCTTouchEventTypeTouchCancel];

    // Force-unregistering all the touches.
    _activeTouches.clear();
    _identifierPool.reset();
  }
}

- (BOOL)canPreventGestureRecognizer:(__unused NSGestureRecognizer *)preventedGestureRecognizer
{
  return NO;
}

- (BOOL)canBePreventedByGestureRecognizer:(NSGestureRecognizer *)preventingGestureRecognizer
{
  // We fail in favour of other external gesture recognizers.
  // iOS will ask `delegate`'s opinion about this gesture recognizer little bit later.
  return ![preventingGestureRecognizer.view isDescendantOf:self.view];
}

#pragma mark - NSGestureRecognizerDelegate

- (BOOL)gestureRecognizer:(__unused NSGestureRecognizer *)gestureRecognizer
    shouldRequireFailureOfGestureRecognizer:(NSGestureRecognizer *)otherGestureRecognizer
{
  // Same condition for `failure of` as for `be prevented by`.
  return [self canBePreventedByGestureRecognizer:otherGestureRecognizer];
}

@end
