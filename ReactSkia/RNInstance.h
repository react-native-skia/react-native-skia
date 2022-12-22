#pragma once

#include "cxxreact/Instance.h"
#include "cxxreact/CxxModule.h"

namespace RnsShell {
  class RendererDelegate;
}

namespace facebook {
namespace react {

using namespace RnsShell;

class ComponentViewRegistry;
class JSITurboModuleManager;
class MessageQueueThreadImpl;
class MountingManager;
class RSkSurfaceWindow;
class Scheduler;

class RNInstance {
 public:
  RNInstance(RendererDelegate &rendererDelegate);
  ~RNInstance();
  RNInstance(RNInstance &&) = default;

  void Start(RSkSurfaceWindow *surface, RendererDelegate &rendererDelegate);
  void Stop(RSkSurfaceWindow *surface);
  xplat::module::CxxModule* moduleForName(std::string moduleName);

  static jsi::Runtime* RskJsRuntime();

 private:
  void InitializeJSCore();
  void InitializeFabric(RendererDelegate &rendererDelegate);
  void RegisterComponents();
  void Invalidate();

 private:
  std::shared_ptr<Instance> instance_;
  std::unique_ptr<JSITurboModuleManager> turboModuleManager_;
  std::shared_ptr<MessageQueueThreadImpl> moduleMessageQueue_;
  std::shared_ptr<ModuleRegistry> moduleRegistry_;
  std::shared_ptr<Scheduler> fabricScheduler_;
  std::unique_ptr<MountingManager> mountingManager_;
  std::unique_ptr<ComponentViewRegistry> componentViewRegistry_;

  static jsi::Runtime *jsRuntime_;
};

} // namespace react
} // namespace facebook
