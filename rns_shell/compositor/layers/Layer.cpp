/*
* Copyright 2016 Google Inc.
* Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "compositor/layers/Layer.h"
#include "compositor/layers/PictureLayer.h"
#include "compositor/layers/ScrollLayer.h"

namespace RnsShell {

#if USE(RNS_SHELL_PARTIAL_UPDATES)
void Layer::addDamageRect(PaintContext& context, SkIRect dirtyAbsFrameRect) {
    addDamageRect(context.damageRect, dirtyAbsFrameRect);
}

void Layer::addDamageRect(FrameDamages& damageRectList, SkIRect dirtyAbsFrameRect) {
    bool checkIfAlreadyCovered = true;
    for (auto it = damageRectList.begin(); it != damageRectList.end(); it++) {
        // Check 1 : If new dirty rect fully covers any of existing dirtyRect in the list then remove them from vector
        SkIRect &dirtRect = *it;
        if (dirtyAbsFrameRect != dirtRect && dirtyAbsFrameRect.contains(dirtRect)) {
            damageRectList.erase(it--); // Need to decrement the iterator.
            RNS_LOG_TRACE("Remove existing dirty rect [" << dirtRect.x() << dirtRect.y() << dirtRect.width() << dirtRect.height() << "] because new dirty rect [" <<
              dirtyAbsFrameRect.x() << dirtyAbsFrameRect.y() << dirtyAbsFrameRect.width() << dirtyAbsFrameRect.height() << "] will cover it");
            // If we entered here, it means that there cannot be an existing rect which already covers this new rect. So we can skip Check 2
            checkIfAlreadyCovered = false;
        }
        // Check 2: If new dirty rect is inside any of existing dirtyRects then ignore and return
        if(checkIfAlreadyCovered && dirtRect.contains(dirtyAbsFrameRect)) {
            RNS_LOG_TRACE("Skip new dirtyrect [" << dirtyAbsFrameRect.x() << dirtyAbsFrameRect.y() << dirtyAbsFrameRect.width() << dirtyAbsFrameRect.height() << "] because existing dirty rect [" <<
              dirtRect.x() << dirtRect.y() << dirtRect.width() << dirtRect.height() << "] already covers it");
            return;
        }
    }
    damageRectList.push_back(dirtyAbsFrameRect); // Add new unique dirty rect
}
#endif

SharedLayer Layer::Create(Client& layerClient, LayerType type) {
    switch(type) {
        case LAYER_TYPE_PICTURE:
            return std::make_shared<PictureLayer>(layerClient);
        case LAYER_TYPE_SCROLL:
            return std::make_shared<ScrollLayer>(layerClient);
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

Layer::EmptyClient& Layer::EmptyClient::singleton() {
    static Layer::EmptyClient client;
    return client;
}

Layer::Layer(Client& layerClient, LayerType type)
    : layerId_(nextUniqueId())
    , parent_(nullptr)
    , type_(type)
    , client_(&layerClient)
    , frame_(SkIRect::MakeEmpty())
    , absFrame_(SkIRect::MakeEmpty())
    , bounds_(SkIRect::MakeEmpty())
    , anchorPosition_(SkPoint::Make(0.5,0.5)) // Default anchor point as centre
    , invalidateMask_(LayerInvalidateAll) {
    RNS_LOG_DEBUG("Layer Constructed(" << this << ") with ID : " << layerId_ << " and LayerClient : " << client_);
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
    if(!child || !child.get()) {
        RNS_LOG_ERROR("Invalid child node passed");
        return;
    }

    child->removeFromParent();
    child->setParent(this);

    index = std::min(index, children_.size());
    RNS_LOG_DEBUG("Insert Child(" << child.get()->layerId() << ") at index : " << index << " and with parent : " << layerId_);
    children_.insert(children_.begin() + index, child);
}

void Layer::removeChild(Layer *child, size_t index) {
    if(index >= children_.size()) {
        RNS_LOG_ERROR("Invalid index passed for remove");
        return;
    }
    child->setParent(nullptr);

    RNS_LOG_DEBUG("Remove Child(" << child->layerId() << ") at index : " << index << " from parent : " << layerId_);
    children_.erase(children_.begin() + index);
}

void Layer::removeChild(Layer *child) {
    size_t index = 0;
    for (auto iter = children_.begin(); iter != children_.end(); ++iter, index++) {
        if (iter->get() != child)
            continue;
        removeChild(child, index);
        return;
    }
}

void Layer::removeFromParent() {
  if (parent_)
    parent_->removeChild(this);
}

void Layer::preRoll(PaintContext& context, bool forceLayout) {
    // Layer Layout has changed or parent has forced Layout change for child. Need to recalculate absolute frame and update absFrame_ & bounds
     if(forceLayout || (invalidateMask_ & LayerLayoutInvalidate)) {
        // Adjust absolute position w.r.t transformation matrix
        calculateTransformMatrix();
        SkRect mapRect=SkRect::Make(frame_);
        absoluteTransformMatrix_.mapRect(&mapRect);
        absFrame_ = SkIRect::MakeXYWH(mapRect.x(), mapRect.y(), mapRect.width(), mapRect.height());
        SkIRect newBounds = absFrame_;
        frameBounds_ = frame_;
        if(shadowFilter) {
            SkMatrix identityMatrix;
            frameBounds_ = shadowFilter->filterBounds(
                                               frame_,
                                               identityMatrix,
                                               SkImageFilter::kForward_MapDirection,
                                               nullptr);

            //Calculate absolute frame bounds
            SkRect mapRect=SkRect::Make(frameBounds_);
            absoluteTransformMatrix_.mapRect(&mapRect);
            newBounds = SkIRect::MakeXYWH(mapRect.x(), mapRect.y(), mapRect.width(), mapRect.height());
        }
#if USE(RNS_SHELL_PARTIAL_UPDATES)
        if((invalidateMask_ & LayerLayoutInvalidate)) {
            // Add previous bounds to damage rect only if layer layout was invalidated and new layout is different from old layout
            if(context.supportPartialUpdate) {
                if(bounds_.isEmpty() != true && newBounds.contains(bounds_) != true ) {
                    addDamageRect(context, bounds_); // Previous bounds
                    RNS_LOG_DEBUG("New bounds is different from previous bounds. Add to damage rect..");
                }
            }
            RNS_LOG_DEBUG("PreRoll Layer(ID:" << layerId_ << ", ParentID:" << (parent_ ? parent_->layerId() : -1) <<
                ") Frame [" << frame_.x() << "," << frame_.y() << "," << frame_.width() << "," << frame_.height() <<
                "], Bounds(Prev,New) ([" << bounds_.x() << "," << bounds_.y() << "," << bounds_.width() << "," << bounds_.height() << "]" <<
                " - [" << newBounds.x() << "," << newBounds.y() << "," << newBounds.width() << "," << newBounds.height() << "])");
        }
#endif
        bounds_ = newBounds; // Save new bounds
    }

#if USE(RNS_SHELL_PARTIAL_UPDATES)
    if(context.supportPartialUpdate && ( invalidateMask_ != LayerInvalidateNone)) {// As long as there is some invalidation , it creates a dirty rect.
        RNS_LOG_DEBUG("AddDamage Layer(ID:" << layerId_ <<
            ") Bounds[" << bounds_.x() << "," << bounds_.y() << "," << bounds_.width() << "," << bounds_.height() << "]");
        addDamageRect(context, bounds_); // new bounds
    }
#endif
}

void Layer::prePaint(PaintContext& context, bool forceLayout) {
    //Adjust absolute Layout frame and dirty rects
    bool forceChildrenLayout = (forceLayout || (invalidateMask_ & LayerLayoutInvalidate));
    preRoll(context, forceLayout);
    // It will reset the invalidate mask.
    invalidateMask_ = static_cast<LayerInvalidateMask>(invalidateMask_ & LayerRemoveInvalidate) ;

    // prePaint children recursively
    size_t index = 0;
    std::map <size_t , SharedLayer> recycleChildList;
    for (auto& layer : children()) {
        layer->prePaint(context, forceChildrenLayout);
        if(layer->invalidateMask_ == LayerRemoveInvalidate){
            recycleChildList[index] = layer;
        }
        index++;
    }

    //remove children marked with remove mask from parent list
    for(auto recycleChildIter = recycleChildList.rbegin();recycleChildIter != recycleChildList.rend();++recycleChildIter)
         removeChild(recycleChildIter->second.get(),recycleChildIter->first);

    recycleChildList.clear();
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

void Layer::paintChildren(PaintContext& context) {
    for (auto& layer : children_) {
        if(layer->needsPainting(context)) {
            RNS_LOG_DEBUG("Paint Layer(ID:" << layer->layerId_ << ", ParentID:" << layerId_ <<
                ") Frame [" << layer->frame_.x() << "," << layer->frame_.y() << "," << layer->frame_.width() << "," << layer->frame_.height() <<
                "], Bounds [" << layer->bounds_.x() << "," << layer->bounds_.y() << "," << layer->bounds_.width() << "," << layer->bounds_.height() << "]");
            layer->paint(context);
        }
    }
}

void Layer::paint(PaintContext& context) {
    RNS_LOG_DEBUG("Layer (" << layerId_ << ") has " << children_.size() << " childrens");

    SkAutoCanvasRestore save(context.canvas, true); // Save current clip and matrix state.

    setLayerTransformMatrix(context);
    setLayerOpacity(context);
    paintSelf(context); // First paint self and then children if any

    if(masksToBounds_) { // Need to clip children.
        SkRect intRect = SkRect::Make(frame_);
        if(!context.dirtyClipBound.isEmpty() && intRect.intersect(context.dirtyClipBound) == false) {
            RNS_LOG_WARN("We should not call paint if it doesnt intersect with non empty dirtyClipBound...");
        }
        context.canvas->clipRect(intRect,SkClipOp::kIntersect);
    }

    paintChildren(context);
}

bool Layer::hasAncestor(const Layer* ancestor) const {
    for (const Layer* layer = parent(); layer; layer = layer->parent()) {
        if (layer == ancestor)
            return true;
    }
    return false;
}

void Layer::setParent(Layer* layer) {
    if(layer && layer->hasAncestor(this)) {
        RNS_LOG_WARN("Child Layer cant be ancestor :)");
        return;
    }
    parent_ = layer;
}

bool Layer::needsPainting(PaintContext& context) {
    if (frame_.isEmpty() || isHidden_) { // If layer is hidden or layers paint bounds is empty then skip paint
      RNS_LOG_TRACE(this << " Layer (" << layerId_ << ") Bounds empty or hidden");
      return false;
    }

#if USE(RNS_SHELL_PARTIAL_UPDATES)
    if(!context.supportPartialUpdate)  //if partial update not supported,need to paint the layer
#else
    if(context.damageRect.size() == 0) // No damage rect set, so need to paint the layer
#endif
        return true;

    // As long as paintBounds interset with one of the dirty rect, we will need repainting.
    SkIRect dummy;
    for (auto& dirtRect : context.damageRect) {
        SkIRect bounds = bounds_;
        //If scrolling offset available,check bounds with offset value
        if(!context.offset.isZero()){
          bounds.offset(context.offset.x(),context.offset.y());
        }
        if(dummy.intersect(bounds, dirtRect)) { // this layer intersects with one of the dirty rect, so needs repainting
            return true;
        }
    }

    RNS_LOG_TRACE("Skip Layer (" << layerId_ << ") Frame [" << frame_.x() << "," << frame_.y() << "," << frame_.width() << "," << frame_.height() << "]");
    return false;
}

void Layer::calculateTransformMatrix() {

/* Step 1: calculate transform matrix set by parent */
    if((!skipParentMatrix_) && parent_) {
        absoluteTransformMatrix_=parent_->absoluteTransformMatrix_;
        absoluteTransformMatrix_.preTranslate(parent_->frame_.x(),parent_->frame_.y());
    }
