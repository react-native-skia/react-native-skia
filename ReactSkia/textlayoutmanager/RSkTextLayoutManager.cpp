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

SkFontStyle::Slant convertFontStyle (FontStyle fontStyle) {
    switch(fontStyle){
        case FontStyle::Italic : return SkFontStyle::kItalic_Slant;
        case FontStyle::Normal : 
        default: 
            return SkFontStyle::kUpright_Slant;
    }
}   

int convertFontWeight (FontWeight fontWeight) {
    switch(fontWeight){
        case FontWeight::Weight100 : return SkFontStyle::kThin_Weight;
        case FontWeight::Weight200 : return SkFontStyle::kExtraLight_Weight;
        case FontWeight::Weight300 : return SkFontStyle::kLight_Weight;
        case FontWeight::Weight500 : return SkFontStyle::kMedium_Weight;
        case FontWeight::Weight600 : return SkFontStyle::kSemiBold_Weight;
        case FontWeight::Weight800 : return SkFontStyle::kExtraBold_Weight;
        case FontWeight::Weight900 : return SkFontStyle::kBlack_Weight;
        case FontWeight::Bold : return SkFontStyle::kBold_Weight;
        case FontWeight::Regular : 
        default:
            return SkFontStyle::kNormal_Weight;
    }
} 

TextDecoration convertDecoration (TextDecorationLineType textDecoration){
    switch(textDecoration){
        case TextDecorationLineType::Underline : return TextDecoration::kUnderline;
        case TextDecorationLineType::Strikethrough : return TextDecoration::kLineThrough;
        case TextDecorationLineType::UnderlineStrikethrough : return (TextDecoration)(TextDecoration::kLineThrough | TextDecoration::kUnderline);
        case TextDecorationLineType::None :
        default:
            return TextDecoration::kNoDecoration;
    }
}

RSkTextLayoutManager::RSkTextLayoutManager() {
    /* Set default font collection */ 
    collection_ = sk_make_sp<FontCollection>();
    collection_->setDefaultFontManager(SkFontMgr::RefDefault());
}

