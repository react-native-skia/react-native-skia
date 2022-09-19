/*
* Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
*
* This source code is licensed under the MIT license found in the
* LICENSE file in the root directory of this source tree.
*/

#pragma once

#include "compositor/layers/Layer.h"
#include "include/core/SkPicture.h"

namespace RnsShell {

class ScrollLayer;
using SharedScrollLayer = std::shared_ptr<ScrollLayer>;

class ScrollLayer : public Layer {
public:

#if ENABLE(FEATURE_SCROLL_INDICATOR)
    class ScrollBar {
      public:
        enum ScrollBarPosition {
            ScrollBarPositionLeft,
            ScrollBarPositionRight,
            ScrollBarPositionTop,
            ScrollBarPositionBottom
        };

        ScrollBar();
        void showScrollBar(bool show);
        void setScrollBarOffset(SkPoint scrollPos);
        void setScrollBarColor(SkColor color);
        void setScrollBarPosition(ScrollBarPosition position);
        void setScrollBarInsets(SkIRect frameInsets);
        void updateScrollLayerLayout(SkISize scrollContentSize, SkIRect scrollFrame);

        SkIRect getScrollBarAbsFrame(SkIRect scrollAbsFrame,LayerInvalidateMask layerMask);
        void paint(SkCanvas *canvas);

      private:
        LayerInvalidateMask mask_{LayerInvalidateNone}; // flag to display/remove bar
        ScrollBarPosition barPosition_{ScrollBarPositionRight};
        SkPaint barColor_;
        SkIRect barFrame_;
        SkIRect barFrameInsets_;
        SkPoint barOffsetTranslate_{0,0}; //absolute translate value ie., parent abs matrix + bar offset translate
        SkPoint barOffsetMultiplier_{1,1};//multiplier to derive offset of bar in the frame
        //float opacity_{1.0}; // to achieve fade-out animation

        SkISize scrollContentSize_;
        SkIRect scrollFrame_;
        SkPoint scrollPos_{0,0};

        void calculateBarLayoutMetrics();
        void updateBarOffset();
    };
#endif //ENABLE_FEATURE_SCROLL_INDICATOR

    static SharedScrollLayer Create(Client& layerClient);
    ScrollLayer(Client& layerClient);
    virtual ~ScrollLayer() {};

    SkPicture* shadowPicture() const { return shadowPicture_.get(); }
    SkPicture* borderPicture() const { return borderPicture_.get(); }

    void prePaint(PaintContext& context, bool forceChildrenLayout = false) override;
    void paint(PaintContext& context) override;
    void paintSelf(PaintContext& context) override;

    void setShadowPicture(sk_sp<SkPicture> picture) { shadowPicture_ = picture; }
    void setBorderPicture(sk_sp<SkPicture> picture) { borderPicture_ = picture; }

    bool setContentSize(SkISize contentSize) ;
    SkISize getContentSize() { return contentSize_;};

    void setScrollPosition(SkPoint scrollPos) ;
    SkPoint getScrollPosition() { return SkPoint::Make(scrollOffsetX_,scrollOffsetY_);};

#if ENABLE(FEATURE_SCROLL_INDICATOR)
    ScrollBar& getScrollBar() { return scrollbar_;}
#endif

private:

#if USE(SCROLL_LAYER_BITMAP)
    void bitmapConfigure();
    void paintChildrenAndSelf(PaintContext& context);
    bool forceBitmapReset_{true}; // Flag to reset bitmap
    SkBitmap scrollBitmap_; // Bitmap for scroll container for childs to draw
    std::unique_ptr<SkCanvas> scrollCanvas_;
    SkRect clipBound_;

    SkIRect drawDestRect_;
    SkIRect drawSrcRect_;
#endif
    std::vector<SkIRect> bitmapSurfaceDamage_;

    int scrollOffsetX_{0};   // Offset to scroll in x direction
    int scrollOffsetY_{0};   // Offset to scroll in y direction
    SkISize contentSize_{0};  // total size of all contents
    sk_sp<SkPicture> shadowPicture_;
    sk_sp<SkPicture> borderPicture_;

    void paintSelfAndChildren(PaintContext& context);
    inline void paintBorder(PaintContext& context);
    inline void paintScrollBar(PaintContext& context);
#if ENABLE(FEATURE_SCROLL_INDICATOR)
    ScrollBar scrollbar_;   // scroll indicator bar
#endif

    typedef Layer INHERITED;
};

}   // namespace RnsShell
