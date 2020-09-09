#pragma once

#include "ReactSkia/components/RSkComponent.h"

namespace facebook {
namespace react {

class RSkComponentView final : public RSkComponent {
 public:
  RSkComponentView(const ShadowView &shadowView);

 protected:
  void OnPaint(const ShadowView &shadowView, SkCanvas *canvas) override;
};

} // namespace react
} // namespace facebook
