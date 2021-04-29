#pragma once

#include "ReactSkia/components/RSkComponent.h"
#include "ReactSkia/views/view/RSkLayoutHelper.h"

namespace facebook {
namespace react {

class RSkComponentImage final : public RSkComponent , public RSkLayoutHelper {
 public:
  RSkComponentImage(const ShadowView &shadowView);

 protected:
  void OnPaint(SkCanvas *canvas) override;
};

} // namespace react
} // namespace facebook
