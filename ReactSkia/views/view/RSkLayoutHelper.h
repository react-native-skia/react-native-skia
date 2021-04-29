/*
* Copyright 2016 Google Inc.
* Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#pragma once

#include "include/core/SkPaint.h"
#include "include/core/SkCanvas.h"
#include "include/effects/SkDashPathEffect.h"

#include <react/renderer/components/view/ViewProps.h>


namespace facebook {
namespace react {

class RSkLayoutHelper  {
 public:
  RSkLayoutHelper();

    enum DrawType {
     None = 0,
     BackGround,
     Border
  };

/*Function: Draw Background & Border using the props stored */

  void RSkDrawBackGround(SkCanvas *canvas,
                               LayoutMetrics layoutMetrics,
                               BorderMetrics borderMetrics,
                               SharedColor bgColor,
                               Float opacity);

  void RSkDrawBorder(SkCanvas *canvas,
                               LayoutMetrics layoutMetrics,
                               BorderMetrics borderMetrics,
                               SharedColor bgColor,
                               Float opacity);


 private:

  enum BorderDrawMethod {
     BorderDrawNone = 0,
     BorderDrawWithRectFill ,
     BorderDrawWithRect,
     BorderDrawWithLines ,
     BorderDrawDefault=BorderDrawNone,
  };

  enum BorderEdges {
     RightEdge = 0,
     LeftEdge,
     TopEdge,
     BottomEdge
  };
  struct PathMetrics{
      Point outterOrigin{0, 0};
      Point outterDest{0, 0};
      Point innerOrigin{0, 0};
      Point innerDest{0, 0};
      Float originRadius{0};
      Float destRadius{0};
      Float width;
      Float angle;
  };
  void SetColor(SharedColor Color,Float opacity,SkPaint *paint);
  void SetStyle(int strokeWidth,SkPaint::Style Style,BorderStyle borderStyle,SkPaint *paint);

  void SetPathEffect(BorderStyle borderStyle,int strokeWidth,SkPaint *paint );
  bool isDrawVisible(SharedColor Color,Float opacity);
  bool hasRoundedBorders(BorderMetrics borderMetrics);
  bool hasUniformBorderEdges(BorderMetrics borderProps);

  void drawRoundedRect(BorderDrawMethod borderDrawMethod,SkCanvas *canvas,
                               LayoutMetrics layoutMetrics,
                               BorderMetrics borderMetrics,
                               SharedColor Color,
                               Float opacity);
  void drawEdges(BorderEdges borderEdge,SkCanvas *canvas,
                               LayoutMetrics layoutMetrics,
                               BorderMetrics borderProps,
                               SharedColor backGroundColor,
                               Float opacity);

void CreateEdge(PathMetrics pathMetrics,BorderEdges borderEdge,SkPath* path);

};

} // namespace react
} // namespace facebook