TextMeasurement RSkTextLayoutManager::doMeasure (SharedColor backGroundColor,
                AttributedString attributedString,
                ParagraphAttributes paragraphAttributes,
                LayoutConstraints layoutConstraints) const {
    TextMeasurement::Attachments attachments;
    struct RSkSkTextLayout textLayout;
    Size size;

    textLayout.builder = std::static_pointer_cast<ParagraphBuilder>(std::make_shared<ParagraphBuilderImpl>(textLayout.paraStyle,collection_));
    buildParagraph(textLayout, backGroundColor, attributedString, paragraphAttributes, false);
    auto paragraph = textLayout.builder->Build();
    paragraph->layout(layoutConstraints.maximumSize.width);

    size.width = paragraph->getMaxIntrinsicWidth() < paragraph->getMaxWidth() ?
	                                        paragraph->getMaxIntrinsicWidth() :
								            paragraph->getMaxWidth();  
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

void RSkTextLayoutManager::buildText (struct RSkSkTextLayout &textLayout,
    SharedColor backGroundColor,
    ParagraphAttributes paragraphAttributes,
    TextAttributes textAttributes,
    std::string textString,
    bool fontDecorationRequired) const {
    TextStyle style;
    auto fontSize = TextAttributes::defaultTextAttributes().fontSize;
    auto fontSizeMultiplier = TextAttributes::defaultTextAttributes().fontSizeMultiplier;
    int fontWeight = SkFontStyle::kNormal_Weight;
    SkFontStyle::Slant fontStyle = SkFontStyle::kUpright_Slant;
    double fontShadowRadius = 0;
    Size fontShadowOffset = {0,0};                         
    SkPoint setShadowPoint; 

    fontSize = (!std::isnan(textAttributes.fontSize)) && (textAttributes.fontSize > 0) ? 
                            textAttributes.fontSize :
                            TextAttributes::defaultTextAttributes().fontSize;

    fontSizeMultiplier = !std::isnan(textAttributes.fontSizeMultiplier) ?
                            textAttributes.fontSizeMultiplier :
                            TextAttributes::defaultTextAttributes().fontSizeMultiplier;

    fontWeight = textAttributes.fontWeight.has_value() ? 
                            convertFontWeight(textAttributes.fontWeight.value()) : 
                            SkFontStyle::kNormal_Weight;

    fontStyle = textAttributes.fontStyle.has_value() ? 
                            convertFontStyle(textAttributes.fontStyle.value()): 
                            SkFontStyle::kUpright_Slant;

    fontShadowOffset = textAttributes.textShadowOffset.has_value() ?
                            textAttributes.textShadowOffset.value() : 
                            fontShadowOffset;

    fontShadowRadius = (!std::isnan(textAttributes.textShadowRadius)) && (textAttributes.textShadowRadius > 0) ?
                            textAttributes.textShadowRadius :
                            fontShadowRadius;

    style.setFontSize((fontSize * fontSizeMultiplier));
    style.setFontFamilies({SkString(textAttributes.fontFamily.c_str())});
    style.setFontStyle(SkFontStyle{fontWeight, SkFontStyle::kNormal_Width, fontStyle});

    if((!std::isnan(textAttributes.lineHeight)) && (textAttributes.lineHeight > 0)){
        style.setHeightOverride(true);
        style.setHeight(textAttributes.lineHeight / fontSize);
    }
    
    style.setDecoration(textAttributes.textDecorationLineType.has_value() ?
                            convertDecoration(textAttributes.textDecorationLineType.value()) :
                            TextDecoration::kNoDecoration);

    if(!textAttributes.backgroundColor) {
        /*TODO For text content shadow shadowOpacity to be handle*/
            style.addShadow(textLayout.shadow);
            textLayout.shadow.fOffset+=setShadowPoint.Make(fontShadowOffset.width,fontShadowOffset.height);
            style.addShadow(textLayout.shadow);
    }

    style.addShadow(TextShadow(convertTextColor(textAttributes.textShadowColor ?
                                                textAttributes.textShadowColor :
                                                textAttributes.foregroundColor).getColor(),
                                                setShadowPoint.Make(fontShadowOffset.width,fontShadowOffset.height),
                                                fontShadowRadius));
    style.setLetterSpacing(!std::isnan(textAttributes.letterSpacing) ?
                            textAttributes.letterSpacing :
                            0);

    /* Build paragraph considering text decoration attributes*/
    /* Required during text paint */
    if(fontDecorationRequired) {
        style.setForegroundColor(convertTextColor(textAttributes.foregroundColor ?
                                                    textAttributes.foregroundColor :
                                                    TextAttributes::defaultTextAttributes().foregroundColor));
        style.setBackgroundColor(convertTextColor(textAttributes.backgroundColor ?
                                                    textAttributes.backgroundColor :
                                                    TextAttributes::defaultTextAttributes().backgroundColor));
        style.setDecorationColor(convertTextColor(textAttributes.textDecorationColor ?
                                                    textAttributes.textDecorationColor :
                                                    textAttributes.foregroundColor).getColor());   
    }

    if(textAttributes.alignment.has_value())
        textLayout.paraStyle.setTextAlign(convertTextAlign(textAttributes.alignment.value()));

    textLayout.paraStyle.setMaxLines(std::numeric_limits<size_t>::max());
    textLayout.builder->setParagraphStyle(textLayout.paraStyle);
    textLayout.builder->pushStyle(style);
    textLayout.builder->addText(textString.c_str(), std::strlen(textString.c_str()));
    textLayout.builder->pop();
}

uint32_t RSkTextLayoutManager::buildParagraph (struct RSkSkTextLayout &textLayout,
                SharedColor backGroundColor,
                AttributedString attributedString,
                ParagraphAttributes paragraphAttributes,
                bool fontDecorationRequired) const {
    uint32_t attachmentCount = 0;  

    for(auto &fragment: attributedString.getFragments()) {
        if(fragment.isAttachment()) {
           attachmentCount++;
           continue;
        }

        buildText (textLayout,
                    backGroundColor,
                    paragraphAttributes,
                    fragment.textAttributes,
                    fragment.string.c_str(),
                    fontDecorationRequired);
    }

    return attachmentCount;
}

} // namespace react 
} // namespace facebook

