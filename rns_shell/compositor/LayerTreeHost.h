/*
* Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/
#pragma once

#include "rns_shell/compositor/Compositor.h"
#include "rns_shell/platform/graphics/PlatformDisplay.h"

namespace RnsShell {
class Application;
class Window;

class LayerTreeHost {
 public:

  LayerTreeHost(Application& client);
  virtual ~LayerTreeHost();

  void setRootCompositingLayer(SharedLayer rootLayer);
  void begin();
  void commitScene(bool immediate);
  void sizeDidChange(SkSize& size);

  PlatformDisplayID displayID() const { return displayID_; }
  Compositor* compositor() { return compositor_.get(); }
  Window *nativeWindow() { return window_.get(); }

 private:
  class CompositorClient : public Compositor::Client {
    RNS_MAKE_NONCOPYABLE(CompositorClient);

   public:
    CompositorClient(LayerTreeHost& layerTreeHost)
        : layerTreeHost_(layerTreeHost) { }
    ~CompositorClient(){};

   private:
    uint64_t nativeSurfaceHandle() override { return layerTreeHost_.nativeSurfaceHandle(); }
    void didRenderFrame() override { layerTreeHost_.didRenderFrame(); }
    LayerTreeHost& layerTreeHost_;
  };

  uint64_t nativeSurfaceHandle();
  void didRenderFrame();

  Application& app_;
  std::unique_ptr<Window> window_;
  CompositorClient compositorClient_;
  std::unique_ptr<Compositor> compositor_;
  PlatformDisplayID displayID_;
};

}   // namespace RnsShell
