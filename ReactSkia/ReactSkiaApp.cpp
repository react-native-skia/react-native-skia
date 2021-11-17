#include "ReactSkiaApp.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkGraphics.h"
#include "include/core/SkSurface.h"
#include "include/effects/SkGradientShader.h"

#include "ReactSkia/views/common/RSkImageCacheManager.h"

using namespace RnsShell;

Application *Application::Create(int argc, char **argv) {
  return new ReactSkiaApp(argc, argv);
}

ReactSkiaApp::ReactSkiaApp(int argc, char **argv) {
  rnInstance_ = std::make_unique<facebook::react::RNInstance>();
  surface_ = std::make_unique<facebook::react::RSkSurfaceWindow>();
  SkGraphics::Init();
  facebook::react::RSkImageCacheManager::configure();//Needs to be called after Gpu backend created,So calling here
  rnInstance_->Start(surface_.get());
}

ReactSkiaApp::~ReactSkiaApp() {
  rnInstance_->Stop();
}

void ReactSkiaApp::onIdle() {
  RNS_LOG_NOT_IMPL;
}

void ReactSkiaApp::onResize(int width, int height) {
  surface_->compositor()->setViewportSize(SkRect::MakeWH(width, height));
}
