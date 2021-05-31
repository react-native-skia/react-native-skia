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

<<<<<<< HEAD
#include "include/core/SkCanvas.h"
=======
>>>>>>> RNS Shell Implementation  (#8)
#include "include/core/SkSurface.h"

#include "ReactSkia/utils/RnsLog.h"
#include "ReactSkia/utils/RnsUtils.h"

namespace RnsShell {

class Layer;

<<<<<<< HEAD
enum LayerType {
  LAYER_TYPE_DEFAULT = 0, // Default layer type which which will use component specific APIs to paint.
  LAYER_TYPE_PICTURE, // SkPiture based layer.
  LAYER_TYPE_TEXTURED, // SkTexture based layer.
};

typedef std::vector<std::shared_ptr<Layer> > LayerList;
using SharedLayer = std::shared_ptr<Layer>;

struct PaintContext {
    SkCanvas* canvas;
    std::vector<SkIRect>* damageRect; // Dirty rects in current frame
    const SkRect& dirtyClipBound; // combined clip bounds based on all the dirty rects.
    GrDirectContext* grContext;
};

class Layer {
public:

    static SharedLayer Create(LayerType type = LAYER_TYPE_DEFAULT);
    Layer(LayerType);
=======
typedef std::vector<std::shared_ptr<Layer> > LayerList;
using SharedLayer = std::shared_ptr<Layer>;

class Layer {
public:

    static SharedLayer Create();
    Layer();
>>>>>>> RNS Shell Implementation  (#8)
    virtual ~Layer() {};

    Layer* rootLayer();
    Layer* parent() { return parent_; }

<<<<<<< HEAD
    LayerType type() { return type_; }
    int layerId() { return layerId_;}

    const LayerList& children() const { return children_; }
    bool needsPainting(PaintContext& context);
=======
    int layerId() { return layerId_;}
>>>>>>> RNS Shell Implementation  (#8)
    void appendChild(SharedLayer child);
    void insertChild(SharedLayer child, size_t index);
    void removeChild(SharedLayer child, size_t index);

<<<<<<< HEAD
    virtual void paintSelf(PaintContext& context);
    virtual void prePaint(PaintContext& context);
    virtual void paint(PaintContext& context);
    virtual void onPaint(SkCanvas*) {}

    const SkIRect& getFrame() const { return frame_; }
    void setFrame(const SkIRect& frame) { frame_ = frame; }

    const SkIRect& getBounds() const { return bounds_; }
    void setBounds(const SkIRect& bounds) { bounds_ = bounds; }

    const SkPoint& anchorPosition() const { return anchorPosition_; }
    void setAnchorPosition(const SkPoint& anchorPosition) { anchorPosition_ = anchorPosition; }

    const bool masksToBounds() const { return masksToBounds_; }
    void setMasksTotBounds(bool masksToBounds) { masksToBounds_ = masksToBounds; }
=======
    void prePaint(SkSurface *surface);
    void paint(SkSurface *surface);
    virtual void onPaint(SkSurface*) {}
>>>>>>> RNS Shell Implementation  (#8)

private:
    static uint64_t nextUniqueId();

    void setParent(Layer* layer);
<<<<<<< HEAD

    int layerId_;
    Layer *parent_;
    LayerType type_;
    LayerList children_;

    //Layer Geometry
    SkIRect frame_; //The paint bounds should include any transform performed by the layer itself in its parents coordinate space
    SkIRect bounds_; //The paint bounds in its own coordinate space
    SkPoint anchorPosition_; // Position of Layer wrt anchor point in parents coordinate space. This will be used during the transformation.

    //Layer’s Appearance
    bool isHidden_ = { false }; // Wheather layer is hidden
    bool masksToBounds_ = { false }; // Clip childrens
    //Borders & Shadows ?
=======
    bool needsPainting();
    const LayerList& children() const { return children_; }

    int layerId_;
    Layer *parent_;
    LayerList children_;
>>>>>>> RNS Shell Implementation  (#8)
};

}   // namespace RnsShell
