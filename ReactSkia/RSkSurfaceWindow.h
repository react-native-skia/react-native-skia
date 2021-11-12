#pragma once

#include "react/renderer/core/LayoutConstraints.h"

#include "ReactSkia/core_modules/RSkSpatialNavigator.h"
#include "rns_shell/compositor/Compositor.h"
#include "ReactSkia/core_modules/RSkInputEventManager.h"

#include <list>

namespace facebook {
namespace react {

using namespace SpatialNavigator;
using namespace RnsShell;

class RSkComponent;

class RSkSurfaceWindow {
 public:
  RSkSurfaceWindow();
  RSkSurfaceWindow(RSkSurfaceWindow &&) = default;
  RSkSurfaceWindow &operator=(RSkSurfaceWindow &&) = default;

  ~RSkSurfaceWindow();

  Compositor* compositor() { return compositor_.get(); }
  RSkSpatialNavigator* navigator() { return navigator_; }

  LayoutConstraints GetLayoutConstraints();

#ifdef RNS_SHELL_HAS_GPU_SUPPORT
// Interfaces to expose direct context of GPU backend
  static GrDirectContext* directContext;
  static void setDirectContext(GrDirectContext*);
  static GrDirectContext* getDirectContext(){return  RSkSurfaceWindow::directContext;};
#endif

 private:
  void RecreateWindowBackend();

 private:
  std::unique_ptr<Compositor> compositor_;
  RSkSpatialNavigator* navigator_;
  RSkInputEventManager* inputEventManager_;
};

} // namespace react
} // namespace facebook
