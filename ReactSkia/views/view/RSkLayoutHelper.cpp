#include "ReactSkia/views/view/RSkLayoutHelper.h"

#include "include/core/SkPaint.h"

#include <glog/logging.h>
#include <iostream>
#include <math.h>

using namespace std;


namespace facebook {
namespace react {

RSkLayoutHelper::RSkLayoutHelper() {}


void RSkLayoutHelper::RSkDrawBackGround(SkCanvas *canvas, 
                               LayoutMetrics layoutMetrics,
                               BorderMetrics borderProps,
                               SharedColor backGroundColor,
                               Float opacity)
{

    if(isDrawVisible(backGroundColor,opacity))
    {
         drawRoundedRect(BorderDrawWithRectFill,canvas,layoutMetrics,borderProps,backGroundColor,opacity);
    }
}

void RSkLayoutHelper::RSkDrawBorder(SkCanvas *canvas,
                               LayoutMetrics layoutMetrics,
                               BorderMetrics borderProps,
                               SharedColor backGroundColor,
                               Float opacity)
{
    BorderDrawMethod borderDrawMethod=BorderDrawNone;

    if(hasUniformBorderEdges(borderProps) && \
       ((backGroundColor != borderProps.borderColors.left)&& \
          (isDrawVisible(borderProps.borderColors.left,opacity) ))) 
    {
         drawRoundedRect(BorderDrawWithRect,canvas,layoutMetrics,borderProps,borderProps.borderColors.left,opacity);

    }
    else
    {
         /*Draw Right Side*/
         if((backGroundColor != borderProps.borderColors.right) && \
                 (isDrawVisible(borderProps.borderColors.right,opacity) ))
         {
             drawEdges(RightEdge,canvas,layoutMetrics,borderProps,backGroundColor,opacity);
         }
         /*Draw Left Side*/
         if((backGroundColor != borderProps.borderColors.left) && \
                 (isDrawVisible(borderProps.borderColors.left,opacity)))
         {
             drawEdges(LeftEdge,canvas,layoutMetrics,borderProps,backGroundColor,opacity);
         }
         /*Draw Top Side*/
         if((backGroundColor != borderProps.borderColors.top) && \
                 (isDrawVisible(borderProps.borderColors.top,opacity) ))
         {
             drawEdges(TopEdge,canvas,layoutMetrics,borderProps,backGroundColor,opacity);
         }
         /*Draw Bottom Side*/
         if((backGroundColor != borderProps.borderColors.bottom) && \
                 (isDrawVisible(borderProps.borderColors.bottom,opacity)))
         {
             drawEdges(BottomEdge,canvas,layoutMetrics,borderProps,backGroundColor,opacity);
         }
    }
}


void RSkLayoutHelper::drawRoundedRect(BorderDrawMethod borderDrawMethod,SkCanvas *canvas,
                                        LayoutMetrics layoutMetrics,
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
     rect=SkRect::MakeXYWH(layoutMetrics.frame.origin.x,layoutMetrics.frame.origin.y,\
         layoutMetrics.frame.size.width,layoutMetrics.frame.size.height);

    SkVector radii[4]={{borderProps.borderRadii.topLeft,borderProps.borderRadii.topLeft},
                       {borderProps.borderRadii.topRight,borderProps.borderRadii.topRight}, \
                       {borderProps.borderRadii.bottomLeft,borderProps.borderRadii.bottomLeft}, \
                       {borderProps.borderRadii.bottomRight,borderProps.borderRadii.bottomRight } \
                    };

    SetColor(Color,opacity,&paint);

    if(!hasUniformBorderEdges(borderProps) && (borderDrawMethod == BorderDrawWithRectFill))
        rectStrokeWidth=0;

    /*Border adjustment needed in case of stroke width, as half the pixels where drawn outside and half inside by SKIA*/
      if(rectStrokeWidth > 0)
      {
          rect.inset(rectStrokeWidth/2,rectStrokeWidth/2);
      }
      rRect.setRectRadii(rect,radii);

    if(borderDrawMethod == BorderDrawWithRectFill)
    {
        SetStyle(rectStrokeWidth,SkPaint::kStrokeAndFill_Style,BorderStyle::Solid,&paint);
    }
    else if(borderDrawMethod == BorderDrawWithRect)
    {
        SetStyle(rectStrokeWidth,SkPaint::kStroke_Style,borderProps.borderStyles.left,&paint);
    }
    canvas->drawRRect(rRect, paint);
}
void RSkLayoutHelper::drawEdges(BorderEdges borderEdge,SkCanvas *canvas,
                                        LayoutMetrics layoutMetrics,
                                        BorderMetrics borderProps,
                                        SharedColor backGroundColor,
                                        Float opacity)
{

    if(canvas == NULL) return;

    SkPath path;
    SkPaint paint;
    PathMetrics pathMetrics;

     /*Constructing draw cordinates*/
    auto rectOriginX=layoutMetrics.frame.origin.x;
    auto rectOriginY=layoutMetrics.frame.origin.y;
    auto rectDestX=layoutMetrics.frame.origin.x+layoutMetrics.frame.size.width;
    auto rectDestY=layoutMetrics.frame.origin.y+layoutMetrics.frame.size.height;

    /*Setting up default Value*/
    auto strokeWidth = borderProps.borderWidths.left;
    auto edgeColor=borderProps.borderColors.left;

    if(borderEdge == RightEdge)
    {
        edgeColor=borderProps.borderColors.right;
        strokeWidth=borderProps.borderWidths.right;

        pathMetrics.outterOrigin.x=rectDestX;
        pathMetrics.outterOrigin.y=rectOriginY;
        pathMetrics.outterDest.x=rectDestX;
        pathMetrics.outterDest.y=rectDestY;
        pathMetrics.innerOrigin.x=rectDestX-strokeWidth;
        pathMetrics.innerOrigin.y=rectOriginY+borderProps.borderWidths.top;
        pathMetrics.innerDest.x=rectDestX-strokeWidth;
        pathMetrics.innerDest.y=rectDestY-borderProps.borderWidths.bottom;
        pathMetrics.originRadius=borderProps.borderRadii.topRight;
        pathMetrics.destRadius=borderProps.borderRadii.bottomRight;
        pathMetrics.width=strokeWidth;
        pathMetrics.angle=0;

    }
    if(borderEdge == BottomEdge)
    {
        edgeColor=borderProps.borderColors.bottom;
        strokeWidth=borderProps.borderWidths.bottom;

        pathMetrics.outterOrigin.x=rectOriginX;
        pathMetrics.outterOrigin.y=rectDestY;
        pathMetrics.outterDest.x=rectDestX;
        pathMetrics.outterDest.y=rectDestY;
        pathMetrics.innerOrigin.x=rectOriginX+borderProps.borderWidths.left;
        pathMetrics.innerOrigin.y=rectDestY-strokeWidth;
        pathMetrics.innerDest.x=rectDestX-borderProps.borderWidths.right;
        pathMetrics.innerDest.y=rectDestY-strokeWidth;
        pathMetrics.originRadius=borderProps.borderRadii.bottomLeft;
        pathMetrics.destRadius=borderProps.borderRadii.bottomRight;
        pathMetrics.width=strokeWidth;
        pathMetrics.angle=90;
    }
     if(borderEdge == LeftEdge)
     {
         edgeColor=borderProps.borderColors.left;
         strokeWidth=borderProps.borderWidths.left;
     
         pathMetrics.outterOrigin.x=rectOriginX;
         pathMetrics.outterOrigin.y=rectOriginY;
         pathMetrics.outterDest.x=rectOriginX;
         pathMetrics.outterDest.y=rectDestY;
         pathMetrics.innerOrigin.x=rectOriginX+strokeWidth;
         pathMetrics.innerOrigin.y=rectOriginY+borderProps.borderWidths.top;
         pathMetrics.innerDest.x=rectOriginX+strokeWidth;
         pathMetrics.innerDest.y=rectDestY-borderProps.borderWidths.bottom;
         pathMetrics.originRadius=borderProps.borderRadii.topLeft;
         pathMetrics.destRadius=borderProps.borderRadii.bottomLeft;
         pathMetrics.width=strokeWidth;
         pathMetrics.angle=180;
     }
     if(borderEdge == TopEdge)
     {
         edgeColor=borderProps.borderColors.top;
         strokeWidth=borderProps.borderWidths.top;
     
         pathMetrics.outterOrigin.x=rectOriginX;
         pathMetrics.outterOrigin.y=rectOriginY;
         pathMetrics.outterDest.x=rectDestX;
         pathMetrics.outterDest.y=rectOriginY;
         pathMetrics.innerOrigin.x=rectOriginX+borderProps.borderWidths.left;
         pathMetrics.innerOrigin.y=rectOriginY+strokeWidth;
         pathMetrics.innerDest.x=rectDestX-borderProps.borderWidths.right;
         pathMetrics.innerDest.y=rectOriginY+strokeWidth;
         pathMetrics.originRadius=borderProps.borderRadii.topLeft;
         pathMetrics.destRadius=borderProps.borderRadii.topRight;
         pathMetrics.width=strokeWidth;
         pathMetrics.angle=270;
     }

     CreateEdge(pathMetrics,borderEdge,&path);
     SetColor(edgeColor,opacity,&paint);
     path.setFillType(SkPathFillType::kEvenOdd);

     canvas->drawPath(path, paint);
}
void RSkLayoutHelper::CreateEdge(PathMetrics pathMetrics,BorderEdges borderEdge,SkPath* path)
{
/*Considered the Path construct requested needs to be constructed in clockwise direction*/

    bool verticalEdge=(((borderEdge == LeftEdge) || (borderEdge == RightEdge)) ? true : false);
    bool growCW=(((borderEdge == TopEdge) ||(borderEdge == RightEdge)) ? true : false);
    bool cornerOnRightEdge=(((borderEdge == BottomEdge) ||(borderEdge== RightEdge)||(borderEdge == TopEdge)) ? true : false);
    bool cornerOnBottomEdge=(((borderEdge == LeftEdge) ||(borderEdge == BottomEdge ) ||(borderEdge == RightEdge)) ? true : false);

    auto outterSweepAngle =(growCW ? 45 : -45);
    auto innerSweepAngle= (growCW ? -45 : +45);;

    auto outterStartX= pathMetrics.outterOrigin.x ;
    auto outterStartY= pathMetrics.outterOrigin.y ;
    auto innerStartX= pathMetrics.innerOrigin.x ;
    auto innerStartY= pathMetrics.innerOrigin.y ;
    auto outterEndX= pathMetrics.outterDest.x ;
    auto outterEndY= pathMetrics.outterDest.y ;
    auto innerEndX= pathMetrics.innerDest.x ;
    auto innerEndY= pathMetrics.innerDest.y ;

    auto innerDestRadius = 0;
    auto innerOriginRadius =0 ;
    SkRect innerStartRect,innerEndRect,outterStartRect,outterEndRect;
    Point cornerRect{0,0};
    Point pathClosurePoint{outterStartX,outterStartY};

    if(pathMetrics.originRadius)
    {
        cornerRect.x =((cornerOnRightEdge && verticalEdge)? outterStartX - (pathMetrics.originRadius*2):outterStartX);
        cornerRect.y =((cornerOnBottomEdge && !verticalEdge )? outterStartY-(pathMetrics.originRadius*2): outterStartY);
        outterStartRect=SkRect::MakeXYWH(cornerRect.x,cornerRect.y,pathMetrics.originRadius*2,pathMetrics.originRadius*2);
    }
    if(pathMetrics.destRadius)
    {
        cornerRect.x =(cornerOnRightEdge ? outterEndX - (pathMetrics.destRadius*2):outterEndX );
        cornerRect.y =(cornerOnBottomEdge ? outterEndY-(pathMetrics.destRadius*2): outterEndY);
        outterEndRect=SkRect::MakeXYWH(cornerRect.x,cornerRect.y,pathMetrics.destRadius*2,pathMetrics.destRadius*2);
        outterEndX= (verticalEdge ? outterEndX : (outterEndX - pathMetrics.destRadius));
        outterEndY= (verticalEdge ? (outterEndY - pathMetrics.destRadius):outterEndY);
    }
    /*Path Building*/
    path->moveTo(outterStartX,outterStartY);
    if(pathMetrics.originRadius)
    {
        path->addArc(outterStartRect,pathMetrics.angle-outterSweepAngle,outterSweepAngle);
        Point centerPoint{outterStartRect.centerX(),outterStartRect.centerY()};
        pathClosurePoint.x=centerPoint.x+(pathMetrics.originRadius *cos((pathMetrics.angle-outterSweepAngle)*(M_PI/180)));
        pathClosurePoint.y=centerPoint.y+(pathMetrics.originRadius *sin((pathMetrics.angle-outterSweepAngle)*(M_PI/180)));
    }
    path->lineTo(outterEndX,outterEndY);
    if(pathMetrics.destRadius)
    {
        path->addArc(outterEndRect,pathMetrics.angle,outterSweepAngle);
    }
    path->lineTo(innerEndX,innerEndY);
    path->lineTo(innerStartX,innerStartY);
    path->lineTo(pathClosurePoint.x,pathClosurePoint.y);

}

void RSkLayoutHelper::SetStyle(int strokeWidth,SkPaint::Style style,BorderStyle borderStyle,SkPaint *paint)
{
    paint->setStyle(style);
    paint->setStrokeWidth(strokeWidth);
    if((borderStyle == BorderStyle::Dashed )|| (borderStyle == BorderStyle::Dotted))
    {
        SetPathEffect(borderStyle,strokeWidth,paint);
    }
}
void RSkLayoutHelper::SetColor(SharedColor Color,Float opacity,SkPaint *paint)
{
    float ratio = 255.9999;
    paint->setAntiAlias(true);
    paint->setAlphaf((opacity/100));
    auto colorValue=colorComponentsFromColor(Color);
    paint->setColor(SkColorSetARGB(
        colorValue.alpha * ratio,
        colorValue.red * ratio,
        colorValue.green * ratio,
        colorValue.blue * ratio));
}
void RSkLayoutHelper::SetPathEffect(BorderStyle borderStyle,int strokeWidth,SkPaint *paint)
{
    float Dash_interval[]={strokeWidth,strokeWidth/2};
    float Dot_interval[]={0,strokeWidth+3};
    
    if(borderStyle == BorderStyle::Dashed)
    {
         int intervalCount=(int)SK_ARRAY_COUNT(Dash_interval);
         paint->setPathEffect(SkDashPathEffect::Make(Dash_interval,intervalCount,0));
         paint->setStrokeJoin(SkPaint::kRound_Join);
    
    }
    if(borderStyle == BorderStyle::Dotted)
    {
         int intervalCount=(int)SK_ARRAY_COUNT(Dash_interval);
         paint->setPathEffect(SkDashPathEffect::Make(Dot_interval,intervalCount,0));
         paint->setStrokeJoin(SkPaint::kRound_Join);
         paint->setStrokeCap(SkPaint::kRound_Cap);
    }
}

bool RSkLayoutHelper::isDrawVisible(SharedColor Color,Float opacity)
{
    return (Color !=clearColor() && opacity >0.0)? true:false;
}

bool RSkLayoutHelper::hasRoundedBorders(BorderMetrics borderProps)
{
    return  ( (borderProps.borderRadii.topLeft != 0 ) ||   \
              (borderProps.borderRadii.topRight != 0 ) ||  \
              (borderProps.borderRadii.bottomLeft != 0 ) | \
              (borderProps.borderRadii.bottomRight != 0 ));

}

bool RSkLayoutHelper::hasUniformBorderEdges(BorderMetrics borderProps)
{
    return  ( borderProps.borderColors.isUniform() &&  borderProps.borderWidths.isUniform());

}


} // namespace react
} // namespace facebook
