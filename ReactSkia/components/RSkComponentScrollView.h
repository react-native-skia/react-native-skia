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

  //RSkComponent override functions
  RnsShell::LayerInvalidateMask updateComponentProps(
    const ShadowView &newShadowView,
    bool forceUpadate) override;

  RnsShell::LayerInvalidateMask updateComponentState(
    const ShadowView &newShadowView,
    bool forceUpadate) override;

  bool isContainer() const override { return true; }

  //RSkSpatialNavigatorContainer override functions
  bool canScrollInDirection(rnsKey direction) override;
  bool isVisible(RSkComponent* candidate) override;
  bool isScrollable() override { return true;}
  SkPoint getScrollOffset() override;

  ScrollStatus scrollInDirection(
    RSkComponent* candidate,
    rnsKey direction) override;

 protected:
  void OnPaint(SkCanvas *canvas) override;

 private:
  bool scrollEnabled_{true};
  std::vector<int> snapToOffsets_;

  void calculateNextScrollOffset(
    ScrollDirectionType scrollDirection,
    int containerLength,
    int frameLength,
    int &scrollOfffset);

  SkPoint getNextScrollPosition(rnsKey direction);
  ScrollStatus handleSnapToOffsetScroll(rnsKey direction,RSkComponent* candidate);
  ScrollStatus handleScroll(rnsKey direction,SkIRect candidateFrame);
  ScrollStatus handleScroll(SkPoint scrollPos);

  bool isHorizontalScroll();

};

} // namespace react
} // namespace facebook
