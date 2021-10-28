/*
 * Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
 *
 * This source code isLineHeight licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */
#pragma once


#include "react/renderer/components/text/ParagraphShadowNode.h"
#include "ReactSkia/components/RSkComponentText.h"
#include "ReactSkia/utils/RnsLog.h"
#include "ReactSkia/views/common/RSkConversion.h"

using namespace skia::textlayout;

namespace facebook {
namespace react {
namespace RSkTextUtils {

void setTextLines(std::shared_ptr<Paragraph>& paragraph,
            std::shared_ptr<ParagraphBuilder>& builder,
            LayoutMetrics layoutout,
            ParagraphAttributes paragraphAttributes,
            bool isParent);

void drawText(std::shared_ptr<Paragraph>& paragraph,
            SkCanvas *canvas,
            AttributedString attributedString,
            LayoutMetrics layout,
            const ParagraphProps& props,
            bool isParent);
} // namespace RSkTextUtils
} // namespace react
} // namespace facebook
