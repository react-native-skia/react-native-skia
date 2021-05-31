#include "ReactSkia/components/RSkComponent.h"

#include "include/core/SkPaint.h"
#include "include/core/SkSurface.h"

namespace facebook {
namespace react {

RSkComponent::RSkComponent(const ShadowView &shadowView)
    : parent_(nullptr)
    , absOrigin_(shadowView.layoutMetrics.frame.origin)
    , component_(shadowView)
#ifdef RNS_ENABLE_API_PERF
    , componentName_(shadowView.componentName ? shadowView.componentName : "Rootview")
#endif
    {
#ifdef RNS_ENABLE_API_PERF
    RNS_UNUSED(componentName_);
#endif
}

RSkComponent::~RSkComponent() {}

void RSkComponent::onPaint(SkSurface *surface) {
  if(surface) {
    auto canvas = surface->getCanvas();
    if(canvas)
        RNS_PROFILE_API_AVG_ON(componentName_ << " Paint:", OnPaint(canvas));
  } else {
      RNS_LOG_ERROR("Invalid canvas ??");
  }
}

void RSkComponent::updateComponentData(const ShadowView &newShadowView , const uint32_t updateMask) {
   if(updateMask & ComponentUpdateMaskProps)
      component_.props = newShadowView.props;
   if(updateMask & ComponentUpdateMaskState)
      component_.state = newShadowView.state;
   if(updateMask & ComponentUpdateMaskEventEmitter)
      component_.eventEmitter = newShadowView.eventEmitter;
   if(updateMask & ComponentUpdateMaskLayoutMetrics)
      component_.layoutMetrics = newShadowView.layoutMetrics;
}

void RSkComponent::mountChildComponent(
    std::shared_ptr<RSkComponent> newChildComponent,
    const int index) {

    if(newChildComponent) {
        newChildComponent->parent_ = this;
        newChildComponent->absOrigin_ =  absOrigin_ + newChildComponent->component_.layoutMetrics.frame.origin;
    }
    this->insertChild(newChildComponent, index);
}

void RSkComponent::unmountChildComponent(
    std::shared_ptr<RSkComponent> oldChildComponent,
    const int index) {

    if(oldChildComponent) {
        oldChildComponent->parent_ = nullptr ;
        oldChildComponent->absOrigin_ = oldChildComponent->component_.layoutMetrics.frame.origin;
    }
    this->removeChild(oldChildComponent, index);
}

} // namespace react
} // namespace facebook
