/*
* Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
*
* This source code is licensed under the MIT license found in the
* LICENSE file in the root directory of this source tree.
*/

#include "third_party/skia/include/core/SkCanvas.h"
#include "third_party/skia/include/core/SkPictureRecorder.h"

#include "compositor/Compositor.h"
#include "compositor/layers/ScrollLayer.h"

namespace RnsShell {

#if ENABLE(FEATURE_SCROLL_INDICATOR)

#define SCROLLBAR_THICKNESS  (5)
#define SCROLLBAR_MIN_LENGTH (20)

ScrollLayer::ScrollBar::ScrollBar() {
    barColor_.setColor(SK_ColorBLACK);
    barFrame_ = SkIRect::MakeEmpty();
    barFrameInsets_ = SkIRect::MakeEmpty();
}

void ScrollLayer::ScrollBar::showScrollBar(bool show) {
    // If show is true: set mask to paint
    // If show is false :
    //    case 1: if previously bar was shown, so we need to clear it by adding bar frame to dirty rect - update mask to remove.
    //    case 2: previously bar was not shown, so nothing to be done - update mask to none.
    if(show) mask_ = LayerPaintInvalidate;
    else if((!show) && (mask_ == LayerPaintInvalidate)) mask_ = LayerRemoveInvalidate;
    else mask_ = LayerInvalidateNone;

    RNS_LOG_DEBUG("["<< this << "] showScrollBar :" << show << " mask_:" << mask_);
    return;
}

void ScrollLayer::ScrollBar::setScrollBarOffset(SkPoint scrollPos) {
    if(scrollPos_ == scrollPos) return;

    scrollPos_ = scrollPos;
    updateBarOffset();
}

void ScrollLayer::ScrollBar::setScrollBarColor(SkColor color) {
    barColor_.setColor(color);
}

void ScrollLayer::ScrollBar::setScrollBarPosition(ScrollBarPosition position) {
    if(barPosition_ == position) return;

    barPosition_ = position;
    RNS_LOG_DEBUG("["<< this << "] Set Bar alignment :" << barPosition_);
    calculateBarLayoutMetrics();
}

void ScrollLayer::ScrollBar::setScrollBarInsets(SkIRect frameInsets) {
    if(barFrameInsets_ ==  frameInsets)  return;

    barFrameInsets_ = frameInsets;

    RNS_LOG_DEBUG("["<< this << "] Set Bar frame insets LTRB[" << barFrameInsets_.left() << "," << barFrameInsets_.top() << ","  << barFrameInsets_.right() << "," << barFrameInsets_.bottom() << "]");
    calculateBarLayoutMetrics();
}

void ScrollLayer::ScrollBar::updateScrollLayerLayout(SkISize scrollContentSize, SkIRect scrollFrame) {
    if((scrollContentSize_ == scrollContentSize) && (scrollFrame_ == scrollFrame)) return;

    scrollContentSize_ = scrollContentSize;
    scrollFrame_ = scrollFrame;
    RNS_LOG_DEBUG("[" <<  this << "] updateScrollLayout Scroll ContentSize WH[" << scrollContentSize_.width() << "," << scrollContentSize_.height()
                      << "] Scroll frame XYWH[" << scrollFrame_.x() << "," << scrollFrame_.y() << "," << scrollFrame_.width() << "," << scrollFrame_.height() << "]");
    calculateBarLayoutMetrics();
}

SkIRect ScrollLayer::ScrollBar::getScrollBarAbsFrame(SkIRect scrollAbsFrame,LayerInvalidateMask layerMask) {
    // If scroll bar needs invalidate(paint/remove),we provide valid abs frame else empty
    if (mask_ != LayerInvalidateNone) {
        // If layer did not update, then dont need to update scrollbar as well
        if((layerMask == LayerInvalidateNone) && (mask_ == LayerPaintInvalidate)) {
            return SkIRect::MakeEmpty();
        }
        SkIRect barAbsFrame = barFrame_;
        barAbsFrame.offset(scrollAbsFrame.x() + barOffsetTranslate_.x(),scrollAbsFrame.y() + barOffsetTranslate_.y());
        RNS_LOG_DEBUG("["<< this << "] Bar Abs frame XYWH[" << barAbsFrame.x() << "," << barAbsFrame.y() << "," << barAbsFrame.width() << "," << barAbsFrame.height() << "]");

        //We set the mask to none,so we dont process dirty rect again once bar is removed
        if(mask_ == LayerRemoveInvalidate) showScrollBar(false);
        return barAbsFrame;
    }
    return SkIRect::MakeEmpty();
}

void ScrollLayer::ScrollBar::paint(SkCanvas *canvas) {
    // draw scroll bar only when paint mask is set
    if(mask_ == LayerPaintInvalidate) {
        //If bar frame does not fit in scroll frame,skip drawing scroll bar
        if((barFrame_.width() > scrollFrame_.width()) || (barFrame_.height() > scrollFrame_.height())) {
            RNS_LOG_ERROR("Scroll bar size["<< barFrame_.width() <<"," << barFrame_.height() <<"] exceeds scroll layer size[" << scrollFrame_.width() << "," << scrollFrame_.height() <<"]");
            return;
        }

        //Bar area can go beyond scroll frame when bar insets are set with negative value
        //So,we Clip scroll frame area here, to ensure scroll bar does not draw outside scroll frame
        //NOTE:When handling opacity for scrollbar,clipRect may not be required, as saveLayerAlpha will consider frame area only
        SkAutoCanvasRestore save(canvas, true);
        canvas->clipRect(SkRect::Make(scrollFrame_));
        canvas->translate(scrollFrame_.x()+barOffsetTranslate_.x(), scrollFrame_.y()+barOffsetTranslate_.y());
        canvas->drawRect(SkRect::Make(barFrame_),barColor_);
    }
}

void ScrollLayer::ScrollBar::calculateBarLayoutMetrics() {
    int barScrollAreaLength = 0.0;
    int scrollBarLength = SCROLLBAR_MIN_LENGTH;

    RNS_LOG_DEBUG("[" <<  this << "] calculateBarLayoutMetrics \n Scroll ContentSize WH[" << scrollContentSize_.width() << "," << scrollContentSize_.height()
                  << "]\n Scroll frame XYWH[" << scrollFrame_.x() << "," <<  scrollFrame_.y() << "," << scrollFrame_.width() << "," << scrollFrame_.height()
                  << "]\n Bar frame insets LTRB[" << barFrameInsets_.left() << "," << barFrameInsets_.top() << ","  << barFrameInsets_.right() << "," << barFrameInsets_.bottom()
                  << "]\n Bar alignment :" << barPosition_);

    //Calculation of bar layout
    // bar thickness  = default value = 5
    // bar scrollable area = frame length - bar insets based on alignment
    // bar length = max of 20 or (bar scrollable area * frame length/content length)
    // bar frame = starting insets , bar length , bar thickness
    // bar offset multiplier = (bar scrollable area - bar length) / (content length - frame length)
    // bar translate position = scroll offset * bar multiplier , frame length - bar thickness

    switch(barPosition_) {
        case ScrollBarPositionTop:
        case ScrollBarPositionBottom:
         {
            if(scrollContentSize_.width() <= scrollFrame_.width()) {
                barFrame_.setEmpty();
                barOffsetMultiplier_.set(0,0);
                barOffsetTranslate_.set(0,0);
                return;
            }
            barScrollAreaLength = scrollFrame_.width() - barFrameInsets_.left() - barFrameInsets_.right();
            // No bar scrollable area available,so set scroll bar position in middle with min scrollbar length
            if(barScrollAreaLength <= SCROLLBAR_MIN_LENGTH) {
                barOffsetMultiplier_.set(0,1);
                barFrame_.setXYWH((scrollFrame_.width()-SCROLLBAR_MIN_LENGTH)/2,0,scrollBarLength,SCROLLBAR_THICKNESS);
            } else {
                //Calculate scroll bar length and offset multiplier
                SkScalar widthFactor = barScrollAreaLength/SkIntToScalar(scrollContentSize_.width());
                scrollBarLength = std::max(SCROLLBAR_MIN_LENGTH,SkScalarRoundToInt(widthFactor*scrollFrame_.width()));

                SkScalar offsetMultiplier = SkIntToScalar(barScrollAreaLength-scrollBarLength)/SkIntToScalar(scrollContentSize_.width() - scrollFrame_.width());
                barOffsetMultiplier_.set(offsetMultiplier,1);

                barFrame_.setXYWH(barFrameInsets_.left(),0,scrollBarLength,SCROLLBAR_THICKNESS);
            }

            if(barPosition_ == ScrollBarPositionTop) barFrame_.offset(0,barFrameInsets_.top());
            else {
                barFrame_.offset(0,-barFrameInsets_.bottom());
                //Update bar translate position
                barOffsetTranslate_.fY = scrollFrame_.height() - SCROLLBAR_THICKNESS;
            }

         }
         break;
        case ScrollBarPositionLeft:
        case ScrollBarPositionRight:
        default:
         {
            if(scrollContentSize_.height() <= scrollFrame_.height()) {
                barFrame_.setEmpty();
                barOffsetMultiplier_.set(0,0);
                barOffsetTranslate_.set(0,0);
                return;
            }
            barScrollAreaLength = scrollFrame_.height() - barFrameInsets_.top() - barFrameInsets_.bottom();
            // No bar scrollable area available,so set scroll bar position in middle with min scrollbar length
            if(barScrollAreaLength <= SCROLLBAR_MIN_LENGTH) {
                barOffsetMultiplier_.set(1,0);
                barFrame_.setXYWH(0,(scrollFrame_.height()-SCROLLBAR_MIN_LENGTH)/2,SCROLLBAR_THICKNESS,scrollBarLength);
            } else {
                //Calculate scroll bar length and offset multiplier
                SkScalar heightFactor = barScrollAreaLength/SkIntToScalar(scrollContentSize_.height());
                scrollBarLength = std::max(SCROLLBAR_MIN_LENGTH,SkScalarRoundToInt(heightFactor*scrollFrame_.height()));

                SkScalar offsetMultiplier = SkIntToScalar(barScrollAreaLength-scrollBarLength)/SkIntToScalar(scrollContentSize_.height() - scrollFrame_.height());
                barOffsetMultiplier_.set(1,offsetMultiplier);

                barFrame_.setXYWH(0,barFrameInsets_.top(),SCROLLBAR_THICKNESS,scrollBarLength);
            }

            if(barPosition_ == ScrollBarPositionLeft) barFrame_.offset(barFrameInsets_.left(),0);
            else {
                barFrame_.offset(-barFrameInsets_.right(),0);
                //Update bar translate position
                barOffsetTranslate_.fX = scrollFrame_.width() - SCROLLBAR_THICKNESS;
            }

         }
         break;
    }
    updateBarOffset();

    RNS_LOG_DEBUG("["<< this << "] Bar frame XYWH[" << barFrame_.x() << "," << barFrame_.y() << "," << barFrame_.width() << "," << barFrame_.height() << "]");
    RNS_LOG_DEBUG("["<< this << "] Bar Position multiplier XY[" << barOffsetMultiplier_.x() << "," << barOffsetMultiplier_.y() << "]");
}

void ScrollLayer::ScrollBar::updateBarOffset() {
    switch(barPosition_) {
        case ScrollBarPositionTop:
        case ScrollBarPositionBottom:
             barOffsetTranslate_.fX = SkScalarRoundToInt(scrollPos_.x() * barOffsetMultiplier_.x());
             break;
        case ScrollBarPositionLeft:
        case ScrollBarPositionRight:
             barOffsetTranslate_.fY = SkScalarRoundToInt(scrollPos_.y() * barOffsetMultiplier_.y());
             break;
    }
    RNS_LOG_DEBUG("["<< this << "] ScrollBar scrollOffset XY[" << scrollPos_.x() << "," << scrollPos_.y() << "] translateOffset XY[" << barOffsetTranslate_.x() << "," << barOffsetTranslate_.y() << "]");
}

#endif // ENABLE_FEATURE_SCROLL_INDICATOR

SharedScrollLayer ScrollLayer::Create(Client& layerClient) {
    return std::make_shared<ScrollLayer>(layerClient);
}

ScrollLayer::ScrollLayer(Client& layerClient)
    : INHERITED(layerClient,LAYER_TYPE_SCROLL) {
#if USE(SCROLL_LAYER_BITMAP)
    drawDestRect_.setEmpty();
    drawSrcRect_.setEmpty();
#endif
    RNS_LOG_DEBUG("Scroll Layer Constructed(" << this << ") with ID : " << layerId());
}

bool ScrollLayer::setContentSize(SkISize contentSize) {
    /* If contentSize has changed, reset bitmap to reconfigure*/
    if(contentSize_ != contentSize) {
      contentSize_ = contentSize;
#if USE(SCROLL_LAYER_BITMAP)
      forceBitmapReset_ = true;
#endif
#if ENABLE(FEATURE_SCROLL_INDICATOR)
       scrollbar_.updateScrollLayerLayout(contentSize_,getFrame());
#endif
       return true;
    }
    return false;
}

void ScrollLayer::setScrollPosition(SkPoint scrollPos) {
    scrollOffsetX_ = scrollPos.x();
    scrollOffsetY_ = scrollPos.y();
    RNS_LOG_DEBUG("Scroll Layer (" << layerId_ << ") Set ScrollOffset :" << scrollOffsetX_ << "," << scrollOffsetY_);
#if ENABLE(FEATURE_SCROLL_INDICATOR)
    scrollbar_.setScrollBarOffset(scrollPos);
#endif
}

#if USE(SCROLL_LAYER_BITMAP)
void ScrollLayer::bitmapConfigure() {
    if(forceBitmapReset_) {
       scrollBitmap_.reset();
    }
    if(scrollBitmap_.drawsNothing()) {
       /* contentSize vs frame size,whichever is max is used for bitmap size allocation*/
       SkISize bitmapSize;
       bitmapSize.fWidth = std::max(contentSize_.width(),frame_.width());
       bitmapSize.fHeight = std::max(contentSize_.height(),frame_.height());
       scrollBitmap_.allocN32Pixels(bitmapSize.width(),bitmapSize.height());

       /* Apply the background color in bitmap */
       scrollBitmap_.eraseColor(backgroundColor);
       scrollCanvas_ = std::make_unique<SkCanvas>(scrollBitmap_);
    }
}
#endif

void ScrollLayer::prePaint(PaintContext& context, bool forceLayout) {
    //Adjust absolute Layout frame and dirty rects
    bool forceChildrenLayout = (forceLayout || (invalidateMask_ & LayerLayoutInvalidate));

#if !defined(GOOGLE_STRIP_LOG) || (GOOGLE_STRIP_LOG <= INFO)
    RNS_LOG_TRACE("Scroll Layer (" << layerId_ << ") Parent damagelist before(" << forceChildrenLayout << ") ============");
    for(auto &rect : context.damageRect)
        RNS_LOG_TRACE("[" << rect.x() << "," <<  rect.y() << "," << rect.width() << "," << rect.height() << "]");
    RNS_LOG_TRACE("====================================");
#endif

    preRoll(context, forceLayout);

    PaintContext bitmapPaintContext = {
            nullptr,  // canvas
            bitmapSurfaceDamage_, // damage rects
#if USE(RNS_SHELL_PARTIAL_UPDATES)
            true, // partialupdate support is required for bitmap
#endif
            SkRect::MakeEmpty(), // combined clip bounds from surfaceDamage
            nullptr, // GrDirectContext
            {0,0} // bitmapContext has no scroll offset,so set to zero
    };

#if USE(SCROLL_LAYER_BITMAP)
    bitmapConfigure();
    scrollCanvas_->save();
#if USE(RNS_SHELL_PARTIAL_UPDATES)
    //If bitmapReset,we have to draw all childrens.So add bitmap Rect as damageRect
    if(forceBitmapReset_) {
        addDamageRect(bitmapPaintContext,{0,0,contentSize_.width(),contentSize_.height()});
    }
#endif
#endif
    /* Prepaint child recursively and then paint self */
    /* Child for prepaint is selected based on below condition */
    /* 1. If bitmap is reset due to content size change,force child to prepaint */
    /* 2. If child frame intersects with visible region and requires an update */
    size_t index = 0;
    SkIRect dummy;
    SkIRect visibleRect = SkIRect::MakeXYWH(scrollOffsetX_,scrollOffsetY_,frame_.width(),frame_.height());
    std::map <size_t , SharedLayer > recycleChildList;
    for (auto& layer : children()) {
        /*avoid adding parent abs frame to child frame*/
        /*As childrens of this layer are painted on bitmap canvas,we do not need parent frame here*/
        layer->setSkipParentMatrix(true);

        RNS_LOG_DEBUG("Layer needs prePaint [" << layer->getBounds().x() <<"," << layer->getBounds().y() << "," << layer->getBounds().width() <<"," << layer->getBounds().height() << "]");
        layer->prePaint(bitmapPaintContext,forceChildrenLayout);
        if(layer->invalidateMask_ & LayerRemoveInvalidate) {
           recycleChildList[index] = layer;
        }

        index++;
    }

    //remove children marked with remove mask from parent list
    for(auto recycleChildIter = recycleChildList.rbegin();recycleChildIter != recycleChildList.rend();++recycleChildIter)
        removeChild(recycleChildIter->second.get(),recycleChildIter->first);


#if ENABLE(FEATURE_SCROLL_INDICATOR)
    //Update scroll bar with scroll layer layout info
    if((invalidateMask_ & LayerLayoutInvalidate) == LayerLayoutInvalidate) scrollbar_.updateScrollLayerLayout(contentSize_,getFrame());

#if USE(RNS_SHELL_PARTIAL_UPDATES)
    //if scrollbar needs update(paint/remove),add its absframe to damage rect list
    if(context.supportPartialUpdate) {
        SkIRect scrollBarFrame = scrollbar_.getScrollBarAbsFrame(absoluteFrame(),invalidateMask_);
        if(scrollBarFrame != SkIRect::MakeEmpty()) addDamageRect(context,scrollBarFrame);
    }
#endif
#endif//ENABLE_FEATURE_SCROLL_INDICATOR


#if USE(RNS_SHELL_PARTIAL_UPDATES)
    // If self has update, we anyways update the whole frame
    // So only if self does not have update, check if any children update is available and update damage rect list accordingly
    if((context.supportPartialUpdate) && (invalidateMask_ == LayerInvalidateNone)) {
       RNS_LOG_TRACE("Scroll Layer (" << layerId_ << ") damageRect list size:" << bitmapSurfaceDamage_.size());
       //Calculate the screen frame for the child dirty frame and add intersected area to parent damageRect list
       for(auto &rect : bitmapSurfaceDamage_) {
           SkIRect screenDirtyRect = rect.makeOffset(-scrollOffsetX_,-scrollOffsetY_).makeOffset(absFrame_.x(),absFrame_.y());
           RNS_LOG_TRACE("Scroll Layer (" << layerId_ << ") damage rect [" << rect.x() << "," << rect.y() << "," << rect.width() << "," << rect.height()
                            << "] absFrame point [" << absFrame_.x() << "," << absFrame_.y()
                            << "] screenDirtyRect [" << screenDirtyRect.x() << "," << screenDirtyRect.y() << "," << screenDirtyRect.width()
                            << "," << screenDirtyRect.height() << "]");

           if(screenDirtyRect.intersect(absFrame_)) {
               addDamageRect(context,screenDirtyRect);
#if USE(SCROLL_LAYER_BITMAP)
               // destRect = combined rect of all visible childrens screen rect(intersected area only)
               // srcRect = combined rect of all visible childrens absolute rect(intersected area only)
               drawDestRect_.join(screenDirtyRect);
               SkIRect srcDirtyRect = screenDirtyRect.makeOffset(-absFrame_.x(),-absFrame_.y()).makeOffset(scrollOffsetX_,scrollOffsetY_);
               drawSrcRect_.join(srcDirtyRect);
#endif
           }
       }
    }
#endif

#if !defined(GOOGLE_STRIP_LOG) || (GOOGLE_STRIP_LOG <= INFO)
    RNS_LOG_TRACE("Scroll Layer (" << layerId_ << ") Parent damagelist after ============");
    for(auto &rect : context.damageRect)
        RNS_LOG_TRACE("[" << rect.x() << "," <<  rect.y() << "," << rect.width() << "," << rect.height() << "]");
    RNS_LOG_TRACE("====================================");
#endif

    //Need to retain RemoveInvalidate mask,since parent requires it for removal from its list
    invalidateMask_ = static_cast<LayerInvalidateMask>(invalidateMask_ & LayerRemoveInvalidate);
    recycleChildList.clear();
#if USE(SCROLL_LAYER_BITMAP)
    forceBitmapReset_ = false;
#endif
}

inline void ScrollLayer::paintBorder(PaintContext& context) {
    if(borderPicture()) {
        RNS_LOG_DEBUG("Scroll Layer (" << layerId_ << ") SkPicture ( "  << borderPicture_ << " )For " <<
                borderPicture()->approximateOpCount() << " operations and size : " << borderPicture()->approximateBytesUsed());
        borderPicture()->playback(context.canvas);
    }
}

inline void ScrollLayer::paintScrollBar(PaintContext& context) {
#if ENABLE(FEATURE_SCROLL_INDICATOR)
    scrollbar_.paint(context.canvas);
#endif
}

void ScrollLayer::paintSelf(PaintContext& context) {
#if !defined(GOOGLE_STRIP_LOG) || (GOOGLE_STRIP_LOG <= INFO)
    RNS_GET_TIME_STAMP_US(start);
#endif
    /* Paint self algorithm */
    /*  1. Draw shadow using shadow picture playback */
    /*  2. Draw visible rect of bitmap (srcRect) to parent canvas frame (dstRect) */
    /*  3. Draw scroll bar (in bitmap mode)*/
    /*  4. Draw border using border picture playback (in bitmap mode)*/
    if(shadowPicture()) {
        RNS_LOG_DEBUG("SkPicture ( "  << shadowPicture_ << " )For " <<
                shadowPicture()->approximateOpCount() << " operations and size : " << shadowPicture()->approximateBytesUsed());
        shadowPicture()->playback(context.canvas);
    }

#if USE(SCROLL_LAYER_BITMAP)
    if(drawDestRect_.isEmpty() || drawSrcRect_.isEmpty()) {
       drawDestRect_ = frame_;
       drawSrcRect_.setXYWH(scrollOffsetX_,scrollOffsetY_,frame_.width(),frame_.height());
    }

    RNS_LOG_TRACE("Scroll Layer (" << layerId_ << ") Draw Image Rect src[" << drawSrcRect_.x() << "," << drawSrcRect_.y() << "," <<  drawSrcRect_.width() << "," << drawSrcRect_.height()
                           << "] dst[" << drawDestRect_.x() << "," << drawDestRect_.y() << "," <<  drawDestRect_.width() << "," << drawDestRect_.height() << "]");

    SkBitmap scrollBitmapSubset;
    scrollBitmap_.extractSubset(&scrollBitmapSubset,drawSrcRect_);
    context.canvas->drawImageRect(SkImage::MakeFromBitmap(scrollBitmapSubset),SkRect::Make(drawDestRect_),NULL);

    paintScrollBar(context);
    paintBorder(context);

#else

    // We will draw only frame rect here, scrollbar and border will be drawn after childrens are drawn
    if(backgroundColor != SK_ColorTRANSPARENT) {
      SkPaint paint;
      paint.setColor(backgroundColor);
      context.canvas->drawRect(SkRect::Make(frame_),paint);
    }

#endif

#if !defined(GOOGLE_STRIP_LOG) || (GOOGLE_STRIP_LOG <= INFO)
    RNS_GET_TIME_STAMP_US(end);
    RNS_LOG_TRACE("Scroll Layer (" << layerId_ << ") took " <<  (end - start) << " us to paint self");
#endif
}

void ScrollLayer::paintSelfAndChildren(PaintContext& context) {
#if !defined(GOOGLE_STRIP_LOG) || (GOOGLE_STRIP_LOG <= INFO)
    RNS_GET_TIME_STAMP_US(start);
#endif
    //Paint sequence
    //1. Paint self on parent's canvas(without scrollbar and border)
    //2. Clip frame rect to ensure children do not draw outside area
    //3. Update self scrollOffset with parent's scroll offset in paint context
    //4. Paint children on parent's canvas
    //5. Paint scroll bar on parent's canvas
    //6. Paint border on parent's canvas
    //7. Revert back updated scroll offset in paint context

    paintSelf(context);

    context.canvas->clipRect(SkRect::Make(frame_));

    SkPoint parentScrollOffset = context.offset;
    context.offset.offset(absFrame_.x()-scrollOffsetX_, absFrame_.y()-scrollOffsetY_);

    paintChildren(context);

    paintScrollBar(context);
    paintBorder(context);

    context.offset = parentScrollOffset;

#if !defined(GOOGLE_STRIP_LOG) || (GOOGLE_STRIP_LOG <= INFO)
    RNS_GET_TIME_STAMP_US(end);
    RNS_LOG_TRACE("Scroll Layer (" << layerId_ << ") took " <<  (end - start) << " us to paint selfAndChildren");
#endif
}

#if USE(SCROLL_LAYER_BITMAP)
void ScrollLayer::paintChildrenAndSelf(PaintContext& context) {
#if !defined(GOOGLE_STRIP_LOG) || (GOOGLE_STRIP_LOG <= INFO)
    RNS_GET_TIME_STAMP_US(start);
#endif
    //Paint sequence
    //1. Create paint context for drawing on bitmap
    //2. Clip path on bitmap based on damageRects on bitmap
    //3. Draw background color
    //4. Paint children on bitmap
    //5. Paint self on parent's canvas

    PaintContext bitmapPaintContext = {
            scrollCanvas_.get(),  // canvas
            bitmapSurfaceDamage_, // damage rects
#if USE(RNS_SHELL_PARTIAL_UPDATES)
            true, // partialupdate support is required for bitmap
#endif
            clipBound_, // combined clip bounds from surfaceDamage
            nullptr, // GrDirectContext
            {0,0}
    };

    // Using default clipPath has internal limit of ~16k.
    // Since bitmap size can be more than 16k,we will use clipRegion for clipping
    clipBound_ = Compositor::beginClip(bitmapPaintContext,true);
    if(bitmapSurfaceDamage_.size() != 0) {
       /* Clear clipped area with background color before painting the children*/
       scrollCanvas_->clear(backgroundColor);
    }

    paintChildren(bitmapPaintContext);

    paintSelf(context);

#if !defined(GOOGLE_STRIP_LOG) || (GOOGLE_STRIP_LOG <= INFO)
    RNS_GET_TIME_STAMP_US(end);
    RNS_LOG_TRACE("Scroll Layer (" << layerId_ << ") took " <<  (end - start) << " us to paint ChildrenAndSelf");
#endif

    scrollCanvas_->restore();
    clipBound_ = SkRect::MakeEmpty();
    drawSrcRect_.setEmpty();
    drawDestRect_.setEmpty();
}
#endif

void ScrollLayer::paint(PaintContext& context) {
    RNS_LOG_TRACE("Scroll Layer (" << layerId() << ") has " << children().size() << " childrens");
    SkAutoCanvasRestore save(context.canvas, true); // Save current clip and matrix state

    applyLayerTransformMatrix(context);

    if(opacity <= 0.0) return; //if transparent,paint self & children not required

    applyLayerOpacity(context);

#if USE(SCROLL_LAYER_BITMAP)
    paintChildrenAndSelf(context);
#else
    paintSelfAndChildren(context);
#endif

    bitmapSurfaceDamage_.clear();
}
}   // namespace RnsShell
