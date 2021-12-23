#include "include/core/SkPaint.h"
#include "include/core/SkClipOp.h"
#include "include/core/SkImageFilter.h"
#include "include/effects/SkImageFilters.h"

#include "react/renderer/components/image/ImageEventEmitter.h"

#include "ReactSkia/components/RSkComponentImage.h"
#include "ReactSkia/views/common/RSkDrawUtils.h"
#include "ReactSkia/views/common/RSkImageUtils.h"
#include "ReactSkia/views/common/RSkImageCacheManager.h"

#include "ReactSkia/utils/RnsLog.h"
#include "ReactSkia/utils/RnsUtils.h"
#include "ReactSkia/views/common/RSkConversion.h"

namespace facebook {
namespace react {

using namespace RSkDrawUtils;
using namespace RSkImageUtils;
using namespace RSkImageCacheManager;

namespace {

sk_sp<SkImage> getLocalImage(ImageSource source) {
  if ( !source.uri.empty() && !(source.uri.substr(0, 14) == "file://assets/")) {
    return nullptr;
  }
  std::string path = "./" + source.uri.substr(7);
  RNS_PROFILE_START(getImageData)
  sk_sp<SkImage> imageData=getImageData(path.c_str());
  RNS_PROFILE_END(path.c_str(),getImageData)
  if(!imageData) {
    RNS_LOG_ERROR("Draw Image Failed :" << path);
  }
  #ifdef RNS_IMAGE_CACHE_USAGE_DEBUG
      printCacheUsage();
  #endif //RNS_IMAGECACHING_DEBUG
  return imageData;
}

}//namespace

RSkComponentImage::RSkComponentImage(const ShadowView &shadowView)
    : RSkComponent(shadowView) {}

void RSkComponentImage::OnPaint(
    SkCanvas *canvas) {
  auto component = getComponentData();
  auto const &imageProps =
      *std::static_pointer_cast<ImageProps const>(component.props);
  sk_sp<SkImage> imageData = nullptr;
  if (!imageProps.sources.empty() && imageProps.sources[0].type == ImageSource::Type::Local ) {
    imageData=getLocalImage(imageProps.sources[0]);
  }
  auto imageEventEmitter = std::static_pointer_cast<ImageEventEmitter const>(component.eventEmitter);
  if(imageData) {
/* Emitting Load completed Event*/
    imageEventEmitter->onLoad();
    Rect frame = component.layoutMetrics.frame;
    SkRect frameRect = SkRect::MakeXYWH(frame.origin.x, frame.origin.y, frame.size.width, frame.size.height);
    auto const &imageBorderMetrics=imageProps.resolveBorderMetrics(component.layoutMetrics);
    SkRect targetRect = computeTargetRect({imageData->width(),imageData->height()},frameRect,imageProps.resizeMode);

/* Draw order 1.Shadow 2. Background 3.Image Shadow 4. Image 5.Border*/
    bool contentShadow = false;
    bool needClipAndRestore =false;
    if(layer()->shadowOpacity && layer()->shadowFilter){
      contentShadow=drawShadow(canvas,frame,imageBorderMetrics,imageProps.backgroundColor,layer()->shadowOpacity,layer()->shadowFilter);
    }
    drawBackground(canvas,frame,imageBorderMetrics,imageProps.backgroundColor);
    SkPaint paint,shadowPaint;
   /*Draw Image Shadow*/
    if(contentShadow) {
        if(imageProps.resizeMode == ImageResizeMode::Repeat) {
        /*TODO Not applied any shadow for solid Image when resizeMode is "Repeat"*/
            sk_sp<SkImageFilter> imageFilter(SkImageFilters::Tile(targetRect,frameRect,layer()->shadowFilter));
            shadowPaint.setImageFilter(std::move(imageFilter));
        } else {
              shadowPaint.setImageFilter(layer()->shadowFilter);
        }
        if(!(isOpaque(layer()->shadowOpacity)))
            canvas->saveLayerAlpha(&frameRect,layer()->shadowOpacity);
        canvas->drawImageRect(imageData, targetRect, &shadowPaint);
        if(!(isOpaque(layer()->shadowOpacity)))
            canvas->restore();
    }
    /*Draw Image */
    if(( frameRect.width() < targetRect.width()) || ( frameRect.height() < targetRect.height()))
      needClipAndRestore= true;
    /* clipping logic to be applied if computed Frame is greater than the target.*/
    if(needClipAndRestore) {
        canvas->save();
        canvas->clipRect(frameRect,SkClipOp::kIntersect);
    }
    /* TODO: Handle filter quality based of configuration. Setting Low Filter Quality as default for now*/
    paint.setFilterQuality(DEFAULT_IMAGE_FILTER_QUALITY);
    if(imageProps.resizeMode == ImageResizeMode::Repeat) {
       sk_sp<SkImageFilter> imageFilter(SkImageFilters::Tile(targetRect,frameRect,nullptr));
       paint.setImageFilter(std::move(imageFilter));
    }
    canvas->drawImageRect(imageData,targetRect,&paint);
    if(needClipAndRestore)
        canvas->restore();
    drawBorder(canvas,frame,imageBorderMetrics,imageProps.backgroundColor);
  } else {
/* Emitting Image Load failed Event*/
        imageEventEmitter->onError();
    }
}

RnsShell::LayerInvalidateMask RSkComponentImage::updateComponentProps(const ShadowView &newShadowView,bool forceUpdate) {

    auto const &newimageProps = *std::static_pointer_cast<ImageProps const>(newShadowView.props);
    auto component = getComponentData();
    auto const &oldimageProps = *std::static_pointer_cast<ImageProps const>(component.props);
    RnsShell::LayerInvalidateMask updateMask=RnsShell::LayerInvalidateNone;

    if((forceUpdate) || (oldimageProps.resizeMode != newimageProps.resizeMode)) {
      imageProps.resizeMode = newimageProps.resizeMode;
      updateMask =static_cast<RnsShell::LayerInvalidateMask>(updateMask | RnsShell::LayerInvalidateAll);
    }
    if((forceUpdate) || (oldimageProps.tintColor != newimageProps.tintColor )) {
      RNS_LOG_NOT_IMPL;
      imageProps.tintColor = RSkColorFromSharedColor(newimageProps.tintColor,SK_ColorTRANSPARENT);
    }
    return updateMask;
}
} // namespace react
} // namespace facebook
