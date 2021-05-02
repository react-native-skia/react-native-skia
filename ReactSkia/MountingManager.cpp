#include "ReactSkia/MountingManager.h"
#include "ReactSkia/RSkSurfaceWindow.h"

#include "react/renderer/scheduler/Scheduler.h"
#include <glog/logging.h>

namespace facebook {
namespace react {

MountingManager::MountingManager(ComponentViewRegistry *componentViewRegistry)
    : componentViewRegistry_(componentViewRegistry) {}

void MountingManager::BindSurface(RSkSurfaceWindow *surface) {
  surface_ = surface;
}

void MountingManager::schedulerDidFinishTransaction(
    MountingCoordinator::Shared const &mountingCoordinator) {
  auto transaction = mountingCoordinator->pullTransaction();
  if (!transaction.has_value()) {
    return;
  }

  auto surfaceId = transaction->getSurfaceId();
  auto &mutations = transaction->getMutations();

  if (mutations.empty()) {
    return;
  }

  // auto telemetry = transaction->getTelemetry();
  // auto number = transaction->getNumber();

  ProcessMutations(mutations, surfaceId);
  surface_->SetNeedPainting();
}

void MountingManager::schedulerDidRequestPreliminaryViewAllocation(
    SurfaceId surfaceId,
    const ShadowView &shadowView) {
  // Does nothing for now.
}

void MountingManager::schedulerDidDispatchCommand(
    const ShadowView &shadowView,
    const std::string &commandName,
    const folly::dynamic args) {
  // TODO
}

void MountingManager::schedulerDidSetJSResponder(
    SurfaceId surfaceId,
    const ShadowView &shadowView,
    const ShadowView &initialShadowView,
    bool blockNativeResponder) {
  // Does nothing for now.
}

void MountingManager::schedulerDidClearJSResponder() {
  // Does nothing for now.
}

void MountingManager::ProcessMutations(
    ShadowViewMutationList const &mutations,
    SurfaceId surfaceId) {

  for (auto const &mutation : mutations) {
    switch (mutation.type) {
      case ShadowViewMutation::Create: {
        CreateMountInstruction(mutation, surfaceId);
        break;
      }
      case ShadowViewMutation::Delete: {
        DeleteMountInstruction(mutation, surfaceId);
        break;
      }
      case ShadowViewMutation::Insert: {
        InsertMountInstruction(mutation, surfaceId);
        break;
      }
      case ShadowViewMutation::Remove: {
        RemoveMountInstruction(mutation, surfaceId);
        break;
      }
      case ShadowViewMutation::Update: {
        UpdateMountInstruction(mutation, surfaceId);
        break;
      }
    }
  }
}


void MountingManager::CreateMountInstruction(
    ShadowViewMutation const &mutation,
    SurfaceId surfaceId) {

  auto provider = GetProvider(mutation.newChildShadowView);
  if (provider) {
    std::shared_ptr<RSkComponent> component =
        provider->CreateAndAddComponent(mutation.newChildShadowView);
    if (component) {
      surface_->AddComponent(component);
    }
  }
}

void MountingManager::DeleteMountInstruction(
    ShadowViewMutation const &mutation,
    SurfaceId surfaceId) {

  auto provider = GetProvider(mutation.oldChildShadowView);
  if (provider) {
      std::shared_ptr<RSkComponent> component = provider->GetComponent(mutation.oldChildShadowView.tag);
      if (component) {
          surface_->DeleteComponent(component);
          provider->DeleteComponent(mutation.oldChildShadowView.tag);
      }
  }
}

void MountingManager::InsertMountInstruction(
    ShadowViewMutation const &mutation,
    SurfaceId surfaceId) {

  std::shared_ptr<RSkComponent> newChildComponent = GetComponent(mutation.newChildShadowView);
  std::shared_ptr<RSkComponent> parentComponent = GetComponent(mutation.parentShadowView);
  if (newChildComponent) {
    newChildComponent->updateComponentData(
        surface_->width(),
        surface_->height(),
        mutation.newChildShadowView,
        ComponentUpdateMaskAll);
  }

  if (parentComponent) {
      parentComponent->mountChildComponent(newChildComponent,mutation.index);
  }

}

void MountingManager::RemoveMountInstruction(
    ShadowViewMutation const &mutation,
    SurfaceId surfaceId) {

  std::shared_ptr<RSkComponent> oldChildComponent = GetComponent(mutation.oldChildShadowView);
  std::shared_ptr<RSkComponent> parentComponent = GetComponent(mutation.parentShadowView);

  if (parentComponent) {
      parentComponent->unmountChildComponent(oldChildComponent,mutation.index);
  }

}

void MountingManager::UpdateMountInstruction(
    ShadowViewMutation const &mutation,
    SurfaceId surfaceId) {

  auto &oldChildShadowView = mutation.oldChildShadowView;
  auto &newChildShadowView = mutation.newChildShadowView;

  std::shared_ptr<RSkComponent> newChildComponent = GetComponent(mutation.newChildShadowView);
  if (newChildComponent) {
    if (oldChildShadowView.props != newChildShadowView.props)
      newChildComponent->updateComponentData(
          surface_->width(),
          surface_->height(),
          mutation.newChildShadowView,
          ComponentUpdateMaskProps);
    if (oldChildShadowView.state != newChildShadowView.state)
      newChildComponent->updateComponentData(
          surface_->width(),
          surface_->height(),
          mutation.newChildShadowView,
          ComponentUpdateMaskState);
    if (oldChildShadowView.eventEmitter != newChildShadowView.eventEmitter)
      newChildComponent->updateComponentData(
          surface_->width(),
          surface_->height(),
          mutation.newChildShadowView,
          ComponentUpdateMaskEventEmitter);
    if (oldChildShadowView.layoutMetrics != newChildShadowView.layoutMetrics)
      newChildComponent->updateComponentData(
          surface_->width(),
          surface_->height(),
          mutation.newChildShadowView,
          ComponentUpdateMaskLayoutMetrics);
  }
}

} // namespace react

} // namespace facebook
