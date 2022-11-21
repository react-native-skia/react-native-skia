 /*
 * Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <math.h>

#include "include/core/SkPaint.h"
#include "include/core/SkClipOp.h"
#include "include/effects/SkDashPathEffect.h"
#include "include/effects/SkImageFilters.h"
#include "src/core/SkMaskFilterBase.h"

#include "RSkDrawUtils.h"
#include "RSkConversion.h"
#include "ReactSkia/utils/RnsLog.h"

#define DEFAULT_BACKGROUND_COLOR  SK_ColorTRANSPARENT /*Transaprent*/
#define DEFAULT_COLOR             SK_ColorBLACK /*Black*/

#define UNDERLINEWIDTH   1
#define BOTTOMALIGNMENT 3

namespace facebook {
namespace react {
namespace {

enum FrameType {
     FilledRect,//Case of BackGround
     MonoChromeStrokedRect,//Case of Rect Border with same color & thickness on All sides
     PolyChromeStrokedRect,//Case of Rect Border with same thickness but vary in color on sides
     DiscretePath,//Case of non uniform Border
     InvisibleFrame // Case of fully transparent frame
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
inline bool isBorderColorVisible(SharedColor Color)
{
    return ((!Color) || //default color is fully opaque black for Border,when color not specifed, so returning true
            (colorComponentsFromColor(Color).alpha != 0.0)
            ) ? true:false;
}
inline bool isBorderEdgeVisible(SharedColor Color,Float thickness=1.0)
{

//Note::Default color for border is fully opaque black, When Color not Specifeid but has visible width/thickness

    return (thickness &&
            isBorderColorVisible(Color)
           )? true:false;
}
inline bool hasVisibleBorder(BorderColors borderColor,BorderWidths borderWidth)
{
    /*Returns true, when atleast one side with visible thickness & color*/

    if(isBorderEdgeVisible(borderColor.left,borderWidth.left) ||
       isBorderEdgeVisible(borderColor.right,borderWidth.right) ||
       isBorderEdgeVisible(borderColor.top,borderWidth.top) ||
       isBorderEdgeVisible(borderColor.bottom,borderWidth.bottom)) {
        return true;
    }
    return false;
}
inline bool hasVisibleBackGround(SharedColor Color){
// Shared Color is an optional varaible, '0' defines value not specified
//When color not specified for background, default color is fully transparent.

    return (Color && (colorComponentsFromColor(Color).alpha != 0.0)) ? true:false;
}
FrameType detectFrameBorderType(BorderColors borderColor,BorderWidths borderWidth)
{
    FrameType frameType;
    if(!hasVisibleBorder(borderColor,borderWidth)) {
        //None of the Side has Valid color or visible thickness
        frameType=InvisibleFrame;
    }else if( !borderWidth.isUniform() ){
        /*Border are of different thickness*/
        frameType=DiscretePath;
    } else {
       /*All the sides confirmed to have uniform visble Thickness.
         confirming visible color on ALL sides*/
        if(borderColor.isUniform()){
            frameType=MonoChromeStrokedRect;//Colors are on the sides are uniform
        } else {
        /* Border Color  differs for each Side.So check all the sides are Visible*/
            if(isBorderColorVisible(borderColor.left) &&
               isBorderColorVisible(borderColor.right) &&
               isBorderColorVisible(borderColor.top) &&
               isBorderColorVisible(borderColor.bottom)){
                frameType=PolyChromeStrokedRect;
            } else {
                frameType=DiscretePath; // Few of the side/s fully transparent
            }
        }
    }
    return frameType;

}
inline bool hasUniformBorderEdges(BorderMetrics borderProps)
{
    return  ( borderProps.borderColors.isUniform() &&  borderProps.borderWidths.isUniform());
}
void drawRect(FrameType frameType,SkCanvas *canvas,
                                        Rect frame,
                                        BorderMetrics borderProps,
                                        SkColor Color,
                                        SkPaint *paint=NULL,
                                        sk_sp<SkMaskFilter> shadowMaskFilter=nullptr
                 )
{
    if(canvas == NULL) return;
/*Case DrawRect assumes same width for all the sides.So referring left */
    auto rectStrokeWidth = borderProps.borderWidths.left;

    SkRRect rRect;
    SkRect rect;
    SkPaint paintObj;
    if(paint != NULL){ paintObj = *paint; }

    paintObj.setAntiAlias(true);
    paintObj.setColor(Color);

    if(shadowMaskFilter != nullptr){
        paintObj.setMaskFilter(shadowMaskFilter);
    }

  /*Creating basic layout from props*/
    rect=SkRect::MakeXYWH(frame.origin.x,frame.origin.y,\
         frame.size.width,frame.size.height);
    /*Skia apply radius in clockwise direction starting from TopLeft*/
    SkVector radii[4]={{borderProps.borderRadii.topLeft,borderProps.borderRadii.topLeft},
                       {borderProps.borderRadii.topRight,borderProps.borderRadii.topRight}, \
                       {borderProps.borderRadii.bottomRight,borderProps.borderRadii.bottomRight }, \
                       {borderProps.borderRadii.bottomLeft,borderProps.borderRadii.bottomLeft}};

    /* To sync with the border draw type, resetting the stroke width for background*/
    if(!hasUniformBorderEdges(borderProps) && (frameType == FilledRect)) {
        rectStrokeWidth=0;
    }

    /*Border adjustment needed in case of stroke width, as half the pixels where drawn outside and half inside by SKIA*/
      if(rectStrokeWidth > 0){
          rect.inset(rectStrokeWidth/2,rectStrokeWidth/2);
      }
      rRect.setRectRadii(rect,radii);
    if(frameType == FilledRect){
        setStyle(rectStrokeWidth,SkPaint::kStrokeAndFill_Style,BorderStyle::Solid,&paintObj);
    } else if((frameType == MonoChromeStrokedRect) ||(frameType == PolyChromeStrokedRect) ){
        setStyle(rectStrokeWidth,SkPaint::kStroke_Style,borderProps.borderStyles.left,&paintObj);
    }
    canvas->drawRRect(rRect, paintObj);
}
inline void drawPath(SkCanvas *canvas,SkPath &path,SharedColor Color,sk_sp<SkImageFilter> shadowImageFilter=nullptr)
{
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(RSkColorFromSharedColor(Color, DEFAULT_COLOR));
    path.setFillType(SkPathFillType::kEvenOdd);
    if(shadowImageFilter != nullptr) {
        paint.setImageFilter(shadowImageFilter);
    }
    canvas->drawPath(path, paint);
}
inline void createPath(PathMetrics pathMetrics,BorderEdges borderEdge,SkPath& path)
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
    path.moveTo(outterPathStart.x,outterPathStart.y);
    if(pathMetrics.startRadius){
        path.addArc(outterStartRect,pathMetrics.angle-outterSweepAngle,outterSweepAngle);
        Point centerPoint{outterStartRect.centerX(),outterStartRect.centerY()};
        pathClosurePoint.x=centerPoint.x+(pathMetrics.startRadius *cos((pathMetrics.angle-outterSweepAngle)*(M_PI/180)));
        pathClosurePoint.y=centerPoint.y+(pathMetrics.startRadius *sin((pathMetrics.angle-outterSweepAngle)*(M_PI/180)));
    }
    path.lineTo(outterPathEnd.x,outterPathEnd.y);
    if(pathMetrics.endRadius){
        path.addArc(outterEndRect,pathMetrics.angle,outterSweepAngle);
    }
    path.lineTo(innerPathEnd.x,innerPathEnd.y);
    /*To Do Enchancement: Corner Radius to be applied for inner path aswell.
           Need for inner Radius happens, when border width < border Radius */
    path.lineTo(innerPathStart.x,innerPathStart.y);
    path.lineTo(pathClosurePoint.x,pathClosurePoint.y);
}
SkPath createAndDrawDiscretePath(BorderEdges borderEdge,SkCanvas *canvas,
                                        Rect frame,
                                        BorderMetrics borderProps,
                                        sk_sp<SkImageFilter> shadowImageFilter=nullptr,
                                        bool doDraw = true)
{

    SkPath path;
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
     createPath(pathMetrics,borderEdge,path);
     if(doDraw && (canvas != NULL)) {
        drawPath(canvas,path,edgeColor,shadowImageFilter);
     }
    return path;
}
inline void drawDiscretePath( SkCanvas *canvas,Rect frame,
                                     BorderMetrics borderProps,
                                     sk_sp<SkImageFilter> shadowImageFilter) {
    SkPath path;

    #define CHECK_SIDE_VISIBILITY_AND_DRAW_SIDE_FOR_DISCRETE_PATH(side,sideColor,sideWidth)   \
        if(isBorderEdgeVisible(sideColor,(sideWidth))){ \
            createAndDrawDiscretePath(side,canvas,frame,borderProps,shadowImageFilter,true); \
        }

    CHECK_SIDE_VISIBILITY_AND_DRAW_SIDE_FOR_DISCRETE_PATH(RightEdge,borderProps.borderColors.right,borderProps.borderWidths.right)
    CHECK_SIDE_VISIBILITY_AND_DRAW_SIDE_FOR_DISCRETE_PATH(LeftEdge,borderProps.borderColors.left,borderProps.borderWidths.left)
    CHECK_SIDE_VISIBILITY_AND_DRAW_SIDE_FOR_DISCRETE_PATH(TopEdge,borderProps.borderColors.top,borderProps.borderWidths.top)
    CHECK_SIDE_VISIBILITY_AND_DRAW_SIDE_FOR_DISCRETE_PATH(BottomEdge,borderProps.borderColors.bottom,borderProps.borderWidths.bottom)
    return;
}

} //namespace
namespace RSkDrawUtils{

void  drawBackground(SkCanvas *canvas, 
                               Rect frame,
                               BorderMetrics borderProps,
                               SharedColor backgroundColor)
{
    if(hasVisibleBackGround(backgroundColor)){
      drawRect(FilledRect,canvas,frame,borderProps,RSkColorFromSharedColor(backgroundColor, DEFAULT_BACKGROUND_COLOR));
    }
}
void drawBorder(SkCanvas *canvas,
                               Rect frame,
                               BorderMetrics borderProps,
                               SharedColor backgroundColor)
{

    FrameType frameType = detectFrameBorderType(borderProps.borderColors,borderProps.borderWidths);

//Note:Default border color is fully opaque black for border

    #define BACKGROUND_COLOR_DIFFERENT_FROM_BORDER_COLOR(color)  \
        (((!color && (*backgroundColor != *blackColor())) ||  \
             (color && (*color != *backgroundColor))) ? true : false)

    #define CHECK_SIDE_VISIBILITY_AND_DRAW_SIDE_FOR_BORDER(side,sideWidth,color)   \
        /* Draw Side, if it has visble color & thickness & color different from background colour*/ \
        if(BACKGROUND_COLOR_DIFFERENT_FROM_BORDER_COLOR(color) && \
            isBorderEdgeVisible(color,(sideWidth)) /*Side has Visible color & thickness*/ \
         ){ \
            createAndDrawDiscretePath(side,canvas,frame,borderProps); \
        }

    if((frameType == MonoChromeStrokedRect) && BACKGROUND_COLOR_DIFFERENT_FROM_BORDER_COLOR(borderProps.borderColors.left)) {
        drawRect(MonoChromeStrokedRect,canvas,frame,borderProps,RSkColorFromSharedColor(borderProps.borderColors.left, DEFAULT_COLOR));
    } else if((frameType == PolyChromeStrokedRect)|| (frameType == DiscretePath)) {
        CHECK_SIDE_VISIBILITY_AND_DRAW_SIDE_FOR_BORDER(RightEdge,borderProps.borderWidths.right,borderProps.borderColors.right)
        CHECK_SIDE_VISIBILITY_AND_DRAW_SIDE_FOR_BORDER(LeftEdge,borderProps.borderWidths.left,borderProps.borderColors.left)
        CHECK_SIDE_VISIBILITY_AND_DRAW_SIDE_FOR_BORDER(TopEdge,borderProps.borderWidths.top,borderProps.borderColors.top)
        CHECK_SIDE_VISIBILITY_AND_DRAW_SIDE_FOR_BORDER(BottomEdge,borderProps.borderWidths.bottom,borderProps.borderColors.bottom)
    }
}
bool  drawShadow(SkCanvas* canvas,Rect frame,
                        BorderMetrics borderProps,
                        SharedColor backgroundColor,
                        SkColor shadowColor,
                        SkSize shadowOffset,
                        float shadowOpacity,
                        float frameOpacity,
                        sk_sp<SkImageFilter> shadowImageFilter,
                        sk_sp<SkMaskFilter> shadowMaskFilter) {

    if(shadowOpacity == 0) {return false;} // won't proceed, if shadow is fully transparent

    FrameType frameType;

    if(hasVisibleBackGround(backgroundColor)) {
        frameType= FilledRect; // Frame has BackGround. So drawing shadow on the background
    } else { //No visible background colour. So have to draw shadow on Border
        frameType=detectFrameBorderType(borderProps.borderColors,borderProps.borderWidths);
    }

    if(frameType == InvisibleFrame) { return true;} // frame doesn't have visible pixel, content in the frame may have

    Rect shadowFrame{{frame.origin.x+shadowOffset.width(),
                     frame.origin.y+shadowOffset.height()},
                     {frame.size.width, frame.size.height}};
    SkRect frameBounds=SkRect::Make(getShadowBounds(SkIRect::MakeXYWH(shadowFrame.origin.x,shadowFrame.origin.y,
                                    shadowFrame.size.width,shadowFrame.size.height),
                                    shadowMaskFilter,
                                    shadowImageFilter));

    bool saveLayerDone=false;
/* Apply opacity*/
    if(!isOpaque(shadowOpacity)) {
        canvas->saveLayerAlpha(&frameBounds,shadowOpacity);
        saveLayerDone=true;
    }
/* Apply Clip to avoid draw shadow on NonVisible Area[behind the opaque frames]*/
    if((frameType == FilledRect) && (isOpaque(frameOpacity))) {
        if(!saveLayerDone) {
            saveLayerDone=true;
            canvas->saveLayer(&frameBounds,nullptr);
        }
        SkRect clipRect = SkRect::MakeXYWH(frame.origin.x, frame.origin.y, frame.size.width, frame.size.height);
        SkVector radii[4]={{borderProps.borderRadii.topLeft,borderProps.borderRadii.topLeft},
                       {borderProps.borderRadii.topRight,borderProps.borderRadii.topRight}, \
                       {borderProps.borderRadii.bottomRight,borderProps.borderRadii.bottomRight }, \
                       {borderProps.borderRadii.bottomLeft,borderProps.borderRadii.bottomLeft}};
        SkRRect clipRRect;
        if(borderProps.borderWidths.left) {
            clipRect.inset(borderProps.borderWidths.left/2,borderProps.borderWidths.left/2);
        }
        clipRRect.setRectRadii(clipRect,radii);
        canvas->clipRRect(clipRRect,SkClipOp::kDifference);
    }
/*Proceed to draw Shadow*/
    if(frameType != DiscretePath ) {
       /*Frame is a Rect*/
       drawRect(frameType,canvas,shadowFrame,borderProps,shadowColor,NULL,shadowMaskFilter);
    } else {
        /*Frame is Non contiguos or Discrete, so draw it as a path*/
        drawDiscretePath(canvas,frame,borderProps,shadowImageFilter);
    }
    if(saveLayerDone) {
        canvas->restore();
    }
    return (frameType != FilledRect) ? true:false; // true for hollow frames to proceed with content Shadow
}

void drawUnderline(SkCanvas *canvas,Rect frame,SharedColor underlineColor){
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(RSkColorFromSharedColor(underlineColor, DEFAULT_COLOR));
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(UNDERLINEWIDTH);
    auto frameOrigin = frame.origin;
    auto frameSize = frame.size;
    canvas->drawLine(frameOrigin.x,frameOrigin.y+frameSize.height-BOTTOMALIGNMENT,frameOrigin.x+frameSize.width,frameOrigin.y+frameSize.height-BOTTOMALIGNMENT, paint);
}

SkIRect getShadowBounds(const SkIRect shadowFrame,
                        sk_sp<SkMaskFilter> shadowMaskFilter,
                        sk_sp<SkImageFilter> shadowImageFilter) {
    if(shadowMaskFilter){
        SkRect storage;
        as_MFB(shadowMaskFilter)->computeFastBounds(SkRect::Make(shadowFrame), &storage);
        return  SkIRect::MakeXYWH(storage.x(), storage.y(), storage.width(), storage.height());
    }
    if(shadowImageFilter) {
        SkMatrix identityMatrix;
        return shadowImageFilter->filterBounds(
                                    shadowFrame,
                                    identityMatrix,
                                    SkImageFilter::kForward_MapDirection,
                                    nullptr);
    }
    return shadowFrame;
}

} // namespace RSkDrawUtils
} // namespace react
} // namespace facebook
