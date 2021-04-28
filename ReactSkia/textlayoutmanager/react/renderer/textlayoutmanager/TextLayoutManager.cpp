/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "TextLayoutManager.h"

namespace facebook {
namespace react {

#ifdef PRINT_TEXT_ATTRIBUTES
void printAttributedStringBox(AttributedStringBox attributedStringBox) {
  
  printf("\n\t\t AttributedStringBox :");
  printf("\n\t\t\t Mode : [%d]", attributedStringBox.getMode());
  if(attributedStringBox.getMode() == AttributedStringBox::Mode::OpaquePointer) {
      printf("\n\t\t\tOpaque Pointer : [%p]", attributedStringBox.getOpaquePointer().get());
  }
  else {
      printf("\n\t\t\t Value : ");
      if( attributedStringBox.getValue().isEmpty())
            printf("Empty  \n");
      else {
            for(auto &fragments : attributedStringBox.getValue().getFragments())
	    {
		printf("\n\t\t\t\t String : %s",fragments.string.c_str());
		printf("\n\t\t\t\t isAttachment : %d",fragments.isAttachment());
		printf("\n\t\t\t\t parentShadowView : %p",&fragments.parentShadowView);
		printf("\n\t\t\t\t\t Tag : %d",fragments.parentShadowView.tag);
		printf("\n\t\t\t\t\t Name : %s",fragments.parentShadowView.componentName);
		printf("\n\t\t\t\t textAttributes :");
		printf("\n\t\t\t\t\t fontFamily: %s",fragments.textAttributes.fontFamily.c_str());
                printf("\n\t\t\t\t\t fontSize: %f",fragments.textAttributes.fontSize);
                printf("\n\t\t\t\t\t fontSizeMultiplier: %f",fragments.textAttributes.fontSizeMultiplier);
		if(fragments.textAttributes.fontWeight.has_value())
                  printf("\n\t\t\t\t\t fontWeight: %d",fragments.textAttributes.fontWeight.value());
		if(fragments.textAttributes.fontStyle.has_value())
                  printf("\n\t\t\t\t\t fontStyle: %d",fragments.textAttributes.fontStyle.value());
		if(fragments.textAttributes.fontVariant.has_value())
                  printf("\n\t\t\t\t\t fontVariant: %d",fragments.textAttributes.fontVariant.value());
		if(fragments.textAttributes.allowFontScaling.has_value())
                  printf("\n\t\t\t\t\t allowFontScaling: %d",fragments.textAttributes.allowFontScaling.value());
                printf("\n\t\t\t\t\t letterSpacing: %f",fragments.textAttributes.letterSpacing);
                printf("\n\t\t\t\t\t lineHeight: %f",fragments.textAttributes.lineHeight);
		if(fragments.textAttributes.alignment.has_value())
                  printf("\n\t\t\t\t\t alignment: %d",fragments.textAttributes.alignment.value());
		if(fragments.textAttributes.baseWritingDirection.has_value())
                  printf("\n\t\t\t\t\t baseWritingDirection: %d",fragments.textAttributes.baseWritingDirection.value());
		if(fragments.textAttributes.layoutDirection.has_value())
                  printf("\n\t\t\t\t\t layoutDirection: %d",fragments.textAttributes.layoutDirection.value());
		printf("\n");
	    }
      } 
  }
}

void printParagraphAttributes(ParagraphAttributes paragraphAttributes) {
  
  printf("\n\t\t ParagraphAttributes :");
  printf("\n\t\t\t maximumNumberOfLines : %d",paragraphAttributes.maximumNumberOfLines);
  printf("\n\t\t\t ellipsizeMode : %d",paragraphAttributes.ellipsizeMode);
  printf("\n\t\t\t textBreakStrategy : %d",paragraphAttributes.textBreakStrategy);
  printf("\n\t\t\t adjustsFontSizeToFit : %d",paragraphAttributes.adjustsFontSizeToFit);
  printf("\n\t\t\t includeFontPadding : %d",paragraphAttributes.includeFontPadding);
  printf("\n\t\t\t minimumFontSize : %f",paragraphAttributes.minimumFontSize);
  printf("\n\t\t\t maximumFontSize : %f",paragraphAttributes.maximumFontSize);
  printf("\n");

}

void printLayoutConstraints(LayoutConstraints layoutConstraints) {
 
  printf("\n\t\t LayoutConstraints :");
  printf("\n\t\t\t minimumSize : %f - %f",layoutConstraints.minimumSize.width,layoutConstraints.minimumSize.height);
  printf("\n\t\t\t maximumSize : %f - %f",layoutConstraints.maximumSize.width,layoutConstraints.maximumSize.height);
  printf("\n\t\t\t layoutDirection : %d",layoutConstraints.layoutDirection);
  printf("\n");
}
#endif /*PRINT_TEXT_ATTRIBUTES*/

TextLayoutManager::~TextLayoutManager() {}

void *TextLayoutManager::getNativeTextLayoutManager() const {
  return self_;
}

TextMeasurement TextLayoutManager::measure(
    AttributedStringBox attributedStringBox,
    ParagraphAttributes paragraphAttributes,
    LayoutConstraints layoutConstraints) const {

#ifdef PRINT_TEXT_ATTRIBUTES
  printAttributedStringBox(attributedStringBox);
  printParagraphAttributes(paragraphAttributes);
  printLayoutConstraints(layoutConstraints);
#endif /*PRINT_TEXT_ATTRIBUTES*/

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
