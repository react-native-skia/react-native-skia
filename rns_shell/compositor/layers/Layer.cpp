/*
* Copyright 2016 Google Inc.
* Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "compositor/layers/Layer.h"

namespace RnsShell {

SharedLayer Layer::Create() {
    return std::make_shared<Layer>();
}

uint64_t Layer::nextUniqueId() {
    static std::atomic<uint64_t> nextId(1);
    uint64_t id;
    do {
        id = nextId.fetch_add(1);
    } while (id == 0);  // 0 invalid id.
    return id;
}

Layer::Layer()
    : layerId_(nextUniqueId())
    , parent_(nullptr) {
    RNS_LOG_DEBUG("Layer Constructed(" << this << ") with ID : " << layerId_);
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

void Layer::prePaint(SkSurface *surface) {
}

void Layer::paint(SkSurface *surface) {
#if !defined(GOOGLE_STRIP_LOG) || (GOOGLE_STRIP_LOG <= INFO)
    RNS_GET_TIME_STAMP_US(start);
#endif
    RNS_LOG_TRACE("Layer (" << layerId_ << ") has " << children_.size() << " childrens with surface : " << surface);
    onPaint(surface); // First paint self and then children if any
    for (auto& layer : children_) {
        if(layer->needsPainting())
            layer->paint(surface);
    }
#if !defined(GOOGLE_STRIP_LOG) || (GOOGLE_STRIP_LOG <= INFO)
    RNS_GET_TIME_STAMP_US(end);
    RNS_LOG_TRACE("Layer (" << layerId_ << ") took " <<  (end - start) << " us  to paint self and children");
#endif
}

void Layer::setParent(Layer* layer) {
    // TODO add checks
    RNS_LOG_TODO("Add checks");
    parent_ = layer;
}

bool Layer::needsPainting() {
    // TODO add checks canvas.quickeReject(paintBounds);
    return true;
}

}   // namespace RnsShell

