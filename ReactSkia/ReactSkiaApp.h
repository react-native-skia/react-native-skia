#pragma once

#include "ReactSkia/RNInstance.h"
#include "ReactSkia/RSkSurfaceWindow.h"

#include "rns_shell/common/Application.h"

class SkCanvas;

namespace facebook {
namespace react {

class ReactSkiaApp : public RnsShell::Application {
 public:

  ReactSkiaApp(int argc, char **argv);
  ~ReactSkiaApp();

  void onIdle();
  void onResize(SkSize newSize);

 private:
  std::unique_ptr<facebook::react::RNInstance> rnInstance_;
  std::unique_ptr<facebook::react::RSkSurfaceWindow> surface_;
};

} // namespace react
} // namespace facebook
