#include "ReactSkia/components/RSkComponentView.h"

#include "include/core/SkPaint.h"
#include "react/renderer/components/view/ViewShadowNode.h"

#include <glog/logging.h>

namespace facebook {
namespace react {

RSkComponentView::RSkComponentView(const ShadowView &shadowView)
    : RSkComponent(shadowView) {}

void RSkComponentView::OnPaint(SkCanvas *canvas) {
  auto component = getComponentData();
  auto const &viewProps = *std::static_pointer_cast<ViewProps const>(component.props);
 
  /* apply view style props */
  auto borderMetrics=viewProps.resolveBorderMetrics(component.layoutMetrics);
  Rect frame;
  frame.origin=getFrameOrigin();
  frame.size=getFrameSize();
  drawBackground(canvas,frame,borderMetrics,viewProps.backgroundColor,viewProps.opacity);
  drawBorder(canvas,frame,borderMetrics,viewProps.backgroundColor,viewProps.opacity);

}

} // namespace react
} // namespace facebook
