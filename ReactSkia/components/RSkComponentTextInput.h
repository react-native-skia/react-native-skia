/*
 * Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

#include "ReactSkia/components/RSkComponent.h"
#include "react/renderer/components/textinput/TextInputEventEmitter.h"
namespace facebook {
namespace react {

class RSkComponentTextInput final : public RSkComponent {
 public:
  RSkComponentTextInput(const ShadowView &shadowView);
  void onHandleKey(rnsKey  eventKeyType,bool *stopPropagate)override;
  RnsShell::LayerInvalidateMask updateComponentProps(const ShadowView &newShadowView,bool forceUpdate) override;
  /*
    TODO  Need to Add command function to Handle Command.
  */
 protected:
  void OnPaint(SkCanvas *canvas) override;
 private:
  bool isInEditingMode_;
  std::string displayString_{}; // Text to be displayed on screen
  std::string placeholderString_{}; // Placeholder Text
  SharedColor placeholderColor_;  // Placeholder Text Color
};

} // namespace react
} // namespace facebook
