/*
 * Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "RSkTextLayoutManager.h"

using namespace skia::textlayout;

namespace facebook {
namespace react {

Point calculateFramePoint( Point origin , Size rect , float layoutWidth) {
   /* Calculate the (x,y) cordinates of fragment attachments,based on the fragment width provided*/
   if(origin.x + rect.width < layoutWidth)
       origin.x += rect.width ;
   else {
       auto delta = layoutWidth - origin.x ;
       origin.x = rect.width - delta ;
       origin.y += rect.height;
   }
   return origin;
}

TextAlign convertTextAlign (TextAlignment alignment) {
   switch(alignment) {
      case TextAlignment::Natural :
      case TextAlignment::Left : return TextAlign::kLeft;
      case TextAlignment::Center : return TextAlign::kCenter;
      case TextAlignment::Right : return TextAlign::kRight;
      case TextAlignment::Justified : return TextAlign::kJustify;
    }
    return TextAlign::kLeft;
}

SkPaint convertTextColor ( SharedColor textColor ) {
   SkPaint paint;
   float ratio = 255.9999;
   auto color = colorComponentsFromColor(textColor);
   paint.setColor(SkColorSetARGB( color.alpha * ratio,
                                     color.red * ratio,
                                     color.green * ratio,
                                     color.blue * ratio));
   paint.setAntiAlias(true);
   return paint;
}

RSkTextLayoutManager::RSkTextLayoutManager() {
    /* Set default font collection */ 
    collection_ = sk_make_sp<FontCollection>();
    collection_->setDefaultFontManager(SkFontMgr::RefDefault());
}

TextMeasurement RSkTextLayoutManager::doMeasure (AttributedString attributedString,
                ParagraphAttributes paragraphAttributes,
                LayoutConstraints layoutConstraints) const {
    TextMeasurement::Attachments attachments;
    ParagraphStyle paraStyle;
    Size size;

    std::shared_ptr<ParagraphBuilder> builder = std::static_pointer_cast<ParagraphBuilder>(std::make_shared<ParagraphBuilderImpl>(paraStyle,collection_));
    buildParagraph(attributedString, paragraphAttributes, false, builder);
    auto paragraph = builder->Build();
    paragraph->layout(layoutConstraints.maximumSize.width);

    size.width = paragraph->getMaxIntrinsicWidth() < paragraph->getMaxWidth() ?                                            paragraph->getMaxIntrinsicWidth() :                                            paragraph->getMaxWidth();
    size.height = paragraph->getHeight();

    Point attachmentPoint = calculateFramePoint({0,0}, size, layoutConstraints.maximumSize.width);

    for (auto const &fragment : attributedString.getFragments()) {
       if (fragment.isAttachment()) {
           Rect rect;
           rect.size.width = fragment.parentShadowView.layoutMetrics.frame.size.width;
           rect.size.height = fragment.parentShadowView.layoutMetrics.frame.size.height;
           /* TODO : We will be unable to calculate exact (x,y) cordinates for the attachments*/
           /* Reason : attachment fragment width is clamped width wrt layout width; */
           /*          so we do not know actual position at which the previous attachment cordinate ends*/
           /* But we need to still calculate total container height here, from all attachments */
           /* NOTE : height value calculated would be approximate,since we lack the knowledge of actual frag width here*/
           attachmentPoint = calculateFramePoint(attachmentPoint, rect.size, layoutConstraints.maximumSize.width);
           attachments.push_back(TextMeasurement::Attachment{rect, false});

       }
    }

    /* Update the container height from all attachments */
    if(!attachments.empty()) {
       size.height = attachmentPoint.y + attachments[attachments.size()-1].frame.size.height;
    }

    return TextMeasurement{size, attachments};
}

uint32_t RSkTextLayoutManager::buildParagraph (AttributedString attributedString,
                ParagraphAttributes paragraphAttributes,
                bool fontDecorationRequired,
                std::shared_ptr<ParagraphBuilder> builder) const {
    uint32_t attachmentCount = 0;
    std::unique_ptr<Paragraph> fPara;
    TextStyle style;
    ParagraphStyle paraStyle;
    auto fontSize = TextAttributes::defaultTextAttributes().fontSize;
    auto fontSizeMultiplier = TextAttributes::defaultTextAttributes().fontSizeMultiplier;

    for(auto &fragment: attributedString.getFragments()) {
        if(fragment.isAttachment()) {
           attachmentCount++;
           continue;
        }

        fontSize = !std::isnan(fragment.textAttributes.fontSize) ?
                                 fragment.textAttributes.fontSize :
                                 TextAttributes::defaultTextAttributes().fontSize;

        fontSizeMultiplier = !std::isnan(fragment.textAttributes.fontSizeMultiplier) ?
                                   fragment.textAttributes.fontSizeMultiplier :
                                   TextAttributes::defaultTextAttributes().fontSizeMultiplier;

        style.setFontSize(fontSize * fontSizeMultiplier);
        style.setFontFamilies({SkString(fragment.textAttributes.fontFamily.c_str())});

        /* Build paragraph considering text decoration attributes*/
        /* Required during text paint */
        if(fontDecorationRequired) {
            style.setForegroundColor(convertTextColor(fragment.textAttributes.foregroundColor ?
                                                      fragment.textAttributes.foregroundColor :
                                                      TextAttributes::defaultTextAttributes().foregroundColor));
            style.setBackgroundColor(convertTextColor(fragment.textAttributes.backgroundColor ?
                                                      fragment.textAttributes.backgroundColor :
                                                      TextAttributes::defaultTextAttributes().backgroundColor));
        }

        if(fragment.textAttributes.alignment.has_value())
            paraStyle.setTextAlign(convertTextAlign(fragment.textAttributes.alignment.value()));

        builder->setParagraphStyle(paraStyle);
        builder->pushStyle(style);
        builder->addText(fragment.string.c_str(),fragment.string.length());
    }

    return attachmentCount;
}

} // namespace react 
} // namespace facebook


