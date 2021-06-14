/*
* Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "compositor/layers/PictureLayer.h"
#include "third_party/skia/include/core/SkCanvas.h"

namespace RnsShell {

SharedPictureLayer PictureLayer::Create(const SkPoint& offset, sk_sp<SkPicture> picture) {
    return std::make_shared<PictureLayer>(offset, picture);
}

PictureLayer::PictureLayer(const SkPoint& offset,
                           sk_sp<SkPicture> picture)
    : offset_(offset),
      picture_(std::move(picture)) {}

void PictureLayer::prePaint(SkSurface *surface) {
}

void PictureLayer::paint(SkSurface *surface) {
    if(!surface) {
        RNS_LOG_ERROR(picture_.get() << "  " << surface);
        return;
    }

    // First paint self and then children if any
    if(picture_.get()) {
        auto canvas = surface->getCanvas();
        SkAutoCanvasRestore save(canvas, true);
        RNS_LOG_TRACE("SkPicture ( "  << picture_ << " )For " <<
                picture_.get()->approximateOpCount() << " operations and size : " << picture_.get()->approximateBytesUsed());
        canvas->translate(offset_.x(), offset_.y());
        picture()->playback(canvas);
    }

#if !defined(GOOGLE_STRIP_LOG) || (GOOGLE_STRIP_LOG <= INFO)
    RNS_GET_TIME_STAMP_US(start);
#endif

    RNS_LOG_DEBUG("Layer (" << layerId() << ") has " << children().size() << " childrens with surface : " << surface);
    for (auto& layer : children()) {
        if(layer->needsPainting())
            layer->paint(surface);
    }

#if !defined(GOOGLE_STRIP_LOG) || (GOOGLE_STRIP_LOG <= INFO)
    RNS_GET_TIME_STAMP_US(end);
    RNS_LOG_TRACE("Layer (" << layerId() << ") took " <<  (end - start) << " us  to paint self and children");
#endif
}

}   // namespace RnsShell
