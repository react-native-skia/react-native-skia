/*
* Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
*
* This source code is licensed under the MIT license found in the
* LICENSE file in the root directory of this source tree.
*/

#pragma once

#include "react/renderer/components/scrollview/ScrollViewShadowNode.h"

#include "ReactSkia/components/RSkComponent.h"
#include "ReactSkia/RSkSurfaceWindow.h"
#include "ReactSkia/sdk/FollyTimer.h"

using namespace rns::sdk;
namespace facebook {
namespace react {

#define SCROLL_LAYER_HANDLE static_cast<RnsShell::ScrollLayer*>(layer().get())
#define SCROLL_LAYER(x) SCROLL_LAYER_HANDLE->x
#define SCROLLVIEW_DEFAULT_ZOOMSCALE 1
#define SCROLLBAR_FADEOUT_TIME 1500

enum ScrollDirectionType {
   ScrollDirectionForward = 1,
   ScrollDirectionBackward
};

class RSkComponentScrollView final : public RSkComponent {
 public:
  RSkComponentScrollView(const ShadowView &shadowView);
  ~RSkComponentScrollView();

  //RSkComponent override functions
  RnsShell::LayerInvalidateMask updateComponentProps(
    SharedProps newviewProps,
    bool forceUpadate) override;

  RnsShell::LayerInvalidateMask updateComponentState(
    const ShadowView &newShadowView,
    bool forceUpadate) override;

  void handleCommand(
    std::string commandName,
    folly::dynamic args) override;

  bool isContainer() const override { return true; }

  //RSkSpatialNavigatorContainer override functions
  bool canScrollInDirection(rnsKey direction) override;
  bool isVisible(RSkComponent* candidate) override;
  bool isScrollable() override { return true;}
  SkPoint getScrollOffset() override;

  ScrollStatus scrollInDirection(
    RSkComponent* candidate,
    rnsKey direction) override;

  ScrollStatus scrollTo(
    RSkComponent* candidate) override;

 protected:
  void OnPaint(SkCanvas *canvas) override;

 private:
  bool pagingEnabled_{false};
  bool scrollEnabled_{true};
  bool isHorizontalScroll_{false};
  std::vector<int> snapToOffsets_;
  SkPoint contentOffset_{0,0};

#if ENABLE(FEATURE_SCROLL_INDICATOR)
  bool showHorizontalScrollIndicator_{false};
  bool showVerticalScrollIndicator_{false};
  bool persistentScrollIndicator_{false};

  bool drawScrollIndicator_{false};
  Timer* scrollbarTimer_{nullptr};

  void fadeOutScrollBar();
#endif //ENABLE_FEATURE_SCROLL_INDICATOR

  void calculateNextScrollOffset(
    ScrollDirectionType scrollDirection,
    int containerLength,
    int frameLength,
    SkScalar &scrollOfffset);

  SkPoint getNextScrollPosition(rnsKey direction);
  void updateScrollOffset(int x,int y);

  ScrollStatus handleSnapToOffsetScroll(rnsKey direction,RSkComponent* candidate);
  ScrollStatus handleScroll(rnsKey direction,SkIRect candidateFrame);
  ScrollStatus handleScroll(int x,int y, bool isFlushDisplay=true);
  ScrollStatus handleScroll(SkPoint scrollPos, bool isFlushDisplay=true);

  void dispatchOnScrollEvent(SkPoint scrollPos);
};

} // namespace react
} // namespace facebook
