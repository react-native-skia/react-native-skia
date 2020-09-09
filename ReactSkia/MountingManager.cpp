#include "ReactSkia/MountingManager.h"
#include "ReactSkia/ComponentViewRegistry.h"
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
        // DeleteMountInstruction(mutation, registry, observerCoordinator,
        // surfaceId);
        break;
      }
      case ShadowViewMutation::Insert: {
        // RNUpdatePropsMountInstruction(mutation, registry);
        // RNUpdateEventEmitterMountInstruction(mutation, registry);
        // RNUpdateStateMountInstruction(mutation, registry);
        // RNUpdateLayoutMetricsMountInstruction(mutation, registry);
        // RNFinalizeUpdatesMountInstruction(mutation,
        // RNComponentViewUpdateMaskAll, registry);
        // RNInsertMountInstruction(mutation, registry);
        break;
      }
      case ShadowViewMutation::Remove: {
        // RNRemoveMountInstruction(mutation, registry);
        break;
      }
      case ShadowViewMutation::Update: {
        // auto const &oldChildShadowView = mutation.oldChildShadowView;
        // auto const &newChildShadowView = mutation.newChildShadowView;
        //
        // auto mask = RNComponentViewUpdateMask{};
        //
        // if (oldChildShadowView.props != newChildShadowView.props) {
        //   RNUpdatePropsMountInstruction(mutation, registry);
        //   mask |= RNComponentViewUpdateMaskProps;
        // }
        // if (oldChildShadowView.eventEmitter !=
        // newChildShadowView.eventEmitter) {
        //   RNUpdateEventEmitterMountInstruction(mutation, registry);
        //   mask |= RNComponentViewUpdateMaskEventEmitter;
        // }
        // if (oldChildShadowView.state != newChildShadowView.state) {
        //   RNUpdateStateMountInstruction(mutation, registry);
        //   mask |= RNComponentViewUpdateMaskState;
        // }
        // if (oldChildShadowView.layoutMetrics !=
        // newChildShadowView.layoutMetrics) {
        //   RNUpdateLayoutMetricsMountInstruction(mutation, registry);
        //   mask |= RNComponentViewUpdateMaskLayoutMetrics;
        // }
        //
        // if (mask != RNComponentViewUpdateMaskNone) {
        //   RNFinalizeUpdatesMountInstruction(mutation, mask, registry);
        // }

        break;
      }
    }
  }
}

void MountingManager::CreateMountInstruction(
    ShadowViewMutation const &mutation,
    SurfaceId surfaceId) {
  auto componentHandle = mutation.newChildShadowView.componentHandle;
  // auto tag = mutation.newChildShadowView.tag;
  RSkComponentProvider *provider =
      componentViewRegistry_->GetProvider(componentHandle);
  if (provider) {
    std::shared_ptr<RSkComponent> component =
        provider->CreateComponent(mutation.newChildShadowView);
    if (component) {
      surface_->AddComponent(component);
    }
  }
}

} // namespace react
} // namespace facebook
