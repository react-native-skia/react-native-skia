#include "ReactSkia/components/RSkComponent.h"
#include "ReactSkia/views/common/RSkConversion.h"

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
    , component_(shadowView) {}

RSkComponent::~RSkComponent() {}

void RSkComponent::onPaint(SkCanvas* canvas) {
  if(canvas) {
    RNS_PROFILE_API_OFF(component_.componentName << " Paint:", OnPaint(canvas));
  } else {
    RNS_LOG_ERROR("Invalid canvas ??");
  }
}

sk_sp<SkPicture> RSkComponent::getPicture() {

  SkPictureRecorder recorder;
  auto frame = component_.layoutMetrics.frame;

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
    // Need to come up with rules to decide wheather we need to create picture layer, texture layer etc"
    // Text components paragraph builder is not compatabile with Picture layer,so use default layer
    if(strcmp(component_.componentName,"Paragraph") == 0)
        layer_ = this->shared_from_this();
    else
        layer_ = RnsShell::Layer::Create(RnsShell::LAYER_TYPE_PICTURE);
}

RnsShell::LayerInvalidateMask RSkComponent::updateProps(const ShadowView &newShadowView,bool forceUpdate) {

   auto const &newviewProps = *std::static_pointer_cast<ViewProps const>(newShadowView.props);
   auto const &oldviewProps = *std::static_pointer_cast<ViewProps const>(component_.props);
   RnsShell::LayerInvalidateMask updateMask=RnsShell::LayerInvalidateNone;

   updateMask= updateComponentProps(newShadowView,forceUpdate);
  //opacity
   if((forceUpdate) || (oldviewProps.opacity != newviewProps.opacity)) {
      layer_->opacity = newviewProps.opacity;
      /*TODO : To be tested and confirm updateMask need for this Prop*/
      updateMask =static_cast<RnsShell::LayerInvalidateMask>(updateMask | RnsShell::LayerInvalidateAll);
   }
  //ShadowOpacity
   if ((forceUpdate) || (oldviewProps.shadowOpacity != newviewProps.shadowOpacity)) {
      layer_->shadowOpacity = newviewProps.shadowOpacity;
      /*TODO : To be tested and confirm updateMask need for this Prop*/
      updateMask =static_cast<RnsShell::LayerInvalidateMask>(updateMask | RnsShell::LayerInvalidateAll);
   }
  //shadowRadius
   if ((forceUpdate) || (oldviewProps.shadowRadius != newviewProps.shadowRadius)) {
      layer_->shadowRadius = newviewProps.shadowRadius;
      /*TODO : To be tested and confirm updateMask need for this Prop*/
      updateMask =static_cast<RnsShell::LayerInvalidateMask>(updateMask | RnsShell::LayerInvalidateAll);
   }
  //shadowoffset
   if ((forceUpdate) || (oldviewProps.shadowOffset != newviewProps.shadowOffset)) {
      layer_->shadowOffset = RSkSkSizeFromSize(newviewProps.shadowOffset);
      /*TODO : To be tested and confirm updateMask need for this Prop*/
      updateMask =static_cast<RnsShell::LayerInvalidateMask>(updateMask | RnsShell::LayerInvalidateAll);
   }
  //shadowcolor
   if ((forceUpdate) || (oldviewProps.shadowColor != newviewProps.shadowColor)) {
      layer_->shadowColor = RSkColorFromSharedColor(newviewProps.shadowColor,SK_ColorBLACK);
      /*TODO : To be tested and confirm updateMask need for this Prop*/
      updateMask =static_cast<RnsShell::LayerInvalidateMask>(updateMask | RnsShell::LayerInvalidateAll);
   }
  //backfaceVisibility
   if ((forceUpdate) || (oldviewProps.backfaceVisibility != newviewProps.backfaceVisibility)) {
      RNS_LOG_NOT_IMPL;
      layer_->backfaceVisibility = (int)newviewProps.backfaceVisibility;
   }
  //backgroundColor
   if ((forceUpdate) || (oldviewProps.backgroundColor != newviewProps.backgroundColor)) {
      component_.commonProps.backgroundColor = RSkColorFromSharedColor(newviewProps.backgroundColor,SK_ColorTRANSPARENT);
      /*TODO : To be tested and confirm updateMask need for this Prop*/
      updateMask =static_cast<RnsShell::LayerInvalidateMask>(updateMask | RnsShell::LayerInvalidateAll);
   }
  //foregroundColor
   if ((forceUpdate) || (oldviewProps.foregroundColor != newviewProps.foregroundColor)) {
      RNS_LOG_NOT_IMPL;
      component_.commonProps.foregroundColor = RSkColorFromSharedColor(newviewProps.foregroundColor,SK_ColorTRANSPARENT);
   }
  /* TODO To be verified when implemented*/
  //pointerEvents
   if ((forceUpdate) || (oldviewProps.pointerEvents != newviewProps.pointerEvents)) {
      RNS_LOG_NOT_IMPL;
      component_.commonProps.pointerEvents = (int)newviewProps.pointerEvents;
   }
  //hitslop
   if ((forceUpdate) || (oldviewProps.hitSlop != newviewProps.hitSlop)) {
      RNS_LOG_NOT_IMPL;
      component_.commonProps.hitSlop = newviewProps.hitSlop;
   }
  //overflow
   if ((forceUpdate) || (oldviewProps.getClipsContentToBounds() != newviewProps.getClipsContentToBounds())) {
      layer_->setMasksTotBounds(newviewProps.getClipsContentToBounds());
      /*TODO : To be tested and confirm updateMask need for this Prop*/
      updateMask =static_cast<RnsShell::LayerInvalidateMask>(updateMask | RnsShell::LayerInvalidateAll);
   }
  //zIndex
   if ((forceUpdate) || (oldviewProps.zIndex != newviewProps.zIndex)) {
      component_.commonProps.zIndex = newviewProps.zIndex.value_or(0);
      /*TODO : To be tested and confirm updateMask need for this Prop*/
      updateMask =static_cast<RnsShell::LayerInvalidateMask>(updateMask | RnsShell::LayerInvalidateAll);
   }
  //transform
   if ((forceUpdate) || (oldviewProps.transform != newviewProps.transform)) {
      layer_->transformMatrix = RSkTransformTo2DMatrix(newviewProps.transform);
      updateMask =static_cast<RnsShell::LayerInvalidateMask>(updateMask | RnsShell::LayerLayoutInvalidate);
   }
    /* TODO Add TVOS properties */
   /*TODO : Return UpdateMask instead of RnsShell::LayerInvalidateAll, once the shadow handling moved to layer
            and all the layer & component props have been verfied*/
   return RnsShell::LayerInvalidateAll;
}

