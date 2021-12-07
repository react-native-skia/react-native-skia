/*
 * Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkPaint.h"
#include "include/core/SkPictureRecorder.h"

#include "ReactSkia/components/RSkComponentScrollView.h"
#include "ReactSkia/utils/RnsUtils.h"
#include "ReactSkia/views/common/RSkConversion.h"

#include "compositor/layers/ScrollLayer.h"

namespace facebook {
namespace react {

#define SCROLL_LAYER_HANDLE static_cast<RnsShell::ScrollLayer*>(layer().get())
#define SCROLL_LAYER(x) SCROLL_LAYER_HANDLE->x


RSkComponentScrollView::RSkComponentScrollView(const ShadowView &shadowView)
    : RSkComponent(shadowView) {
}

bool RSkComponentScrollView::isHorizontalScroll() {
  return SCROLL_LAYER(getContentSize().width()) > getComponentData().layoutMetrics.frame.size.width;
}

RnsShell::LayerInvalidateMask RSkComponentScrollView::updateComponentProps(
    const ShadowView &newShadowView,
    bool forceUpadate) {

  auto const &scrollViewProps = *std::static_pointer_cast<ScrollViewProps const>(newShadowView.props);
  setScrollEnabled(scrollViewProps.scrollEnabled);
  return RnsShell::LayerInvalidateNone;

}

RnsShell::LayerInvalidateMask RSkComponentScrollView::updateComponentState(
    const ShadowView &newShadowView,
    bool forceUpdate) {

  auto state = std::static_pointer_cast<ScrollViewShadowNode::ConcreteStateT const>(newShadowView.state);

  SkISize contentSize = RSkSkSizeFromSize(state->getData().getContentSize()).toRound();
  RnsShell::ScrollLayer* scrollLayer= SCROLL_LAYER_HANDLE;
  /* If contentSize has changed:*/
  /*  - if content size is less than scrollOffset,set to last frame region*/
  /*  - if content size is less than frame,set scrolloffset to start position*/
  if(scrollLayer->setContentSize(contentSize)) {
    Size frameSize = getComponentData().layoutMetrics.frame.size;
    if(isHorizontalScroll()) {
      if(contentSize.width() <= frameSize.width) {
         scrollLayer->scrollOffsetX = 0;
      } else if(scrollLayer->scrollOffsetX >= (contentSize.width()-frameSize.width)){
         scrollLayer->scrollOffsetX = contentSize.width()-frameSize.width;
      }
    } else if (contentSize.height() >= frameSize.height ) {
      scrollLayer->scrollOffsetY = 0;
    } else if (scrollLayer->scrollOffsetY >= (contentSize.height()-frameSize.height)) {
      scrollLayer->scrollOffsetY = contentSize.height()-frameSize.height;
    }
    return RnsShell::LayerInvalidateAll;
  }

  return RnsShell::LayerInvalidateNone;
}

bool RSkComponentScrollView::setScrollOffset(
   ScrollDirectionType scrollDirection,
   float contentLength,
   float viewLength,
   int &scrollOffset) {

   /* Default offset derived from experiments in Android TV emulator*/
   float defaultOffset = 0;
   if(contentLength < viewLength)
      return false;
   else
      defaultOffset = viewLength/2;

   if(scrollDirection == ScrollDirectionForward) {
     /* If next scrolling,check if area available to scroll */
     if(scrollOffset + viewLength == contentLength)
       return false;

     /* Scrollable area available,Set next scrolling offset */
     scrollOffset += defaultOffset;

     /* Scrollable area left is less than frame length,re-update offset to fit */
     if((contentLength - scrollOffset) <= viewLength)
       scrollOffset = contentLength - viewLength;

   }else{
     /* If scrolling previous , */
     /*    if scrollable area is less set to start position*/
     /*    else reduce by default offset */
     if(scrollOffset <= defaultOffset)
        scrollOffset = 0;
     else if (scrollOffset > defaultOffset)
        scrollOffset -= defaultOffset;
   }
   return true;

}

void RSkComponentScrollView::onHandleKey(
    rnsKey  eventKeyType,
    bool* stopPropagate) {

    bool handled = false;
    RNS_LOG_DEBUG("onHandleKey component tag["<< getComponentData().tag << "] eventKeyType[" << eventKeyType << "]");
    Size frameSize = getComponentData().layoutMetrics.frame.size;

    RnsShell::ScrollLayer* scrollLayer= SCROLL_LAYER_HANDLE;
    SkISize contentSize = scrollLayer->getContentSize();

    if(isHorizontalScroll()) {
       switch(eventKeyType) {
         case RNS_KEY_Right:
             handled = setScrollOffset(ScrollDirectionForward,contentSize.width(),frameSize.width,scrollLayer->scrollOffsetX);
             break;
         case RNS_KEY_Left:
             handled = setScrollOffset(ScrollDirectionBackward,contentSize.width(),frameSize.width,scrollLayer->scrollOffsetX);
         default: break;
       }
    } else {
      switch(eventKeyType) {
         case RNS_KEY_Down:
             handled = setScrollOffset(ScrollDirectionForward,contentSize.height(),frameSize.height,scrollLayer->scrollOffsetY);
             break;
         case RNS_KEY_Up:
             handled = setScrollOffset(ScrollDirectionBackward,contentSize.height(),frameSize.height,scrollLayer->scrollOffsetY);
         default: break;
      }
    }

    if(handled) {
       scrollLayer->invalidate(LayerPaintInvalidate);
       scrollLayer->client().notifyFlushRequired();
    } else {
      /* TODO Element cannot handle event,so propogate the event to parent */
    }

    *stopPropagate = handled;
    return;
}

void RSkComponentScrollView::OnPaint(SkCanvas *canvas) {}

} // namespace react
} // namespace facebook
