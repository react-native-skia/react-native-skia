#pragma once

#include "cxxreact/Instance.h"

#include "react/renderer/core/LayoutConstraints.h"
#include "react/renderer/core/ReactPrimitives.h"

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

  void setSize(SkSize size);

#ifdef RNS_SHELL_HAS_GPU_SUPPORT
// Interfaces to expose direct context of GPU backend
  static GrDirectContext* directContext;
  static GrDirectContext* getDirectContext(){return  RSkSurfaceWindow::directContext;};
  void setDirectContext(GrDirectContext*);
#endif

  SurfaceId surfaceId;
  std::string moduleName;
  folly::dynamic properties;
  SkSize minimumSize {0, 0};
  SkSize maximumSize {1920, 1080};
  SkPoint viewportOffset {0, 0};

 private:
  void RecreateWindowBackend();

 private:
};

} // namespace react
} // namespace facebook
