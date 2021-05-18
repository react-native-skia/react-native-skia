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

void RSkComponentText::OnPaint(SkCanvas *canvas) {
}

RSkComponentRawText::RSkComponentRawText(const ShadowView &shadowView)
    : RSkComponent(shadowView) {}

void RSkComponentRawText::OnPaint(SkCanvas *canvas) {}

RSkComponentParagraph::RSkComponentParagraph(const ShadowView &shadowView)
    : RSkComponent(shadowView) {}

void RSkComponentParagraph::OnPaint(SkCanvas *canvas) {
  auto component = getComponentData();
  auto state =
      std::static_pointer_cast<ParagraphShadowNode::ConcreteStateT const>(
          component.state);
  auto const &props =
      *std::static_pointer_cast<ParagraphProps const>(component.props);
  auto data = state->getData();

  auto framePoint = getFrameOrigin();
  auto frameSize = getFrameSize();

  std::unique_ptr<skia::textlayout::Paragraph> fPara;
  /* RSkTextLayoutManager to build paragraph, set build with true to consider font decoration */
  fPara = textLayoutManager_.buildParagraph(data.attributedString , props.paragraphAttributes , frameSize ,true);
  fPara->paint(canvas, framePoint.x, framePoint.y);

}

} // namespace react
} // namespace facebook
