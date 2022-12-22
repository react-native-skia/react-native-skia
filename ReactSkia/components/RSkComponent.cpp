/*
* Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
* Copyright (C) Kudo Chien
*
* This source code is licensed under the MIT license found in the
* LICENSE file in the root directory of this source tree.
*/

#include "include/core/SkPaint.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkSurface.h"
#include "include/effects/SkImageFilters.h"

#include "ReactSkia/components/RSkComponent.h"
#include "ReactSkia/views/common/RSkConversion.h"

#include "rns_shell/compositor/layers/PictureLayer.h"
#include "rns_shell/compositor/layers/ScrollLayer.h"
#include "rns_shell/platform/linux/TaskLoop.h"

namespace facebook {
namespace react {

using namespace RnsShell;

RSkComponent::RSkComponent(const ShadowView &shadowView)
    : INHERITED(Layer::EmptyClient::singleton(), LAYER_TYPE_DEFAULT)
    , parent_(nullptr)
    , component_(shadowView) {}

RSkComponent::~RSkComponent() {}

void RSkComponent::onPaint(SkCanvas* canvas) {
  if(canvas) {
    RNS_PROFILE_API_OFF(component_.componentName << " Paint:", OnPaint(canvas));
  } else {
    RNS_LOG_ERROR("Invalid canvas ??");
  }
}

void RSkComponent::OnPaintBorder(SkCanvas *canvas) {
  auto const &viewProps = *std::static_pointer_cast<ViewProps const>(component_.props);

  /* apply view style props */
  auto borderMetrics= viewProps.resolveBorderMetrics(component_.layoutMetrics);
  drawBorder(canvas,component_.layoutMetrics.frame,borderMetrics,layer_->backgroundColor);
}

void RSkComponent::OnPaintShadow(SkCanvas *canvas) {
  auto const &viewProps = *std::static_pointer_cast<ViewProps const>(component_.props);

  /* apply view style props */
  auto borderMetrics= viewProps.resolveBorderMetrics(component_.layoutMetrics);
  if(layer_->isShadowVisible) {
      drawShadow(canvas,component_.layoutMetrics.frame,borderMetrics,
                 layer_->backgroundColor,
                 layer_->shadowColor,layer_->shadowOffset,layer_->shadowOpacity,
                 layer_->opacity,
                 layer_->shadowImageFilter,layer_->shadowMaskFilter
                 );
  }
}

sk_sp<SkPicture> RSkComponent::getPicture(PictureType type) {

  SkPictureRecorder recorder;
  auto frame = component_.layoutMetrics.frame;

  auto *canvas = recorder.beginRecording(SkRect::MakeXYWH(0, 0, frame.size.width, frame.size.height));

  if(canvas) {
     switch(type) {
       case PictureTypeShadow:
          RNS_PROFILE_API_OFF("Recording Shadow Picture" << component_.componentName << " Paint:", OnPaintShadow(canvas));
          break;
       case PictureTypeBorder:
          RNS_PROFILE_API_OFF("Recording Border Picture" << component_.componentName << " Paint:", OnPaintBorder(canvas));
          break;
       case PictureTypeAll:
       default:
          RNS_PROFILE_API_OFF("Recording " << component_.componentName << " Paint:", OnPaint(canvas));
          break;
    }
  } else {
    RNS_LOG_ERROR("Invalid canvas ??");
    return nullptr;
  }

  return recorder.finishRecordingAsPicture();
}

void RSkComponent::requiresLayer(const ShadowView &shadowView, Layer::Client& layerClient) {
    // Need to come up with rules to decide wheather we need to create picture layer, texture layer etc"
    // Text components paragraph builder is not compatabile with Picture layer,so use default layer
    if(strcmp(component_.componentName,"Paragraph") == 0) {
        layer_ = this->shared_from_this();
        layer_->setClient(layerClient); // Need to set client for Default layer type.
    } else if (strcmp(component_.componentName,"ScrollView") == 0)
        layer_ = Layer::Create(layerClient, LAYER_TYPE_SCROLL);
    else
        layer_ = Layer::Create(layerClient, LAYER_TYPE_PICTURE);
}

void RSkComponent::setNeedFocusUpdate(){
  TaskLoop::main().dispatch([weakthis = this->weak_from_this()]() {
    auto self = weakthis.lock();
    if(self) {
      RNS_LOG_DEBUG("setNeedFocusUpdate tag:" << self->getComponentData().tag << " name:" << self->getComponentData().componentName);
      //set focus update(handle blur update->scroll->handle focus update) with needScroll as true
      SpatialNavigator::RSkSpatialNavigator::sharedSpatialNavigator()->updateFocusCandidate(self.get(),true);
    }
  });
}

RnsShell::LayerInvalidateMask RSkComponent::updateProps(SharedProps newViewProps,bool forceUpdate) {

   auto const &newviewProps = *std::static_pointer_cast<ViewProps const>(newViewProps);
   auto const &oldviewProps = *std::static_pointer_cast<ViewProps const>(component_.props);
   RnsShell::LayerInvalidateMask updateMask=RnsShell::LayerInvalidateNone;
   bool createShadowFilter=false;

   updateMask= updateComponentProps(newViewProps,forceUpdate);
  //opacity
   if((forceUpdate) || (oldviewProps.opacity != newviewProps.opacity)) {
      layer_->opacity = ((newviewProps.opacity > 1.0)? 1.0:newviewProps.opacity)*MAX_8BIT;
      /*TODO : To be tested and confirm updateMask need for this Prop*/
      updateMask =static_cast<RnsShell::LayerInvalidateMask>(updateMask | RnsShell::LayerInvalidateAll);
   }
  //ShadowOpacity
   if ((forceUpdate) || (oldviewProps.shadowOpacity != newviewProps.shadowOpacity)) {
      layer_->shadowOpacity = ((newviewProps.shadowOpacity > 1.0) ? 1.0:newviewProps.shadowOpacity)*MAX_8BIT;
      /*TODO : To be tested and confirm updateMask need for this Prop*/
      updateMask =static_cast<RnsShell::LayerInvalidateMask>(updateMask | RnsShell::LayerInvalidateAll);
      createShadowFilter=true;
   }
  //shadowRadius
   if ((forceUpdate) || (oldviewProps.shadowRadius != newviewProps.shadowRadius)) {
      layer_->shadowRadius = newviewProps.shadowRadius;
      /*TODO : To be tested and confirm updateMask need for this Prop*/
      updateMask =static_cast<RnsShell::LayerInvalidateMask>(updateMask | RnsShell::LayerInvalidateAll);
      createShadowFilter=true;
   }
  //shadowoffset
   if ((forceUpdate) || (oldviewProps.shadowOffset != newviewProps.shadowOffset)) {
      layer_->shadowOffset = RSkSkSizeFromSize(newviewProps.shadowOffset);
      /*TODO : To be tested and confirm updateMask need for this Prop*/
      updateMask =static_cast<RnsShell::LayerInvalidateMask>(updateMask | RnsShell::LayerInvalidateAll);
      createShadowFilter=true;
   }
  //shadowcolor
   if ((forceUpdate) || (oldviewProps.shadowColor != newviewProps.shadowColor)) {
      layer_->shadowColor = RSkColorFromSharedColor(newviewProps.shadowColor,SK_ColorBLACK);
      /*TODO : To be tested and confirm updateMask need for this Prop*/
      updateMask =static_cast<RnsShell::LayerInvalidateMask>(updateMask | RnsShell::LayerInvalidateAll);
      createShadowFilter=true;
   }

   layer_->isShadowVisible=needsShadowPainting();

   if( createShadowFilter ) {
/*Creating both Skia's Mask & Image filters here.
   Mask Flter will be used for Rect frames / Affine Frames.
   Image Filter will be used for discrete frames such as path or frames with transparent pixels.
*/
//TO DO : Need to Miantain only the filter needed , not the both always.

       layer_->shadowImageFilter= SkImageFilters::DropShadowOnly(layer_->shadowOffset.width(),
                                       layer_->shadowOffset.height(),
                                       layer_->shadowRadius, layer_->shadowRadius,
                                       layer_->shadowColor, nullptr);

       layer_->shadowMaskFilter= SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, layer_->shadowRadius);
   }

