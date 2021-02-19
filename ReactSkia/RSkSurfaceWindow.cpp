#include "ReactSkia/RSkSurfaceWindow.h"
#include "ReactSkia/components/RSkComponent.h"

#include "include/core/SkString.h"

namespace facebook {
namespace react {

RSkSurfaceWindow::RSkSurfaceWindow(void *platformData)
    : backendType_(sk_app::Window::kNativeGL_BackendType) {
  window_.reset(sk_app::Window::CreateNativeWindow(platformData));
  window_->setRequestedDisplayParams(sk_app::DisplayParams());
  window_->attach(backendType_);
  RecreateWindowBackend();
}

RSkSurfaceWindow::~RSkSurfaceWindow() {
  window_->detach();
  window_.reset();
}

void RSkSurfaceWindow::ToggleBackendType() {
  if (backendType_ == sk_app::Window::kNativeGL_BackendType)
    backendType_ = sk_app::Window::kRaster_BackendType;
  else
    backendType_ = sk_app::Window::kNativeGL_BackendType;

  window_->detach();
  window_->attach(backendType_);
  RecreateWindowBackend();
}

LayoutConstraints RSkSurfaceWindow::GetLayoutConstraints() {
  Size windowSize{static_cast<Float>(window_->width()),
                  static_cast<Float>(window_->height())};
  return {windowSize, windowSize};
}

void RSkSurfaceWindow::AddComponent(std::shared_ptr<RSkComponent> component) {
  /* FIXME: components_ list is not been used as of now */
  // components_.push_back(component);
  window_->pushLayer(component.get());
}

void RSkSurfaceWindow::DeleteComponent(std::shared_ptr<RSkComponent> component) {
  window_->popLayer(window_->findLayer(component.get()));
}

void RSkSurfaceWindow::RecreateWindowBackend() {
  if (window_->sampleCount() > 1) {
    SkString title("Hello World ");
    title.append(
        sk_app::Window::kRaster_BackendType == GetBackendType() ? "Raster"
                                                                : "OpenGL");
    window_->setTitle(title.c_str());
  }
  window_->show();
  window_->inval();
}

} // namespace react
} // namespace facebook