void RSkComponent::updateComponentData(const ShadowView &newShadowView,const uint32_t updateMask,bool forceUpdate) {

   RNS_LOG_ASSERT((layer_ && layer_.get()), "Layer Object cannot be null");
   RNS_LOG_DEBUG("->Update " << component_.componentName << " layer(" << layer_->layerId() << ")");

   RnsShell::LayerInvalidateMask invalidateMask=RnsShell::LayerInvalidateNone;

   if(updateMask & ComponentUpdateMaskProps) {
      RNS_LOG_DEBUG("\tUpdate Property");
      invalidateMask = static_cast<RnsShell::LayerInvalidateMask>(invalidateMask | updateProps(newShadowView,forceUpdate));
      component_.props = newShadowView.props;
   }
   if(updateMask & ComponentUpdateMaskState){
      RNS_LOG_DEBUG("\tUpdate State");
      invalidateMask =static_cast<RnsShell::LayerInvalidateMask>(invalidateMask | RnsShell::LayerInvalidateAll);
      component_.state = newShadowView.state;
   }
   if(updateMask & ComponentUpdateMaskEventEmitter){
      RNS_LOG_DEBUG("\tUpdate Emitter");
      component_.eventEmitter = newShadowView.eventEmitter;
   }
   if(updateMask & ComponentUpdateMaskLayoutMetrics) {
      RNS_LOG_DEBUG("\tUpdate Layout");
      component_.layoutMetrics = newShadowView.layoutMetrics;
      invalidateMask =static_cast<RnsShell::LayerInvalidateMask>(invalidateMask | RnsShell::LayerInvalidateAll);

      Rect frame = component_.layoutMetrics.frame;
      SkIRect frameIRect = SkIRect::MakeXYWH(frame.origin.x, frame.origin.y, frame.size.width, frame.size.height);
      if(layer() && layer().get())
        layer_->setFrame(frameIRect);
   }

   if(layer_ && layer_.get()) {
     layer_->invalidate(invalidateMask);
     if(layer_->type() == RnsShell::LAYER_TYPE_PICTURE) {
       RNS_PROFILE_API_OFF(component_.componentName << " getPicture :", static_cast<RnsShell::PictureLayer*>(layer_.get())->setPicture(getPicture()));
     }
   } 
}

void RSkComponent::mountChildComponent(
    std::shared_ptr<RSkComponent> newChildComponent,
    const int index) {

    if(newChildComponent) {
        newChildComponent->parent_ = this;
        RNS_LOG_ASSERT((this->layer_ && newChildComponent->layer_), "Layer Object cannot be null");
        if(this->layer_)
            this->layer_->insertChild(newChildComponent->layer_, index);
    }
}

void RSkComponent::unmountChildComponent(
    std::shared_ptr<RSkComponent> oldChildComponent,
    const int index) {

    if(oldChildComponent) {
        oldChildComponent->parent_ = nullptr ;
        oldChildComponent->absOrigin_ = oldChildComponent->component_.layoutMetrics.frame.origin;
    }

    RNS_LOG_ASSERT((this->layer_ && oldChildComponent->layer_), "Layer Object cannot be null");
    if(this->layer_)
        this->layer_->removeChild(oldChildComponent->layer_.get(), index);
}

} // namespace react
} // namespace facebook
