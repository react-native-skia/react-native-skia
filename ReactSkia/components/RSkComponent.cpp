#include "ReactSkia/components/RSkComponent.h"

#include "include/core/SkPaint.h"
#include "include/core/SkSurface.h"

namespace facebook {
namespace react {

RSkComponent::RSkComponent(const ShadowView &shadowView)
    : shadowView_(shadowView) {}

RSkComponent::~RSkComponent() {}

void RSkComponent::onPaint(SkSurface *surface) {
  auto canvas = surface->getCanvas();
  OnPaint(shadowView_, canvas);
}

} // namespace react
} // namespace facebook
