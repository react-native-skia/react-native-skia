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
  RNS_LOG_TODO("Need to come up with proper way to set surfaceId, moduleName and properties members");
  surfaceId = 1;
  moduleName = "SimpleViewApp";
  properties = folly::dynamic::object();

}

RSkSurfaceWindow::~RSkSurfaceWindow() {
}

void RSkSurfaceWindow::setSize(SkSize size) {
  RNS_LOG_INFO("Set Layout MinMax Size : " << size.width() << "x" << size.height());
  minimumSize = maximumSize = size;
}

#ifdef RNS_SHELL_HAS_GPU_SUPPORT
void RSkSurfaceWindow::setDirectContext(GrDirectContext* context) {
  RSkSurfaceWindow::directContext=context;
}
#endif

} // namespace react
} // namespace facebook
