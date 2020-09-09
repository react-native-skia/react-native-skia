#pragma once

#include "ReactSkia/components/RSkComponent.h"

namespace facebook {
namespace react {

class RSkComponentImage final : public RSkComponent {
 public:
  RSkComponentImage(const ShadowView &shadowView);

 protected:
  void OnPaint(const ShadowView &shadowView, SkCanvas *canvas) override;
};

} // namespace react
} // namespace facebook
