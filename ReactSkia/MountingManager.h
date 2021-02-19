#pragma once

#include "react/renderer/mounting/ShadowViewMutation.h"
#include "react/renderer/scheduler/SchedulerDelegate.h"

namespace facebook {
namespace react {

class ComponentViewRegistry;
class RSkSurfaceWindow;

class MountingManager : public SchedulerDelegate {
 public:
  MountingManager(ComponentViewRegistry *componentViewRegistry);
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

 private:
  ComponentViewRegistry *componentViewRegistry_;
  RSkSurfaceWindow *surface_;
};

} // namespace react
} // namespace facebook
