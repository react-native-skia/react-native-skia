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

typedef std::vector<std::shared_ptr<Layer> > LayerList;
using SharedLayer = std::shared_ptr<Layer>;

class Layer {
public:

    static SharedLayer Create();
    Layer();
    virtual ~Layer() {};

    Layer* rootLayer();
    Layer* parent() { return parent_; }

    int layerId() { return layerId_;}
    void appendChild(SharedLayer child);
    void insertChild(SharedLayer child, size_t index);
    void removeChild(SharedLayer child, size_t index);

    void prePaint(SkSurface *surface);
    void paint(SkSurface *surface);
    virtual void onPaint(SkSurface*) {}

private:
    static uint64_t nextUniqueId();

    void setParent(Layer* layer);
    bool needsPainting();
    const LayerList& children() const { return children_; }

    int layerId_;
    Layer *parent_;
    LayerList children_;
};

}   // namespace RnsShell
