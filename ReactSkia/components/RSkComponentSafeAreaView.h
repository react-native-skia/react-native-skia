/*
* Copyright (C) 1994-2023 OpenTV, Inc. and Nagravision S.A.
*
* This source code is licensed under the MIT license found in the
* LICENSE file in the root directory of this source tree.
*/

#pragma once

#include "pluginfactory/RnsPlugin.h"
#include "pluginfactory/RnsPluginFactory.h"

#include "react/renderer/components/safeareaview/SafeAreaViewShadowNode.h"
#include "react/renderer/components/safeareaview/SafeAreaViewState.h"
#include "ReactSkia/components/RSkComponent.h"

namespace facebook {
namespace react {

using namespace rnsplugin;

class RSkComponentSafeAreaView final : public RSkComponent {
 public:
  RSkComponentSafeAreaView(const ShadowView &shadowView);
  RnsShell::LayerInvalidateMask updateComponentProps(SharedProps newviewProps,bool forceUpdate) override;
  RnsShell::LayerInvalidateMask updateComponentState(const State::Shared &newState, bool forceUpadate) override;
  //CallBackClient for Platform Events
  class PlatformCallBackClient : public RNSPlatformManagerInterface::CallbackClient {
   public:
    PlatformCallBackClient(RSkComponentSafeAreaView &safeAreaViewComp);
    ~PlatformCallBackClient(){}

    void onSafeAreaInsetsDidChange() override;

   private:
    RSkComponentSafeAreaView &safeAreaViewComp_;
  };
 private:
  void updateStateIfNecessary();
  SkRect safeAreaInsets();
  void safeAreaInsetsDidChange();
  RnsPluginFactory *pluginFactory_;
  PlatformCallBackClient platformCallBackClient_;
  std::unique_ptr<RNSPlatformManagerInterface> platformManagerHandle_;
 protected:
  void OnPaint(SkCanvas *canvas) override;

  SafeAreaViewShadowNode::ConcreteStateTellerT stateTeller_;
  EdgeInsets lastPaddingStateWasUpdatedWith_;
};

} // namespace react
} // namespace facebook
