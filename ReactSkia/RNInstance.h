#pragma once

#include "cxxreact/Instance.h"

namespace facebook {
namespace react {

class ComponentViewRegistry;
class JSITurboModuleManager;
class MountingManager;
class Scheduler;
class RSkSurfaceWindow;

class RNInstance {
 public:
  RNInstance();
  ~RNInstance();
  RNInstance(RNInstance &&) = default;

  void Start(RSkSurfaceWindow *surface);
  void Stop();

 private:
  void InitializeJSCore();
  void InitializeFabric();
  void RegisterComponents();

 private:
  std::unique_ptr<Instance> instance_;
  std::unique_ptr<JSITurboModuleManager> turboModuleManager_;
  std::shared_ptr<Scheduler> fabricScheduler_;
  std::unique_ptr<MountingManager> mountingManager_;
  std::unique_ptr<ComponentViewRegistry> componentViewRegistry_;
};

} // namespace react
} // namespace facebook
