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

#if defined(TARGET_OS_TV) && TARGET_OS_TV
// The textAlignVertical property is available only in the TVOS RN repo
inline SkScalar yPosOffset(AttributedString attributedString, SkScalar paraHeight, Float frameHeight ) {
    for(auto &fragment: attributedString.getFragments()) {
        if((paraHeight < frameHeight) && (!fragment.textAttributes.textAlignVertical.empty())) {
            if(!strcmp(fragment.textAttributes.textAlignVertical.c_str(),"center"))
                return (frameHeight - paraHeight)/2;
            else if(!strcmp(fragment.textAttributes.textAlignVertical.c_str(),"bottom"))
                return (frameHeight - paraHeight);
        }
    }
    return 0;
}
#endif //TARGET_OS_TV

} //namespace

namespace RSkTextUtils{
void setTextLines(struct RSkSkTextLayout &textLayout,
            LayoutMetrics layout,
            ParagraphAttributes paragraphAttributes) {
    int numberOfLines = 0;
    std::vector<LineMetrics> metrics;

    textLayout.paragraph->layout(layout.getContentFrame().size.width);
    textLayout.paragraph->getLineMetrics(metrics);
    numberOfLines = getTextLines(metrics, paragraphAttributes.maximumNumberOfLines, layout.getContentFrame().size.height);
    if (numberOfLines) {
        textLayout.paraStyle.setMaxLines(numberOfLines);
        if (paragraphAttributes.maximumNumberOfLines) {
            if ((EllipsizeMode::Tail) == (paragraphAttributes.ellipsizeMode))
                textLayout.paraStyle.setEllipsis(u"\u2026");
        }
        textLayout.builder->setParagraphStyle(textLayout.paraStyle);
        textLayout.paragraph = textLayout.builder->Build();
        textLayout.paragraph->layout(layout.getContentFrame().size.width);
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
    SkAutoCanvasRestore save(canvas, true);

    if (isParent){
        canvas->clipRect(SkRect::MakeXYWH(0, 0, frame.size.width, frame.size.height), SkClipOp::kIntersect);
    } else {
        canvas->clipRect(SkRect::MakeXYWH(frame.origin.x, frame.origin.y, frame.size.width, frame.size.height), SkClipOp::kIntersect);
    }

    if (props.backgroundColor ){
        SkColor bgColor = RSkColorFromSharedColor(props.backgroundColor, SK_ColorTRANSPARENT);
        if (bgColor && SkColorGetA(bgColor)){
            canvas->drawColor(bgColor);
        }
    }

#if defined(TARGET_OS_TV) && TARGET_OS_TV
    yOffset = yPosOffset(attributedString, paragraph->getHeight(), layout.getContentFrame().size.height);
#endif //TARGET_OS_TV
    if (isParent){
        paragraph->paint(canvas, layout.contentInsets.left, layout.contentInsets.top + yOffset);
    } else {
        paragraph->paint(canvas, frame.origin.x + layout.contentInsets.left, frame.origin.y + layout.contentInsets.top + yOffset);
    }
}
} //RSkTextUtils
} //react
} //facebook
