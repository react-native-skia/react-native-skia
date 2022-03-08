#pragma once

#include "react/renderer/mounting/ShadowViewMutation.h"
#include "react/renderer/scheduler/SchedulerDelegate.h"
#include "ReactSkia/ComponentViewRegistry.h"

namespace RnsShell {
  class RendererDelegate;
}

namespace facebook {
namespace react {

using namespace RnsShell;

class ComponentViewRegistry;
class RSkSurfaceWindow;
class ReactSkiaApp;

class MountingManager : public SchedulerDelegate {
 public:
  MountingManager(ComponentViewRegistry *componentViewRegistry, RendererDelegate &rendererDelegate);
  MountingManager(MountingManager &&) = default;

  void BindSurface(RSkSurfaceWindow *surface);

 private:
  // SchedulerDelegate implementations
  void schedulerDidFinishTransaction(
      MountingCoordinator::Shared const &mountingCoordinator) override;

  void schedulerDidRequestPreliminaryViewAllocation(
      SurfaceId surfaceId,
      const ShadowView &shadowView) override;

  void schedulerDidDispatchCommand(
      const ShadowView &shadowView,
      const std::string &commandName,
      const folly::dynamic args) override;

  void schedulerDidSetJSResponder(
      SurfaceId surfaceId,
      const ShadowView &shadowView,
      const ShadowView &initialShadowView,
      bool blockNativeResponder) override;

  void schedulerDidClearJSResponder() override;

 private:
  void ProcessMutations(
      ShadowViewMutationList const &mutations,
      SurfaceId surfaceId);

  // `Create` instruction
  void CreateMountInstruction(
      ShadowViewMutation const &mutation,
      SurfaceId surfaceId);

  // `Delete` instruction
  void DeleteMountInstruction(
      ShadowViewMutation const &mutation,
      SurfaceId surfaceId);

  // `Insert` instruction
  void InsertMountInstruction(
      ShadowViewMutation const &mutation,
      SurfaceId surfaceId);

  // `Remove` instruction
  void RemoveMountInstruction(
      ShadowViewMutation const &mutation,
      SurfaceId surfaceId);

  // `Update` instruction
  void UpdateMountInstruction(
      ShadowViewMutation const &mutation,
      SurfaceId surfaceId);

  RSkComponentProvider *GetProvider (const ShadowView &shadowView) {
      return componentViewRegistry_->GetProvider(shadowView.componentHandle);
  }

  std::shared_ptr<RSkComponent> GetComponent (const ShadowView &shadowView) {
      auto provider = GetProvider(shadowView);
      if (provider) {
          return provider->GetComponent(shadowView.tag);
      }
      return nullptr;
  }


 private:
  RendererDelegate& nativeRenderDelegate_;
  ComponentViewRegistry *componentViewRegistry_;
  RSkSurfaceWindow *surface_;
};

} // namespace react
} // namespace facebook
