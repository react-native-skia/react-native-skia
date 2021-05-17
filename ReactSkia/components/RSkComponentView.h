#pragma once

#include "ReactSkia/components/RSkComponent.h"
#include "ReactSkia/views/common/RSkDrawUtils.h"

namespace facebook {
namespace react {

class RSkComponentView final : public RSkComponent, public RSkDrawUtils{
 public:
  RSkComponentView(const ShadowView &shadowView);

 protected:
  void OnPaint(SkCanvas *canvas) override;
};

} // namespace react
} // namespace facebook
