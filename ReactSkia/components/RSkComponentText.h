#pragma once

#include "ReactSkia/components/RSkComponent.h"

namespace facebook {
namespace react {

class RSkComponentText final : public RSkComponent {
 public:
  RSkComponentText(const ShadowView &shadowView);

 protected:
  void OnPaint(SkCanvas *canvas) override;
};

class RSkComponentRawText final : public RSkComponent {
 public:
  RSkComponentRawText(const ShadowView &shadowView);

 protected:
  void OnPaint(SkCanvas *canvas) override;
};

class RSkComponentParagraph final : public RSkComponent {
 public:
  RSkComponentParagraph(const ShadowView &shadowView);

 protected:
  void OnPaint(SkCanvas *canvas) override;
};

} // namespace react
} // namespace facebook
