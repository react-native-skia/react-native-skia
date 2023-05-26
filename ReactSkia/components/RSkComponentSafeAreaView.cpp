/*
* Copyright (C) 1994-2023 OpenTV, Inc. and Nagravision S.A.
*
* This source code is licensed under the MIT license found in the
* LICENSE file in the root directory of this source tree.
*/

#include <glog/logging.h>

#include "include/core/SkPaint.h"

#include "ReactSkia/components/RSkComponentSafeAreaView.h"
#include "ReactSkia/views/common/RSkConversion.h"

namespace facebook {
namespace react {

RSkComponentSafeAreaView::RSkComponentSafeAreaView(const ShadowView &shadowView)
    : RSkComponent(shadowView)
      , pluginFactory_(new RnsPluginFactory)
      , platformCallBackClient_(*this){
  if(pluginFactory_) {
    platformManagerHandle_ = pluginFactory_->createPlatformManagerHandle(platformCallBackClient_);
    if(platformManagerHandle_ == nullptr) {
      RNS_LOG_ERROR("Could not get Platform handle from RNS platform Plugin");
    }
  }
}

RnsShell::LayerInvalidateMask RSkComponentSafeAreaView::updateComponentProps(SharedProps newviewProps,bool forceUpdate) {
  RNS_LOG_NOT_IMPL;
  return RnsShell::LayerInvalidateNone;
}

void RSkComponentSafeAreaView::safeAreaInsetsDidChange() {
  updateStateIfNecessary();
}

SkRect RSkComponentSafeAreaView::safeAreaInsets() {
  if(platformManagerHandle_) {
    shared_ptr<RNSPlatformManagerInterface::PlatformDevice>  device = platformManagerHandle_->currentDevice();
    if(device) {
      return device->getSafeAreaInsets();
    }
  }
  return SkRect::MakeEmpty();
}

void RSkComponentSafeAreaView::updateStateIfNecessary() {
  SkRect insets = safeAreaInsets();
  auto newPadding = RCTEdgeInsetsFromSkRect(insets);
  auto threshold = 1.0 / 1.0 /* TODO RCTScreenScale()*/ + 0.01; // Size of a pixel plus some small threshold.
  auto deltaPadding = newPadding - lastPaddingStateWasUpdatedWith_;

  if (std::abs(deltaPadding.left) < threshold && std::abs(deltaPadding.top) < threshold &&
      std::abs(deltaPadding.right) < threshold && std::abs(deltaPadding.bottom) < threshold) {
    return;
  }

  lastPaddingStateWasUpdatedWith_ = newPadding;
  stateTeller_.updateState(SafeAreaViewState{newPadding});
}

RnsShell::LayerInvalidateMask RSkComponentSafeAreaView::updateComponentState(const State::Shared &state, bool forceUpadate) {
  stateTeller_.setConcreteState(state);
  updateStateIfNecessary();

  return RnsShell::LayerInvalidateAll;
}

void RSkComponentSafeAreaView::OnPaint(SkCanvas *canvas) {
  auto component = getComponentData();
  auto const &viewProps = *std::static_pointer_cast<SafeAreaViewProps const>(component.props);
  /* apply view style props */
  auto borderMetrics=viewProps.resolveBorderMetrics(component.layoutMetrics);
  Rect frame = component.layoutMetrics.frame;

  /*Draw Order : 1. Shadow 2. BackGround 3 Border*/
  auto layerRef=layer();
  if(layerRef->isShadowVisible) {
    drawShadow(canvas,frame,borderMetrics,
                    viewProps.backgroundColor,
                    layerRef->shadowColor,layerRef->shadowOffset,layerRef->shadowOpacity,
                    layerRef->opacity,
                    layerRef->shadowImageFilter,layerRef->shadowMaskFilter
                   );
  }
  drawBackground(canvas,frame,borderMetrics,viewProps.backgroundColor);
  drawBorder(canvas,frame,borderMetrics,viewProps.backgroundColor);
}

// Callback client

RSkComponentSafeAreaView::PlatformCallBackClient::PlatformCallBackClient(RSkComponentSafeAreaView& safeAreaViewComp)
    : safeAreaViewComp_(safeAreaViewComp) {}

void RSkComponentSafeAreaView::PlatformCallBackClient::onSafeAreaInsetsDidChange() {
  safeAreaViewComp_.safeAreaInsetsDidChange();
}


} // namespace react
} // namespace facebook
