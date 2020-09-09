#pragma once

#include "ReactSkia/components/RSkComponent.h"

namespace facebook {
namespace react {

class RSkComponentRootView final : public RSkComponent {
 public:
  RSkComponentRootView(const ShadowView &shadowView);

 protected:
  void OnPaint(const ShadowView &shadowView, SkCanvas *canvas) override;
};

} // namespace react
} // namespace facebook
