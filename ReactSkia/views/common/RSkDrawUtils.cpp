 /*
 * Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "ReactSkia/views/common/RSkDrawUtils.h"
#include "include/core/SkPaint.h"
#include <math.h>

namespace facebook {
namespace react {

RSkDrawUtils::RSkDrawUtils() {}


void RSkDrawUtils::drawBackground(SkCanvas *canvas, 
                               Rect frame,
                               BorderMetrics borderProps,
                               SharedColor backgroundColor,
                               Float opacity)
{

    if(isDrawVisible(backgroundColor,opacity)){
         drawRect(DrawFillRect,canvas,frame,borderProps,backgroundColor,opacity);
    }
}

void RSkDrawUtils::drawBorder(SkCanvas *canvas,
                               Rect frame,
                               BorderMetrics borderProps,
                               SharedColor backgroundColor,
                               Float opacity)
{
    if(hasUniformBorderEdges(borderProps) && \
       ((backgroundColor != borderProps.borderColors.left)&& \
          (isDrawVisible(borderProps.borderColors.left,opacity) ))){
         drawRect(DrawRect,canvas,frame,borderProps,borderProps.borderColors.left,opacity);

    }else{
         /*Draw Right Side*/
         if((backgroundColor != borderProps.borderColors.right) && \
                 (isDrawVisible(borderProps.borderColors.right,opacity) )){
             drawEdges(RightEdge,canvas,frame,borderProps,backgroundColor,opacity);
         }
         /*Draw Left Side*/
         if((backgroundColor != borderProps.borderColors.left) && \
                 (isDrawVisible(borderProps.borderColors.left,opacity))){
             drawEdges(LeftEdge,canvas,frame,borderProps,backgroundColor,opacity);
         }
         /*Draw Top Side*/
         if((backgroundColor != borderProps.borderColors.top) && \
                 (isDrawVisible(borderProps.borderColors.top,opacity) )){
             drawEdges(TopEdge,canvas,frame,borderProps,backgroundColor,opacity);
         }
         /*Draw Bottom Side*/
         if((backgroundColor != borderProps.borderColors.bottom) && \
                 (isDrawVisible(borderProps.borderColors.bottom,opacity))){
             drawEdges(BottomEdge,canvas,frame,borderProps,backgroundColor,opacity);
         }
    }
}


