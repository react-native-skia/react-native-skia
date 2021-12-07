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
            true, // partialupdate support is required for bitmap
            clipBound_, // combined clip bounds from surfaceDamage
            nullptr, // GrDirectContext
    };

    /* Prepaint child recursively and then paint self */
    /* Child for prepaint is selected based on below condition */
    /* 1. If bitmap is reset due to content size change,force child to prepaint */
    /* 2. If child frame intersects with visible region and requires an update */
    SkIRect visibleRect = SkIRect::MakeXYWH(scrollOffsetX,scrollOffsetY,frame_.width(),frame_.height());
    SkIRect dummy;
    for (auto& layer : children()) {
        /*setting parent to null is a hack to avoid adding parent abs frame to child frame*/
        /*As childrens of this layer are painted on bitmap canvas,we do not need parent frame here*/
        /*TODO Find proper place/better way to avoid it */
        layer->setParent(nullptr);

        if(forceBitmapReset_){
           layer->invalidate();
        }
        if(forceBitmapReset_ || (dummy.intersect(visibleRect,layer->absoluteFrame()) && layer->requireInvalidate())){
          RNS_LOG_DEBUG("Layer needs prePaint [" << layer->absoluteFrame().x() <<"," << layer->absoluteFrame().y() << "," << layer->absoluteFrame().width() <<"," << layer->absoluteFrame().height() << "]");
          layer->prePaint(bitmapPaintContext,forceChildrenLayout);
        }
    }

    clipBound_ = Compositor::beginClip(bitmapPaintContext);

    /* If child requires paint,then self needs to paint,so set invalidate self*/
    if(bitmapSurfaceDamage_.size() != 0) {
       invalidate(static_cast<LayerInvalidateMask>(invalidateMask_|LayerPaintInvalidate));
    }

    preRoll(context, forceLayout);
    invalidateMask_ = LayerInvalidateNone;
    forceBitmapReset_ = false;
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

    PaintContext bitmapPaintContext = {
            scrollCanvas_.get(),  // canvas
            bitmapSurfaceDamage_, // damage rects
            true, // partialupdate support is required for bitmap
            clipBound_, // combined clip bounds from surfaceDamage
            nullptr, // GrDirectContext
    };

    // First paint children and then self
    for (auto& layer : children()) {
        if(layer->needsPainting(bitmapPaintContext)) {
            RNS_LOG_DEBUG("Layer needs paint [" << layer->absoluteFrame().x() <<"," << layer->absoluteFrame().y() << "," << layer->absoluteFrame().width() <<"," << layer->absoluteFrame().height() << "]");
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
