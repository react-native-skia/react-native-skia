/*
* Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "compositor/layers/PictureLayer.h"
#include "third_party/skia/include/core/SkCanvas.h"

namespace RnsShell {

SharedPictureLayer PictureLayer::Create(Client& layerClient) {
    return std::make_shared<PictureLayer>(layerClient);
}

PictureLayer::PictureLayer(Client& layerClient)
    : INHERITED(layerClient, LAYER_TYPE_PICTURE) {
    RNS_LOG_INFO("Picture Layer Constructed(" << this << ") with ID : " << layerId() << " and LayerClient : " << &layerClient);
}

void PictureLayer::prePaint(PaintContext& context, bool forceLayout) {
    //Adjust absolute Layout frame and dirty rects
    bool forceChildrenLayout = (forceLayout || (invalidateMask_ & LayerLayoutInvalidate));
    preRoll(context, forceLayout);
    invalidateMask_ = LayerInvalidateNone;

    // prePaint children recursively
    for (auto& layer : children()) {
        layer->prePaint(context, forceChildrenLayout);
    }
}

void PictureLayer::paintSelf(PaintContext& context) {
#if !defined(GOOGLE_STRIP_LOG) || (GOOGLE_STRIP_LOG <= INFO)
    RNS_GET_TIME_STAMP_US(start);
#endif
    if(picture_.get()) {
        RNS_LOG_DEBUG("SkPicture ( "  << picture_ << " )For " <<
                picture_.get()->approximateOpCount() << " operations and size : " << picture_.get()->approximateBytesUsed());
        picture()->playback(context.canvas);
    }
#if !defined(GOOGLE_STRIP_LOG) || (GOOGLE_STRIP_LOG <= INFO)
    RNS_GET_TIME_STAMP_US(end);
    RNS_LOG_TRACE("PictureLayer (" << layerId() << ") took " <<  (end - start) << " us to paint self");
#endif
}

void PictureLayer::paint(PaintContext& context) {
    RNS_LOG_DEBUG("Picture Layer (" << layerId() << ") has " << children().size() << " childrens");
    SkAutoCanvasRestore save(context.canvas, true); // Save current clip and matrix state

    context.canvas->setMatrix(absoluteTransformMatrix_);

    if(opacity <= 0.0) return; //if transparent,paint self & children not required
    if(opacity < 0xFF) {
      SkRect layerBounds = SkRect::Make(absFrame_);
      context.canvas->saveLayerAlpha(&layerBounds,opacity);
    }

    paintSelf(context);// First paint self and then children if any

    if(masksToBounds()) { // Need to clip children.
        SkRect intRect = SkRect::Make(getFrame());
        if(!context.dirtyClipBound.isEmpty() && intRect.intersect(context.dirtyClipBound) == false) {
            RNS_LOG_WARN("We should not call paint if it doesnt intersect with non empty dirtyClipBound...");
        }
        context.canvas->clipRect(intRect);
    }

    for (auto& layer : children()) {
        if(layer->needsPainting(context))
            layer->paint(context);
    }
}
}   // namespace RnsShell
