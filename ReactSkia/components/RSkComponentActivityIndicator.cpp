/*
 * Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"

#include <ReactSkia/activityindicator/react/renderer/components/activityindicator/ActivityIndicatorProps.h>
#include "ReactSkia/components/RSkComponentActivityIndicator.h"
#include "ReactSkia/components/RSkComponentActivityIndicatorManager.h"

#include "ReactSkia/views/common/RSkConversion.h"
#include "ReactSkia/utils/RnsLog.h"

#define ACTIVITY_INDICATOR_DEFAULT_ARC_COLOR            SkColorSetARGB(0xFF, 0x99, 0x99, 0x99) // As per IOS documentation
#define ACTIVITY_INDICATOR_STROKE_WIDTH(x)              ((x * 12.5)/100) // WEB Reference
#define ACTIVITY_INDICATOR_BACKGROUND_CRICLE_ALPHA(y)   ((y * 20)/100) // WEB Reference

namespace facebook {
namespace react {

RSkComponentActivityIndicator::RSkComponentActivityIndicator(const ShadowView &shadowView)
: RSkComponent(shadowView) {
  actIndManager_ = RSkComponentActivityIndicatorManager::getActivityIndicatorManager();
}

RnsShell::LayerInvalidateMask  RSkComponentActivityIndicator::updateComponentProps(const ShadowView &newShadowView,bool forceUpdate) {
  auto component = getComponentData();
  auto const &activityIndicatorOldProps = *std::static_pointer_cast<ActivityIndicatorProps const>(component.props);
  auto const &activityIndicatorNewProps = *std::static_pointer_cast<ActivityIndicatorProps const>(newShadowView.props);

  if((initialPropertiesParsed_ == false) || (activityIndicatorOldProps.animating != activityIndicatorNewProps.animating)){
    initialPropertiesParsed_ = true;
    
    if(activityIndicatorNewProps.animating == true){
      actIndManager_->addComponent(this->weak_from_this());
    }else {
      actIndManager_->removeComponent(component.tag);
    }
  }
  return RnsShell::LayerInvalidateNone;
}

void RSkComponentActivityIndicator::OnPaint(SkCanvas *canvas) {
  auto component = getComponentData();
  auto const &activityIndicatorProps = *std::static_pointer_cast<ActivityIndicatorProps const>(component.props);
  auto borderMetrics=activityIndicatorProps.resolveBorderMetrics(component.layoutMetrics);
  Rect frame = component.layoutMetrics.frame;
  SkColor color = RSkColorFromSharedColor(activityIndicatorProps.color, ACTIVITY_INDICATOR_DEFAULT_ARC_COLOR);
  SkColor alphaVal = SkColorGetA(color);
  SkPaint paint;
  SkScalar strokeWidth;

  if( activityIndicatorProps.animating == false && activityIndicatorProps.hidesWhenStopped == true ){
    return;
  }

  paint.setAntiAlias(true);
  paint.setStyle(SkPaint::kStroke_Style);

  strokeWidth = ACTIVITY_INDICATOR_STROKE_WIDTH(frame.size.width);
  if(!strokeWidth){
    return;
  }
  paint.setStrokeWidth(strokeWidth);

  if (color && alphaVal){
    SkColor backgroundCircleAlphaValue;
    SkPath backgroundCirclePath;
    SkPath foregroundArcPath;
    SkRect rect = SkRect::MakeXYWH(frame.origin.x, frame.origin.y, frame.size.width, frame.size.height);
    
    rect.inset(strokeWidth/2, strokeWidth/2);
    backgroundCircleAlphaValue = ACTIVITY_INDICATOR_BACKGROUND_CRICLE_ALPHA(alphaVal);
    
    if(backgroundCircleAlphaValue){
      backgroundCirclePath.addArc(rect, 0, 360);
      paint.setColor(color);
      paint.setAlpha(backgroundCircleAlphaValue);
      canvas->drawPath(backgroundCirclePath, paint);
    }

    foregroundArcPath.addArc(rect, 0, 80);
    paint.setColor(color);
    canvas->drawPath(foregroundArcPath, paint);
  }
}

RSkComponentActivityIndicator::~RSkComponentActivityIndicator(){
  auto component = getComponentData();
  actIndManager_->removeComponent(component.tag);
  actIndManager_ = nullptr;
  initialPropertiesParsed_ = false;
}

} // namespace react
} // namespace facebook
