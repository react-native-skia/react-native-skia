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

RSkComponentScrollView::RSkComponentScrollView(const ShadowView &shadowView)
    : RSkComponent(shadowView) {
}

void RSkComponentScrollView::OnPaint(SkCanvas *canvas) {}

RnsShell::LayerInvalidateMask RSkComponentScrollView::updateComponentProps(
    const ShadowView &newShadowView,
    bool forceUpadate) {

  auto const &scrollViewProps = *std::static_pointer_cast<ScrollViewProps const>(newShadowView.props);
  scrollEnabled_ = scrollViewProps.scrollEnabled;

  snapToOffsets_.clear();
  for(auto &offset : scrollViewProps.snapToOffsets) {
    snapToOffsets_.push_back(static_cast<int>(offset));
  }
  sort(snapToOffsets_.begin(),snapToOffsets_.end());

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
    SkIRect frame = scrollLayer->getFrame();
    if(isHorizontalScroll()) {
      if(contentSize.width() <= frame.width()) {
         scrollLayer->scrollOffsetX = 0;
      } else if(scrollLayer->scrollOffsetX >= (contentSize.width()-frame.width())){
         scrollLayer->scrollOffsetX = contentSize.width()-frame.width();
      }
    } else if (contentSize.height() >= frame.height()) {
      scrollLayer->scrollOffsetY = 0;
    } else if (scrollLayer->scrollOffsetY >= (contentSize.height()-frame.height())) {
      scrollLayer->scrollOffsetY = contentSize.height()-frame.height();
    }
    return RnsShell::LayerInvalidateAll;
  }

  return RnsShell::LayerInvalidateNone;
}

void RSkComponentScrollView::handleCommand(std::string commandName,folly::dynamic args){
  if(commandName == "scrollToEnd") {
    if(args.size() != 1 ) {
      RNS_LOG_ERROR("Command scrollToEnd received " << args.size() << " arguments,expected 1");
      return;
    }

    RNS_LOG_DEBUG("handleCommand commandName[scrollToEnd] args[" << args[0] <<"]");

    RnsShell::ScrollLayer* scrollLayer= SCROLL_LAYER_HANDLE;
    SkISize contentSize = scrollLayer->getContentSize();
    SkIRect frameRect = scrollLayer->getFrame();
    SkPoint lastScrollOffset = SkPoint::Make(0,0);

    if(isHorizontalScroll()) {
      if(contentSize.width() <= frameRect.width()) {
         RNS_LOG_DEBUG("No scrollable content to scroll");
         return;
      }
      lastScrollOffset.fX = contentSize.width() - frameRect.width();
    } else {
      if(contentSize.height() <= frameRect.height()) {
         RNS_LOG_DEBUG("No scrollable content to scroll");
         return;
      }
      lastScrollOffset.fY = contentSize.height() - frameRect.height();
    }

    if(lastScrollOffset.equals(scrollLayer->scrollOffsetX,scrollLayer->scrollOffsetY)) {
      RNS_LOG_DEBUG("Scroll position is already at end");
      return;
    }

    if(args[0].getBool()) RNS_LOG_TODO("Animated not supported,fallback to scroll immediately");
    handleScroll(lastScrollOffset);
    return;

  } else if(commandName == "scrollTo") {
    if(args.size() != 3 ) {
      RNS_LOG_ERROR("Command scrollTo received " << args.size() << " arguments,expected 3");
      return;
    }

    RNS_LOG_DEBUG("handleCommand commandName[scrollTo] args[" << args[0] <<"," << args[1] << "," << args[2] <<"]");

    int x = static_cast<int>(args[0].getDouble());
    int y = static_cast<int>(args[1].getDouble());

    RnsShell::ScrollLayer * scrollLayer = SCROLL_LAYER_HANDLE;
    SkISize contentSize = scrollLayer->getContentSize();
    SkIRect frameRect = scrollLayer->getFrame();
    SkPoint scrollOffset = SkPoint::Make(0,0);

    if(isHorizontalScroll()) {
      if(contentSize.width() <= frameRect.width()) {
         RNS_LOG_DEBUG("No scrollable content to scroll");
         return;
      }
      scrollOffset.fX = std::min(std::max(0,x),(contentSize.width()-frameRect.width()));
    } else {
      if(contentSize.height() <= frameRect.height()) {
         RNS_LOG_DEBUG("No scrollable content to scroll");
         return;
      }
      scrollOffset.fY = std::min(std::max(0,y),(contentSize.height()-frameRect.height()));
    }

    if(scrollOffset.equals(scrollLayer->scrollOffsetX,scrollLayer->scrollOffsetY)) {
      RNS_LOG_DEBUG("Scroll position is already at same offset");
      return;
    }

    if(args[2].getBool()) RNS_LOG_TODO("Animated not supported,fallback to scroll immediately");

    handleScroll(scrollOffset);
    return;

  } else {
    RNS_LOG_TODO("handleCommand commandName[" << commandName << "] args size[" << args.size() << "]");
  }
}