void RSkDrawUtils::drawRect(DrawMethod drawMethod,SkCanvas *canvas,
                                        Rect frame,
                                        BorderMetrics borderProps,
                                        SharedColor Color,
                                        Float opacity)
{

    if(canvas == NULL) return;

    auto rectStrokeWidth = borderProps.borderWidths.left;

    SkRRect rRect ;
    SkRect rect;
    SkPaint paint;

  /*Creating basic layout from props*/
    rect=SkRect::MakeXYWH(frame.origin.x,frame.origin.y,\
         frame.size.width,frame.size.height);

    SkVector radii[4]={{borderProps.borderRadii.topLeft,borderProps.borderRadii.topLeft},
                       {borderProps.borderRadii.topRight,borderProps.borderRadii.topRight}, \
                       {borderProps.borderRadii.bottomLeft,borderProps.borderRadii.bottomLeft}, \
                       {borderProps.borderRadii.bottomRight,borderProps.borderRadii.bottomRight } };

    setColor(Color,opacity,&paint);
    /* To sync with the border draw type, resetting the stroke width for background*/
    if(!hasUniformBorderEdges(borderProps) && (drawMethod == DrawFillRect))
        rectStrokeWidth=0;

    /*Border adjustment needed in case of stroke width, as half the pixels where drawn outside and half inside by SKIA*/
      if(rectStrokeWidth > 0){
          rect.inset(rectStrokeWidth/2,rectStrokeWidth/2);
      }
      rRect.setRectRadii(rect,radii);

    if(drawMethod == DrawFillRect){
        setStyle(rectStrokeWidth,SkPaint::kStrokeAndFill_Style,BorderStyle::Solid,&paint);
    }else if(drawMethod == DrawRect){
        setStyle(rectStrokeWidth,SkPaint::kStroke_Style,borderProps.borderStyles.left,&paint);
    }
    canvas->drawRRect(rRect, paint);
}
void RSkDrawUtils::drawEdges(BorderEdges borderEdge,SkCanvas *canvas,
                                        Rect frame,
                                        BorderMetrics borderProps,
                                        SharedColor backgroundColor,
                                        Float opacity)
{

    if(canvas == NULL) return;

    SkPath path;
    SkPaint paint;
    PathMetrics pathMetrics;

     /*Constructing draw cordinates*/
    auto rectOriginX=frame.origin.x;
    auto rectOriginY=frame.origin.y;
    auto rectDestX=frame.origin.x+frame.size.width;
    auto rectDestY=frame.origin.y+frame.size.height;

    /*Setting up default Value*/
    auto strokeWidth = borderProps.borderWidths.left;
    auto edgeColor=borderProps.borderColors.left;

    if(borderEdge == RightEdge){
        edgeColor=borderProps.borderColors.right;
        strokeWidth=borderProps.borderWidths.right;

        pathMetrics.outterStart.x=rectDestX;
        pathMetrics.outterStart.y=rectOriginY;
        pathMetrics.outterEnd.x=rectDestX;
        pathMetrics.outterEnd.y=rectDestY;
        pathMetrics.innerStart.x=rectDestX-strokeWidth;
        pathMetrics.innerStart.y=rectOriginY+borderProps.borderWidths.top;
        pathMetrics.innerEnd.x=rectDestX-strokeWidth;
        pathMetrics.innerEnd.y=rectDestY-borderProps.borderWidths.bottom;
        pathMetrics.startRadius=borderProps.borderRadii.topRight;
        pathMetrics.endRadius=borderProps.borderRadii.bottomRight;
        pathMetrics.width=strokeWidth;
        pathMetrics.angle=0;

    }
    if(borderEdge == BottomEdge){
        edgeColor=borderProps.borderColors.bottom;
        strokeWidth=borderProps.borderWidths.bottom;

        pathMetrics.outterStart.x=rectOriginX;
        pathMetrics.outterStart.y=rectDestY;
        pathMetrics.outterEnd.x=rectDestX;
        pathMetrics.outterEnd.y=rectDestY;
        pathMetrics.innerStart.x=rectOriginX+borderProps.borderWidths.left;
        pathMetrics.innerStart.y=rectDestY-strokeWidth;
        pathMetrics.innerEnd.x=rectDestX-borderProps.borderWidths.right;
        pathMetrics.innerEnd.y=rectDestY-strokeWidth;
        pathMetrics.startRadius=borderProps.borderRadii.bottomLeft;
        pathMetrics.endRadius=borderProps.borderRadii.bottomRight;
        pathMetrics.width=strokeWidth;
        pathMetrics.angle=90;
    }
     if(borderEdge == LeftEdge){
         edgeColor=borderProps.borderColors.left;
         strokeWidth=borderProps.borderWidths.left;
     
         pathMetrics.outterStart.x=rectOriginX;
         pathMetrics.outterStart.y=rectOriginY;
         pathMetrics.outterEnd.x=rectOriginX;
         pathMetrics.outterEnd.y=rectDestY;
         pathMetrics.innerStart.x=rectOriginX+strokeWidth;
         pathMetrics.innerStart.y=rectOriginY+borderProps.borderWidths.top;
         pathMetrics.innerEnd.x=rectOriginX+strokeWidth;
         pathMetrics.innerEnd.y=rectDestY-borderProps.borderWidths.bottom;
         pathMetrics.startRadius=borderProps.borderRadii.topLeft;
         pathMetrics.endRadius=borderProps.borderRadii.bottomLeft;
         pathMetrics.width=strokeWidth;
         pathMetrics.angle=180;
     }
     if(borderEdge == TopEdge){
         edgeColor=borderProps.borderColors.top;
         strokeWidth=borderProps.borderWidths.top;
     
         pathMetrics.outterStart.x=rectOriginX;
         pathMetrics.outterStart.y=rectOriginY;
         pathMetrics.outterEnd.x=rectDestX;
         pathMetrics.outterEnd.y=rectOriginY;
         pathMetrics.innerStart.x=rectOriginX+borderProps.borderWidths.left;
         pathMetrics.innerStart.y=rectOriginY+strokeWidth;
         pathMetrics.innerEnd.x=rectDestX-borderProps.borderWidths.right;
         pathMetrics.innerEnd.y=rectOriginY+strokeWidth;
         pathMetrics.startRadius=borderProps.borderRadii.topLeft;
         pathMetrics.endRadius=borderProps.borderRadii.topRight;
         pathMetrics.width=strokeWidth;
         pathMetrics.angle=270;
     }

     createEdge(pathMetrics,borderEdge,&path);
     setColor(edgeColor,opacity,&paint);
     path.setFillType(SkPathFillType::kEvenOdd);

     canvas->drawPath(path, paint);
}
void RSkDrawUtils::createEdge(PathMetrics pathMetrics,BorderEdges borderEdge,SkPath* path)
{
/*Considered the Path construct requested needs to be constructed in clockwise direction*/

    bool verticalEdge=(((borderEdge == LeftEdge) || (borderEdge == RightEdge)) ? true : false);
    bool growCW=(((borderEdge == TopEdge) ||(borderEdge == RightEdge)) ? true : false);
    bool cornerOnRightEdge=(((borderEdge == BottomEdge) ||(borderEdge== RightEdge)||(borderEdge == TopEdge)) ? true : false);
    bool cornerOnBottomEdge=(((borderEdge == LeftEdge) ||(borderEdge == BottomEdge ) ||(borderEdge == RightEdge)) ? true : false);

    auto outterSweepAngle =(growCW ? 45 : -45);
    auto innerSweepAngle= (growCW ? -45 : +45);;

    auto outterStartX= pathMetrics.outterStart.x ;
    auto outterStartY= pathMetrics.outterStart.y ;
    auto innerStartX= pathMetrics.innerStart.x ;
    auto innerStartY= pathMetrics.innerStart.y ;
    auto outterEndX= pathMetrics.outterEnd.x ;
    auto outterEndY= pathMetrics.outterEnd.y ;
    auto innerEndX= pathMetrics.innerEnd.x ;
    auto innerEndY= pathMetrics.innerEnd.y ;

    auto innerEndRadius = 0;
    auto innerStartRadius =0 ;
    SkRect innerStartRect,innerEndRect,outterStartRect,outterEndRect;
    Point cornerRect{0,0};
    Point pathClosurePoint{outterStartX,outterStartY};

    if(pathMetrics.startRadius){
        cornerRect.x =((cornerOnRightEdge && verticalEdge)? outterStartX - (pathMetrics.startRadius*2):outterStartX);
        cornerRect.y =((cornerOnBottomEdge && !verticalEdge )? outterStartY-(pathMetrics.startRadius*2): outterStartY);
        outterStartRect=SkRect::MakeXYWH(cornerRect.x,cornerRect.y,pathMetrics.startRadius*2,pathMetrics.startRadius*2);
    }
    if(pathMetrics.endRadius){
        cornerRect.x =(cornerOnRightEdge ? outterEndX - (pathMetrics.endRadius*2):outterEndX );
        cornerRect.y =(cornerOnBottomEdge ? outterEndY-(pathMetrics.endRadius*2): outterEndY);
        outterEndRect=SkRect::MakeXYWH(cornerRect.x,cornerRect.y,pathMetrics.endRadius*2,pathMetrics.endRadius*2);
        outterEndX= (verticalEdge ? outterEndX : (outterEndX - pathMetrics.endRadius));
        outterEndY= (verticalEdge ? (outterEndY - pathMetrics.endRadius):outterEndY);
    }
    /*Path Building*/
    path->moveTo(outterStartX,outterStartY);
    if(pathMetrics.startRadius){
        path->addArc(outterStartRect,pathMetrics.angle-outterSweepAngle,outterSweepAngle);
        Point centerPoint{outterStartRect.centerX(),outterStartRect.centerY()};
        pathClosurePoint.x=centerPoint.x+(pathMetrics.startRadius *cos((pathMetrics.angle-outterSweepAngle)*(M_PI/180)));
        pathClosurePoint.y=centerPoint.y+(pathMetrics.startRadius *sin((pathMetrics.angle-outterSweepAngle)*(M_PI/180)));
    }
    path->lineTo(outterEndX,outterEndY);
    if(pathMetrics.endRadius){
        path->addArc(outterEndRect,pathMetrics.angle,outterSweepAngle);
    }
    path->lineTo(innerEndX,innerEndY);
    path->lineTo(innerStartX,innerStartY);
    path->lineTo(pathClosurePoint.x,pathClosurePoint.y);

}

