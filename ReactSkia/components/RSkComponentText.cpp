#include "ReactSkia/components/RSkComponentText.h"

#include "include/core/SkFont.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPictureRecorder.h"
#include "react/renderer/components/text/ParagraphShadowNode.h"
#include "react/renderer/components/text/RawTextShadowNode.h"
#include "react/renderer/components/text/TextShadowNode.h"

#include <glog/logging.h>

namespace facebook {
namespace react {

RSkComponentText::RSkComponentText(const ShadowView &shadowView)
    : RSkComponent(shadowView) {}

sk_sp<SkPicture> RSkComponentText::CreatePicture(
    int surfaceWidth,
    int surfaceHeight) {
  return nullptr;
}

RSkComponentRawText::RSkComponentRawText(const ShadowView &shadowView)
    : RSkComponent(shadowView) {}

sk_sp<SkPicture> RSkComponentRawText::CreatePicture(
    int surfaceWidth,
    int surfaceHeight) {
  return nullptr;
}

RSkComponentParagraph::RSkComponentParagraph(const ShadowView &shadowView)
    : RSkComponent(shadowView) {}

sk_sp<SkPicture> RSkComponentParagraph::CreatePicture(
    int surfaceWidth,
    int surfaceHeight) {
  SkPictureRecorder recorder;
  auto *canvas = recorder.beginRecording(
      SkRect::MakeXYWH(0, 0, surfaceWidth, surfaceHeight));
  if (!canvas) {
    return nullptr;
  }
  auto component = getComponentData();
  auto state =
      std::static_pointer_cast<ParagraphShadowNode::ConcreteStateT const>(
          component.state);
  auto const &props =
      *std::static_pointer_cast<ParagraphProps const>(component.props);
  auto data = state->getData();
  auto text = data.attributedString.getString();
  auto fontSize = !std::isnan(props.textAttributes.fontSize)
      ? props.textAttributes.fontSize
      : TextAttributes::defaultTextAttributes().fontSize;
  float ratio = 255.9999;
  auto color = colorComponentsFromColor(
      props.textAttributes.foregroundColor
          ? props.textAttributes.foregroundColor
          : TextAttributes::defaultTextAttributes().foregroundColor);

  SkFont font;
  font.setSubpixel(true);
  font.setSize(fontSize);
  SkPaint paint;
  paint.setColor(SkColorSetARGB(
      color.alpha * ratio,
      color.red * ratio,
      color.green * ratio,
      color.blue * ratio));

  auto framePoint = getFrameOrigin();

  canvas->drawSimpleText(
      text.c_str(),
      text.length(),
      SkTextEncoding::kUTF8,
      framePoint.x,
      framePoint.y,
      font,
      paint);

  return recorder.finishRecordingAsPicture();
}

} // namespace react
} // namespace facebook
