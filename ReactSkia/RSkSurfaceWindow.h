#pragma once

#include "react/renderer/core/LayoutConstraints.h"

#include "rns_shell/compositor/Compositor.h"

#include <list>

namespace facebook {
namespace react {

class RSkComponent;

class RSkSurfaceWindow {
 public:
  RSkSurfaceWindow();
  RSkSurfaceWindow(RSkSurfaceWindow &&) = default;
  RSkSurfaceWindow &operator=(RSkSurfaceWindow &&) = default;

  ~RSkSurfaceWindow();

  RnsShell::Compositor* compositor() { return compositor_.get(); }

  LayoutConstraints GetLayoutConstraints();

 private:
  void RecreateWindowBackend();

 private:
  std::unique_ptr<RnsShell::Compositor> compositor_;
};

} // namespace react
} // namespace facebook
