 /*
 * Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "ReactSkia/views/common/RSkTextUtils.h"

using namespace skia::textlayout;

namespace facebook {
namespace react {
namespace {
inline int getLines(std::vector<LineMetrics>& metrics, float frameHeight) {
    int minNumberOfLines = metrics.size();
    for(int i=0; i< metrics.size(); i++) {
        if ((metrics[i]).fBaseline > frameHeight) {
            minNumberOfLines = (metrics[i]).fLineNumber;
            break;
        }
    }
    return minNumberOfLines;
}

inline int getTextLines(std::vector<LineMetrics> metrics, float maxNumberOfLines, float textFrameHeight) {
    int numberOfLines = getLines(metrics, textFrameHeight);
    if ((maxNumberOfLines) && (numberOfLines) && (maxNumberOfLines < numberOfLines)) {
            return maxNumberOfLines;        
    }
    return numberOfLines;
}
} //namespace

namespace RSkTextUtils{
void setTextLines(std::shared_ptr<Paragraph>& paragraph,
            std::shared_ptr<ParagraphBuilder>& builder,
            LayoutMetrics layout,
            ParagraphAttributes paragraphAttributes,
            bool isParent) {
    int numberOfLines = 0;
    ParagraphStyle paraStyle;
    std::vector<LineMetrics> metrics;

    paragraph->layout(layout.getContentFrame().size.width);
    paragraph->getLineMetrics(metrics);
    numberOfLines = getTextLines(metrics, paragraphAttributes.maximumNumberOfLines, layout.getContentFrame().size.height);
    if (numberOfLines) {
        if (paragraphAttributes.maximumNumberOfLines) {
            if ((EllipsizeMode::Tail) == (paragraphAttributes.ellipsizeMode))
                paraStyle.setEllipsis(u"\u2026");
        }
        paraStyle.setMaxLines(numberOfLines);
        builder->setParagraphStyle(paraStyle);
        paragraph = builder->Build();
        paragraph->layout(layout.getContentFrame().size.width);
    }
}

void drawText(std::shared_ptr<Paragraph>& paragraph,
            SkCanvas *canvas,
            AttributedString attributedString,
            LayoutMetrics layout,
            const ParagraphProps& props,
            bool isParent) {
    SkPaint paint;
    SkScalar yOffset = 0;
    Rect frame = layout.frame;
    auto fontLineHeight = (!std::isnan(props.textAttributes.lineHeight)) && (props.textAttributes.lineHeight >= 0) ?
                            props.textAttributes.lineHeight :
                            true;

    if ((props.backgroundColor) || (props.textAttributes.lineHeight >= 0)) {
        if (isParent)
            canvas->clipRect(SkRect::MakeXYWH(0, 0, frame.size.width, frame.size.height));
        else
            canvas->clipRect(SkRect::MakeXYWH(frame.origin.x, frame.origin.y, frame.size.width, frame.size.height));
        canvas->drawColor(RSkColorFromSharedColor(props.backgroundColor, SK_ColorTRANSPARENT));
    }
    if (props.textAttributes.lineHeight) {
        if (isParent)
            paragraph->paint(canvas, layout.contentInsets.left, layout.contentInsets.top);
        else
            paragraph->paint(canvas, frame.origin.x + layout.contentInsets.left, frame.origin.y + layout.contentInsets.top);
    }
}
} //RSkTextUtils
} //react
} //facebook