#pragma once

#include "react/renderer/mounting/ShadowViewMutation.h"
#include "react/renderer/scheduler/SchedulerDelegate.h"
#include "ReactSkia/ComponentViewRegistry.h"

namespace RnsShell {
class RendererDelegate;
} // namespace RnsShell

namespace facebook {
namespace react {

class ComponentViewRegistry;
class RSkSurfaceWindow;
class ReactSkiaApp;

class MountingManager : public SchedulerDelegate {
 public:
  MountingManager(ComponentViewRegistry *componentViewRegistry, RnsShell::RendererDelegate &rendererDelegate);
  MountingManager(MountingManager &&);

  void BindSurface(RSkSurfaceWindow *surface);

 private:
  // SchedulerDelegate implementations
  void schedulerDidFinishTransaction(
      MountingCoordinator::Shared const &mountingCoordinator) override;

  void schedulerDidRequestPreliminaryViewAllocation(
      SurfaceId surfaceId,
      const ShadowNode &shadowView) override;

  void schedulerDidDispatchCommand(
      const ShadowView &shadowView,
      const std::string &commandName,
      const folly::dynamic &args) override;

  void schedulerDidSendAccessibilityEvent(
      const ShadowView &shadowView,
      const std::string &eventType) override;

  void schedulerDidSetIsJSResponder(
      const ShadowView &shadowView,
      bool isJSResponder,
      bool blockNativeResponder) override;

 private:
  void performTransaction(
    MountingCoordinator::Shared const &mountingCoordinator);

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
  RnsShell::RendererDelegate& nativeRenderDelegate_;
  ComponentViewRegistry *componentViewRegistry_;
  RSkSurfaceWindow *surface_;
  std::atomic <bool> followUpTransactionRequired_{false};
  std::atomic <bool> transactionInFlight_{false};
};

} // namespace react
} // namespace facebook