/* Step 2: calculate transform matrix to be set on it's own on top it's parent's,if any */
    if(!transformMatrix.isIdentity()) {
/*
  Logic to do transformation w.r.t anchor point
     Step 2.1: Move anchor point of frame to the origin
     Step 2.2: Apply  transformation
     Step 2.3: Move back the origin to the orginal anchor point
Order of opertaion have to be reversed as  pre matrix opertaion used,
so starting from step 3 to step 1
*/
        int Xtrans =  frame_.x()+(frame_.width()*anchorPosition_.fX);
        int Ytrans = frame_.y()+(frame_.height()*anchorPosition_.fY);

        absoluteTransformMatrix_.preTranslate(Xtrans,Ytrans);
        absoluteTransformMatrix_.preConcat(transformMatrix);
        absoluteTransformMatrix_.preTranslate(-Xtrans,-Ytrans);

    }
}

bool Layer::requireInvalidate(bool skipChildren) {
    if(invalidateMask_ != LayerInvalidateNone) return true;
    if(!skipChildren) {
       for (auto &layer :children())
          if(layer->requireInvalidate(skipChildren)) return true;
    }
    return false;
}

void Layer::setLayerOpacity(PaintContext& context) {
    if(opacity <= 0.0) return; //if transparent,paint self & children not required
    if(opacity < 0xFF) {
      SkRect layerBounds = SkRect::Make(frameBounds_);
      context.canvas->saveLayerAlpha(&layerBounds,opacity);
    }
}

void Layer::setLayerTransformMatrix(PaintContext& context) {
    SkMatrix screenMatrix;
    //If scrolling offset is available,concat the offset to transform matrix
    if(!context.offset.isZero()) {
        screenMatrix.setTranslate(context.offset.x(),context.offset.y());
    }
    screenMatrix.preConcat(absoluteTransformMatrix_);
    context.canvas->setMatrix(screenMatrix);
}

}   // namespace RnsShell

