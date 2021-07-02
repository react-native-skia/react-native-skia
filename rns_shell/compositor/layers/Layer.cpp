/*
* Copyright 2016 Google Inc.
* Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "compositor/layers/Layer.h"
#include "compositor/layers/PictureLayer.h"

namespace RnsShell {

SharedLayer Layer::Create(LayerType type) {
    switch(type) {
        case LAYER_TYPE_PICTURE:
            return std::make_shared<PictureLayer>();
        case LAYER_TYPE_DEFAULT:
        default:
            RNS_LOG_ASSERT(false, "Default layers can be created only from RSkComponent constructor");
            return nullptr;
    }
}

uint64_t Layer::nextUniqueId() {
    static std::atomic<uint64_t> nextId(1);
    uint64_t id;
    do {
        id = nextId.fetch_add(1);
    } while (id == 0);  // 0 invalid id.
    return id;
}

Layer::Layer(LayerType type)
    : layerId_(nextUniqueId())
    , parent_(nullptr)
    , type_(type)
    , frame_(SkIRect::MakeEmpty())
    , anchorPosition_(SkPoint::Make(0,0)) {
    RNS_LOG_INFO("Layer Constructed(" << this << ") with ID : " << layerId_);
}

Layer* Layer::rootLayer() {
    Layer* layer = this;
    while (layer->parent())
        layer = layer->parent();
    return layer;
}

void Layer::appendChild(SharedLayer child) {
    insertChild(child, children_.size());
}

void Layer::insertChild(SharedLayer child, size_t index) {
    // TODO
    // Add sanity checks
    // Check if child already has a parent, and remove it if so
    RNS_LOG_TODO("Check if child already has a parent, if so then first remove it");
    child->setParent(this);

    index = std::min(index, children_.size());
    RNS_LOG_DEBUG("Insert Child(" << child.get()->layerId() << ") at index : " << index << " and with parent : " << layerId_);
    children_.insert(children_.begin() + index, child);
}

void Layer::removeChild(SharedLayer child, size_t index) {
    if(index >= children_.size()) {
        RNS_LOG_ERROR("Invalid index passed for remove");
        return;
    }
    child->setParent(nullptr);

    RNS_LOG_DEBUG("Remove Child(" << child.get()->layerId() << ") at index : " << index << " from parent : " << layerId_);
    children_.erase(children_.begin() + index);
}

void Layer::prePaint(PaintContext& context) {
}

void Layer::paintSelf(PaintContext& context) {
#if !defined(GOOGLE_STRIP_LOG) || (GOOGLE_STRIP_LOG <= INFO)
    RNS_GET_TIME_STAMP_US(start);
#endif

    this->onPaint(context.canvas);

#if !defined(GOOGLE_STRIP_LOG) || (GOOGLE_STRIP_LOG <= INFO)
    RNS_GET_TIME_STAMP_US(end);
    RNS_LOG_TRACE("Layer (" << layerId_ << ") took " <<  (end - start) << " us to paint self");
#endif
}

void Layer::paint(PaintContext& context) {
    RNS_LOG_DEBUG("Layer (" << layerId_ << ") has " << children_.size() << " childrens");
    SkAutoCanvasRestore save(context.canvas, true); // Save current clip and matrix state.

    // TODO Concat matrix
    paintSelf(context); // First paint self and then children if any

    if(masksToBounds_) { // Need to clip children.
        SkRect intRect = SkRect::Make(frame_);
        if(!context.dirtyClipBound.isEmpty() && intRect.intersect(context.dirtyClipBound) == false) {
            RNS_LOG_WARN("We should not call paint if it doesnt intersect with non empty dirtyClipBound...");
        }
        context.canvas->clipRect(intRect);
    }

    for (auto& layer : children_) {
        if(layer->needsPainting(context))
            layer->paint(context);
    }
}

void Layer::setParent(Layer* layer) {
    // TODO add checks
    RNS_LOG_TODO("Add checks");
    parent_ = layer;
}

bool Layer::needsPainting(PaintContext& context) {

    if (frame_.isEmpty() || isHidden_) { // If layer is hidden or layers paint bounds is empty then skip paint
      RNS_LOG_TRACE(this << " Layer (" << layerId_ << ") Bounds empty or hidden");
      return false;
    }

    if(context.damageRect->size() == 0) // No damage rect set, so need to paint the layer
        return true;

    // As long as paintBounds interset with one of the dirty rect, we will need repainting.
    SkIRect dummy;
    for (auto& dirtRect : *context.damageRect) {
        if(dummy.intersect(frame_, dirtRect)) // this layer intersects with one of the dirty rect, so needs repainting
            return true;
    }

    RNS_LOG_TRACE("Skip Layer (" << layerId_ << ")");
    return false;
}

}   // namespace RnsShell

