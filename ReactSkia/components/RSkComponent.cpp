#include "ReactSkia/components/RSkComponent.h"

#include "include/core/SkPaint.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkSurface.h"

#include "rns_shell/compositor/layers/PictureLayer.h"

namespace facebook {
namespace react {

RSkComponent::RSkComponent(const ShadowView &shadowView)
    : parent_(nullptr)
    , absOrigin_(shadowView.layoutMetrics.frame.origin)
    , component_(shadowView)
{
    requiresLayer(shadowView);
}

RSkComponent::~RSkComponent() {}

void RSkComponent::onPaint(SkSurface *surface) {
  if(surface) {
    auto canvas = surface->getCanvas();
    if(canvas)
        RNS_PROFILE_API_AVG_ON(component_.componentName << " Paint:", OnPaint(canvas));
  } else {
      RNS_LOG_ERROR("Invalid Surface ??");
  }
}

sk_sp<SkPicture> RSkComponent::getPicture() {

  SkPictureRecorder recorder;
  auto frame = getAbsoluteFrame();

  auto *canvas = recorder.beginRecording(SkRect::MakeXYWH(0, 0, frame.size.width, frame.size.height));

  if(canvas) {
    RNS_PROFILE_API_OFF("Recording " << component_.componentName << " Paint:", OnPaint(canvas));
  } else {
    RNS_LOG_ERROR("Invalid canvas ??");
    return nullptr;
  }

  return recorder.finishRecordingAsPicture();
}

void RSkComponent::requiresLayer(const ShadowView &shadowView) {
    RNS_LOG_TODO("Need to come up with rules to decide wheather we need to create picture layer, texture layer etc");
    RNS_LOG_TODO("For now use 0,0 as offset, in future this offset should represent abosulte x,y");
    // Text components paragraph builder is not compatabile with Picture layer,so use default layer
    if(strcmp(component_.componentName,"Paragraph"))
       layer_ = RnsShell::PictureLayer::Create({0,0}, nullptr);
}

void RSkComponent::updateComponentData(const ShadowView &newShadowView , const uint32_t updateMask) {
   if(updateMask & ComponentUpdateMaskProps)
      component_.props = newShadowView.props;
   if(updateMask & ComponentUpdateMaskState)
      component_.state = newShadowView.state;
   if(updateMask & ComponentUpdateMaskEventEmitter)
      component_.eventEmitter = newShadowView.eventEmitter;
   if(updateMask & ComponentUpdateMaskLayoutMetrics) {
      component_.layoutMetrics = newShadowView.layoutMetrics;
      /* TODO : Analyze if this computation can be handled in RNS shell Layer */
      absOrigin_ =  parent_ ? (parent_->absOrigin_ + component_.layoutMetrics.frame.origin) : component_.layoutMetrics.frame.origin;
   }
   
   if(layer_) {
     RNS_PROFILE_API_OFF(component_.componentName << " getPicture :", static_cast<RnsShell::PictureLayer*>(layer_.get())->setPicture(getPicture()));
   } 
}

void RSkComponent::mountChildComponent(
    std::shared_ptr<RSkComponent> newChildComponent,
    const int index) {

    if(newChildComponent) {
        newChildComponent->parent_ = this;
        newChildComponent->absOrigin_ =  absOrigin_ + newChildComponent->component_.layoutMetrics.frame.origin;
    }

    if(this->layer_) { // If parent has a layer
        if(newChildComponent->layer_)
            this->layer_->insertChild(newChildComponent->layer_, index);
        else
            this->layer_->insertChild(newChildComponent, index);
    }else {  // If parent itself is a layer type
        if(newChildComponent->layer_)
            this->insertChild(newChildComponent->layer_, index);
        else
            this->insertChild(newChildComponent, index);
    }
}

void RSkComponent::unmountChildComponent(
    std::shared_ptr<RSkComponent> oldChildComponent,
    const int index) {

    if(oldChildComponent) {
        oldChildComponent->parent_ = nullptr ;
        oldChildComponent->absOrigin_ = oldChildComponent->component_.layoutMetrics.frame.origin;
    }

    if(this->layer_) { // If parent has a layer
        if(oldChildComponent->layer_)
            this->layer_->removeChild(oldChildComponent->layer_, index);
        else
            this->layer_->removeChild(oldChildComponent, index);
    }else {  // If parent itself is a layer type
        if(oldChildComponent->layer_)
            this->removeChild(oldChildComponent->layer_, index);
        else
            this->removeChild(oldChildComponent, index);
    }
}

} // namespace react
} // namespace facebook
