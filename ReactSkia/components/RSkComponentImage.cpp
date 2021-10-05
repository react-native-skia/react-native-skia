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
    SkPaint paint;
/* TO DO: Handle filter quality based of configuration. Setting Low Filter Quality as default for now*/
    paint.setFilterQuality(DEFAULT_IMAGE_FILTER_QUALITY);
    if(imageProps.resizeMode == ImageResizeMode::Repeat) {
      sk_sp<SkImageFilter> imageFilter(SkImageFilters::Tile(targetRect,frameRect ,nullptr));
      paint.setImageFilter(std::move(imageFilter));
    }
/* Draw order 1. Background 2. Image 3. Border*/
    drawBackground(canvas,frame,imageBorderMetrics,imageProps.backgroundColor,imageProps.opacity);
    canvas->save();
/* clipping logic to be applied if computed Frame is greater than the target.*/
    if(( frameRect.width() < targetRect.width()) || ( frameRect.height() < targetRect.height())) {
      canvas->clipRect(frameRect,SkClipOp::kIntersect);
    }
    canvas->drawImageRect(imageData,targetRect,&paint);
    canvas->restore();
    drawBorder(canvas,frame,imageBorderMetrics,imageProps.backgroundColor,imageProps.opacity);
  } else {
/* Emitting Image Load failed Event*/
    imageEventEmitter->onError();
  }
}

void RSkComponentImage::updateComponentProps(const ShadowView &newShadowView,bool forceUpdate) {

    auto const &newimageProps = *std::static_pointer_cast<ImageProps const>(newShadowView.props);
    auto component = getComponentData();
    auto const &oldimageProps = *std::static_pointer_cast<ImageProps const>(component.props);

    if ((forceUpdate) || (oldimageProps.resizeMode != newimageProps.resizeMode)) {
         imageProps.resizeMode = newimageProps.resizeMode;
    }
    if ((forceUpdate) || (oldimageProps.tintColor != newimageProps.tintColor )) {
	 imageProps.tintColor = RSkColorFromSharedColor(newimageProps.tintColor,SK_ColorTRANSPARENT);
    }
}
} // namespace react
} // namespace facebook
