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

struct ShadowMetrics{
    SharedColor shadowColor{};
    Size shadowOffset{0, 0};
    Float shadowOpacity{0};
    Float shadowRadius{0};
};
                               Rect frame,
                               BorderMetrics borderMetrics,
                               SharedColor bgColor,
                               Float opacity);
  void drawBorder(SkCanvas *canvas,
                               Rect frame,
                               BorderMetrics borderMetrics,
                               SharedColor bgColor,
                               Float opacity);
  void drawShadow(SkCanvas *canvas,
                               Rect frame,
                               BorderMetrics borderMetrics,
                               ShadowMetrics shadowMetrics);

}//namespace RSkDrawUtils
} // namespace react
} // namespace facebook
