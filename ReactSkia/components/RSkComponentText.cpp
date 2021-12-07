#include "ReactSkia/components/RSkComponentText.h"

#include "include/core/SkFont.h"
#include "include/core/SkPaint.h"
#include "react/renderer/components/text/ParagraphShadowNode.h"
#include "react/renderer/components/text/RawTextShadowNode.h"
#include "react/renderer/components/text/TextShadowNode.h"
#include "ReactSkia/views/common/RSkDrawUtils.h"
#include "ReactSkia/views/common/RSkTextUtils.h"

#include "ReactSkia/utils/RnsLog.h"

using namespace skia::textlayout;
using namespace facebook::react::RSkDrawUtils;
using namespace facebook::react::RSkTextUtils;

namespace facebook {
namespace react {

RSkComponentText::RSkComponentText(const ShadowView &shadowView)
    : RSkComponent(shadowView) {}

RnsShell::LayerInvalidateMask RSkComponentText::updateComponentProps(const ShadowView &newShadowView,bool forceUpadte) {return RnsShell::LayerInvalidateNone;}
void RSkComponentText::OnPaint(SkCanvas *canvas) {}

RSkComponentRawText::RSkComponentRawText(const ShadowView &shadowView)
    : RSkComponent(shadowView) {}

RnsShell::LayerInvalidateMask RSkComponentRawText::updateComponentProps(const ShadowView &newShadowView,bool forceUpadte) {return RnsShell::LayerInvalidateNone;}
void RSkComponentRawText::OnPaint(SkCanvas *canvas) {}

RSkComponentParagraph::RSkComponentParagraph(const ShadowView &shadowView)
    : RSkComponent(shadowView)
    , paraBuilder(nullptr)
    , expectedAttachmentCount(0)
    , currentAttachmentCount(0){}

RnsShell::LayerInvalidateMask RSkComponentParagraph::updateComponentProps(const ShadowView &newShadowView,bool forceUpadte) {

  auto const &paragraphProps = *std::static_pointer_cast<ParagraphProps const>(newShadowView.props);
  paragraphAttributes_ = paragraphProps.paragraphAttributes;
  return RnsShell::LayerInvalidateAll;
}

void RSkComponentParagraph::OnPaint(SkCanvas *canvas) {
    SkAutoCanvasRestore(canvas, true);
    auto component = getComponentData();
    auto state = std::static_pointer_cast<ParagraphShadowNode::ConcreteStateT const>(component.state);
    auto const &props = *std::static_pointer_cast<ParagraphProps const>(component.props);
    auto data = state->getData();
    ParagraphStyle paraStyle;
    auto borderMetrics = props.resolveBorderMetrics(component.layoutMetrics);
    Rect borderFrame = component.layoutMetrics.frame;
    bool isParent = false;
    TextShadow shadow;

    /* Check if this component has parent Paragraph component */
    RSkComponentParagraph* parent = getParentParagraph();
    /* If parent, this text component is part of nested text(aka fragment attachment)*/
    /*    - use parent paragraph builder to add text & push style */
    /*    - draw the paragraph, when we reach the last fragment attachment*/
    if(parent) {

        auto parentComponent = parent->getComponentData();
        auto const &parentProps = *std::static_pointer_cast<ParagraphProps const>(parentComponent.props);
        auto parentBorderMetrics = parentProps.resolveBorderMetrics(parentComponent.layoutMetrics);
        
        if (parent->paraBuilder) {
            isParent = true;
            parent->expectedAttachmentCount += data.layoutManager->buildParagraph(props.backgroundColor,
                                                            data.attributedString,
                                                            paragraphAttributes_,
                                                            shadow,
                                                            true,
                                                            parent->paraBuilder);
            std::shared_ptr<Paragraph> para = parent->paraBuilder->Build();
            parent->currentAttachmentCount++;
            borderFrame = parentComponent.layoutMetrics.frame;
            borderFrame.origin.x=0;
            borderFrame.origin.y=0;

            if(!parent->expectedAttachmentCount || (parent->expectedAttachmentCount == parent->currentAttachmentCount)) {
                setTextLines(para,
                            parent->paraBuilder,
                            parentComponent.layoutMetrics,
                            paragraphAttributes_,
                            isParent);

                drawText(para,
                            canvas,
                            data.attributedString,
                            parentComponent.layoutMetrics,
                            parentProps,
                            isParent);

                drawBorder(canvas,
                            borderFrame,
                            parentBorderMetrics,
                            parentProps.backgroundColor,
                            parentProps.opacity);
            }
        }
    }
    else {
        /* If previously created builder is available,using it will append the text in builder*/
        /* If it reaches here,means there is an update in text.So create new paragraph builder*/
        if(nullptr != paraBuilder) {
            paraBuilder.reset();
        }
        paraBuilder = std::static_pointer_cast<ParagraphBuilder>(std::make_shared<ParagraphBuilderImpl>(paraStyle,data.layoutManager->collection_));
        if(layer()->shadowOpacity && layer()->shadowFilter) {
            shadow={layer()->shadowColor,SkPoint::Make(layer()->shadowOffset.width(),layer()->shadowOffset.height()),layer()->shadowRadius};
        }
        expectedAttachmentCount = data.layoutManager->buildParagraph(props.backgroundColor, data.attributedString, paragraphAttributes_,shadow,true, paraBuilder);
        currentAttachmentCount = 0;
        std::shared_ptr<Paragraph> para = paraBuilder->Build();

        /* If the count is 0,means we have no fragment attachments.So paint right away*/
        if(!expectedAttachmentCount) {

            if(layer()->shadowOpacity && layer()->shadowFilter) {
                drawShadow(canvas,
                          borderFrame,
                          borderMetrics,
                          props.backgroundColor,
                          layer()->shadowOpacity,
                          layer()->shadowFilter);
            }

            setTextLines(para,
                        paraBuilder,
                        component.layoutMetrics,
                        paragraphAttributes_,
                        isParent);

            drawText(para,
                        canvas,
                        data.attributedString,
                        component.layoutMetrics,
                        props,
                        isParent);

            drawBorder(canvas,
                        borderFrame,
                        borderMetrics,
                        props.backgroundColor,
                        props.opacity);
        }
    }
}
} // namespace react
} // namespace facebook