  //backgroundColor
   if ((forceUpdate) || (oldviewProps.backgroundColor != newviewProps.backgroundColor)) {
      layer_->backgroundColor = RSkColorFromSharedColor(newviewProps.backgroundColor,SK_ColorTRANSPARENT);
      /*TODO : To be tested and confirm updateMask need for this Prop*/
      updateMask =static_cast<RnsShell::LayerInvalidateMask>(updateMask | RnsShell::LayerInvalidateAll);
   }
  //transform
   if ((forceUpdate) || (oldviewProps.transform != newviewProps.transform)) {
      layer_->transformMatrix = RSkTransformTo2DMatrix(newviewProps.transform);
      updateMask =static_cast<RnsShell::LayerInvalidateMask>(updateMask | RnsShell::LayerLayoutInvalidate);
   }

  /* TODO : Following properties needs implementation and update invalidate mask accordingly*/
  //foregroundColor
   if ((forceUpdate) || (oldviewProps.foregroundColor != newviewProps.foregroundColor)) {
      component_.commonProps.foregroundColor = RSkColorFromSharedColor(newviewProps.foregroundColor,SK_ColorTRANSPARENT);
   }
  //backfaceVisibility
   if ((forceUpdate) || (oldviewProps.backfaceVisibility != newviewProps.backfaceVisibility)) {
      layer_->backfaceVisibility = (int)newviewProps.backfaceVisibility;
   }
  //pointerEvents
   if ((forceUpdate) || (oldviewProps.pointerEvents != newviewProps.pointerEvents)) {
      component_.commonProps.pointerEvents = (int)newviewProps.pointerEvents;
   }
  //hitslop
   if ((forceUpdate) || (oldviewProps.hitSlop != newviewProps.hitSlop)) {
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
#if defined(TARGET_OS_TV) && TARGET_OS_TV
   if((forceUpdate) || (oldviewProps.hasTVPreferredFocus != newviewProps.hasTVPreferredFocus)) {
      component_.commonProps.hasTVPreferredFocus = newviewProps.hasTVPreferredFocus;
      //No invalidate mask setting required for this prop
      if(component_.commonProps.hasTVPreferredFocus) {
         setNeedFocusUpdate();
      }
   }
   /* TODO Add TVOS remaining properties */
#endif
   /*TODO : Return UpdateMask instead of RnsShell::LayerInvalidateAll, once the shadow handling moved to layer
            and all the layer & component props have been verfied*/
   return RnsShell::LayerInvalidateAll;
}

void RSkComponent::updateComponentData(const ShadowView &newShadowView,const uint32_t updateMask,bool forceUpdate) {

   RNS_LOG_ASSERT((layer_ && layer_.get()), "Layer Object cannot be null");
   RNS_LOG_DEBUG("->Update " << component_.componentName << " layer(" << layer_->layerId() << ")");

   RnsShell::LayerInvalidateMask invalidateMask=RnsShell::LayerInvalidateNone;

   //Check and update LayoutMetrics first, so that prop/state update can use the latest frame values.
   if(updateMask & ComponentUpdateMaskLayoutMetrics) {
      RNS_LOG_DEBUG("\tUpdate Layout");
      component_.layoutMetrics = newShadowView.layoutMetrics;
      invalidateMask =static_cast<RnsShell::LayerInvalidateMask>(invalidateMask | RnsShell::LayerInvalidateAll);

      Rect frame = component_.layoutMetrics.frame;
      SkIRect frameIRect = SkIRect::MakeXYWH(frame.origin.x, frame.origin.y, frame.size.width, frame.size.height);
      if(layer() && layer().get())
        layer_->setFrame(frameIRect);
   }
   if(updateMask & ComponentUpdateMaskProps) {
      RNS_LOG_DEBUG("\tUpdate Property");
      invalidateMask = static_cast<RnsShell::LayerInvalidateMask>(invalidateMask | updateProps(newShadowView.props,forceUpdate));
      component_.props = newShadowView.props;

      //TODO only if TV related proeprties have changed ?
      SpatialNavigator::Container *containerInUpdate = nearestAncestorContainer();
      if(containerInUpdate)
          containerInUpdate->updateComponent(this);

   }
   if(updateMask & ComponentUpdateMaskState){
      RNS_LOG_DEBUG("\tUpdate State");
      invalidateMask =static_cast<RnsShell::LayerInvalidateMask>(invalidateMask | updateComponentState(newShadowView,forceUpdate));
      component_.state = newShadowView.state;
   }
   if(updateMask & ComponentUpdateMaskEventEmitter){
      RNS_LOG_DEBUG("\tUpdate Emitter");
      component_.eventEmitter = newShadowView.eventEmitter;
   }

#if defined(TARGET_OS_TV) && TARGET_OS_TV
   //If layout has changed and preferredFocus is true, then set focus to this item
   if((!forceUpdate) &&
      (component_.commonProps.hasTVPreferredFocus) &&
      (updateMask & ComponentUpdateMaskLayoutMetrics)){

      setNeedFocusUpdate();
   }
#endif
   drawAndSubmit(invalidateMask);
}

void RSkComponent::drawAndSubmit(RnsShell::LayerInvalidateMask invalidateMask) {
   if(layer_ && layer_.get()) {
     layer_->invalidate(invalidateMask);
     if(layer_->type() == RnsShell::LAYER_TYPE_PICTURE) {
       RNS_PROFILE_API_OFF(component_.componentName << " getPicture :", static_cast<RnsShell::PictureLayer*>(layer_.get())->setPicture(getPicture()));
     } else if(layer_->type() == RnsShell::LAYER_TYPE_SCROLL) {
       RNS_PROFILE_API_OFF(component_.componentName << " getShadowPicture :", static_cast<RnsShell::ScrollLayer*>(layer_.get())->setShadowPicture(getPicture(PictureTypeShadow)));
       RNS_PROFILE_API_OFF(component_.componentName << " getBorderPicture :", static_cast<RnsShell::ScrollLayer*>(layer_.get())->setBorderPicture(getPicture(PictureTypeBorder)));
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

        SpatialNavigator::Container *containerToAdd = nullptr;
        if(isContainer() == true || ((containerToAdd = nearestAncestorContainer()) == nullptr))
            containerToAdd = this;

        if(newChildComponent->isFocusable()) {
            containerToAdd->addComponent(newChildComponent.get());
            if(!newChildComponent->isContainer() && newChildComponent->navComponentList_.size() != 0)
                containerToAdd->mergeComponent(newChildComponent->navComponentList_); // Move focusable decendents to parent
        } else if(newChildComponent->navComponentList_.size() != 0) {
            containerToAdd->mergeComponent(newChildComponent->navComponentList_); // Move focusable decendents to parent
        }
    }
}

void RSkComponent::unmountChildComponent(
    std::shared_ptr<RSkComponent> oldChildComponent,
    const int index) {

    SpatialNavigator::Container *containerRemoveFrom = nullptr;
    if(oldChildComponent) {
        containerRemoveFrom = oldChildComponent->nearestAncestorContainer();
        if(containerRemoveFrom && oldChildComponent->isFocusable())
            containerRemoveFrom->removeComponent(oldChildComponent.get());

        oldChildComponent->parent_ = nullptr ;

        RNS_LOG_ASSERT((this->layer_ && oldChildComponent->layer_), "Layer Object cannot be null");
        if(oldChildComponent->layer_)
            oldChildComponent->layer_->invalidate(RnsShell::LayerRemoveInvalidate);
    }
}

bool RSkComponent::isFocusable() {
#if defined(TARGET_OS_TV) && TARGET_OS_TV
  Component canData = getComponentData();
  auto const &viewProps = *std::static_pointer_cast<ViewProps const>(canData.props);
  if (viewProps.isTVSelectable == true || viewProps.focusable == true || isContainer())
    return true;
#else
  if (isContainer())
    return true;
#endif //TARGET_OS_TV
  return false;
}

SpatialNavigator::Container* RSkComponent::nearestAncestorContainer() {
  RSkComponent *container = nullptr;
  for (container = parent_; container; container = container->getParent()) {
    if (container->isContainer())
      return container;
  }
  return nullptr;
}

bool RSkComponent::hasAncestor(const SpatialNavigator::Container* ancestor) {
  for (RSkComponent *container = parent_; container; container = container->getParent()) {
    if (container->isContainer() && container == ancestor)
      return true;
  }
  return false;
}

const SkIRect RSkComponent::getScreenFrame() {
  SpatialNavigator::Container* container = nearestAncestorContainer();
  SkIRect absFrame = getLayerAbsoluteFrame();

  /* If ancestor container is not scrollable,return absFrame*/
  /* else , returns containerScreenFrame + ( component absFrame - container scrollOffset)*/
  if((container == nullptr) || (!container->isScrollable())) return absFrame;

  SkIRect containerScreenFrame = static_cast<RSkComponent*>(container)->getScreenFrame();
  SkPoint containerScrollOffset = container->getScrollOffset();
  return absFrame.makeOffset(-containerScrollOffset.x(),
                             -containerScrollOffset.y()).makeOffset(
                              containerScreenFrame.x(),
                              containerScreenFrame.y());
}

bool RSkComponent::needsShadowPainting() {


 /*Shadow will not visible on below scenarios
  1. Shadow opacity is completely Transparent.
  2.Frame is compelety transparent.
  3. Shadow color has Alpha as fully transparent.
  4. shadow is directly under the frame & frame is fully opaque.
*/
  if ((layer_->shadowOpacity ==0)||(layer_->opacity == 0) ||
      (SkColorGetA(layer_->shadowColor) == SK_AlphaTRANSPARENT) ||
      (layer_->shadowOffset.isEmpty() && isOpaque(layer_->opacity)) ) {
    return false;
  }
  return true;
}

} // namespace react
} // namespace facebook
