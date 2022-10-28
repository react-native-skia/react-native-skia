/*
 * Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */
#pragma once

#include "include/core/SkPaint.h"
#include "include/core/SkCanvas.h"
#include "include/effects/SkDashPathEffect.h"
#include <react/renderer/components/view/ViewProps.h>


namespace facebook {
namespace react {

namespace RSkDrawUtils{

/*Function: Draw Background & Border */
  void drawBackground(SkCanvas *canvas,
                               Rect frame,
                               BorderMetrics borderMetrics,
                               SharedColor bgColor);
  void drawBorder(SkCanvas *canvas,
                               Rect frame,
                               BorderMetrics borderMetrics,
                               SharedColor bgColor);
  bool drawShadow(SkCanvas *canvas,
                               Rect frame,
                               BorderMetrics borderMetrics,
                               SharedColor bgColor,
                               Float shadowOpacity,
                               sk_sp<SkImageFilter> shadowFilter);
  void drawUnderline(SkCanvas *canvas,Rect frame,SharedColor underlineColor);

}//namespace RSkDrawUtils
} // namespace react
} // namespace facebook
