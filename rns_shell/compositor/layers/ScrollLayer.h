/*
* Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#pragma once

#include "compositor/layers/Layer.h"
#include "include/core/SkPicture.h"

namespace RnsShell {

class ScrollLayer;
using SharedScrollLayer = std::shared_ptr<ScrollLayer>;

class ScrollLayer : public Layer {
public:
    static SharedScrollLayer Create(Client& layerClient);
    ScrollLayer(Client& layerClient);
    virtual ~ScrollLayer() {};

    int scrollOffsetX{0};   // Offset to scroll in x direction
    int scrollOffsetY{0};   // Offset to scroll in y direction

    SkPicture* shadowPicture() const { return shadowPicture_.get(); }
    SkPicture* borderPicture() const { return borderPicture_.get(); }

    void prePaint(PaintContext& context, bool forceChildrenLayout = false) override;
    void paint(PaintContext& context) override;
    void paintSelf(PaintContext& context) override;

    void setShadowPicture(sk_sp<SkPicture> picture) { shadowPicture_ = picture; }
    void setBorderPicture(sk_sp<SkPicture> picture) { borderPicture_ = picture; }

    bool setContentSize(SkISize contentSize) ;
    SkISize getContentSize() { return contentSize_;};

private:
    void bitmapConfigure();

    SkISize contentSize_{0};  // total size of all contents
    bool forceBitmapReset_{true}; // Flag to reset bitmap

    SkBitmap scrollBitmap_; // Bitmap for scroll container for childs to draw
    std::unique_ptr<SkCanvas> scrollCanvas_;
    std::vector<SkIRect> bitmapSurfaceDamage_;
    SkRect clipBound_;

    sk_sp<SkPicture> shadowPicture_;
    sk_sp<SkPicture> borderPicture_;

    typedef Layer INHERITED;
};

}   // namespace RnsShell