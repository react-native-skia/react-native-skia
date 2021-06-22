#include "ReactSkia/components/RSkComponent.h"

#include "include/core/SkPaint.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkSurface.h"

#include "rns_shell/compositor/layers/PictureLayer.h"

namespace facebook {
namespace react {

RSkComponent::RSkComponent(const ShadowView &shadowView)
    : INHERITED(RnsShell::LAYER_TYPE_DEFAULT)
    , parent_(nullptr)
    , absOrigin_(shadowView.layoutMetrics.frame.origin)
    , component_(shadowView)
<<<<<<< HEAD
	{
#ifdef RNS_ENABLE_API_PERF
    , componentName_(shadowView.componentName ? shadowView.componentName : "Rootview")
#endif
=======
>>>>>>> Native Text component & Text Layout Manager handle computation & draw of nested text  (#17)
{
    requiresLayer(shadowView);
}

RSkComponent::~RSkComponent() {}

<<<<<<< HEAD
void RSkComponent::onPaint(SkCanvas* canvas) {
  if(canvas) {
    RNS_PROFILE_API_OFF(component_.componentName << " Paint:", OnPaint(canvas));
=======
void RSkComponent::onPaint(SkSurface *surface) {
  if(surface) {
    auto canvas = surface->getCanvas();
    if(canvas)
        RNS_PROFILE_API_AVG_ON(component_.componentName << " Paint:", OnPaint(canvas));
>>>>>>> Native Text component & Text Layout Manager handle computation & draw of nested text  (#17)
  } else {
    RNS_LOG_ERROR("Invalid canvas ??");
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
<<<<<<< HEAD
    // Text components paragraph builder is not compatabile with Picture layer,so use default layer
    if(strcmp(component_.componentName,"Paragraph") == 0)
        layer_ = this->shared_from_this();
    else
        layer_ = RnsShell::Layer::Create(RnsShell::LAYER_TYPE_PICTURE);

=======
    RNS_LOG_TODO("For now use 0,0 as offset, in future this offset should represent abosulte x,y");
    // Text components paragraph builder is not compatabile with Picture layer,so use default layer
    if(strcmp(component_.componentName,"Paragraph"))
       layer_ = RnsShell::PictureLayer::Create({0,0}, nullptr);
>>>>>>> Native Text component & Text Layout Manager handle computation & draw of nested text  (#17)
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

      Rect frame = component_.layoutMetrics.frame;
      SkIRect frameIRect = SkIRect::MakeXYWH(frame.origin.x, frame.origin.y, frame.size.width, frame.size.height);
      if(layer() && layer().get())
        layer().get()->setFrame(frameIRect);
   }

   if(layer_ && layer_->type() == RnsShell::LAYER_TYPE_PICTURE) {
     RNS_PROFILE_API_OFF(component_.componentName << " getPicture :", static_cast<RnsShell::PictureLayer*>(layer_.get())->setPicture(getPicture()));

void RSkComponent::mountChildComponent(
    std::shared_ptr<RSkComponent> newChildComponent,
    const int index) {

    if(newChildComponent) {
        newChildComponent->parent_ = this;
        newChildComponent->absOrigin_ =  absOrigin_ + newChildComponent->component_.layoutMetrics.frame.origin;
    }

    RNS_LOG_ASSERT((this->layer_ && newChildComponent->layer_), "Layer Object cannot be null");
    if(this->layer_)
        this->layer_->insertChild(newChildComponent->layer_, index);
    const int index) {

    if(oldChildComponent) {
        oldChildComponent->parent_ = nullptr ;
        oldChildComponent->absOrigin_ = oldChildComponent->component_.layoutMetrics.frame.origin;
    }
    if(this->layer_)
        this->layer_->removeChild(oldChildComponent->layer_, index);
}

} // namespace react
} // namespace facebook
