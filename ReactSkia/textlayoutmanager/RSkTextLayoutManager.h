/*
 * Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once
#include <react/renderer/attributedstring/AttributedString.h>
#include <react/renderer/attributedstring/ParagraphAttributes.h>
#include <react/renderer/core/LayoutConstraints.h>
#include <react/renderer/graphics/Geometry.h>
#include <react/renderer/textlayoutmanager/TextMeasureCache.h>
#include "modules/skparagraph/include/Paragraph.h"
#include "modules/skparagraph/include/ParagraphBuilder.h"
#include "modules/skparagraph/src/ParagraphBuilderImpl.h"

#include "include/core/SkImageFilter.h"
#include "include/effects/SkImageFilters.h"

namespace facebook {
namespace react {

   struct RSkSkTextLayout {
      skia::textlayout::TextShadow shadow;
      skia::textlayout::ParagraphStyle paraStyle;
      std::shared_ptr<skia::textlayout::Paragraph> paragraph{nullptr};
      std::shared_ptr<skia::textlayout::ParagraphBuilder> builder{nullptr};
   };

class RSkTextLayoutManager {

public:
   

   RSkTextLayoutManager();
   ~RSkTextLayoutManager(){};

   /* Measures attributedString layout using skia's skparagraph module */ 
   TextMeasurement doMeasure (SharedColor backGroundColor, 
                                 AttributedString attributedString,
                                 ParagraphAttributes paragraphAttributes,
                                 LayoutConstraints layoutConstraints) const;
   
   void buildText (struct RSkSkTextLayout &textLayout,
                     SharedColor backGroundColor,
                     ParagraphAttributes paragraphAttributes,
                     TextAttributes textAttributes,
                     std::string textString,
                     bool fontDecorationRequired) const;

   /* Build attributedString  paragraph using skia's skparagraph module */ 
   /* Set fontDecorationRequired to true, to consider font paint & decoration attributes for paragraph build */
   uint32_t buildParagraph (struct RSkSkTextLayout &textLayout,
                              SharedColor backGroundColor,
                              AttributedString attributedString,
                              ParagraphAttributes paragraphAttributes,
                              bool fontDecorationRequired=false) const;

   /* Font collection manager */
   sk_sp<skia::textlayout::FontCollection> collection_;

};

} // namespace react
} // namespace facebook
