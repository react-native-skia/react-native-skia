#pragma once

#include "ReactSkia/components/RSkComponent.h"

#define DEFAULT_IMAGE_FILTER_QUALITY kLow_SkFilterQuality /*Skia's Defualt is kNone_SkFilterQuality*/

namespace facebook {
namespace react {

class RSkComponentImage final : public RSkComponent {
 public:
  RSkComponentImage(const ShadowView &shadowView);

 protected:
  void OnPaint(SkCanvas *canvas) override;
};

} // namespace react
} // namespace facebook
