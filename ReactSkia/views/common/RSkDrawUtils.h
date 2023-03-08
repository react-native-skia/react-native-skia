/*
 * Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */
#pragma once

#include "include/core/SkCanvas.h"
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
                               SkColor shadowColor,
                               SkSize shadowOffset,
                               float shadowOpacity,
                               float frameOpacity,
                               sk_sp<SkImageFilter> shadowImageFilter,
                               sk_sp<SkMaskFilter> shadowMaskFilter
                  );

  void drawUnderline(SkCanvas *canvas,Rect frame,SharedColor underlineColor);


  SkIRect getShadowBounds(const SkIRect shadowFrame,
                        sk_sp<SkMaskFilter> shadowMaskFilter,
                        sk_sp<SkImageFilter> shadowImageFilter=nullptr);

}//namespace RSkDrawUtils
} // namespace react
} // namespace facebook
