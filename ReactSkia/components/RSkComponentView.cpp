#include "ReactSkia/components/RSkComponentView.h"

#include "include/core/SkPaint.h"
#include "include/core/SkPictureRecorder.h"
#include "react/renderer/components/view/ViewShadowNode.h"

#include <glog/logging.h>

namespace facebook {
namespace react {

RSkComponentView::RSkComponentView(const ShadowView &shadowView)
    : RSkComponent(shadowView) {}

sk_sp<SkPicture> RSkComponentView::CreatePicture(
    int surfaceWidth,
    int surfaceHeight) {
  SkPictureRecorder recorder;
  auto *canvas = recorder.beginRecording(
      SkRect::MakeXYWH(0, 0, surfaceWidth, surfaceHeight));
  if (!canvas) {
    return nullptr;
  }
  auto component = getComponentData();
  auto const &viewProps =
      *std::static_pointer_cast<ViewProps const>(component.props);

  auto bgcolor = colorComponentsFromColor(viewProps.backgroundColor);
  float ratio = 255.9999;
  SkPaint paint;
  paint.setColor(SkColorSetARGB(
      bgcolor.alpha * ratio,
      bgcolor.red * ratio,
      bgcolor.green * ratio,
      bgcolor.blue * ratio));

  auto framePoint = getFrameOrigin();
  auto frameSize = getFrameSize();
  SkRect rect = SkRect::MakeXYWH(
      framePoint.x, framePoint.y, frameSize.width, frameSize.height);
  canvas->drawRect(rect, paint);

  return recorder.finishRecordingAsPicture();
}

} // namespace react
} // namespace facebook
