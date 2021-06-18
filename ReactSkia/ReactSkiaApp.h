#pragma once

#include "ReactSkia/RNInstance.h"
#include "ReactSkia/RSkSurfaceWindow.h"

#include "rns_shell/common/Application.h"

class SkCanvas;

class ReactSkiaApp : public RnsShell::Application {
 public:

  ReactSkiaApp(int argc, char **argv);
  ~ReactSkiaApp();

  void onIdle();
  void onResize(int width, int height);

 private:
  std::unique_ptr<facebook::react::RNInstance> rnInstance_;
  std::unique_ptr<facebook::react::RSkSurfaceWindow> surface_;
};
