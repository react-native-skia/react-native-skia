#include "ReactSkia/RSkSurfaceWindow.h"
#include "ReactSkia/components/RSkComponent.h"

#include "include/core/SkString.h"

using namespace RnsShell;

namespace facebook {
namespace react {

#ifdef RNS_SHELL_HAS_GPU_SUPPORT
GrDirectContext* RSkSurfaceWindow::directContext=nullptr;
#endif

RSkSurfaceWindow::RSkSurfaceWindow() {
  SkRect viewPort(SkRect::MakeEmpty());
  compositor_ = Compositor::create(viewPort);
#ifdef RNS_SHELL_HAS_GPU_SUPPORT
  if(compositor_) {
    setDirectContext(compositor_->getDirectContext());
  }
#endif
  navigator_ = RSkSpatialNavigator::sharedSpatialNavigator();
}

RSkSurfaceWindow::~RSkSurfaceWindow() {
  compositor_->invalidate();
  compositor_.reset();
}

LayoutConstraints RSkSurfaceWindow::GetLayoutConstraints() {
  Size windowSize{static_cast<Float>(compositor_->viewport().width()),
                  static_cast<Float>(compositor_->viewport().height())};
  return {windowSize, windowSize};
}

#ifdef RNS_SHELL_HAS_GPU_SUPPORT
void RSkSurfaceWindow::setDirectContext(GrDirectContext* context) {
  RSkSurfaceWindow::directContext=context;
}
#endif

} // namespace react
} // namespace facebook
