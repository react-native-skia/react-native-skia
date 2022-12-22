/*
* Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
*
* This source code is licensed under the MIT license found in the
* LICENSE file in the root directory of this source tree.
*/

#include <include/effects/Sk2DPathEffect.h>

#include "ReactSkia/components/RSkComponentUnimplementedView.h"

#include "include/core/SkPaint.h"
#include "react/renderer/components/unimplementedview/UnimplementedViewShadowNode.h"

namespace facebook {
namespace react {


RSkComponentUnimplementedView::RSkComponentUnimplementedView(const ShadowView &shadowView)
    : RSkComponent(shadowView) {}

RnsShell::LayerInvalidateMask RSkComponentUnimplementedView::updateComponentProps(SharedProps newviewProps,bool forceUpdate) {
  RNS_LOG_NOT_IMPL;
  return RnsShell::LayerInvalidateNone;
}

void RSkComponentUnimplementedView::handleCommand(std::string commandName,folly::dynamic args){
  RNS_LOG_WARN("[RSkComponentUnimplementedView][handleCommand] commandName : "<< commandName);
}

void RSkComponentUnimplementedView::OnPaint(
    SkCanvas *canvas) {
  auto component = getComponentData();
  auto const &viewProps = *std::static_pointer_cast<ViewProps const>(component.props);

  // Apply view style props
  auto borderMetrics=viewProps.resolveBorderMetrics(component.layoutMetrics);
  Rect frame = component.layoutMetrics.frame;

  RNS_LOG_WARN("Painting an unimplemented view : " << component.componentName);

  // Draw Order : 1. Shadow 2. BackGround 3 Border
  auto layerRef=layer();
  if(layerRef->isShadowVisible) {
    drawShadow(canvas,frame,borderMetrics,
                viewProps.backgroundColor,
                layerRef->shadowColor,layerRef->shadowOffset,layerRef->shadowOpacity,
                layerRef->opacity,
                layerRef->shadowImageFilter,layerRef->shadowMaskFilter
              );
  }
  drawBackground(canvas,frame,borderMetrics,SK_ColorWHITE);
  drawBorder(canvas,frame,borderMetrics,viewProps.backgroundColor);

  // Draw lattice to show that this is an unimplemented view.
  SkPaint paint;
  SkMatrix lattice;
  lattice.setScale(5.0f, 5.0f);
  lattice.preRotate(30.0f);
  paint.setPathEffect(SkLine2DPathEffect::Make(0.0f, lattice));
  paint.setAntiAlias(true);
  SkRect bounds = SkRect::MakeXYWH(frame.origin.x, frame.origin.y, frame.size.width, frame.size.height);
  bounds.outset(-2.0f, -2.0f);
  canvas->drawRect(bounds, paint);
}

bool RSkComponentUnimplementedView::isVisible(RSkComponent* candidate) {
  if(candidate == nullptr)
    return false;

  const SkIRect& rootRect = getLayerAbsoluteFrame();
  const SkIRect& candidateRect = candidate->getLayerAbsoluteFrame();

  RNS_LOG_TODO("Should use screenFrame for calculations");
  RNS_LOG_DEBUG("Visible Check : RootContainer [" << rootRect.x() << "," << rootRect.y() << "," << rootRect.width() << "," << rootRect.height() <<
                "] Candidtae [" << candidateRect.x() << "," << candidateRect.y() << "," << candidateRect.width() << "," << candidateRect.height() << "]");

  return(rootRect.contains(candidateRect));
}

} // namespace react
} // namespace facebook
