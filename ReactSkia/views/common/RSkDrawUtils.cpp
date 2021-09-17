 /*
 * Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "ReactSkia/views/common/RSkDrawUtils.h"
#include "include/core/SkPaint.h"
<<<<<<< HEAD
<<<<<<< HEAD
#include "include/core/SkMaskFilter.h"
#include <math.h>

#define DEFAULT_COLOUR   SK_ColorBLACK /*Black*/

namespace facebook {
namespace react {
namespace {

enum DrawMethod {
     Background,
     Border 
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
void setPathEffect(BorderStyle borderStyle,int strokeWidth,SkPaint *paint)
{
    float dashInterval[]={strokeWidth,strokeWidth/2};
    float dotInterval[]={0,strokeWidth+3};
    
    if(borderStyle == BorderStyle::Dashed){
         int intervalCount=(int)SK_ARRAY_COUNT(dashInterval);
         paint->setPathEffect(SkDashPathEffect::Make(dashInterval,intervalCount,0));
         paint->setStrokeJoin(SkPaint::kRound_Join);
    }
    if(borderStyle == BorderStyle::Dotted){
         int intervalCount=(int)SK_ARRAY_COUNT(dotInterval);
         paint->setPathEffect(SkDashPathEffect::Make(dotInterval,intervalCount,0));
         paint->setStrokeJoin(SkPaint::kRound_Join);
         paint->setStrokeCap(SkPaint::kRound_Cap);
    }
}
void setStyle(int strokeWidth,SkPaint::Style style,BorderStyle borderStyle,SkPaint *paint)
{
    paint->setStyle(style);
    paint->setStrokeWidth(strokeWidth);
    if((borderStyle == BorderStyle::Dashed )|| (borderStyle == BorderStyle::Dotted)){
        setPathEffect(borderStyle,strokeWidth,paint);
    }
}
void setColor(SharedColor Color,Float opacity,SkPaint *paint)
{
    float ratio = 255.9999;
    paint->setAntiAlias(true);
    if(Color){
        auto colorValue=colorComponentsFromColor(Color);
        paint->setColor(SkColorSetARGB(
            colorValue.alpha * ratio,
            colorValue.red * ratio,
            colorValue.green * ratio,
            colorValue.blue * ratio));
    }else{
        paint->setColor(DEFAULT_COLOUR);
    }
    paint->setAlphaf((opacity >1.0 ? 1.0:opacity));
}
bool isDrawVisible(SharedColor Color,Float opacity,Float thickness=1.0)
{
    return (Color !=clearColor() && opacity >0.0 && thickness > 0.0)? true:false;
}
bool hasUniformBorderEdges(BorderMetrics borderProps)
{
    return  ( borderProps.borderColors.isUniform() &&  borderProps.borderWidths.isUniform());
}
void createEdge(PathMetrics pathMetrics,BorderEdges borderEdge,SkPath* path)
{
/*Considered the Path construct requested needs to be constructed in clockwise direction*/

    bool verticalEdge=(((borderEdge == LeftEdge) || (borderEdge == RightEdge)) ? true : false);
    bool growCW=(((borderEdge == TopEdge) ||(borderEdge == RightEdge)) ? true : false);
    bool cornerOnRightEdge=(((borderEdge == BottomEdge) ||(borderEdge== RightEdge)||(borderEdge == TopEdge)) ? true : false);
    bool cornerOnBottomEdge=(((borderEdge == LeftEdge) ||(borderEdge == BottomEdge ) ||(borderEdge == RightEdge)) ? true : false);

    auto outterSweepAngle =(growCW ? 45 : -45);

    Point outterPathStart{ pathMetrics.outterStart.x,pathMetrics.outterStart.y};
    Point innerPathStart{ pathMetrics.innerStart.x ,pathMetrics.innerStart.y};
    Point outterPathEnd{pathMetrics.outterEnd.x,pathMetrics.outterEnd.y};
    Point innerPathEnd{pathMetrics.innerEnd.x,pathMetrics.innerEnd.y};

    SkRect outterStartRect,outterEndRect;
    Point cornerRect{0,0};
    Point pathClosurePoint{outterPathStart};

    if(pathMetrics.startRadius){
        cornerRect.x =((cornerOnRightEdge && verticalEdge)? outterPathStart.x - (pathMetrics.startRadius*2):outterPathStart.x);
        cornerRect.y =((cornerOnBottomEdge && !verticalEdge )? outterPathStart.y-(pathMetrics.startRadius*2): outterPathStart.y);
        outterStartRect=SkRect::MakeXYWH(cornerRect.x,cornerRect.y,pathMetrics.startRadius*2,pathMetrics.startRadius*2);
    }
    if(pathMetrics.endRadius){
        cornerRect.x =(cornerOnRightEdge ? outterPathEnd.x - (pathMetrics.endRadius*2):outterPathEnd.x );
        cornerRect.y =(cornerOnBottomEdge ? outterPathEnd.y-(pathMetrics.endRadius*2): outterPathEnd.y);
        outterEndRect=SkRect::MakeXYWH(cornerRect.x,cornerRect.y,pathMetrics.endRadius*2,pathMetrics.endRadius*2);
        outterPathEnd.x= (verticalEdge ? outterPathEnd.x : (outterPathEnd.x - pathMetrics.endRadius));
        outterPathEnd.y= (verticalEdge ? (outterPathEnd.y - pathMetrics.endRadius):outterPathEnd.y);
    }
    /*Path Building*/
    path->moveTo(outterPathStart.x,outterPathStart.y);
    if(pathMetrics.startRadius){
        path->addArc(outterStartRect,pathMetrics.angle-outterSweepAngle,outterSweepAngle);
        Point centerPoint{outterStartRect.centerX(),outterStartRect.centerY()};
        pathClosurePoint.x=centerPoint.x+(pathMetrics.startRadius *cos((pathMetrics.angle-outterSweepAngle)*(M_PI/180)));
        pathClosurePoint.y=centerPoint.y+(pathMetrics.startRadius *sin((pathMetrics.angle-outterSweepAngle)*(M_PI/180)));
    }
    path->lineTo(outterPathEnd.x,outterPathEnd.y);
    if(pathMetrics.endRadius){
        path->addArc(outterEndRect,pathMetrics.angle,outterSweepAngle);
    }
    path->lineTo(innerPathEnd.x,innerPathEnd.y);
    /*To Do Enchancement: Corner Radius to be applied for inner path aswell.
           Need for inner Radius happens, when border width < border Radius */
    path->lineTo(innerPathStart.x,innerPathStart.y);
    path->lineTo(pathClosurePoint.x,pathClosurePoint.y);
}
void drawRect(DrawMethod drawMethod,SkCanvas *canvas,
                                        Rect frame,
                                        BorderMetrics borderProps,
                                        SharedColor Color,
                                        Float opacity,
                                        SkPaint *paint=NULL)
{
    if(canvas == NULL) return;
/*Case DrawRect assumes same width for all the sides.So referring left */
}
void drawBorder(SkCanvas *canvas,
                               Rect frame,
                               BorderMetrics borderProps,
                               SharedColor backgroundColor,
                               Float opacity)
{
    if(hasUniformBorderEdges(borderProps) && \
       ((backgroundColor != borderProps.borderColors.left)&& \
   /*Skia draw with hairline thickness in case of Strokewidth Zero. So avoid drawing border if 
     borderWidth/StrokeWidth is Zero*/
          (isDrawVisible(borderProps.borderColors.left,opacity,borderProps.borderWidths.left) ))){
             drawRect(Border,canvas,frame,borderProps,borderProps.borderColors.left,opacity);
    }else{
         /*Draw Right Side*/
         if((backgroundColor != borderProps.borderColors.right) && \
                 (isDrawVisible(borderProps.borderColors.right,opacity,borderProps.borderWidths.right) )){
             drawEdges(RightEdge,canvas,frame,borderProps,backgroundColor,opacity);
         }
         /*Draw Left Side*/
         if((backgroundColor != borderProps.borderColors.left) && \
                 (isDrawVisible(borderProps.borderColors.left,opacity,borderProps.borderWidths.left))){
             drawEdges(LeftEdge,canvas,frame,borderProps,backgroundColor,opacity);
         }
         /*Draw Top Side*/
         if((backgroundColor != borderProps.borderColors.top) && \
                 (isDrawVisible(borderProps.borderColors.top,opacity,borderProps.borderWidths.top) )){
             drawEdges(TopEdge,canvas,frame,borderProps,backgroundColor,opacity);
         }
         /*Draw Bottom Side*/
         if((backgroundColor != borderProps.borderColors.bottom) && \
                 (isDrawVisible(borderProps.borderColors.bottom,opacity,borderProps.borderWidths.bottom))){
             drawEdges(BottomEdge,canvas,frame,borderProps,backgroundColor,opacity);
         }
    }
}
void  drawShadow(SkCanvas *canvas, 
                               Rect frame,
                               BorderMetrics borderProps,
                               ShadowMetrics shadowMetrics)
{
 
    if(isDrawVisible(shadowMetrics.shadowColor,shadowMetrics.shadowOpacity)){
/*+ve shadowOffset.width  => move shadow towards Right, -ve => move shadow towards left
  +ve shadowOffset.height => move shadow towards bottom, -ve => mov shadow towards top
 */
         frame.origin.x= frame.origin.x+shadowMetrics.shadowOffset.width;
         frame.origin.y=frame.origin.y+shadowMetrics.shadowOffset.height;
/*Shadow effect acheived by drawing background with Blur effect*/
         SkPaint paint;
         paint.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, shadowMetrics.shadowRadius));
         drawRect(Background,canvas,frame,borderProps,shadowMetrics.shadowColor,shadowMetrics.shadowOpacity,&paint);
    }
} 
} // namespace RSkDrawUtils
} // namespace react
} // namespace facebook
