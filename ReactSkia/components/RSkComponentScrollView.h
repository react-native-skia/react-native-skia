/*
 * Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

#include "react/renderer/components/scrollview/ScrollViewShadowNode.h"

#include "ReactSkia/components/RSkComponent.h"
#include "ReactSkia/RSkSurfaceWindow.h"

namespace facebook {
namespace react {

enum ScrollDirectionType {
   ScrollDirectionForward = 1,
   ScrollDirectionBackward
};

class RSkComponentScrollView final : public RSkComponent {
 public:
  RSkComponentScrollView(const ShadowView &shadowView);

  RnsShell::LayerInvalidateMask updateComponentProps(
    const ShadowView &newShadowView,
    bool forceUpadate) override;

  RnsShell::LayerInvalidateMask updateComponentState(
    const ShadowView &newShadowView,
    bool forceUpadate) override;

  void onHandleKey(
    rnsKey  eventKeyType,
    bool* stopPropagate) override;

 protected:
  void OnPaint(SkCanvas *canvas) override;

 private:
  bool setScrollOffset(
    ScrollDirectionType scrollDirection,
    float containerLength,
    float frameLength,
    int &scrollOfffset);

  bool isHorizontalScroll();

};

} // namespace react
} // namespace facebook
