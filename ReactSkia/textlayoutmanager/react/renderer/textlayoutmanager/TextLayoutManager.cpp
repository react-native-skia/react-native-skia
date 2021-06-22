/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 * Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "TextLayoutManager.h"

namespace facebook {
namespace react {

#ifdef RNS_DEBUG_TEXT_ATTRIBUTES
void debugAttributedStringBox(AttributedStringBox attributedStringBox) {
  LOG(INFO) << "\t AttributedStringBox :";
  LOG(INFO) << "\t\t Mode : " << static_cast<int>(attributedStringBox.getMode());
  if(attributedStringBox.getMode() == AttributedStringBox::Mode::OpaquePointer) {
      LOG(INFO) << "\t\t Opaque Pointer : " << attributedStringBox.getOpaquePointer().get();
  }
  else {
      LOG(INFO) << "\t\t Value : ";
      if(!attributedStringBox.getValue().isEmpty()) {
            for(auto &fragments : attributedStringBox.getValue().getFragments()) {
               LOG(INFO) << "\t\t\t String : " << fragments.string.c_str();
               LOG(INFO) << "\t\t\t isAttachment : " << fragments.isAttachment();
               LOG(INFO) << "\t\t\t parentShadowView : " << &fragments.parentShadowView;
               LOG(INFO) << "\t\t\t\t Tag : " << fragments.parentShadowView.tag;
               LOG(INFO) << "\t\t\t\t Name : " << fragments.parentShadowView.componentName;
               LOG(INFO) << "\t\t\t textAttributes :";
               LOG(INFO) << "\t\t\t\t fontFamily: " << fragments.textAttributes.fontFamily.c_str();
               LOG(INFO) << "\t\t\t\t fontSize: " << fragments.textAttributes.fontSize;
               LOG(INFO) << "\t\t\t\t fontSizeMultiplier: " << fragments.textAttributes.fontSizeMultiplier;
               if(fragments.textAttributes.fontWeight.has_value())
                  LOG(INFO) << "\t\t\t\t fontWeight: " << static_cast<int>(fragments.textAttributes.fontWeight.value());
               if(fragments.textAttributes.fontStyle.has_value())
                  LOG(INFO) << "\t\t\t\t fontStyle: " << static_cast<int>(fragments.textAttributes.fontStyle.value());
               if(fragments.textAttributes.fontVariant.has_value())
                  LOG(INFO) << "\t\t\t\t fontVariant: " << static_cast<int>(fragments.textAttributes.fontVariant.value());
               if(fragments.textAttributes.allowFontScaling.has_value())
                  LOG(INFO) << "\t\t\t\t allowFontScaling: " << static_cast<int>(fragments.textAttributes.allowFontScaling.value());
               LOG(INFO) << "\t\t\t\t letterSpacing: " << fragments.textAttributes.letterSpacing;
               LOG(INFO) << "\t\t\t\t lineHeight: " << fragments.textAttributes.lineHeight;
               if(fragments.textAttributes.alignment.has_value())
                  LOG(INFO) << "\t\t\t\t alignment: " << static_cast<int>(fragments.textAttributes.alignment.value());
               if(fragments.textAttributes.baseWritingDirection.has_value())
                  LOG(INFO) << "\t\t\t\t baseWritingDirection: " << static_cast<int>(fragments.textAttributes.baseWritingDirection.value());
               if(fragments.textAttributes.layoutDirection.has_value())
                  LOG(INFO) << "\t\t\t\t layoutDirection: " << static_cast<int>(fragments.textAttributes.layoutDirection.value());
           }
       }
   }
}

void debugParagraphAttributes(ParagraphAttributes paragraphAttributes) {
  LOG(INFO) << "\t ParagraphAttributes :";
  LOG(INFO) << "\t\t maximumNumberOfLines : " << paragraphAttributes.maximumNumberOfLines;
  LOG(INFO) << "\t\t ellipsizeMode : " << static_cast<int>(paragraphAttributes.ellipsizeMode);
  LOG(INFO) << "\t\t textBreakStrategy : " << static_cast<int>(paragraphAttributes.textBreakStrategy);
  LOG(INFO) << "\t\t adjustsFontSizeToFit : " << paragraphAttributes.adjustsFontSizeToFit;
  LOG(INFO) << "\t\t includeFontPadding : " << paragraphAttributes.includeFontPadding;
  LOG(INFO) << "\t\t minimumFontSize : " << paragraphAttributes.minimumFontSize;
  LOG(INFO) << "\t\t maximumFontSize : " << paragraphAttributes.maximumFontSize;
}

void debugLayoutConstraints(LayoutConstraints layoutConstraints) {
  LOG(INFO) << "\t LayoutConstraints :";
  LOG(INFO) << "\t\t minimumSize : " << layoutConstraints.minimumSize.width << "-" << layoutConstraints.minimumSize.height;
  LOG(INFO) << "\t\t maximumSize : "<< layoutConstraints.maximumSize.width << "-" << layoutConstraints.maximumSize.height;
  LOG(INFO) << "\t\t layoutDirection : " << static_cast<int>(layoutConstraints.layoutDirection);
}
#endif /*RNS_DEBUG_TEXT_ATTRIBUTES*/

TextLayoutManager::~TextLayoutManager() {}

TextMeasurement TextLayoutManager::measure(
    AttributedStringBox attributedStringBox,
    ParagraphAttributes paragraphAttributes,
    LayoutConstraints layoutConstraints) const {
#ifdef RNS_DEBUG_TEXT_ATTRIBUTES
  LOG(INFO) << " measure :";
  debugAttributedStringBox(attributedStringBox);
  debugParagraphAttributes(paragraphAttributes);
  debugLayoutConstraints(layoutConstraints);
#endif /*RNS_DEBUG_TEXT_ATTRIBUTES*/

  auto &attributedString = attributedStringBox.getValue();
  auto measurement = TextMeasurement{};
  measurement = measureCache_.get(
      {attributedString, paragraphAttributes, layoutConstraints},
      [&](TextMeasureCacheKey const &key) {
        return doMeasure(
            attributedString, paragraphAttributes, layoutConstraints);
      });
  measurement.size = layoutConstraints.clamp(measurement.size);
  return measurement;
}

LinesMeasurements TextLayoutManager::measureLines(
    AttributedString attributedString,
    ParagraphAttributes paragraphAttributes,
    Size size) const {

  /* TODO Calculate line metrics */
  return LinesMeasurements{};
}

} // namespace react
} // namespace facebook