void RSkDrawUtils::setStyle(int strokeWidth,SkPaint::Style style,BorderStyle borderStyle,SkPaint *paint)
{
    paint->setStyle(style);
    paint->setStrokeWidth(strokeWidth);
    if((borderStyle == BorderStyle::Dashed )|| (borderStyle == BorderStyle::Dotted)){
        setPathEffect(borderStyle,strokeWidth,paint);
    }
}
void RSkDrawUtils::setColor(SharedColor Color,Float opacity,SkPaint *paint)
{
    float ratio = 255.9999;
    paint->setAntiAlias(true);
    auto colorValue=colorComponentsFromColor(Color);
    paint->setColor(SkColorSetARGB(
        colorValue.alpha * ratio,
        colorValue.red * ratio,
        colorValue.green * ratio,
        colorValue.blue * ratio));
    paint->setAlphaf((opacity/100));
}
void RSkDrawUtils::setPathEffect(BorderStyle borderStyle,int strokeWidth,SkPaint *paint)
{
    float Dash_interval[]={strokeWidth,strokeWidth/2};
    float Dot_interval[]={0,strokeWidth+3};
    
    if(borderStyle == BorderStyle::Dashed){
         int intervalCount=(int)SK_ARRAY_COUNT(Dash_interval);
         paint->setPathEffect(SkDashPathEffect::Make(Dash_interval,intervalCount,0));
         paint->setStrokeJoin(SkPaint::kRound_Join);
    
    }
    if(borderStyle == BorderStyle::Dotted){
         int intervalCount=(int)SK_ARRAY_COUNT(Dash_interval);
         paint->setPathEffect(SkDashPathEffect::Make(Dot_interval,intervalCount,0));
         paint->setStrokeJoin(SkPaint::kRound_Join);
         paint->setStrokeCap(SkPaint::kRound_Cap);
    }
}

bool RSkDrawUtils::isDrawVisible(SharedColor Color,Float opacity)
{
    return (Color !=clearColor() && opacity >0.0)? true:false;
}

bool RSkDrawUtils::hasRoundedBorders(BorderMetrics borderProps)
{
    return  ( (borderProps.borderRadii.topLeft != 0 ) ||   \
              (borderProps.borderRadii.topRight != 0 ) ||  \
              (borderProps.borderRadii.bottomLeft != 0 ) | \
              (borderProps.borderRadii.bottomRight != 0 ));

}

bool RSkDrawUtils::hasUniformBorderEdges(BorderMetrics borderProps)
{
    return  ( borderProps.borderColors.isUniform() &&  borderProps.borderWidths.isUniform());

}


} // namespace react
} // namespace facebook
