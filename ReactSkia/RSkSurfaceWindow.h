#pragma once

#include "react/renderer/core/LayoutConstraints.h"
#include "sk_app/Window.h"

#include <list>

namespace facebook {
namespace react {

class RSkComponent;

class RSkSurfaceWindow {
 public:
  RSkSurfaceWindow(void *platformData);
  RSkSurfaceWindow(RSkSurfaceWindow &&) = default;
  RSkSurfaceWindow &operator=(RSkSurfaceWindow &&) = default;

  ~RSkSurfaceWindow();

  sk_app::Window *GetWindow() {
    return window_.get();
  }

  sk_app::Window::BackendType GetBackendType() const {
    return backendType_;
  }

  void ToggleBackendType();

  LayoutConstraints GetLayoutConstraints();

  // Components management
  void AddComponent(std::shared_ptr<RSkComponent> component);

 private:
  void RecreateWindowBackend();

 private:
  std::unique_ptr<sk_app::Window> window_;
  sk_app::Window::BackendType backendType_;
  std::list<std::shared_ptr<RSkComponent>> components_;
};

} // namespace react
} // namespace facebook
