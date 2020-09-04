#pragma once

#include "include/core/SkCanvas.h"
#include "react/renderer/mounting/ShadowView.h"
#include "skia/sk_app/Window.h"

namespace facebook {
namespace react {

class RSkComponent : public sk_app::Window::Layer {
 public:
  RSkComponent(const ShadowView &shadowView);
  RSkComponent(RSkComponent &&) = default;
  RSkComponent &operator=(RSkComponent &&) = default;

  virtual ~RSkComponent();

 protected:
  virtual void OnPaint(const ShadowView &shadowView, SkCanvas *canvas) = 0;

 private:
  // sk_app::Window::Layer implementations
  void onPaint(SkSurface *surface) override;

 private:
  ShadowView shadowView_;
};

} // namespace react
} // namespace facebook
