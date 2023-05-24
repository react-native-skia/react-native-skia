/*
* Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "rns_shell/compositor/LayerTreeHost.h"

#include <vector>

#include "rns_shell/common/Application.h"
#include "rns_shell/common/Window.h"
#include "rns_shell/compositor/layers/ScrollLayer.h"
#include "rns_shell/input/MouseWheelEvent.h"
#include "ReactSkia/utils/RnsLog.h"

namespace RnsShell {

namespace {

// Runs hit-testing from layer tree to layer list.
// React Native organizes layer tree with implicit z-index order,
// the last layer in the list will be the topmost layer.
void RunHitTestToLayerList(
    Layer *root,
    int eventX,
    int eventY,
    std::vector<Layer *> &result) {
  if (root != nullptr) {
    if (root->absoluteFrame().contains(eventX, eventY)) {
      result.push_back(root);
    }
    for (auto &child : root->children()) {
      RunHitTestToLayerList(child.get(), eventX, eventY, result);
    }
  }
}

} // namespace

LayerTreeHost::LayerTreeHost(Application& app)
    : app_(app),
      window_(Window::createNativeWindow(&PlatformDisplay::sharedDisplayForCompositing())),
      compositorClient_(*this),
      displayID_(std::numeric_limits<uint32_t>::max() - app_.identifier()) {
  window_->BindInputEventDelegate(this);
  SkSize viewPort(SkSize::MakeEmpty());
  compositor_ = Compositor::create(compositorClient_, displayID_, viewPort, PlatformDisplay::sharedDisplayForCompositing().scaleFactor());
}

LayerTreeHost::~LayerTreeHost() {
  //TODO cancel Pending schedules
  compositor_->invalidate();
  compositor_.reset();
  window_ = nullptr;
}

uint64_t LayerTreeHost::nativeSurfaceHandle() {
  return window_->nativeWindowHandle();
}

void LayerTreeHost::didRenderFrame() {
  if (window_)
    window_->didRenderFrame();
}

// InputEventDelegate implementations

void LayerTreeHost::DispatchInputEvent(MouseWheelEvent &&event) {
  // From the topmost layer receieving the event,
  // we should further find the scroll layer from ancestors.
  auto *target = HitTest(event.eventX, event.eventY);
  while (target != nullptr) {
    if (target->type() == LAYER_TYPE_SCROLL) {
      break;
    }
    target = target->parent();
  }

  if (target != nullptr) {
    ScrollLayer *scrollLayer = static_cast<ScrollLayer *>(target);
    SkPoint scrollPos = scrollLayer->getScrollPosition();
    scrollPos.offset(event.deltaX, event.deltaY);
    scrollLayer->scrollTo(scrollPos);
  }
}

// private implementations

Layer *LayerTreeHost::HitTest(int eventX, int eventY) {
  std::vector<Layer *> candidates;
  RunHitTestToLayerList(compositor()->rootLayer(), eventX, eventY, candidates);
  if (!candidates.empty()) {
    return candidates.back();
  } else {
    return nullptr;
  }
}

void LayerTreeHost::sizeDidChange(SkSize& size) {
  compositor_->setViewportSize(size);
}

void LayerTreeHost::begin() {
  compositor_->begin();
}

void LayerTreeHost::commitScene(bool immediate) {
  // TODO Check compositor state idle, progress, scheduled
  compositor_->commit(immediate);
}

void LayerTreeHost::setRootCompositingLayer(SharedLayer rootLayer) {
  compositor_->setRootLayer(rootLayer);
}

}   // namespace RnsShell
