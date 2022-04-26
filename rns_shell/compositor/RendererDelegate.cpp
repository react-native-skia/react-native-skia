/*
* Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "RendererDelegate.h"

namespace RnsShell {

RendererDelegate::RendererDelegate(Application& app) {
  layerTreeHost_ = std::make_unique<LayerTreeHost>(app);
}

SkSize RendererDelegate::viewPort() {
  return(layerTreeHost_->compositor()->viewport());
}

#ifdef RNS_SHELL_HAS_GPU_SUPPORT
GrDirectContext* RendererDelegate::graphicsDirectContext() {
  return(layerTreeHost_->compositor()->getDirectContext());
}
#endif

void RendererDelegate::begin() {
  layerTreeHost_->begin();
}

void RendererDelegate::commit(bool immediate) {
  layerTreeHost_->commitScene(immediate);
}

void RendererDelegate::setRootLayer(SharedLayer rootLayer) {
  layerTreeHost_->setRootCompositingLayer(rootLayer);
}

void RendererDelegate::scheduleRenderingUpdate() {
  commit(false);
}

void RendererDelegate::beginRenderingUpdate() {
  begin();
}


}   // namespace RnsShell
