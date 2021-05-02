#pragma once

#include "ReactSkia/components/RSkComponent.h"

namespace facebook {
namespace react {

class RSkComponentView final : public RSkComponent {
 public:
  RSkComponentView(const ShadowView &shadowView);

 protected:
  sk_sp<SkPicture> CreatePicture(int surfaceWidth, int surfaceHeight) override;
};

} // namespace react
} // namespace facebook
