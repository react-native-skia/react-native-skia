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

typedef std::vector<std::shared_ptr<Layer> > LayerList;
using SharedLayer = std::shared_ptr<Layer>;

class Layer {
public:

    static SharedLayer Create(LayerType type = LAYER_TYPE_DEFAULT);
    Layer(LayerType);
    virtual ~Layer() {};

    Layer* rootLayer();
    Layer* parent() { return parent_; }

    LayerType type() { return type_; }
    int layerId() { return layerId_;}

    const LayerList& children() const { return children_; }
    bool needsPainting();
    void appendChild(SharedLayer child);
    void insertChild(SharedLayer child, size_t index);
    void removeChild(SharedLayer child, size_t index);

    virtual void prePaint(SkSurface *surface);
    virtual void paint(SkSurface *surface);
    virtual void onPaint(SkSurface*) {}

private:
    static uint64_t nextUniqueId();

    void setParent(Layer* layer);

    int layerId_;
    Layer *parent_;
    LayerType type_;
    LayerList children_;
};

}   // namespace RnsShell
