 /*
 * Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <math.h>

#include "include/core/SkPaint.h"
#include "include/effects/SkImageFilters.h"
#include "include/core/SkClipOp.h"

#include "ReactSkia/views/common/RSkDrawUtils.h"
#include "ReactSkia/views/common/RSkConversion.h"


#define DEFAULT_COLOUR   SK_ColorBLACK /*Black*/
#define UNDERLINEWIDTH   1
#define BOTTOMALIGNMENT 3

namespace facebook {
namespace react {
namespace {

enum DrawMethod {
     Background,
     Border,
     None
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
void setColor(SharedColor Color,SkPaint *paint)
{
    paint->setAntiAlias(true);
    paint->setColor(RSkColorFromSharedColor(Color, DEFAULT_COLOUR));
}
bool isDrawVisible(SharedColor Color,Float thickness=1.0)
{
    return (Color !=clearColor() && thickness > 0.0)? true:false;
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
                                        SkPaint *paint=NULL)
{
    if(canvas == NULL) return;
/*Case DrawRect assumes same width for all the sides.So referring left */
    auto rectStrokeWidth = borderProps.borderWidths.left;

    SkRRect rRect;
    SkRect rect;
    SkPaint paintObj;
    if(paint != NULL){ paintObj = *paint; }

  /*Creating basic layout from props*/
    rect=SkRect::MakeXYWH(frame.origin.x,frame.origin.y,\
         frame.size.width,frame.size.height);
    /*Skia apply radius in clockwise direction starting from TopLeft*/
    SkVector radii[4]={{borderProps.borderRadii.topLeft,borderProps.borderRadii.topLeft},
                       {borderProps.borderRadii.topRight,borderProps.borderRadii.topRight}, \
                       {borderProps.borderRadii.bottomRight,borderProps.borderRadii.bottomRight }, \
                       {borderProps.borderRadii.bottomLeft,borderProps.borderRadii.bottomLeft}};

    setColor(Color,&paintObj );
    /* To sync with the border draw type, resetting the stroke width for background*/
    if(!hasUniformBorderEdges(borderProps) && (drawMethod == Background))
        rectStrokeWidth=0;

    /*Border adjustment needed in case of stroke width, as half the pixels where drawn outside and half inside by SKIA*/
      if(rectStrokeWidth > 0){
          rect.inset(rectStrokeWidth/2,rectStrokeWidth/2);
      }
      rRect.setRectRadii(rect,radii);
    if(drawMethod == Background){
        setStyle(rectStrokeWidth,SkPaint::kStrokeAndFill_Style,BorderStyle::Solid,&paintObj);
    }
    if(drawMethod == Border){
        setStyle(rectStrokeWidth,SkPaint::kStroke_Style,borderProps.borderStyles.left,&paintObj);
    }
    canvas->drawRRect(rRect, paintObj);
}
SkPath drawEdges(BorderEdges borderEdge,SkCanvas *canvas,
                                        Rect frame,
                                        BorderMetrics borderProps,
                                        bool needsDraw = true)
{

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
    if(needsDraw && (canvas != NULL) ) {
       setColor(edgeColor,&paint);
       path.setFillType(SkPathFillType::kEvenOdd);
       canvas->drawPath(path, paint);
    }
    return path;
}
bool createshadowPath( SkCanvas *canvas,Rect frame,
                        BorderMetrics borderProps,
                        SkPath *shadowPath ) {

    bool pathExist = false;
    if(shadowPath == NULL)
        return false;

    if(isDrawVisible(borderProps.borderColors.right,borderProps.borderWidths.right)){
        pathExist=true;
        shadowPath->addPath(drawEdges(RightEdge,canvas,frame,borderProps,false));
    }
    if(isDrawVisible(borderProps.borderColors.left,borderProps.borderWidths.left)){
        shadowPath->addPath(drawEdges(LeftEdge,canvas,frame,borderProps,false));
        pathExist=true;
    }
    if(isDrawVisible(borderProps.borderColors.top,borderProps.borderWidths.top)){
        shadowPath->addPath(drawEdges(TopEdge,canvas,frame,borderProps,false));
        pathExist=true;
    }
    if(isDrawVisible(borderProps.borderColors.bottom,borderProps.borderWidths.bottom)){
        shadowPath->addPath(drawEdges(BottomEdge,canvas,frame,borderProps,false));
        pathExist=true;
    }
    return pathExist;
}
} //namespace
namespace RSkDrawUtils{
void  drawBackground(SkCanvas *canvas, 
                               Rect frame,
                               BorderMetrics borderProps,
                               SharedColor backgroundColor)
{
    if( backgroundColor && isDrawVisible(backgroundColor) ){
      drawRect(Background,canvas,frame,borderProps,backgroundColor);
    }
}
void drawBorder(SkCanvas *canvas,
                               Rect frame,
                               BorderMetrics borderProps,
                               SharedColor backgroundColor)
{
    if(hasUniformBorderEdges(borderProps) && \
       ((backgroundColor != borderProps.borderColors.left)&& \
   /*Skia draw with hairline thickness in case of Strokewidth Zero. So avoid drawing border if 
     borderWidth/StrokeWidth is Zero*/
          (isDrawVisible(borderProps.borderColors.left,borderProps.borderWidths.left) ))){
             drawRect(Border,canvas,frame,borderProps,borderProps.borderColors.left);
    }else{
         /*Draw Right Side*/
         if((backgroundColor != borderProps.borderColors.right) && \
                 (isDrawVisible(borderProps.borderColors.right,borderProps.borderWidths.right) )){
             drawEdges(RightEdge,canvas,frame,borderProps);
         }
         /*Draw Left Side*/
         if((backgroundColor != borderProps.borderColors.left) && \
                 (isDrawVisible(borderProps.borderColors.left,borderProps.borderWidths.left))){
             drawEdges(LeftEdge,canvas,frame,borderProps);
         }
         /*Draw Top Side*/
         if((backgroundColor != borderProps.borderColors.top) && \
                 (isDrawVisible(borderProps.borderColors.top,borderProps.borderWidths.top) )){
             drawEdges(TopEdge,canvas,frame,borderProps);
         }
         /*Draw Bottom Side*/
         if((backgroundColor != borderProps.borderColors.bottom) && \
                 (isDrawVisible(borderProps.borderColors.bottom,borderProps.borderWidths.bottom))){
             drawEdges(BottomEdge,canvas,frame,borderProps);
         }
    }
}
bool  drawShadow(SkCanvas *canvas,Rect frame,
                        BorderMetrics borderProps,
                        SharedColor backgroundColor,
                        Float shadowOpacity,
                        sk_sp<SkImageFilter> shadowFilter) {

    if(shadowFilter == NULL) return false;
    SkPaint paint;
    paint.setImageFilter( shadowFilter);

    DrawMethod shadowOn = None;
    /*Shadow on Background : If bg  visible */
    if(backgroundColor && isDrawVisible(backgroundColor)) {
        shadowOn=Background;
    }
    /*Shadow on Border : None of the Border width to be '0' and border color not transparent */
    else if(borderProps.borderColors.isUniform() && isDrawVisible(borderProps.borderColors.left) \
        && (borderProps.borderWidths.left !=0) && (borderProps.borderWidths.top !=0)  \
        && (borderProps.borderWidths.right !=0) && (borderProps.borderWidths.bottom !=0)) {

           shadowOn = Border;
    }
    if(shadowOn != None) {
        SkRect clipRect = SkRect::MakeXYWH(frame.origin.x, frame.origin.y, frame.size.width, frame.size.height);
        SkVector radii[4]={{borderProps.borderRadii.topLeft,borderProps.borderRadii.topLeft},
                       {borderProps.borderRadii.topRight,borderProps.borderRadii.topRight}, \
                       {borderProps.borderRadii.bottomRight,borderProps.borderRadii.bottomRight }, \
                       {borderProps.borderRadii.bottomLeft,borderProps.borderRadii.bottomLeft}};
        if(borderProps.borderWidths.left) {
            clipRect.inset(borderProps.borderWidths.left/2,borderProps.borderWidths.left/2);
        }

        SkRRect clipRRect;
        clipRRect.setRectRadii(clipRect,radii);

        bool needsSaveLayer =((!(isOpaque(shadowOpacity))) || (shadowOn == Background));

        if(needsSaveLayer)
            canvas->saveLayerAlpha(NULL,shadowOpacity);
        if(shadowOn == Background)
            canvas->clipRRect(clipRRect,SkClipOp::kDifference);
        drawRect(shadowOn,canvas,frame,borderProps,backgroundColor,&paint);
        if(needsSaveLayer)
            canvas->restore();
    /* Return true only for shadow on Border Case , to update components to handle shadowOnContent.*/
        return (shadowOn == Background) ? false : true;
    }

/*Shadow on Path */
    if(!borderProps.borderWidths.isUniform() ) {
        SkPath shadowPath;
        bool pathExist=createshadowPath(canvas,frame,borderProps,&shadowPath);;

       if(pathExist) {
           if(!(isOpaque(shadowOpacity)))
               canvas->saveLayerAlpha(NULL,shadowOpacity);
           canvas->clipPath(shadowPath,SkClipOp::kDifference);
           canvas->drawPath(shadowPath,paint);
           if(!(isOpaque(shadowOpacity)))
               canvas->restore();
        }
        return true;
    }
    return true;
}

void drawUnderline(SkCanvas *canvas,Rect frame,SharedColor underlineColor){
    SkPaint paint;
    setColor(underlineColor, &paint);
    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(UNDERLINEWIDTH);
    auto frameOrigin = frame.origin;
    auto frameSize = frame.size;
    canvas->drawLine(frameOrigin.x,frameOrigin.y+frameSize.height-BOTTOMALIGNMENT,frameOrigin.x+frameSize.width,frameOrigin.y+frameSize.height-BOTTOMALIGNMENT, paint);
}
} // namespace RSkDrawUtils
} // namespace react
} // namespace facebook
