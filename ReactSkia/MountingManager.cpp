#include "ReactSkia/MountingManager.h"
#include "ReactSkia/RSkSurfaceWindow.h"

#include "react/renderer/scheduler/Scheduler.h"
#include <glog/logging.h>
#include "ReactSkia/utils/RnsLog.h"
#include <iostream>
using namespace std;
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
    printf("\n\n=====================\n");
    cout<<"-----------------------shadowView tag"<<shadowView.tag<<endl;
    cout<<"-----------------------shadowView componentName"<<shadowView.componentName<<endl;
    cout<<"-----------------------shadowView commandName"<<commandName<<endl;
    RNS_LOG_INFO("**********************************");
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

  surface_->compositor()->begin();

    for (auto const &mutation : mutations) {
    printf("\n\n=====================\n");
    printf("Mutation type [%d] Index [%d]\n",mutation.type,mutation.index);
    printf("Parent ShadowView Tag [%d] Name [%s] \n",mutation.parentShadowView.tag,mutation.parentShadowView.componentName);
    printf("OldChild ShadowView Tag [%d] Name [%s] \n",mutation.oldChildShadowView.tag,mutation.oldChildShadowView.componentName);
    printf("NewChild ShadowView Tag [%d] Name [%s] \n",mutation.newChildShadowView.tag,mutation.newChildShadowView.componentName);
    printf("=====================\n\n");
    }
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

#if !defined(GOOGLE_STRIP_LOG) || (GOOGLE_STRIP_LOG <= INFO)
  static double prevTime = SkTime::GetMSecs();
  RNS_LOG_INFO_EVERY_N(60, "Calling Compositor Commit(" << std::this_thread::get_id()) << ") : after " << SkTime::GetMSecs() - prevTime << " ms";
  prevTime = SkTime::GetMSecs();
#endif
  surface_->compositor()->commit();
}


void MountingManager::CreateMountInstruction(
    ShadowViewMutation const &mutation,
    SurfaceId surfaceId) {
  auto provider = GetProvider(mutation.newChildShadowView);
  if (provider) {
    std::shared_ptr<RSkComponent> component =
        provider->CreateAndAddComponent(mutation.newChildShadowView);
    component.get()->requiresLayer(mutation.newChildShadowView);
  }
}

void MountingManager::DeleteMountInstruction(
    ShadowViewMutation const &mutation,
    SurfaceId surfaceId) {

  auto provider = GetProvider(mutation.oldChildShadowView);
  if (provider) {
      std::shared_ptr<RSkComponent> component = provider->GetComponent(mutation.oldChildShadowView.tag);
      if (component) {
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
      newChildComponent->updateComponentData(mutation.newChildShadowView,ComponentUpdateMaskAll);
  }

  if (parentComponent) {
      parentComponent->mountChildComponent(newChildComponent,mutation.index);
      surface_->navigator()->addToNavList(newChildComponent);
  }

}

void MountingManager::RemoveMountInstruction(
    ShadowViewMutation const &mutation,
    SurfaceId surfaceId) {

  std::shared_ptr<RSkComponent> oldChildComponent = GetComponent(mutation.oldChildShadowView);
  std::shared_ptr<RSkComponent> parentComponent = GetComponent(mutation.parentShadowView);

  if (parentComponent) {
      parentComponent->unmountChildComponent(oldChildComponent,mutation.index);
      surface_->navigator()->removeFromNavList(oldChildComponent);
  }

}

void MountingManager::UpdateMountInstruction(
    ShadowViewMutation const &mutation,
    SurfaceId surfaceId) {

  auto &oldChildShadowView = mutation.oldChildShadowView;
  auto &newChildShadowView = mutation.newChildShadowView;

  std::shared_ptr<RSkComponent> newChildComponent = GetComponent(mutation.newChildShadowView);
  if(newChildComponent)
  {
       if(oldChildShadowView.props != newChildShadowView.props) {
           newChildComponent->updateComponentData(mutation.newChildShadowView,ComponentUpdateMaskProps);
           surface_->navigator()->updateInNavList(newChildComponent); //TODO only if TV related proeprties have changed ?
       }
       if(oldChildShadowView.state != newChildShadowView.state)
           newChildComponent->updateComponentData(mutation.newChildShadowView,ComponentUpdateMaskState);
       if(oldChildShadowView.eventEmitter != newChildShadowView.eventEmitter)
           newChildComponent->updateComponentData(mutation.newChildShadowView,ComponentUpdateMaskEventEmitter);
       if(oldChildShadowView.layoutMetrics != newChildShadowView.layoutMetrics)
           newChildComponent->updateComponentData(mutation.newChildShadowView,ComponentUpdateMaskLayoutMetrics);
#if USE(RNS_SHELL_PARTIAL_UPDATES)
       surface_->compositor()->addDamageRect(newChildComponent->layer().get()->getFrame());
#endif
  }
}

} // namespace react

} // namespace facebook
