/*
* Copyright 2016 Google Inc.
* Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#pragma once

#include <stddef.h>
#include <stdint.h>
#include <array>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
#include "ReactSkia/utils/RnsLog.h"
#include "ReactSkia/utils/RnsUtils.h"

namespace RnsShell {

class Layer;

enum LayerType {
    LAYER_TYPE_DEFAULT = 0, // Default layer type which which will use component specific APIs to paint.
    LAYER_TYPE_PICTURE, // SkPiture based layer.
    LAYER_TYPE_TEXTURED, // SkTexture based layer.
};

enum LayerInvalidateMask {
    LayerInvalidateNone = 0,
    LayerPaintInvalidate = 1 << 0,
    LayerLayoutInvalidate = 1 << 1,
    LayerInvalidateAll = LayerPaintInvalidate | LayerLayoutInvalidate
};

typedef std::vector<std::shared_ptr<Layer> > LayerList;
using SharedLayer = std::shared_ptr<Layer>;

struct PaintContext {
    SkCanvas* canvas;
    std::vector<SkIRect>& damageRect; // Dirty rects in current frame
#if USE(RNS_SHELL_PARTIAL_UPDATES)
    bool supportPartialUpdate;
#endif
    const SkRect& dirtyClipBound; // combined clip bounds based on all the dirty rects.
    GrDirectContext* grContext;
};

class Layer {
public:
    class Client {
    public:
        virtual ~Client() = default;
        virtual void notifyFlushRequired() { }
    };

    // Singleton defualt client used for default layers.
    class EmptyClient final : public Client {
    public:
        static EmptyClient& singleton();
    };

    int  backfaceVisibility;
    float opacity{1.0};

    float shadowOpacity{};
    float shadowRadius{3};
    SkColor shadowColor;
    SkSize shadowOffset{0,-3};

    SkMatrix transformMatrix;

    static SharedLayer Create(Client& layerClient, LayerType type = LAYER_TYPE_DEFAULT);
    Layer(Client&, LayerType);
    virtual ~Layer() {};

    Client& client() const { return *client_; }
    void setClient(Client& client) { client_ = &client; } // Used for Default Layer Type TODO: Need to remove this once we introduce Paintclient for default Layer.

    Layer* rootLayer();
    Layer* parent() { return parent_; }
    const Layer* parent() const { return parent_; }
    bool hasAncestor(const Layer* ancestor) const;

    LayerType type() { return type_; }
    int layerId() { return layerId_;}

    const LayerList& children() const { return children_; }
    bool needsPainting(PaintContext& context);
    void preRoll(PaintContext& context, bool forceLayout = false);

    void applyTransformations(PaintContext& context);
    void appendChild(SharedLayer child);
    void insertChild(SharedLayer child, size_t index);
    void removeChild(Layer *child, size_t index);
    void removeChild(Layer *child);
    void removeFromParent();

    virtual void paintSelf(PaintContext& context);
    virtual void prePaint(PaintContext& context, bool forceChildrenLayout = false);
    virtual void paint(PaintContext& context);
    virtual void onPaint(SkCanvas*) {}

    SkIRect& absoluteFrame() { return absFrame_; }
    const SkIRect& getFrame() const { return frame_; }
    void setFrame(const SkIRect& frame) { frame_ = frame; }
    void invalidate(LayerInvalidateMask mask = LayerInvalidateAll) { invalidateMask_ = mask; }

    const SkIRect& getBounds() const { return bounds_; }
    void setBounds(const SkIRect& bounds) { bounds_ = bounds; }

    const SkPoint& anchorPosition() const { return anchorPosition_; }
    void setAnchorPosition(const SkPoint& anchorPosition) { anchorPosition_ = anchorPosition; }

    const bool masksToBounds() const { return masksToBounds_; }
    void setMasksTotBounds(bool masksToBounds) { masksToBounds_ = masksToBounds; }

public:
    friend class PictureLayer;

private:
    static uint64_t nextUniqueId();

    void setParent(Layer* layer);

    void calculateTransformMatrix();

    int layerId_;
    Layer *parent_;
    LayerType type_;
    LayerList children_;
    Client* client_;

    //Layer Geometry
    SkIRect frame_; //The frame bounds should include any transform performed by the layer itself in its parent's coordinate space
    SkIRect absFrame_; // Absolute frame include any transform performed by the layer itself in rootview's coordinate space
    SkIRect bounds_; //The paint bounds in its own coordinate space
    SkPoint anchorPosition_; // Position of Layer wrt anchor point in parents coordinate space. This will be used during the transformation.
    SkMatrix absoluteTransformMatrix_; // Combined Transformation Matrix of self & parent's
    //Layer’s Appearance
    bool isHidden_ = { false }; // Wheather layer is hidden
    bool masksToBounds_ = { false }; // Clip childrens
    //Borders & Shadows ?

    LayerInvalidateMask invalidateMask_;
};

}   // namespace RnsShell