// RSkSpatialNavigatorContainer functions

bool RSkComponentScrollView::canScrollInDirection(rnsKey direction){
  /* scrolling checks
   * 1: If scrolling not enabled,no
   * 2: If cannot scroll in the direction provided based on vertical/horizontal scroll,no
   * 3: If content is less than frame, no
   * 4: If region available for scrolling next,yes
   */
  if(!scrollEnabled_) return false;

  RnsShell::ScrollLayer* scrollLayer= SCROLL_LAYER_HANDLE;
  SkISize contentSize = scrollLayer->getContentSize();
  SkIRect frameSize = scrollLayer->getFrame();

  if(isHorizontalScroll()){
    switch(direction){
      case RNS_KEY_Right:
         return (contentSize.width() - scrollLayer->scrollOffsetX) > frameSize.width();
      case RNS_KEY_Left:
         return (scrollLayer->scrollOffsetX != 0);
      default : return false;
    }
  }

  if(contentSize.height() < frameSize.height()) return false;

  switch(direction){
    case RNS_KEY_Down:
      return (contentSize.height() - scrollLayer->scrollOffsetY) > frameSize.height();
    case RNS_KEY_Up:
      return (scrollLayer->scrollOffsetY != 0);
    default : return false;
  }

  return false;
}

ScrollStatus RSkComponentScrollView::scrollInDirection(RSkComponent* candidate, rnsKey direction) {
  /* 1. Check if scrollable in direction
   * 2. If candidate is empty/self , scroll to next default offset
   * 3. If candidate is not self, scroll to candidate if falls in next scroll area
   * 4. Fallback to scroll to next default offset
   */
  if(!canScrollInDirection(direction)) return noScroll;
  if(snapToOffsets_.size()) return handleSnapToOffsetScroll(direction,candidate);

  SkPoint scrollPos = getNextScrollPosition(direction);
  if((candidate == nullptr) || (candidate == this))
    return handleScroll(scrollPos);

  if(isVisible(candidate)) return noScroll;

  RnsShell::ScrollLayer* scrollLayer= SCROLL_LAYER_HANDLE;
  SkIRect frame = scrollLayer->getFrame();
  SkIRect visibleRect = SkIRect::MakeXYWH(scrollPos.x(),scrollPos.y(),frame.width(),frame.height());

  SkIRect candidateFrame = candidate->getLayerAbsoluteFrame();
  if(!visibleRect.intersect(candidateFrame)) return handleScroll(scrollPos);

  return handleScroll(direction,candidateFrame);
}

