#include "ReactSkia/components/RSkComponentView.h"

#include "include/core/SkPaint.h"
#include "react/renderer/components/view/ViewShadowNode.h"

#include <glog/logging.h>

namespace facebook {
namespace react {

RSkComponentView::RSkComponentView(const ShadowView &shadowView)
    : RSkComponent(shadowView) {}

void RSkComponentView::OnPaint(const ShadowView &shadowView, SkCanvas *canvas) {
  auto const &viewProps =
      *std::static_pointer_cast<ViewProps const>(shadowView.props);
  auto bgcolor = colorComponentsFromColor(viewProps.backgroundColor);
  float ratio = 255.9999;

  SkPaint paint;
  paint.setColor(SkColorSetARGB(
      bgcolor.alpha * ratio,
      bgcolor.red * ratio,
      bgcolor.green * ratio,
      bgcolor.blue * ratio));

  auto frame = shadowView.layoutMetrics.frame;
  SkRect rect = SkRect::MakeXYWH(
      frame.origin.x, frame.origin.y, frame.size.width, frame.size.height);
  canvas->drawRect(rect, paint);
}

} // namespace react
} // namespace facebook
