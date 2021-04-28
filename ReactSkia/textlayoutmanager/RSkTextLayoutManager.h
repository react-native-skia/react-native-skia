#pragma once
#include <react/renderer/attributedstring/AttributedString.h>
#include <react/renderer/attributedstring/ParagraphAttributes.h>
#include <react/renderer/core/LayoutConstraints.h>
#include <react/renderer/graphics/Geometry.h>
#include <react/renderer/textlayoutmanager/TextMeasureCache.h>
#include "modules/skparagraph/include/Paragraph.h"
#include "modules/skparagraph/include/ParagraphBuilder.h"

namespace facebook {
namespace react {


class RSkTextLayoutManager {

public:
   RSkTextLayoutManager();
   
   ~RSkTextLayoutManager(){};

   /* Measures attributedString layout using skia's skparagraph module */ 
   TextMeasurement doMeasure (AttributedString attributedString,
                                 ParagraphAttributes paragraphAttributes,
                                 LayoutConstraints layoutConstraints) const;

   /* Build attributedString  paragraph using skia's skparagraph module */ 
   /* Set fontDecorationRequired to true, to consider font paint & decoration attributes for paragraph build */
   std::unique_ptr<skia::textlayout::Paragraph> buildParagraph (AttributedString attributedString,
                                                                  ParagraphAttributes paragraphAttributes,
                                                                  Size size,
		                                                  bool fontDecorationRequired=false) const;
   /* Font collection manager */
   sk_sp<skia::textlayout::FontCollection> collection_;
};	

} // namespace react 
} // namespace facebook
