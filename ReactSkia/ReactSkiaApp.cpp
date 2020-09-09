#include "ReactSkiaApp.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkGraphics.h"
#include "include/core/SkSurface.h"
#include "include/effects/SkGradientShader.h"

using namespace sk_app;

Application *Application::Create(int argc, char **argv, void *platformData) {
  return new ReactSkiaApp(argc, argv, platformData);
}

ReactSkiaApp::ReactSkiaApp(int argc, char **argv, void *platformData) {
  rnInstance_ = std::make_unique<facebook::react::RNInstance>();
  surface_ = std::make_unique<facebook::react::RSkSurfaceWindow>(platformData);
  SkGraphics::Init();
  rnInstance_->Start(surface_.get());
}

ReactSkiaApp::~ReactSkiaApp() {}

void ReactSkiaApp::onIdle() {
  // Just re-paint continously
  surface_->GetWindow()->inval();
}
