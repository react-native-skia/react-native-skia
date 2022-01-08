#include "ReactSkia/components/RSkComponentRootView.h"

#include "include/core/SkPaint.h"
#include "react/renderer/components/root/RootShadowNode.h"

namespace facebook {
namespace react {

RSkComponentRootView::RSkComponentRootView(const ShadowView &shadowView)
    : RSkComponent(shadowView) {}

RnsShell::LayerInvalidateMask RSkComponentRootView::updateComponentProps(const ShadowView &newShadowView,bool forceUpdate) {return RnsShell::LayerInvalidateNone;}
void RSkComponentRootView::OnPaint(
    SkCanvas *canvas) {
  canvas->clear(SK_ColorWHITE);
}

bool RSkComponentRootView::isVisible(RSkComponent* candidate) {
  if(candidate == nullptr)
    return false;

  const SkIRect& rootRect = getLayerAbsoluteFrame();
  const SkIRect& candidateRect = candidate->getLayerAbsoluteFrame();

  RNS_LOG_TODO("Should use screenFrame for calculations");
  RNS_LOG_DEBUG("Visible Check : RootContainer [" << rootRect.x() << "," << rootRect.y() << "," << rootRect.width() << "," << rootRect.height() <<
                "] Candidtae [" << candidateRect.x() << "," << candidateRect.y() << "," << candidateRect.width() << "," << candidateRect.height() << "]");

  return(rootRect.contains(candidateRect));
}

} // namespace react
} // namespace facebook
