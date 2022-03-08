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

    static SharedPictureLayer Create(Client& layerClient);
    PictureLayer(Client& layerClient);
    virtual ~PictureLayer() {};

    SkPicture* picture() const { return picture_.get(); }
    virtual void paintSelf(PaintContext& context) override;
    void prePaint(PaintContext& context, bool forceChildrenLayout = false) override;
    void paint(PaintContext& context) override;

    void setPicture(sk_sp<SkPicture> picture) { picture_ = picture; }

private:
    // Picture may reference images that have a reference to a GPU resource.
    // TODO what will happen for NON-GPU ?
    sk_sp<SkPicture> picture_;

    typedef Layer INHERITED;
};

}   // namespace RnsShell
