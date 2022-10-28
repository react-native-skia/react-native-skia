/*
* Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#pragma once

#include "compositor/LayerTreeHost.h"

namespace RnsShell {

class RendererDelegate : public RnsShell::Layer::Client {
public:
    RendererDelegate(Application& app);
    virtual ~RendererDelegate() {}

#ifdef RNS_SHELL_HAS_GPU_SUPPORT
    GrDirectContext* graphicsDirectContext();
#endif
    SkSize viewPort();
    void scheduleRenderingUpdate();
    void beginRenderingUpdate();
    void setRootLayer(SharedLayer rootLayer);
    void commit(bool immediate);
    void begin();

    // Layer Client Implementation
    void notifyFlushRequired() override { scheduleRenderingUpdate(); }
    void notifyFlushBegin() override { beginRenderingUpdate(); }

protected:
    std::unique_ptr<LayerTreeHost> layerTreeHost_;
};

}   // namespace RnsShell
