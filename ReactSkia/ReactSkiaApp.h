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

  static facebook::react::RNInstance *currentBridge() {
    return currentBridgeInstance;
  }
  static void setCurrentBridge(facebook::react::RNInstance *instance) {
    currentBridgeInstance = instance;
  }

  void onIdle();
  void onResize(SkSize newSize);

  static facebook::react::RNInstance *currentBridgeInstance;
 private:
  std::unique_ptr<facebook::react::RNInstance> rnInstance_;
  std::unique_ptr<facebook::react::RSkSurfaceWindow> surface_;
};

} // namespace react
} // namespace facebook
