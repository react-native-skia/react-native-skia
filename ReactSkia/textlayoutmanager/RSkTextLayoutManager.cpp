
#include "RSkTextLayoutManager.h"

using namespace skia::textlayout;

namespace facebook {
namespace react {

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
    std::unique_ptr<Paragraph> paragraph = buildParagraph (attributedString, paragraphAttributes, layoutConstraints.maximumSize);
    
    for (auto const &fragment : attributedString.getFragments()) {
       if (fragment.isAttachment()) {
           /* TODO : Compute attachment frame details */
           attachments.push_back(
                TextMeasurement::Attachment{{{0, 0}, {0, 0}}, false});
       }
    }
    return TextMeasurement{{paragraph->getMaxWidth(), paragraph->getHeight()}, attachments};
}

std::unique_ptr<Paragraph> RSkTextLayoutManager::buildParagraph (AttributedString attributedString,
                ParagraphAttributes paragraphAttributes,
                Size size ,
                bool fontDecorationRequired) const {
    std::unique_ptr<Paragraph> fPara;
    TextStyle style;
    ParagraphStyle paraStyle;
    auto fontSize = TextAttributes::defaultTextAttributes().fontSize;
    auto fontSizeMultiplier = TextAttributes::defaultTextAttributes().fontSizeMultiplier;

    paraStyle.setTextStyle(style);
    auto builder = ParagraphBuilder::make(paraStyle, collection_);

    for(auto &fragment: attributedString.getFragments()) {

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

    fPara = builder->Build();
    fPara->layout(size.width);
    return fPara;
}

} // namespace react 
} // namespace facebook


