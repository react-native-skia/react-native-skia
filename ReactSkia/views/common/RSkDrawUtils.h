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

class RSkDrawUtils  {
 public:
  RSkDrawUtils();
/*Function: Draw Background & Border */
  void drawBackground(SkCanvas *canvas,
                               Rect frame,
                               BorderMetrics borderMetrics,
                               SharedColor bgColor,
                               Float opacity);
  void drawBorder(SkCanvas *canvas,
                               Rect frame,
                               BorderMetrics borderMetrics,
                               SharedColor bgColor,
                               Float opacity);

 private:
  enum DrawMethod {
     DrawFillRect ,
     DrawRect,
  };
  enum BorderEdges {
     RightEdge = 0,
     LeftEdge,
     TopEdge,
     BottomEdge
  };
  struct PathMetrics{
      Point outterStart{0, 0};
      Point outterEnd{0, 0};
      Point innerStart{0, 0};
      Point innerEnd{0, 0};
      Float startRadius{0};
      Float endRadius{0};
      Float width;
      Float angle;
  };
  void setColor(SharedColor Color,Float opacity,SkPaint *paint);
  void setStyle(int strokeWidth,SkPaint::Style Style,BorderStyle borderStyle,SkPaint *paint);
  void setPathEffect(BorderStyle borderStyle,int strokeWidth,SkPaint *paint );
  bool isDrawVisible(SharedColor Color,Float opacity);
  bool hasRoundedBorders(BorderMetrics borderMetrics);
  bool hasUniformBorderEdges(BorderMetrics borderProps);
  void drawRect(DrawMethod drawMethod,SkCanvas *canvas,
                                Rect frame,
                               BorderMetrics borderMetrics,
                               SharedColor Color,
                               Float opacity);
  void drawEdges(BorderEdges borderEdge,SkCanvas *canvas,
                                Rect frame,
                               BorderMetrics borderProps,
                               SharedColor backGroundColor,
                               Float opacity);
void createEdge(PathMetrics pathMetrics,BorderEdges borderEdge,SkPath* path);

};

} // namespace react
} // namespace facebook
