/*
* Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
*
* This source code is licensed under the MIT license found in the
* LICENSE file in the root directory of this source tree.
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
    : RSkComponent(shadowView, LAYER_TYPE_SCROLL) {
}

RSkComponentScrollView::~RSkComponentScrollView() {
#if ENABLE(FEATURE_SCROLL_INDICATOR)
  if(scrollbarTimer_ != nullptr) {
    delete scrollbarTimer_;
    scrollbarTimer_ = nullptr;
  }
#endif //ENABLE_FEATURE_SCROLL_INDICATOR
}

void RSkComponentScrollView::OnPaint(SkCanvas *canvas) {}

#if ENABLE(FEATURE_SCROLL_INDICATOR)
void RSkComponentScrollView::fadeOutScrollBar() {
  //TODO Hiding the scroll indicator to be achieved
  // using fade-out animation : when native animation support is added
  // in RAF notification instead of handleScroll call : when rns-shell add support for RAF

  auto hideScrollBar = [&]() {
     RnsShell::ScrollLayer* scrollLayer= SCROLL_LAYER_HANDLE;
     scrollLayer->client().notifyFlushBegin();
     scrollLayer->getScrollBar().showScrollBar(false);
     scrollLayer->client().notifyFlushRequired();
  };

  if(scrollbarTimer_ == nullptr)
    scrollbarTimer_ = new Timer(SCROLLBAR_FADEOUT_TIME,false,hideScrollBar,true);
  else
    scrollbarTimer_->reschedule(SCROLLBAR_FADEOUT_TIME,false);
}
#endif //ENABLE_FEATURE_SCROLL_INDICATOR

RnsShell::LayerInvalidateMask RSkComponentScrollView::updateComponentProps(
    SharedProps newviewProps,
    bool forceUpdate) {

  auto const &newScrollViewProps = *std::static_pointer_cast<ScrollViewProps const>(newviewProps);
  auto const &oldScrollViewProps = *std::static_pointer_cast<ScrollViewProps const>(getComponentData().props);
  RnsShell::LayerInvalidateMask updateMask=RnsShell::LayerInvalidateNone;

  //Update scroll view props
  scrollEnabled_ = newScrollViewProps.scrollEnabled;
  pagingEnabled_ = newScrollViewProps.pagingEnabled;
  snapToOffsets_.clear();
  for(auto &offset : newScrollViewProps.snapToOffsets) {
    snapToOffsets_.push_back(static_cast<int>(offset));
  }
  sort(snapToOffsets_.begin(),snapToOffsets_.end());

  if(forceUpdate || (oldScrollViewProps.contentOffset != newScrollViewProps.contentOffset)) {
    contentOffset_.set(newScrollViewProps.contentOffset.x , newScrollViewProps.contentOffset.y);
    updateScrollOffset(newScrollViewProps.contentOffset.x, newScrollViewProps.contentOffset.y);
    updateMask = static_cast<RnsShell::LayerInvalidateMask>(updateMask | RnsShell::LayerPaintInvalidate);
  }

#if ENABLE(FEATURE_SCROLL_INDICATOR)
  RnsShell::ScrollLayer* scrollLayer= SCROLL_LAYER_HANDLE;

  //Update scroll indicator props
  //Note : we do not set the scroll layer mask for the indicator
  RnsShell::ScrollLayer::ScrollBar &scrollbar = scrollLayer->getScrollBar();
  if(forceUpdate || (oldScrollViewProps.indicatorStyle != newScrollViewProps.indicatorStyle)) {
    SkColor scrollIndicatorColor = SK_ColorBLACK;
    if(newScrollViewProps.indicatorStyle == ScrollViewIndicatorStyle::White) scrollIndicatorColor = SK_ColorWHITE;
    scrollbar.setScrollBarColor(scrollIndicatorColor);
  }

  if(forceUpdate || (oldScrollViewProps.scrollIndicatorInsets != newScrollViewProps.scrollIndicatorInsets)) {
    EdgeInsets indicatorInsets = newScrollViewProps.scrollIndicatorInsets;
    RNS_LOG_DEBUG("IndicatorInsets:" << indicatorInsets.left << "," << indicatorInsets.top << "," <<  indicatorInsets.right << "," << indicatorInsets.bottom);
    scrollbar.setScrollBarInsets(SkIRect::MakeLTRB(
                                    SkScalarRoundToInt(indicatorInsets.left),
                                    SkScalarRoundToInt(indicatorInsets.top),
                                    SkScalarRoundToInt(indicatorInsets.right),
                                    SkScalarRoundToInt(indicatorInsets.bottom)));
  }

  showHorizontalScrollIndicator_ = newScrollViewProps.showsHorizontalScrollIndicator;
  showVerticalScrollIndicator_ = newScrollViewProps.showsVerticalScrollIndicator;

  drawScrollIndicator_ = false;

  //TODO persistentScrollIndicator prop not yet supported in ReactCommon,so we comment the reading of this prop here
#if 0
  persistentScrollIndicator_ = newScrollViewProps.persistentScrollIndicator;
#endif

  if(isHorizontalScroll_ && showHorizontalScrollIndicator_) drawScrollIndicator_ = true;
  if(!isHorizontalScroll_ && showVerticalScrollIndicator_) drawScrollIndicator_ = true;
  RNS_LOG_DEBUG("Show indicator :" << drawScrollIndicator_ << " Vertical:" << showVerticalScrollIndicator_ << " Horizontal:" << showHorizontalScrollIndicator_);
#endif //ENABLE_FEATURE_SCROLL_INDICATOR

  return updateMask;

}

RnsShell::LayerInvalidateMask RSkComponentScrollView::updateComponentState(
    const ShadowView &newShadowView,
    bool forceUpdate) {

  auto state = std::static_pointer_cast<ScrollViewShadowNode::ConcreteStateT const>(newShadowView.state);
  SkISize contentSize = RSkSkSizeFromSize(state->getData().getContentSize()).toRound();
  RnsShell::ScrollLayer* scrollLayer= SCROLL_LAYER_HANDLE;

  //ContentSize has changed?
  //NOTE:
  //We consider the contentSize provided by the ShadowView state object to handle scroll layer content size
  //When scroll view items are defined with absolute positions, the content size provided is not proper.
  //TODO : Needs handling to compute/determine content size when items are defined with absolute position
  if(scrollLayer->setContentSize(contentSize)) {
    SkIRect frame = scrollLayer->getFrame();
    SkPoint scrollPos;

    //Check scroll view type has changed? update dependent values accordingly
    bool isHorizontalScroll = contentSize.width() > frame.width();
    if(isHorizontalScroll != isHorizontalScroll_) {
      isHorizontalScroll_ = isHorizontalScroll;

      //Update the contentOffset when type has changed
      scrollPos = contentOffset_;

#if ENABLE(FEATURE_SCROLL_INDICATOR)
      drawScrollIndicator_ = false;
      if(isHorizontalScroll_ && showHorizontalScrollIndicator_) drawScrollIndicator_ = true;
      if(!isHorizontalScroll_ && showVerticalScrollIndicator_) drawScrollIndicator_ = true;
      scrollLayer->getScrollBar().setScrollBarPosition(isHorizontalScroll_?
                                                        RnsShell::ScrollLayer::ScrollBar::ScrollBarPositionBottom :
                                                        RnsShell::ScrollLayer::ScrollBar::ScrollBarPositionRight);
#endif //ENABLE_FEATURE_SCROLL_INDICATOR

    } else {
      //Check current scrollOffset requires change ?
      scrollPos = scrollLayer->getScrollPosition();
    }

    updateScrollOffset(scrollPos.x(),scrollPos.y());
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
    SkPoint lastScrollPos = SkPoint::Make(0,0);

    if(isHorizontalScroll_) {
      lastScrollPos.fX = contentSize.width() - frameRect.width();
    } else {
      if(contentSize.height() <= frameRect.height()) {
         RNS_LOG_DEBUG("No scrollable content to scroll");
         return;
      }
      lastScrollPos.fY = contentSize.height() - frameRect.height();
    }

    if(args[0].getBool()) RNS_LOG_TODO("Animated not supported,fallback to scroll immediately");
    handleScroll(lastScrollPos);
    return;

  } else if(commandName == "scrollTo") {
    if(args.size() != 3 ) {
      RNS_LOG_ERROR("Command scrollTo received " << args.size() << " arguments,expected 3");
      return;
    }

    RNS_LOG_DEBUG("handleCommand commandName[scrollTo] args[" << args[0] <<"," << args[1] << "," << args[2] <<"]");
    if(args[2].getBool()) RNS_LOG_TODO("Animated not supported,fallback to scroll immediately");

    handleScroll(static_cast<int>(args[0].getDouble()), static_cast<int>(args[1].getDouble()));
    return;

  } else if(commandName == "flashScrollIndicators") {

#if ENABLE(FEATURE_SCROLL_INDICATOR)
    RNS_LOG_DEBUG("handleCommand commandName[flashScrollIndicators]");
    if((!drawScrollIndicator_) || (persistentScrollIndicator_)) return;

    RnsShell::ScrollLayer* scrollLayer= SCROLL_LAYER_HANDLE;
    RnsShell::ScrollLayer::ScrollBar &scrollbar = scrollLayer->getScrollBar();

    scrollLayer->client().notifyFlushBegin();
    scrollbar.showScrollBar(true);
    scrollLayer->client().notifyFlushRequired();
    fadeOutScrollBar();
#else
    RNS_LOG_WARN("handleCommand commandName[flashScrollIndicators] feature disabled !!");
#endif //ENABLE_FEATURE_SCROLL_INDICATOR
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
  SkPoint scrollPos = scrollLayer->getScrollPosition();

  if(isHorizontalScroll_){
    switch(direction){
      case RNS_KEY_Right:
         return (contentSize.width() - scrollPos.fX) > frameSize.width();
      case RNS_KEY_Left:
         return (scrollPos.fX != 0);
      default : return false;
    }
  }

  if(contentSize.height() < frameSize.height()) return false;

  switch(direction){
    case RNS_KEY_Down:
      return (contentSize.height() - scrollPos.fY) > frameSize.height();
    case RNS_KEY_Up:
      return (scrollPos.fY != 0);
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
  if(!visibleRect.intersect(candidateFrame)) {
    return handleScroll(scrollPos);
  }

  // When we reach here, candidate frame is found to be available in next calculated offset
  // There are two cases,
  //    1. Paging enabled - scroll to next page and return scroll to focus
  //    2. Paging disabled - scroll to candidate frame and return scroll to focus
  pagingEnabled_ ? handleScroll(scrollPos) : handleScroll(direction,candidateFrame);
  return scrollToFocus;
}

ScrollStatus RSkComponentScrollView::scrollTo(RSkComponent* candidate) {
  if((candidate == nullptr) || (candidate == this) || (isVisible(candidate))) {
    return noScroll;
  }

  RnsShell::ScrollLayer *scrollLayer = SCROLL_LAYER_HANDLE;
  SkPoint currentScrollPos = scrollLayer->getScrollPosition();
  SkIRect candidateFrame = candidate->getLayerAbsoluteFrame();
  rnsKey direction;

  //Check if need to scroll in backward/forward direction based on scroll position
  if(isHorizontalScroll_) {
     direction = (candidateFrame.x() < currentScrollPos.x()) ? RNS_KEY_Left : RNS_KEY_Right;
  } else {
     direction = (candidateFrame.y() < currentScrollPos.y()) ? RNS_KEY_Up : RNS_KEY_Down;
  }
  return handleScroll(direction,candidateFrame);
}

bool RSkComponentScrollView::isVisible(RSkComponent* candidate) {
  /* 1. Get container visible frame {scrollOffset x,y,frame w,h}
   * 2. Get component abs frame
   * 3. if abs frame fully contained within visible frame
   */
  RnsShell::ScrollLayer *scrollLayer = SCROLL_LAYER_HANDLE;

  SkIRect visibleRect = SkIRect::MakeXYWH(scrollLayer->getScrollPosition().x(),
                                          scrollLayer->getScrollPosition().y(),
                                          scrollLayer->getFrame().width(),
                                          scrollLayer->getFrame().height());

  return visibleRect.contains(candidate->getLayerAbsoluteFrame());
}

