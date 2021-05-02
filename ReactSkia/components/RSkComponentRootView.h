#pragma once

#include "ReactSkia/components/RSkComponent.h"

namespace facebook {
namespace react {

class RSkComponentRootView final : public RSkComponent {
 public:
  RSkComponentRootView(const ShadowView &shadowView);

 protected:
  sk_sp<SkPicture> CreatePicture(int surfaceWidth, int surfaceHeight) override;
};

} // namespace react
} // namespace facebook
