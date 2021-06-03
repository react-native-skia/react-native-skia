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

  auto frame = getAbsoluteFrame();
  std::unique_ptr<skia::textlayout::Paragraph> fPara;
  /* RSkTextLayoutManager to build paragraph, set build with true to consider font decoration */
  fPara = buildParagraph(data.attributedString , props.paragraphAttributes , frame.size ,true);
  fPara->paint(canvas, frame.origin.x, frame.origin.y);

}

} // namespace react
} // namespace facebook
