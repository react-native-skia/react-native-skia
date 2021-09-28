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

void RSkComponentText::OnPaint(SkCanvas *canvas) {
}

RSkComponentRawText::RSkComponentRawText(const ShadowView &shadowView)
    : RSkComponent(shadowView) {}

void RSkComponentRawText::OnPaint(SkCanvas *canvas) {}

RSkComponentParagraph::RSkComponentParagraph(const ShadowView &shadowView)
    : RSkComponent(shadowView)
    , paraBuilder(nullptr)
    , expectedAttachmentCount(0)
    , currentAttachmentCount(0){}

inline SkScalar yPosOffset(AttributedString attributedString, SkScalar paraHeight, Rect frame) {
    for(auto &fragment: attributedString.getFragments()) {
        if((paraHeight < frame.size.height) && (!fragment.textAttributes.textAlignVertical.empty())) {
            if(!strcmp(fragment.textAttributes.textAlignVertical.c_str(),"center"))
                return ((frame.size.height/2) - (paraHeight/2));
            else if(!strcmp(fragment.textAttributes.textAlignVertical.c_str(),"bottom"))
                return (frame.size.height - paraHeight);
        }
    }
    return 0;
}

inline int getNumberOfLines(std::vector<LineMetrics>& metrics, int maxNumberOfLines, Float frameHeight) {
    int i=0;
    int minNumberOfLines = 0;
    for(; i< metrics.size(); i++) {
        if ((metrics[i]).fBaseline > frameHeight) {
            minNumberOfLines = (metrics[i]).fLineNumber;
            break;
        }
    }
    if ((minNumberOfLines > 0) && (minNumberOfLines < maxNumberOfLines))
        return minNumberOfLines;
    
    return maxNumberOfLines;
}

void RSkComponentParagraph::OnPaint(SkCanvas *canvas) {
    auto component = getComponentData();
    auto state = std::static_pointer_cast<ParagraphShadowNode::ConcreteStateT const>(component.state);
    auto const &props = *std::static_pointer_cast<ParagraphProps const>(component.props);
    auto data = state->getData();

    /* Check if this component has parent Paragraph component */
    RSkComponentParagraph * parent = getParentParagraph();
    ParagraphStyle paraStyle;
    std::vector<LineMetrics> metrics;
    SkScalar yOffset = 0;
    int numberOfLines = 0;
  
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
            auto frame = parent->getAbsoluteFrame();
            paragraph->layout(frame.size.width);
            paragraph->getLineMetrics(metrics);

            numberOfLines = getNumberOfLines(metrics, props.paragraphAttributes.maximumNumberOfLines, frame.size.height);
            if (numberOfLines){
                if (EllipsizeMode::Tail == props.paragraphAttributes.ellipsizeMode) {
                    paraStyle.setEllipsis(u"\u2026");
                }
                paraStyle.setMaxLines(numberOfLines);
                parent->paraBuilder->setParagraphStyle(paraStyle);
                paragraph = parent->paraBuilder->Build();
                paragraph->layout(frame.size.width);
            }
            yOffset = yPosOffset(data.attributedString, paragraph->getHeight(), frame);
            paragraph->paint(canvas, frame.origin.x, frame.origin.y + yOffset);
        }
    }else {
        /* If previously created builder is available,using it will append the text in builder*/
        /* If it reaches here,means there is an update in text.So create new paragraph builder*/
        if(nullptr != paraBuilder) {
            paraBuilder.reset();
        }

        paraBuilder = std::static_pointer_cast<ParagraphBuilder>(std::make_shared<ParagraphBuilderImpl>(paraStyle,data.layoutManager->collection_));

        expectedAttachmentCount = data.layoutManager->buildParagraph(data.attributedString, props.paragraphAttributes, true, paraBuilder);
        currentAttachmentCount = 0;
        auto paragraph = paraBuilder->Build();

        /* If the count is 0,means we have no fragment attachments.So paint right away*/
        if(!expectedAttachmentCount) {
            auto frame = getAbsoluteFrame();
            paragraph->layout(frame.size.width);
            paragraph->getLineMetrics(metrics);

            numberOfLines = getNumberOfLines(metrics, props.paragraphAttributes.maximumNumberOfLines, frame.size.height);
            if (numberOfLines){
                if (EllipsizeMode::Tail == props.paragraphAttributes.ellipsizeMode) {
                    paraStyle.setEllipsis(u"\u2026");
                }
                paraStyle.setMaxLines(numberOfLines);
                paraBuilder->setParagraphStyle(paraStyle);
                paragraph = paraBuilder->Build();
                paragraph->layout(frame.size.width);
            }
            yOffset = yPosOffset(data.attributedString, paragraph->getHeight(), frame);
            paragraph->paint(canvas, frame.origin.x, frame.origin.y + yOffset);
        }
    }
}

} // namespace react
} // namespace facebook
