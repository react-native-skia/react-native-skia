#pragma once

#include "ReactSkia/components/RSkComponent.h"

namespace facebook {
namespace react {

class RSkComponentText final : public RSkComponent {
 public:
  RSkComponentText(const ShadowView &shadowView);

 protected:
  sk_sp<SkPicture> CreatePicture(int surfaceWidth, int surfaceHeight) override;
};

class RSkComponentRawText final : public RSkComponent {
 public:
  RSkComponentRawText(const ShadowView &shadowView);

 protected:
  sk_sp<SkPicture> CreatePicture(int surfaceWidth, int surfaceHeight) override;
};

class RSkComponentParagraph final : public RSkComponent {
 public:
  RSkComponentParagraph(const ShadowView &shadowView);

 protected:
  sk_sp<SkPicture> CreatePicture(int surfaceWidth, int surfaceHeight) override;
};

} // namespace react
} // namespace facebook
