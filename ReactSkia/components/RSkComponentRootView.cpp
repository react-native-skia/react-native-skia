#include "ReactSkia/components/RSkComponentRootView.h"

#include "include/core/SkPaint.h"
#include "include/core/SkPictureRecorder.h"
#include "react/renderer/components/root/RootShadowNode.h"

namespace facebook {
namespace react {

RSkComponentRootView::RSkComponentRootView(const ShadowView &shadowView)
    : RSkComponent(shadowView) {}

sk_sp<SkPicture> RSkComponentRootView::CreatePicture(
    int surfaceWidth,
    int surfaceHeight) {
  SkPictureRecorder recorder;
  auto *canvas = recorder.beginRecording(
      SkRect::MakeXYWH(0, 0, surfaceWidth, surfaceHeight));
  if (!canvas) {
    return nullptr;
  }

  canvas->clear(SK_ColorWHITE);

  return recorder.finishRecordingAsPicture();
}

} // namespace react
} // namespace facebook