SkPoint RSkComponentScrollView::getScrollOffset() {
  if(isHorizontalScroll_) return SkPoint::Make(SCROLL_LAYER(getScrollPosition().x()),0);

  return SkPoint::Make(0,SCROLL_LAYER(getScrollPosition().y()));
}

void RSkComponentScrollView::calculateNextScrollOffset(
   ScrollDirectionType scrollDirection,
   int contentLength,
   int viewLength,
   SkScalar &scrollOffset) {

   /* If pagingEnabled, use full view length to scroll next
    *             else, use half of view length similar to Android TV emulator*/
   int defaultOffset = pagingEnabled_ ? viewLength : SkScalarRoundToInt(viewLength/2);

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
  SkPoint scrollPos = scrollLayer->getScrollPosition();

  switch(direction) {
    case RNS_KEY_Right:
    case RNS_KEY_Left:
      calculateNextScrollOffset(direction==RNS_KEY_Right ? ScrollDirectionForward:ScrollDirectionBackward,
                                scrollLayer->getContentSize().width(),
                                scrollLayer->getFrame().width(),
                                scrollPos.fX);
      break;
    case RNS_KEY_Down:
    case RNS_KEY_Up:
      calculateNextScrollOffset(direction==RNS_KEY_Down ? ScrollDirectionForward:ScrollDirectionBackward,
                                scrollLayer->getContentSize().height(),
                                scrollLayer->getFrame().height(),
                                scrollPos.fY);
      break;
    default: RNS_LOG_WARN("Invalid key :" << direction);
  }
  return scrollPos;
}

