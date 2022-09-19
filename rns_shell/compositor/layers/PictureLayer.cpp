/*
* Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
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
    RNS_LOG_DEBUG("Picture Layer Constructed(" << this << ") with ID : " << layerId() << " and LayerClient : " << &layerClient);
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

}   // namespace RnsShell
