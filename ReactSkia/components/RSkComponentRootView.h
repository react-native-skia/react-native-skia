/*
* Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
* Copyright (C) Kudo Chien
*
* This source code is licensed under the MIT license found in the
* LICENSE file in the root directory of this source tree.
*/

#pragma once

#include "ReactSkia/components/RSkComponent.h"

namespace facebook {
namespace react {

class RSkComponentRootView final : public RSkComponent {
 public:
  RSkComponentRootView(const ShadowView &shadowView);
  RnsShell::LayerInvalidateMask updateComponentProps(SharedProps newviewProps,bool forceUpdate) override;

  virtual bool isContainer() const override { return true; }
  virtual bool isVisible(RSkComponent* candidate) override;

 protected:
  void OnPaint(SkCanvas *canvas) override;
};

} // namespace react
} // namespace facebook
