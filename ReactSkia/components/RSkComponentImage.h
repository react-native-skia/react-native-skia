#pragma once

#include "include/core/SkRect.h"

#include "react/renderer/components/image/ImageShadowNode.h"

#include "ReactSkia/components/RSkComponent.h"

#define DEFAULT_IMAGE_FILTER_QUALITY kLow_SkFilterQuality /*Skia's Defualt is kNone_SkFilterQuality*/

namespace facebook {
namespace react {
struct ImgProps{
    ImageResizeMode resizeMode;
    SkColor tintColor;
};
class RSkComponentImage final : public RSkComponent {
 public:
  RSkComponentImage(const ShadowView &shadowView);
  void updateComponentProps(const ShadowView &newShadowView,bool forceUpdate) override;
 private :
  ImgProps imageProps;
 protected:
  void OnPaint(SkCanvas *canvas) override;
};

} // namespace react
} // namespace facebook
