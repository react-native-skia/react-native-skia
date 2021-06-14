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

class PictureLayer;

using SharedPictureLayer = std::shared_ptr<PictureLayer>;

class PictureLayer : public Layer {
public:

    static SharedPictureLayer Create(const SkPoint& offset, sk_sp<SkPicture> picture);
    PictureLayer(const SkPoint& offset,
               sk_sp<SkPicture> picture);
    virtual ~PictureLayer() {};

    SkPicture* picture() const { return picture_.get(); }
    void prePaint(SkSurface *surface) override;
    void paint(SkSurface *surface) override;

    void setPicture(sk_sp<SkPicture> picture) { picture_ = picture; }

private:
    SkPoint offset_;
    // Picture may reference images that have a reference to a GPU resource.
    // TODO what will happen for NON-GPU ?
    sk_sp<SkPicture> picture_;
};

}   // namespace RnsShell