inline void RSkComponentScrollView::updateScrollOffset(int x,int y) {
  //Update scroll offset without trigerring render
  handleScroll(x,y,false);
}

ScrollStatus RSkComponentScrollView::handleSnapToOffsetScroll(rnsKey direction,RSkComponent* candidate) {
  ScrollStatus status = scrollToFocus;
  RnsShell::ScrollLayer* scrollLayer= SCROLL_LAYER_HANDLE;
  int frameLength = isHorizontalScroll_ ? scrollLayer->getFrame().width() : scrollLayer->getFrame().height();
  int contentLength = isHorizontalScroll_ ? scrollLayer->getContentSize().width() : scrollLayer->getContentSize().height();
  SkPoint scrollPos = scrollLayer->getScrollPosition();
  SkPoint nextScrollPos = scrollPos;
  int currentOffset = isHorizontalScroll_ ? scrollPos.x() : scrollPos.y();
  int prevOffset = -1;
  int nextOffset = currentOffset;

  std::vector<int>::iterator upper,lower;
  if((candidate == nullptr) || (candidate == this)) {
     upper = std::upper_bound(snapToOffsets_.begin(),snapToOffsets_.end(),currentOffset+frameLength);
     lower = std::lower_bound(snapToOffsets_.begin(),snapToOffsets_.end(),currentOffset);
     status = scrollOnly;
  } else {
     currentOffset = isHorizontalScroll_ ? candidate->getLayerAbsoluteFrame().x() : candidate->getLayerAbsoluteFrame().y();
     upper = std::upper_bound(snapToOffsets_.begin(),snapToOffsets_.end(),currentOffset);
     lower = std::lower_bound(snapToOffsets_.begin(),snapToOffsets_.end(),currentOffset);
  }

  if(upper != snapToOffsets_.end()) {
    nextOffset = *upper;
  }
  if(lower != snapToOffsets_.begin()) {
    prevOffset = *(lower-1);
  }

  if((direction == RNS_KEY_Right) || (direction == RNS_KEY_Down)) {
    if(nextOffset == currentOffset) {
      return noScroll;
    }
    if((contentLength - nextOffset) < frameLength) {
      nextOffset = contentLength-frameLength;
    }
  } else if((direction == RNS_KEY_Left) || (direction == RNS_KEY_Up)) {
    if(prevOffset == -1) {
      return noScroll;
    }
  }

  /* TODO */
  /* If candidate is null/this : smooth scroll to nextOffset/prevOffset */
  /* If candidate available : smooth scroll between next & prev Offset */
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
     default:
        RNS_LOG_WARN("Invalid key :" << direction);
        return noScroll;
  }

  handleScroll(nextScrollPos);
  return status;

}

