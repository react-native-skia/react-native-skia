/*
* Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "third_party/skia/include/core/SkCanvas.h"

#include "compositor/Compositor.h"
#include "compositor/layers/ScrollLayer.h"

namespace RnsShell {

SharedScrollLayer ScrollLayer::Create(Client& layerClient) {
    return std::make_shared<ScrollLayer>(layerClient);
}

ScrollLayer::ScrollLayer(Client& layerClient)
    : INHERITED(layerClient,LAYER_TYPE_SCROLL) {
    RNS_LOG_DEBUG("Scroll Layer Constructed(" << this << ") with ID : " << layerId());
}

bool ScrollLayer::setContentSize(SkISize contentSize) {
    /* If contentSize has changed, reset bitmap to reconfigure*/
    if(contentSize_ != contentSize) {
       contentSize_ = contentSize;
       forceBitmapReset_ = true;
       return true;
    }
    return false;
}

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

void ScrollLayer::prePaint(PaintContext& context, bool forceLayout) {
    //Adjust absolute Layout frame and dirty rects
    bool forceChildrenLayout = (forceLayout || (invalidateMask_ & LayerLayoutInvalidate));

    bitmapConfigure();
    scrollCanvas_->save();

    PaintContext bitmapPaintContext = {
            scrollCanvas_.get(),  // canvas
            bitmapSurfaceDamage_, // damage rects
#if USE(RNS_SHELL_PARTIAL_UPDATES)
            true, // partialupdate support is required for bitmap
#endif
            clipBound_, // combined clip bounds from surfaceDamage
            nullptr, // GrDirectContext
    };

    /* Prepaint child recursively and then paint self */
    /* Child for prepaint is selected based on below condition */
    /* 1. If bitmap is reset due to content size change,force child to prepaint */
    /* 2. If child frame intersects with visible region and requires an update */
    size_t index = 0;
    SkIRect dummy;
    SkIRect visibleRect = SkIRect::MakeXYWH(scrollOffsetX,scrollOffsetY,frame_.width(),frame_.height());
    std::map <size_t , SharedLayer > recycleChildList;
    for (auto& layer : children()) {
        /*avoid adding parent abs frame to child frame*/
        /*As childrens of this layer are painted on bitmap canvas,we do not need parent frame here*/
        layer->setSkipParentMatrix(true);

        if(forceBitmapReset_){
           layer->invalidate();
        }

        /* Prepaint children in below conditions */
        /* 1. bitmap size is updated */
        /* 2. child has update and intersects with visible Area */
        /* 3. child bounds is not calculated (happens when new child is added runtime)*/
        if(forceBitmapReset_ || (dummy.intersect(visibleRect,layer->getBounds()) && layer->requireInvalidate(false)) || layer->getBounds().isEmpty()){
          RNS_LOG_DEBUG("Layer needs prePaint [" << layer->getBounds().x() <<"," << layer->getBounds().y() << "," << layer->getBounds().width() <<"," << layer->getBounds().height() << "]");
          layer->prePaint(bitmapPaintContext,forceChildrenLayout);
          if(layer->invalidateMask_ == LayerRemoveInvalidate) {
              recycleChildList[index] = layer;
          }
        }

        index++;
    }

    //remove children marked with remove mask from parent list
    for(auto recycleChildIter = recycleChildList.rbegin();recycleChildIter != recycleChildList.rend();++recycleChildIter)
       removeChild(recycleChildIter->second.get(),recycleChildIter->first);

    clipBound_ = Compositor::beginClip(bitmapPaintContext);

    /* If child requires paint,then self needs to paint,so set invalidate self*/
    if(bitmapSurfaceDamage_.size() != 0) {
       /* Clear area before paint */
       scrollCanvas_->clear(backgroundColor);
       invalidate(static_cast<LayerInvalidateMask>(invalidateMask_|LayerPaintInvalidate));
    }

    preRoll(context, forceLayout);
    invalidateMask_ = LayerInvalidateNone;
    forceBitmapReset_ = false;
    recycleChildList.clear();
}

void ScrollLayer::paintSelf(PaintContext& context) {
#if !defined(GOOGLE_STRIP_LOG) || (GOOGLE_STRIP_LOG <= INFO)
    RNS_GET_TIME_STAMP_US(start);
#endif
    /* Paint self algorithm */
    /*  1. Draw shadow using shadow picture playback */
    /*  2. Draw visible rect of bitmap (srcRect) to parent canvas frame (dstRect) */
    /*  3. Draw border using border picture playback */
    if(shadowPicture()) {
        RNS_LOG_DEBUG("SkPicture ( "  << shadowPicture_ << " )For " <<
                shadowPicture()->approximateOpCount() << " operations and size : " << shadowPicture()->approximateBytesUsed());
        shadowPicture()->playback(context.canvas);
    }

    RNS_LOG_INFO("Draw scroll bitmap offset X["<< scrollOffsetX << "] Y[" << scrollOffsetY << "]");
    SkRect srcRect = SkRect::MakeXYWH(scrollOffsetX,scrollOffsetY,frame_.width(),frame_.height());
    SkRect dstRect = SkRect::Make(frame_);
    context.canvas->drawBitmapRect(scrollBitmap_,srcRect,dstRect,NULL);

    if(borderPicture()) {
        RNS_LOG_DEBUG("SkPicture ( "  << borderPicture_ << " )For " <<
                borderPicture()->approximateOpCount() << " operations and size : " << borderPicture()->approximateBytesUsed());
        borderPicture()->playback(context.canvas);
    }

#if !defined(GOOGLE_STRIP_LOG) || (GOOGLE_STRIP_LOG <= INFO)
    RNS_GET_TIME_STAMP_US(end);
    RNS_LOG_TRACE("Layer (" << layerId_ << ") took " <<  (end - start) << " us to paint self");
#endif
}

void ScrollLayer::paint(PaintContext& context) {
    RNS_LOG_DEBUG("Scroll Layer (" << layerId() << ") has " << children().size() << " childrens");
    SkAutoCanvasRestore save(context.canvas, true); // Save current clip and matrix state

    if(opacity <= 0.0) return; //if transparent,paint self & children not required
    if(opacity < 0xFF) {
      SkRect layerBounds = SkRect::Make(bounds_);
      context.canvas->saveLayerAlpha(&layerBounds,opacity);
    }

    PaintContext bitmapPaintContext = {
            scrollCanvas_.get(),  // canvas
            bitmapSurfaceDamage_, // damage rects
#if USE(RNS_SHELL_PARTIAL_UPDATES)
            true, // partialupdate support is required for bitmap
#endif
            clipBound_, // combined clip bounds from surfaceDamage
            nullptr, // GrDirectContext
    };

    // First paint children and then self
    for (auto& layer : children()) {
        if(layer->needsPainting(bitmapPaintContext)) {
            RNS_LOG_DEBUG("Layer needs paint [" << layer->getBounds().x() <<"," << layer->getBounds().y() << "," << layer->getBounds().width() <<"," << layer->getBounds().height() << "]");
            layer->paint(bitmapPaintContext);
        }
    }

    context.canvas->setMatrix(absoluteTransformMatrix_);
    paintSelf(context);

    scrollCanvas_->restore();
    bitmapSurfaceDamage_.clear();
    clipBound_ = SkRect::MakeEmpty();

}
}   // namespace RnsShell
