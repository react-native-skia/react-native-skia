#include "ReactSkiaApp.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkSurface.h"
#include "include/effects/SkGradientShader.h"

#include "ReactSkia/views/common/RSkImageCacheManager.h"

using namespace RnsShell;
using namespace facebook::react;

Application *Application::Create(int argc, char **argv) {
  return new ReactSkiaApp(argc, argv);
}

namespace facebook {
namespace react {

ReactSkiaApp::ReactSkiaApp(int argc, char **argv) {
  surface_ = std::make_unique<facebook::react::RSkSurfaceWindow>();
  surface_->setSize(viewPort());
#ifdef RNS_SHELL_HAS_GPU_SUPPORT
  surface_->setDirectContext(graphicsDirectContext());
#endif
  rnInstance_ = std::make_unique<facebook::react::RNInstance>(*this);
  rnInstance_->Start(surface_.get(), *this);

  RSkImageCacheManager::init();//Needs to be called after Gpu backend created,So calling here
}

ReactSkiaApp::~ReactSkiaApp() {
  rnInstance_->Stop(surface_.get());
}

void ReactSkiaApp::onIdle() {
  RNS_LOG_NOT_IMPL;
}

void ReactSkiaApp::onResize(SkSize newSize) {
  surface_->setSize(newSize);
}

} // namespace react
} // namespace facebook