bool RSkComponentScrollView::isVisible(RSkComponent* candidate) {
  /* 1. Get container visible frame {scrollOffset x,y,frame w,h}
   * 2. Get component abs frame
   * 3. if abs frame intersect with visible frame
   */
  RnsShell::ScrollLayer *scrollLayer = SCROLL_LAYER_HANDLE;

  SkIRect visibleRect = SkIRect::MakeXYWH(scrollLayer->scrollOffsetX,
                                          scrollLayer->scrollOffsetY,
                                          scrollLayer->getFrame().width(),
                                          scrollLayer->getFrame().height());

  return visibleRect.contains(candidate->getLayerAbsoluteFrame());
}

SkPoint RSkComponentScrollView::getScrollOffset() {
  if(isHorizontalScroll()) return SkPoint::Make(SCROLL_LAYER(scrollOffsetX),0);

  return SkPoint::Make(0,SCROLL_LAYER(scrollOffsetY));
}


bool RSkComponentScrollView::isHorizontalScroll() {
  return SCROLL_LAYER(getContentSize().width()) > SCROLL_LAYER(getFrame().width());
}

void RSkComponentScrollView::calculateNextScrollOffset(
   ScrollDirectionType scrollDirection,
   int contentLength,
   int viewLength,
   int &scrollOffset) {

   /* Default offset derived from experiments in Android TV emulator*/
   int defaultOffset = SkScalarRoundToInt(viewLength/2);

   if(scrollDirection == ScrollDirectionForward) {
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

}

SkPoint RSkComponentScrollView::getNextScrollPosition(rnsKey direction) {
  RnsShell::ScrollLayer* scrollLayer= SCROLL_LAYER_HANDLE;
  int scrollOffsetX = scrollLayer->scrollOffsetX;
  int scrollOffsetY = scrollLayer->scrollOffsetY;

  switch(direction) {
    case RNS_KEY_Right:
    case RNS_KEY_Left:
      calculateNextScrollOffset(direction==RNS_KEY_Right ? ScrollDirectionForward:ScrollDirectionBackward,
                                scrollLayer->getContentSize().width(),
                                scrollLayer->getFrame().width(),
                                scrollOffsetX);
      break;
    case RNS_KEY_Down:
    case RNS_KEY_Up:
      calculateNextScrollOffset(direction==RNS_KEY_Down ? ScrollDirectionForward:ScrollDirectionBackward,
                                scrollLayer->getContentSize().height(),
                                scrollLayer->getFrame().height(),
                                scrollOffsetY);
      break;
    default: RNS_LOG_WARN("Invalid key :" << direction);
  }
  return SkPoint::Make(scrollOffsetX,scrollOffsetY);

}

ScrollStatus RSkComponentScrollView::handleSnapToOffsetScroll(rnsKey direction,RSkComponent* candidate) {
  ScrollStatus status = scrollToFocus;
  RnsShell::ScrollLayer* scrollLayer= SCROLL_LAYER_HANDLE;
  bool isHorizontal = isHorizontalScroll();
  int frameLength = isHorizontal ? scrollLayer->getFrame().width() : scrollLayer->getFrame().height();
  int contentLength = isHorizontal ? scrollLayer->getContentSize().width() : scrollLayer->getContentSize().height();
  int currentOffset = isHorizontal ? scrollLayer->scrollOffsetX : scrollLayer->scrollOffsetY;
  int prevOffset = 0;
  int nextOffset = currentOffset;

  std::vector<int>::iterator upper,lower;
  if((candidate == nullptr) || (candidate == this)) {
     upper = std::upper_bound(snapToOffsets_.begin(),snapToOffsets_.end(),currentOffset+frameLength);
     lower = std::lower_bound(snapToOffsets_.begin(),snapToOffsets_.end(),currentOffset);
     status = scrollOnly;
  } else {
     currentOffset = isHorizontal ? candidate->getLayerAbsoluteFrame().x() : candidate->getLayerAbsoluteFrame().y();
     upper = std::upper_bound(snapToOffsets_.begin(),snapToOffsets_.end(),currentOffset);
     lower = std::lower_bound(snapToOffsets_.begin(),snapToOffsets_.end(),currentOffset);
  }

  if(upper != snapToOffsets_.end()) nextOffset = *upper;
  if(lower != snapToOffsets_.begin()) prevOffset = *(lower-1);

  if((direction == RNS_KEY_Right) || (direction == RNS_KEY_Down)) {
     if(nextOffset == currentOffset) return noScroll;
     if((contentLength - nextOffset) < frameLength)
        nextOffset = contentLength-frameLength;
  }
  /* TODO */
  /* If candidate is null/this : smooth scroll to nextOffset/prevOffset */
  /* If candidate available : smooth scroll between next & prev Offset */
  SkPoint nextScrollPos = SkPoint::Make(scrollLayer->scrollOffsetX,scrollLayer->scrollOffsetY);
  switch(direction) {
     case RNS_KEY_Right:
        nextScrollPos.fX = nextOffset;
        break;
     case RNS_KEY_Left:
        nextScrollPos.fX = prevOffset;
        break;
     case RNS_KEY_Down:
        nextScrollPos.fY = nextOffset;
        break;
     case RNS_KEY_Up:
        nextScrollPos.fY = prevOffset;
        break;
    default: RNS_LOG_WARN("Invalid key :" << direction);
  }

  handleScroll(nextScrollPos);
  return status;

}

ScrollStatus RSkComponentScrollView::handleScroll(rnsKey direction,SkIRect candidateFrame) {
  RnsShell::ScrollLayer* scrollLayer= SCROLL_LAYER_HANDLE;
  SkIRect frame = scrollLayer->getFrame();
  SkPoint nextScrollPos = SkPoint::Make(scrollLayer->scrollOffsetX,scrollLayer->scrollOffsetY);

  switch(direction) {
     case RNS_KEY_Right:
        nextScrollPos.fX = candidateFrame.right()-frame.width();
        break;
     case RNS_KEY_Left:
        nextScrollPos.fX = candidateFrame.left();
        break;
     case RNS_KEY_Down:
        nextScrollPos.fY = candidateFrame.bottom()-frame.height();
        break;
     case RNS_KEY_Up:
        nextScrollPos.fY = candidateFrame.top();
        break;
     default: RNS_LOG_WARN("Invalid key :" << direction);
  }

  handleScroll(nextScrollPos);
  return scrollToFocus;
}

ScrollStatus RSkComponentScrollView::handleScroll(SkPoint scrollPos) {

  RnsShell::ScrollLayer* scrollLayer= SCROLL_LAYER_HANDLE;

  scrollLayer->client().notifyFlushBegin();

  scrollLayer->scrollOffsetX = scrollPos.x();
  scrollLayer->scrollOffsetY = scrollPos.y();

  scrollLayer->invalidate(LayerPaintInvalidate);
  scrollLayer->client().notifyFlushRequired();

  dispatchOnScrollEvent(scrollPos);
  return scrollOnly;
}

void RSkComponentScrollView::dispatchOnScrollEvent(SkPoint scrollPos) {

  RnsShell::ScrollLayer* scrollLayer= SCROLL_LAYER_HANDLE;

  ScrollViewMetrics scrollMetrics;

  scrollMetrics.contentSize = Size{scrollLayer->getContentSize().width(),scrollLayer->getContentSize().height()};
  scrollMetrics.contentOffset = Point{scrollPos.x(),scrollPos.y()};
  scrollMetrics.containerSize = Size{scrollLayer->getFrame().width(),scrollLayer->getFrame().height()};
  scrollMetrics.zoomScale = SCROLLVIEW_DEFAULT_ZOOMSCALE;
  /* TODO Need to send contentInset value when contentInset prop is handled*/
  //scrollMetrics.contentInset = contentInset_;

  std::static_pointer_cast<ScrollViewEventEmitter const>(getComponentData().eventEmitter)->onScroll(scrollMetrics);
}

} // namespace react
} // namespace facebook
