#include "ReactSkia/components/RSkComponentText.h"

#include "include/core/SkFont.h"
#include "include/core/SkPaint.h"
#include "react/renderer/components/text/ParagraphShadowNode.h"
#include "react/renderer/components/text/RawTextShadowNode.h"
#include "react/renderer/components/text/TextShadowNode.h"

#include <glog/logging.h>

using namespace skia::textlayout;

namespace facebook {
namespace react {

RSkComponentText::RSkComponentText(const ShadowView &shadowView)
    : RSkComponent(shadowView) {}

void RSkComponentText::updateComponentProps(const ShadowView &newShadowView,bool forceUpadte) {}
void RSkComponentText::OnPaint(SkCanvas *canvas) {
}

RSkComponentRawText::RSkComponentRawText(const ShadowView &shadowView)
    : RSkComponent(shadowView) {}

void RSkComponentRawText::updateComponentProps(const ShadowView &newShadowView,bool forceUpadte) {}
void RSkComponentRawText::OnPaint(SkCanvas *canvas) {}

RSkComponentParagraph::RSkComponentParagraph(const ShadowView &shadowView)
    : RSkComponent(shadowView)
    , paraBuilder(nullptr)
    , expectedAttachmentCount(0)
    , currentAttachmentCount(0){}

void RSkComponentParagraph::updateComponentProps(const ShadowView &newShadowView,bool forceUpadte) {

  auto const &paragraphProps = *std::static_pointer_cast<ParagraphProps const>(newShadowView.props);
  paragraphAttributes_ = paragraphProps.paragraphAttributes;
}

void RSkComponentParagraph::OnPaint(SkCanvas *canvas) {
  auto component = getComponentData();
  auto state =
      std::static_pointer_cast<ParagraphShadowNode::ConcreteStateT const>(
          component.state);
  auto const &props =
      *std::static_pointer_cast<ParagraphProps const>(component.props);
  auto data = state->getData();

  /* Check if this component has parent Paragraph component */
  RSkComponentParagraph * parent = getParentParagraph();

  /* If parent, this text component is part of nested text(aka fragment attachment)*/
  /*    - use parent paragraph builder to add text & push style */
  /*    - draw the paragraph, when we reach the last fragment attachment*/
  if(parent) {
      parent->expectedAttachmentCount += data.layoutManager->buildParagraph(data.attributedString,
                                                          props.paragraphAttributes,
                                                          true,
                                                          parent->paraBuilder);
      auto paragraph = parent->paraBuilder->Build();
      parent->currentAttachmentCount++;
      if(!parent->expectedAttachmentCount || (parent->expectedAttachmentCount == parent->currentAttachmentCount)) {
            auto frame = parent->getComponentData().layoutMetrics.frame;
            paragraph->layout(frame.size.width);
            paragraph->paint(canvas, frame.origin.x, frame.origin.y);
      }
   } else {
      /* If previously created builder is available,using it will append the text in builder*/
      /* If it reaches here,means there is an update in text.So create new paragraph builder*/
      if(nullptr != paraBuilder) {
          paraBuilder.reset();
      }

      ParagraphStyle paraStyle;
      paraBuilder = std::static_pointer_cast<ParagraphBuilder>(std::make_shared<ParagraphBuilderImpl>(paraStyle,data.layoutManager->collection_));

      expectedAttachmentCount = data.layoutManager->buildParagraph(data.attributedString, props.paragraphAttributes, true, paraBuilder);
      currentAttachmentCount = 0;
      auto paragraph = paraBuilder->Build();

      /* If the count is 0,means we have no fragment attachments.So paint right away*/
      if(!expectedAttachmentCount) {
          auto frame = component.layoutMetrics.frame;
          paragraph->layout(frame.size.width);
          paragraph->paint(canvas, frame.origin.x, frame.origin.y);
      }
   }
}

} // namespace react
} // namespace facebook
