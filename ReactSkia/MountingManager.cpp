/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 * Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <glog/logging.h>

#include "build/build_config.h"
#include "react/renderer/scheduler/Scheduler.h"
#include "ReactSkia/MountingManager.h"
#include "ReactSkia/RSkSurfaceWindow.h"
#include "rns_shell/compositor/RendererDelegate.h"

#if BUILDFLAG(IS_MAC)
#include "rns_shell/platform/mac/TaskLoop.h"
#elif BUILDFLAG(IS_LINUX)
#include "rns_shell/platform/linux/TaskLoop.h"
#endif

namespace facebook {
namespace react {

MountingManager::MountingManager(ComponentViewRegistry *componentViewRegistry, RnsShell::RendererDelegate &rendererDelegate)
    : nativeRenderDelegate_(rendererDelegate),
      componentViewRegistry_(componentViewRegistry) {}

void MountingManager::BindSurface(RSkSurfaceWindow *surface) {
  surface_ = surface;
}

void MountingManager::schedulerDidFinishTransaction(
    MountingCoordinator::Shared const &mountingCoordinator) {

  RNS_LOG_DEBUG("schedulerDidFinishTransaction transactionInFlight[" << transactionInFlight_ << "] followUpTransactionRequired["<< followUpTransactionRequired_ << "]");
  /* If transaction processing ongoing,set flag for followUp and return*/
  if(transactionInFlight_){
    followUpTransactionRequired_ = true;
    return;
  }

  /* Set the flag when schedule for transaction processing*/
  transactionInFlight_ = true;
  RnsShell::TaskLoop::main().dispatch([&,mountingCoordinator]() {
     performTransaction(mountingCoordinator);
  });

}

void MountingManager::schedulerDidRequestPreliminaryViewAllocation(
    SurfaceId surfaceId,
    const ShadowNode &shadowView) {
  /* TODO : Needs implementation*/
  RNS_LOG_DEBUG("surfaceId:" << surfaceId << " shadowView tag[" << shadowView.getTag() <<  "] name["<< shadowView.getComponentName() << "]");
}

void MountingManager::schedulerDidDispatchCommand(
    const ShadowView &shadowView,
    const std::string &commandName,
    const folly::dynamic &args) {

  RNS_LOG_DEBUG("dispatch command shadowView tag[" << shadowView.tag <<  "] name["<< shadowView.componentName << "] commandName [" << commandName <<"]");

  RnsShell::TaskLoop::main().dispatch([&,shadowView,commandName,args]() {
     auto component = GetComponent(shadowView);
     if( component != NULL ){
       component->handleCommand(commandName,args);
     }
  });
}

void MountingManager::schedulerDidSendAccessibilityEvent(
    const ShadowView &shadowView,
    const std::string &eventType) {
  RNS_LOG_NOT_IMPL;
}

void MountingManager::schedulerDidSetIsJSResponder(
    const ShadowView &shadowView,
    bool isJSResponder,
    bool blockNativeResponder) {
  RNS_LOG_NOT_IMPL;
  RNS_LOG_TODO("shadowView tag[" << shadowView.tag <<  "] name["<< shadowView.componentName << "] isJSResponder=[" << isJSResponder << "] blockNativeResponder: " << blockNativeResponder);
}

void MountingManager::performTransaction(
    MountingCoordinator::Shared const &mountingCoordinator) {

  do {
    followUpTransactionRequired_ = false;
    transactionInFlight_ = true;

    auto surfaceId = mountingCoordinator->getSurfaceId();
    mountingCoordinator->getTelemetryController().pullTransaction(
      [](MountingTransaction const &transaction, SurfaceTelemetry const &surfaceTelemetry) {
          RNS_LOG_DEBUG("[TODO] : TelemetryController PullTransaction WillMount");
      },
      [&,surfaceId](MountingTransaction const &transaction, SurfaceTelemetry const &surfaceTelemetry) {
          ProcessMutations(transaction.getMutations(), surfaceId);
      },
      [](MountingTransaction const &transaction, SurfaceTelemetry const &surfaceTelemetry) {
          RNS_LOG_DEBUG("[TODO] : TelemetryController PullTransaction DidMount");
      });

     transactionInFlight_ = false;
   } while (followUpTransactionRequired_);

}


void MountingManager::ProcessMutations(
    ShadowViewMutationList const &mutations,
    SurfaceId surfaceId) {

  if(mutations.empty()) return;

  RNS_LOG_DEBUG(" ProcessMutations mutations[" << mutations.size() <<"]");

  nativeRenderDelegate_.begin();

  for (auto const &mutation : mutations) {
    RNS_LOG_DEBUG("\n============\n Mutation type : "<< mutation.type <<
                 "\n ParentShadowView" <<
                 "\n\tTag:" << mutation.parentShadowView.tag <<
                 "\n\tName:" << (mutation.parentShadowView.componentName?mutation.parentShadowView.componentName : "null") <<
                 "\n OldChildShadowView" <<
                 "\n\tTag:" << mutation.oldChildShadowView.tag <<
                 "\n\tName:" << (mutation.oldChildShadowView.componentName?mutation.oldChildShadowView.componentName : "null") <<
                 "\n NewChildShadowView" <<
                 "\n\tTag:" << mutation.newChildShadowView.tag <<
                 "\n\tName:" << (mutation.newChildShadowView.componentName?mutation.newChildShadowView.componentName : "null") <<
                 "\n Mutation index : "<< mutation.index <<
                 "\n============\n");
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
      case ShadowViewMutation::RemoveDeleteTree: {
        // TODO - not supported yet
        break;
      }
    }
  }

#if !defined(GOOGLE_STRIP_LOG) || (GOOGLE_STRIP_LOG <= INFO)
  static double prevTime = SkTime::GetMSecs();
  RNS_LOG_INFO_EVERY_N(60, "Calling Compositor Commit(" << std::this_thread::get_id()) << ") : after " << SkTime::GetMSecs() - prevTime << " ms";
  prevTime = SkTime::GetMSecs();
#endif
  nativeRenderDelegate_.commit(true);

}

void MountingManager::CreateMountInstruction(
    ShadowViewMutation const &mutation,
    SurfaceId surfaceId) {
  auto provider = GetProvider(mutation.newChildShadowView);
  if (provider) {
    std::shared_ptr<RSkComponent> component =
        provider->CreateAndAddComponent(mutation.newChildShadowView);
    component.get()->requiresLayer(mutation.newChildShadowView, nativeRenderDelegate_);
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
      newChildComponent->updateComponentData(mutation.newChildShadowView,ComponentUpdateMaskAll,true);
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
  uint32_t updateMask = ComponentUpdateMaskNone;
  std::shared_ptr<RSkComponent> newChildComponent = GetComponent(mutation.newChildShadowView);
  if(newChildComponent) {
    if(oldChildShadowView.props != newChildShadowView.props)
      updateMask |= ComponentUpdateMaskProps;
    if(oldChildShadowView.state != newChildShadowView.state)
      updateMask |= ComponentUpdateMaskState;
    if(oldChildShadowView.eventEmitter != newChildShadowView.eventEmitter)
      updateMask |= ComponentUpdateMaskEventEmitter;
    if(oldChildShadowView.layoutMetrics != newChildShadowView.layoutMetrics)
      updateMask |= ComponentUpdateMaskLayoutMetrics;

    if(updateMask != ComponentUpdateMaskNone)
      newChildComponent->updateComponentData(mutation.newChildShadowView,updateMask,false);
  }
}

} // namespace react

} // namespace facebook
