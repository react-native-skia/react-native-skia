#include "include/core/SkBitmap.h"
#include "include/core/SkData.h"
#include "include/core/SkImageGenerator.h"
#include "include/core/SkPaint.h"

#include "react/renderer/components/image/ImageShadowNode.h"

#include "ReactSkia/components/RSkComponentImage.h"
#include "ReactSkia/views/common/RSkDrawUtils.h"
#include "ReactSkia/views/common/RSkImageCacheManager.h"
#include "ReactSkia/utils/RnsLog.h"
#include "ReactSkia/utils/RnsUtils.h"
#include "ReactSkia/views/common/RSkConversion.h"

namespace facebook {
namespace react {

using namespace RSkDrawUtils;
using namespace RSkImageCacheManager;

RSkComponentImage::RSkComponentImage(const ShadowView &shadowView)
    : RSkComponent(shadowView) {}

void RSkComponentImage::OnPaint(
    SkCanvas *canvas) {
  auto component = getComponentData();
  auto const &imageProps =
      *std::static_pointer_cast<ImageProps const>(component.props);
  if (imageProps.sources.empty()) {
    return;
  }
  const auto source = imageProps.sources[0];

  if (source.type == ImageSource::Type::Local && !source.uri.empty()) {
    assert(source.uri.substr(0, 14) == "file://assets/");
    std::string path = "./" + source.uri.substr(7);

    Rect frame = getAbsoluteFrame();
    SkRect rect = SkRect::MakeXYWH(frame.origin.x, frame.origin.y, frame.size.width, frame.size.height);
    auto const &imageBorderMetrics=imageProps.resolveBorderMetrics(component.layoutMetrics);

 /* Draw order 1. Background 2. Image 3. Border*/
    drawBackground(canvas,frame,imageBorderMetrics,imageProps.backgroundColor,imageProps.opacity);
    RNS_PROFILE_START(drawImage)
    sk_sp<SkImage> imageData=getImageData(path.c_str());
    if(imageData) {
      canvas->drawImageRect(imageData, rect, nullptr);
    } else {
      RNS_LOG_ERROR("Draw Image Failed for:" << path);
    }
    RNS_PROFILE_END(path.c_str(),drawImage)
#ifdef RNS_IMAGE_CACHE_USAGE_DEBUG
      printCacheUsage();
#endif //RNS_IMAGECACHING_DEBUG
    drawBorder(canvas,frame,imageBorderMetrics,imageProps.backgroundColor,imageProps.opacity);
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