ScrollStatus RSkComponentScrollView::handleScroll(rnsKey direction,SkIRect candidateFrame) {
  RnsShell::ScrollLayer* scrollLayer= SCROLL_LAYER_HANDLE;
  SkIRect frame = scrollLayer->getFrame();
  SkPoint nextScrollPos = scrollLayer->getScrollPosition();

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

  handleScroll(nextScrollPos.fX,nextScrollPos.fY);
  return scrollToFocus;
}

ScrollStatus RSkComponentScrollView::handleScroll(int x, int y, bool isFlushDisplay) {
  RnsShell::ScrollLayer * scrollLayer = SCROLL_LAYER_HANDLE;
  SkISize contentSize = scrollLayer->getContentSize();
  SkIRect frameRect = scrollLayer->getFrame();
  SkPoint scrollPos = SkPoint::Make(0,0);

  if(isHorizontalScroll_) {
    scrollPos.fX = std::min(std::max(0,x),(contentSize.width()-frameRect.width()));
  } else {
    if(contentSize.height() <= frameRect.height()) scrollPos.fY = 0;
    else scrollPos.fY = std::min(std::max(0,y),(contentSize.height()-frameRect.height()));
  }

  return handleScroll(scrollPos,isFlushDisplay);
}

ScrollStatus RSkComponentScrollView::handleScroll(SkPoint scrollPos, bool isFlushDisplay) {

  RnsShell::ScrollLayer* scrollLayer= SCROLL_LAYER_HANDLE;
  if(scrollPos == scrollLayer->getScrollPosition()) return noScroll;

  if(isFlushDisplay) scrollLayer->client().notifyFlushBegin();

  scrollLayer->setScrollPosition(scrollPos);

#if ENABLE(FEATURE_SCROLL_INDICATOR)
  if(drawScrollIndicator_) scrollLayer->getScrollBar().showScrollBar(true);
#endif

  scrollLayer->invalidate(LayerPaintInvalidate);
  if(isFlushDisplay) scrollLayer->client().notifyFlushRequired();

  dispatchOnScrollEvent(scrollPos);

#if ENABLE(FEATURE_SCROLL_INDICATOR)
  if(drawScrollIndicator_ && (!persistentScrollIndicator_)) fadeOutScrollBar();
#endif
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
