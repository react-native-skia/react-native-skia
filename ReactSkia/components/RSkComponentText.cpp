#include "ReactSkia/components/RSkComponentText.h"

#include "include/core/SkFont.h"
#include "include/core/SkPaint.h"
#include "react/renderer/components/text/ParagraphShadowNode.h"
#include "react/renderer/components/text/RawTextShadowNode.h"
#include "react/renderer/components/text/TextShadowNode.h"

#include <glog/logging.h>

namespace facebook {
namespace react {

RSkComponentText::RSkComponentText(const ShadowView &shadowView)
    : RSkComponent(shadowView) {}

void RSkComponentText::OnPaint(const ShadowView &shadowView, SkCanvas *canvas) {
}

RSkComponentRawText::RSkComponentRawText(const ShadowView &shadowView)
    : RSkComponent(shadowView) {}

void RSkComponentRawText::OnPaint(
    const ShadowView &shadowView,
    SkCanvas *canvas) {}

RSkComponentParagraph::RSkComponentParagraph(const ShadowView &shadowView)
    : RSkComponent(shadowView) {}

void RSkComponentParagraph::OnPaint(
    const ShadowView &shadowView,
    SkCanvas *canvas) {
  auto state =
      std::static_pointer_cast<ParagraphShadowNode::ConcreteStateT const>(
          shadowView.state);
  auto const &props =
      *std::static_pointer_cast<ParagraphProps const>(shadowView.props);
  auto data = state->getData();
  auto text = data.attributedString.getString();
  auto fontSize = !std::isnan(props.textAttributes.fontSize) ? props.textAttributes.fontSize : TextAttributes::defaultTextAttributes().fontSize;
  float ratio = 256.0;
  auto color = colorComponentsFromColor(props.textAttributes.foregroundColor ? props.textAttributes.foregroundColor : TextAttributes::defaultTextAttributes().foregroundColor);
  auto frame = shadowView.layoutMetrics.frame;

  SkFont font;
  font.setSubpixel(true);
  font.setSize(fontSize);
  SkPaint paint;
  paint.setColor(SkColorSetARGB(
      color.alpha * ratio,
      color.red * ratio,
      color.green * ratio,
      color.blue * ratio));

  // NOTE(kudo): Since cxx TextLayoutManager is not ready yet, try to adjust
  // some magic offset here.
  int drawPosX = frame.origin.x - 200;
  int drawPosY = frame.origin.y;

  canvas->drawSimpleText(
      text.c_str(),
      text.length(),
      SkTextEncoding::kUTF8,
      drawPosX,
      drawPosY,
      font,
      paint);
}

} // namespace react
} // namespace facebook
