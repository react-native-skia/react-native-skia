#pragma once

#include "ReactSkia/RNInstance.h"
#include "ReactSkia/RSkSurfaceWindow.h"
#include "sk_app/Application.h"

class SkCanvas;

class ReactSkiaApp : public sk_app::Application {
 public:
  ReactSkiaApp(int argc, char **argv, void *platformData);
  ~ReactSkiaApp() override;

  void onIdle() override;

 private:
  std::unique_ptr<facebook::react::RNInstance> rnInstance_;
  std::unique_ptr<facebook::react::RSkSurfaceWindow> surface_;
};
