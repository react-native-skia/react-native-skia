 /*
 * Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "include/core/SkFont.h"
#include "include/core/SkFontMgr.h"

#include "ReactSkia/utils/RnsUtils.h"

#include "OnScreenKeyBoard.h"
#include "OSKConfig.h"
#include "OSKLayout.h"

namespace rns {
namespace sdk {

std::mutex oskLaunchExitCtrlMutex;//For synchronized OSK Launch & Exit

OnScreenKeyboard& OnScreenKeyboard::getInstance() {
  static OnScreenKeyboard oskHandle;
  return oskHandle;
}

OSKErrorCode OnScreenKeyboard::launch(OSKConfig oskConfig) {
  RNS_LOG_TODO("Need to do emit , keyboardWillShow Event to APP");

  OnScreenKeyboard &oskHandle=OnScreenKeyboard::getInstance();

  if((oskHandle.oskState_ == OSK_STATE_LAUNCH_INPROGRESS) || (oskHandle.oskState_ == OSK_STATE_ACTIVE))
      return OSK_ERROR_ANOTHER_INSTANCE_ACTIVE;

  std::scoped_lock lock(oskLaunchExitCtrlMutex);

  oskHandle.oskState_=OSK_STATE_LAUNCH_INPROGRESS;
  oskHandle.launchOSKWindow(oskConfig);
  return OSK_LAUNCH_SUCCESS;
}

void OnScreenKeyboard::exit() {
  RNS_LOG_TODO("Need to emitkeyboardWillHide Event  to APP");

  OnScreenKeyboard &oskHandle=OnScreenKeyboard::getInstance();

  if((oskHandle.oskState_ == OSK_STATE_EXIT_INPROGRESS) || (oskHandle.oskState_ == OSK_STATE_INACTIVE))
    return;

  oskHandle.oskState_=OSK_STATE_EXIT_INPROGRESS;
/* Stop Listening for Hw Key Event*/
  if(oskHandle.subWindowKeyEventId_ != -1) {
      NotificationCenter::subWindowCenter().removeListener(oskHandle.subWindowKeyEventId_);
      oskHandle.subWindowKeyEventId_= -1;
  }
  std::scoped_lock lock(oskLaunchExitCtrlMutex);
  oskHandle.closeWindow();
  oskHandle.oskState_=OSK_STATE_INACTIVE;
}

void  OnScreenKeyboard::updatePlaceHolderString(std::string PHdisplayString) {
  RNS_LOG_TODO("updatePlaceHolderString:Implementation yet to be done");
  return;
}

void OnScreenKeyboard::launchOSKWindow(OSKConfig oskConfig) {

  memcpy(&oskConfig_,&oskConfig,sizeof(oskConfig));

  SkSize mainscreenSize_=RnsShell::PlatformDisplay::sharedDisplay().screenSize();
  if(screenSize_ != mainscreenSize_) {
    generateOSKLayout_=true;
    screenSize_=mainscreenSize_;
  } else {
      generateOSKLayout_=false;
  }
  //Set up OSk configuration
  if(oskConfig_.theme == OSK_LIGHT_THEME) {
    bgColor_ = OSK_LIGHT_THEME_BACKGROUND_COLOR ;
    fontColor_ = OSK_LIGHT_THEME_FONT_COLOR;
  } else {
    bgColor_ = OSK_DARK_THEME_BACKGROUND_COLOR ;
    fontColor_ =OSK_DARK_THEME_FONT_COLOR;
  }
  if(oskConfig_.type == OSK_ALPHA_NUMERIC_KB)
    oskLayout_.kbLayoutType = ALPHA_LOWERCASE_LAYOUT;

  unsigned int XscaleFactor = screenSize_.width()/baseScreenSize.width();
  unsigned int YscaleFactor =screenSize_.height()/baseScreenSize.height();
  oskLayout_.textFontSize= OSK_FONT_SIZE *XscaleFactor;
  oskLayout_.textHLFontSize= OSK_HIGHLIGHT_FONT_SIZE *XscaleFactor;
  oskLayout_.horizontalStartOffset= ((screenSize_.width()-(screenSize_.width()*OSK_PLACEHOLDER_LENGTH))/2)*XscaleFactor;

  /*Craeting OSK Window*/
  std::function<void()> createWindowCB = std::bind(&OnScreenKeyboard::windowReadyToDrawCB,this);
  createWindow(screenSize_,createWindowCB);

}

void OnScreenKeyboard::drawOSK() {

if(oskState_!= OSK_STATE_ACTIVE) return;

/*1. Set up OSK Window background*/
  windowDelegatorCanvas->clear(bgColor_);

/*2. Draw PlaceHolder*/
  SkPaint paint;
  if(oskConfig_.placeHolderName) {
    SkFont font;
    paint.setColor(fontColor_);
    font.setSize(OSK_FONT_SIZE);
    windowDelegatorCanvas->drawSimpleText(oskConfig_.placeHolderName,strlen(oskConfig_.placeHolderName), SkTextEncoding::kUTF8,
                               oskLayout_.horizontalStartOffset,
                               screenSize_.height()*OSK_PLACEHOLDER_NAME_VERTICAL_OFFSET,
                               font, paint);
  }
  paint.setColor((oskConfig_.theme == OSK_LIGHT_THEME) ? OSK_LIGHT_THEME_PLACEHOLDER_COLOR: OSK_DARK_THEME_PLACEHOLDER_COLOR);
  SkRect rect=SkRect::MakeXYWH( oskLayout_.horizontalStartOffset,
                                screenSize_.height()*OSK_PLACEHOLDER_VERTICAL_OFFSET,
                                screenSize_.width()*OSK_PLACEHOLDER_LENGTH,
                                OSK_PLACEHOLDER_HEIGHT
                              );
  windowDelegatorCanvas->drawRect(rect,paint);

/*3. Draw PLaceHolder Display String */
  RNS_LOG_TODO("Update PlaceHolder String on start Up");

/*4. Draw  KeyBoard Layout*/
  drawKBLayout(oskConfig_.type);

/*5. Finally Listen for  Key Pressed event */
  if((oskState_== OSK_STATE_ACTIVE) && (subWindowKeyEventId_ == -1) ) {
    std::function<void(rnsKey, rnsKeyAction)> handler = std::bind(&OnScreenKeyboard::onHWkeyHandler,this,
                                                                   std::placeholders::_1,
                                                                   std::placeholders::_2);
    subWindowKeyEventId_ = NotificationCenter::subWindowCenter().addListener("onHWKeyEvent", handler);
  }
  return;
}

void OnScreenKeyboard::drawKBLayout(OSKTypes oskType) {

  if(oskState_!= OSK_STATE_ACTIVE) return;

  RNS_PROFILE_START(OSKLayoutCreate)
  createOSKLayout(oskType);
  RNS_PROFILE_END("OSk Layout Create Done : ",OSKLayoutCreate)
  RNS_PROFILE_START(OSKDraw)
//clear KeyBoard Area
  SkRect kbArea=SkRect::MakeXYWH( oskLayout_.horizontalStartOffset,
                                  screenSize_.height()*OSK_KB_VERTICAL_OFFSET,
                                  screenSize_.width(),
                                  screenSize_.height()- screenSize_.height()*OSK_KB_VERTICAL_OFFSET
                                );
  SkPaint paintObj;
  paintObj.setColor(bgColor_);
  windowDelegatorCanvas->drawRect(kbArea,paintObj);
  if(oskLayout_.keyInfo && oskLayout_.keyPos) {
    /*1. Draw Keys */
    for (unsigned int rowIndex = 0; rowIndex < oskLayout_.keyInfo->size(); rowIndex++) {
      for (unsigned int keyIndex = 0; keyIndex < oskLayout_.keyInfo->at(rowIndex).size(); keyIndex++) {
        if(oskState_== OSK_STATE_ACTIVE) drawKBKeyFont({keyIndex,rowIndex},fontColor_);
        else return;
      }
    }
    /*2. Draw KB partition*/
    if(oskConfig_.type != OSK_NUMERIC_KB) {
      SkPaint paint;
      paint.setColor((oskConfig_.theme == OSK_LIGHT_THEME) ? OSK_LIGHT_THEME_PLACEHOLDER_COLOR: OSK_DARK_THEME_PLACEHOLDER_COLOR);
      paint.setStrokeWidth(2);
      unsigned int startY,endY,xpos,rowCount=oskLayout_.keyInfo->size()-1;
      unsigned int KeyCount=oskLayout_.keyInfo->at(rowCount).size()-1;
      endY=oskLayout_.keyPos->at(rowCount).at(KeyCount).highlightTile.fBottom+3;
      startY=oskLayout_.keyPos->at(0).at(0).highlightTile.fTop - oskLayout_.kbGroupConfig[oskLayout_.keyInfo->at(0).at(0).kbPartitionId].groupKeySpacing.y();
      for (unsigned int index=1;index<oskLayout_.keyInfo->at(0).size();index++) {
        if(oskLayout_.keyInfo->at(0).at(index).kbPartitionId != oskLayout_.keyInfo->at(0).at(index-1).kbPartitionId) {
          xpos=oskLayout_.keyPos->at(0).at(index).highlightTile.x()-(oskLayout_.keyPos->at(0).at(index).highlightTile.fLeft - oskLayout_.keyPos->at(0).at(index-1).highlightTile.fRight)/2;
          windowDelegatorCanvas->drawLine(xpos,startY,xpos,endY,paint);
        }
      }
    }
    /*3. Highlighlight default focussed key*/
    drawHighLightOnKey(oskLayout_.defaultFocussIndex);
    currentFocussIndex_=oskLayout_.defaultFocussIndex;
  }
  RNS_PROFILE_END("OSk Draw completion : ",OSKDraw)
}

inline void OnScreenKeyboard::drawKBKeyFont(SkPoint index,SkColor color,bool onHLTile) {

  if(oskState_!= OSK_STATE_ACTIVE) return;

  unsigned int rowIndex=index.y(),keyIndex=index.x();

  if(oskLayout_.keyInfo->at(rowIndex).at(keyIndex).keyName ) {
    SkPaint textPaint;
    SkFont font;
    SkString uniChar;
    char *keyName=(char*)oskLayout_.keyInfo->at(rowIndex).at(keyIndex).keyName;
    char upperCase[2];
    unsigned int textX,textY,groupID;

    textPaint.setColor(color);
    textPaint.setAntiAlias(true);
    groupID=oskLayout_.keyInfo->at(rowIndex).at(keyIndex).kbPartitionId;

    if(( oskLayout_.keyInfo->at(rowIndex).at(keyIndex).keyType == KEY_TYPE_TEXT ) &&
     (oskLayout_.kbLayoutType == ALPHA_UPPERCASE_LAYOUT) && (isalpha(*keyName))) {
      upperCase[0] = *keyName-LOWER_TO_UPPER_CASE_OFFSET;
      upperCase[1]='\0';
      keyName=upperCase;
    }

    if(onHLTile) {
      font.setSize(oskLayout_.textHLFontSize* oskLayout_.kbGroupConfig[groupID].fontScaleFactor);
      if(( oskLayout_.keyInfo->at(rowIndex).at(keyIndex).keyType == KEY_TYPE_TEXT ) &&
         (isalpha(*keyName)) && oskLayout_.kbLayoutType == ALPHA_UPPERCASE_LAYOUT ) {
        textX=oskLayout_.keyPos->at(rowIndex).at(keyIndex).textCapsHLXY.x();
        textY=oskLayout_.keyPos->at(rowIndex).at(keyIndex).textCapsHLXY.y();
      } else {
        textX=oskLayout_.keyPos->at(rowIndex).at(keyIndex).textHLXY.x();
        textY=oskLayout_.keyPos->at(rowIndex).at(keyIndex).textHLXY.y();
      }
    } else {
      if (oskLayout_.keyInfo->at(rowIndex).at(keyIndex).keyType == KEY_TYPE_TOGGLE) {
          ToggleKeyMap :: iterator keyFunction =toggleKeyMap.find(keyName);
          if(keyFunction != toggleKeyMap.end()) {
             if(keyFunction->second != oskLayout_.kbLayoutType)
                 textPaint.setColor((oskConfig_.theme == OSK_LIGHT_THEME) ? OSK_LIGHT_THEME_INACTIVE_FONT_COLOR: OSK_DARK_THEME_INACTIVE_FONT_COLOR);
          }
      }
      font.setSize(oskLayout_.textFontSize * oskLayout_.kbGroupConfig[groupID].fontScaleFactor);
      textX=oskLayout_.keyPos->at(rowIndex).at(keyIndex).textXY.x();
      textY=oskLayout_.keyPos->at(rowIndex).at(keyIndex).textXY.y();
    }

    if(oskLayout_.keyInfo->at(rowIndex).at(keyIndex).keyType == KEY_TYPE_FUNCTION) {
        FunctionKeymap :: iterator keyFunction =functionKeyMap.find(keyName);
        keyName=(char *)DRAW_FONT_FAILURE_INDICATOR;
        if(keyFunction != functionKeyMap.end()) {
            UnicharFontConfig_t uniCharConfig = keyFunction->second;
            sk_sp<SkFontMgr> mgr(SkFontMgr::RefDefault());
            sk_sp<SkTypeface> typeface(mgr->matchFamilyStyleCharacter(nullptr, SkFontStyle(), nullptr, 0, uniCharConfig.unicharValue));
            if (typeface) {
                uniChar.appendUnichar(uniCharConfig.unicharValue);
                font.setTypeface(typeface);
                if(onHLTile)
                    font.setSize(oskLayout_.textHLFontSize* uniCharConfig.fontScaleFactor);
                else
                  font.setSize(oskLayout_.textFontSize* uniCharConfig.fontScaleFactor);
                keyName=(char *)uniChar.c_str();
            }
        }
    }
    windowDelegatorCanvas->drawSimpleText(keyName, strlen(keyName), SkTextEncoding::kUTF8,textX,textY,font, textPaint);
#ifdef SHOW_FONT_PLACING_ON_HLTILE
    SkRect bounds;
    textPaint.setColor(SK_ColorRED);
    textPaint.setStrokeWidth(2);
    //Text draw point
    windowDelegatorCanvas->drawPoint(textX,textY,textPaint);
    textPaint.setColor(SK_ColorGREEN);
    textPaint.setStrokeWidth(2);
    textPaint.setStyle(SkPaint::kStroke_Style);
    font.measureText(keyName, strlen(keyName), SkTextEncoding::kUTF8, &bounds);
    bounds.offset(textX,textY);
    // Font bounds
    windowDelegatorCanvas->drawRect(bounds,textPaint);
    textPaint.setColor(SK_ColorBLUE);
    // Highlight Tile Coverage
    windowDelegatorCanvas->drawRect(oskLayout_.keyPos->at(rowIndex).at(keyIndex).highlightTile,textPaint);
    textPaint.setColor(SK_ColorYELLOW);
    textPaint.setStrokeWidth(1);
    //HighLightTile Centre
    windowDelegatorCanvas->drawLine(oskLayout_.keyPos->at(rowIndex).at(keyIndex).highlightTile.fLeft,
                         oskLayout_.keyPos->at(rowIndex).at(keyIndex).highlightTile.fTop+(oskLayout_.keyPos->at(rowIndex).at(keyIndex).highlightTile.height()/2),
                         oskLayout_.keyPos->at(rowIndex).at(keyIndex).highlightTile.fRight,
                         oskLayout_.keyPos->at(rowIndex).at(keyIndex).highlightTile.fTop+(oskLayout_.keyPos->at(rowIndex).at(keyIndex).highlightTile.height()/2),
                         textPaint);
    windowDelegatorCanvas->drawLine(oskLayout_.keyPos->at(rowIndex).at(keyIndex).highlightTile.fLeft+(oskLayout_.keyPos->at(rowIndex).at(keyIndex).highlightTile.width()/2),
                         oskLayout_.keyPos->at(rowIndex).at(keyIndex).highlightTile.fTop,
                         oskLayout_.keyPos->at(rowIndex).at(keyIndex).highlightTile.fLeft+(oskLayout_.keyPos->at(rowIndex).at(keyIndex).highlightTile.width()/2),
                         oskLayout_.keyPos->at(rowIndex).at(keyIndex).highlightTile.fBottom,
                         textPaint);
    // Bounds Mid portion
    textPaint.setColor(SK_ColorMAGENTA);
    windowDelegatorCanvas->drawLine(bounds.fLeft,bounds.fTop+(bounds.height()/2),bounds.fRight,bounds.fTop+(bounds.height()/2),textPaint);
    windowDelegatorCanvas->drawLine(bounds.fLeft+(bounds.width()/2),bounds.fTop,bounds.fLeft+(bounds.width()/2),bounds.fBottom,textPaint);
  #endif/*SHOW_FONT_PLACING_ON_HLTILE*/
  }
}

void OnScreenKeyboard ::drawHighLightOnKey(SkPoint index) {

  if(oskState_!= OSK_STATE_ACTIVE) return;

  SkPaint paintObj;
  unsigned int rowIndex=index.y(),keyIndex=index.x();
  unsigned int lastRowIndex=lastFocussIndex_.y(),lastKeyIndex=lastFocussIndex_.x();

   RNS_PROFILE_START(HighlightOSKKey)
   //reset last focussed item
   paintObj.setColor(bgColor_);
   paintObj.setAntiAlias(true);
   windowDelegatorCanvas->drawRect(oskLayout_.keyPos->at(lastRowIndex).at(lastKeyIndex).highlightTile,paintObj);
   drawKBKeyFont({lastKeyIndex,lastRowIndex},fontColor_);

  //Hight current focussed item
  paintObj.setColor(OSK_HIGHLIGHT_BACKGROUND_COLOR);
  windowDelegatorCanvas->drawRect(oskLayout_.keyPos->at(rowIndex).at(keyIndex).highlightTile,paintObj);
  drawKBKeyFont({keyIndex,rowIndex},OSK_HIGHLIGHT_FONT_COLOR,true);
  RNS_PROFILE_END(" Highlight Completion : ",HighlightOSKKey)
}

void OnScreenKeyboard::onHWkeyHandler(rnsKey keyValue, rnsKeyAction eventKeyAction) {
  if((eventKeyAction != RNS_KEY_Press) || (oskState_ != OSK_STATE_ACTIVE)) return;
  SkPoint hlCandidate;
  bool drawCallPendingToRender{false};
  hlCandidate=lastFocussIndex_=currentFocussIndex_;
  rnsKey OSKkeyValue{RNS_KEY_UnKnown};
  unsigned int rowIndex=currentFocussIndex_.y(),keyIndex=currentFocussIndex_.x();
  RNS_LOG_DEBUG("KEY RECEIVED : "<<RNSKeyMap[keyValue]);
  switch( keyValue ) {
  /*Case 1: handle Enter/selection key*/
    case RNS_KEY_Select:
    {
      if(oskLayout_.keyInfo->at(rowIndex).at(keyIndex).keyValue == RNS_KEY_Select) {
        exit();
      } else if (oskLayout_.keyInfo->at(rowIndex).at(keyIndex).keyType == KEY_TYPE_TOGGLE){
        ToggleKeyMap :: iterator keyFunction =toggleKeyMap.find(oskLayout_.keyInfo->at(rowIndex).at(keyIndex).keyName);
        if((keyFunction != toggleKeyMap.end()) && (keyFunction->second != oskLayout_.kbLayoutType)) {
          oskLayout_.kbLayoutType=keyFunction->second;
          drawKBLayout(OSK_ALPHA_NUMERIC_KB);
          drawCallPendingToRender=true;
        }
      }else {
        OSKkeyValue=oskLayout_.keyInfo->at(rowIndex).at(keyIndex).keyValue;
        if(( oskLayout_.keyInfo->at(rowIndex).at(keyIndex).keyType == KEY_TYPE_TEXT ) &&
           (oskLayout_.kbLayoutType == ALPHA_UPPERCASE_LAYOUT) &&
           (isalpha(*oskLayout_.keyInfo->at(rowIndex).at(keyIndex).keyName))) {
          OSKkeyValue = static_cast<rnsKey>(OSKkeyValue-26);
        }
      }
    }
    break;
/* Case  2: Process Navigation Keys*/
    case RNS_KEY_Right:
      hlCandidate= oskLayout_.siblingInfo->at(rowIndex).at(keyIndex).siblingRight;
    break;
    case RNS_KEY_Left:
      hlCandidate= oskLayout_.siblingInfo->at(rowIndex).at(keyIndex).siblingLeft;
    break;
    case RNS_KEY_Up:
      hlCandidate= oskLayout_.siblingInfo->at(rowIndex).at(keyIndex).siblingUp;
    break;
    case RNS_KEY_Down:
      hlCandidate= oskLayout_.siblingInfo->at(rowIndex).at(keyIndex).siblingDown;
    break;
    /*Case 3: Emit back other known keys*/
    default:
    {
      bool keyFound=false;
      /*Process only KB keys*/
      if((keyValue < RNS_KEY_UnKnown ) && (keyValue >= RNS_KEY_1)) {
        rnsKey layoutKeyValue{RNS_KEY_UnKnown};
        for (unsigned int rowIndex=0; (rowIndex < oskLayout_.keyInfo->size()) && (!keyFound);rowIndex++) {
          for (unsigned int keyIndex=0; keyIndex<oskLayout_.keyInfo->at(rowIndex).size();keyIndex++) {
            layoutKeyValue=oskLayout_.keyInfo->at(rowIndex).at(keyIndex).keyValue;
            if(( oskLayout_.keyInfo->at(rowIndex).at(keyIndex).keyType == KEY_TYPE_TEXT ) &&
               (oskLayout_.kbLayoutType == ALPHA_UPPERCASE_LAYOUT) &&
               (isalpha(*oskLayout_.keyInfo->at(rowIndex).at(keyIndex).keyName))) {
                   layoutKeyValue = static_cast<rnsKey>(layoutKeyValue-26);
            }
            if(layoutKeyValue == keyValue) {
              hlCandidate.set(keyIndex,rowIndex);
              keyFound=true;
              OSKkeyValue =keyValue;
              break;
            }
          }
        }
      }
    }
    break;
  }
  if((lastFocussIndex_ != hlCandidate)) {
    drawHighLightOnKey(hlCandidate);
    currentFocussIndex_=hlCandidate;
    drawCallPendingToRender=true;
  }
  if( drawCallPendingToRender && (oskState_== OSK_STATE_ACTIVE)) commitDrawCall();

  RNS_LOG_DEBUG("OSK KEY VALUE RECEIVED : "<<RNSKeyMap[OSKkeyValue]);
  if(OSKkeyValue != RNS_KEY_UnKnown)  NotificationCenter::subWindowCenter().emit("onOSKKeyEvent", OSKkeyValue, RNS_KEY_Press);
}

void OnScreenKeyboard::createOSKLayout(OSKTypes oskType) {

  if(oskType == OSK_NUMERIC_KB) {
    RNS_LOG_DEBUG("DRAW call for Numeric KB");
    oskLayout_.keyInfo=&numericKBKeyKeyInfo;
    oskLayout_.keyPos=&numericKBKeyPos;
    oskLayout_.siblingInfo=&numericKBKeySiblingInfo;
    oskLayout_.kbGroupConfig=numericKBGroupConfig;
  } else {
    if(oskLayout_.kbLayoutType == SYMBOL_LAYOUT) {
      RNS_LOG_DEBUG("DRAW call for AlphaNumeric-symbol KB");
      oskLayout_.keyInfo=&symbolKBKBKeyInfo;
      oskLayout_.keyPos=&symbolKBKBKeyPos;
      oskLayout_.siblingInfo=&symbolKBKBKeySiblingInfo;
      oskLayout_.kbGroupConfig=symbolKBGroupConfig;
    } else {
      RNS_LOG_DEBUG("DRAW call for AlphaNumeric KB : "<<((oskLayout_.kbLayoutType == ALPHA_UPPERCASE_LAYOUT)? "UpperCase" : "LowerCase"));
      oskLayout_.keyInfo=&alphaNumericKBKeyInfo;
      oskLayout_.keyPos=&alphaNumericKBKeyPos;
      oskLayout_.siblingInfo=&alphaNumericKBKeySiblingInfo;
      oskLayout_.kbGroupConfig=alphaNumericKBGroupConfig;
    }
  }

  // Generate key Position info based on screen Size
  // Not needed, if screen Size is not changed & Layout already created
  if((!oskLayout_.keyPos->empty()) && (! generateOSKLayout_ )) return;

  generateOSKLayout_=false;//reset on creation
  RNS_LOG_DEBUG("Creating new Layout info for KB type : "<<oskConfig_.type<< " and Key Type : "<<oskLayout_.kbLayoutType);
  RNS_LOG_DEBUG("Create Layout request on resolution change : "<<generateOSKLayout_ <<" on Launch : "<<oskLayout_.keyPos->empty());

  if(oskLayout_.keyPos && oskLayout_.siblingInfo ) {
    oskLayout_.keyPos->clear();
    oskLayout_.siblingInfo->clear();
  }

  unsigned int groupKeyIndex{0},hlX,hlY,groupHLTileWidth,groupHLTileHeigth,groupID;
  float fontHeightAdjustment=0;
  SkPoint groupOffset,groupKeySpacing;
  SkRect bounds,boundsCapsHL,boundsHL;
  SkFont font,fontHL;
  char * keyName;
  SkString uniChar;
  sk_sp<SkTypeface> defaultTypeface;

  unsigned int YscaleFactor=screenSize_.height()/baseScreenSize.height();
  unsigned int rowSize=oskLayout_.keyInfo->size();
  oskLayout_.keyPos->resize(rowSize);
  oskLayout_.siblingInfo->resize(rowSize);
  SkPoint oskStartpt{ oskLayout_.horizontalStartOffset,
                     (OSK_KB_VERTICAL_OFFSET*screenSize_.height()*YscaleFactor)};

  for (unsigned int rowIndex = 0; rowIndex < rowSize; rowIndex++) {

    unsigned int columnSize=oskLayout_.keyInfo->at(rowIndex).size();
    oskLayout_.keyPos->at(rowIndex).resize(columnSize);
    oskLayout_.siblingInfo->at(rowIndex).resize(columnSize);
    if(!rowIndex ) oskLayout_.defaultFocussIndex.set((columnSize/2),rowIndex);

    for (unsigned int columnIndex = 0; columnIndex < columnSize; columnIndex++) {
  //1.  Calculate Highlight Tile
      groupID=oskLayout_.keyInfo->at(rowIndex).at(columnIndex).kbPartitionId;
      groupKeyIndex=(columnIndex !=0) ?((groupID != oskLayout_.keyInfo->at(rowIndex).at(columnIndex-1).kbPartitionId) ? 0: (groupKeyIndex+1)): columnIndex;
      keyName=(char*)oskLayout_.keyInfo->at(rowIndex).at(columnIndex).keyName;

      RNS_LOG_DEBUG("Group Index : "<<groupKeyIndex<<"Partition Id : "<<groupID);
      RNS_LOG_DEBUG("Group Offset : x "<<oskLayout_.kbGroupConfig[groupID].groupOffset.x()<<"y : "<<oskLayout_.kbGroupConfig[groupID].groupOffset.y());
      RNS_LOG_DEBUG("Group Key Spacing : x "<<oskLayout_.kbGroupConfig[groupID].groupKeySpacing.x()<<"y : "<<oskLayout_.kbGroupConfig[groupID].groupKeySpacing.x());

      if(!groupKeyIndex) {
        groupHLTileWidth=groupHLTileHeigth =(oskLayout_.textHLFontSize * oskLayout_.kbGroupConfig[groupID].maxTextLength* oskLayout_.kbGroupConfig[groupID].fontScaleFactor) * oskLayout_.kbGroupConfig[groupID].hlTileFontSizeMultiplier;
        if(oskLayout_.kbGroupConfig[groupID].maxTextLength) {
          groupHLTileHeigth =(oskLayout_.textHLFontSize * oskLayout_.kbGroupConfig[groupID].fontScaleFactor * oskLayout_.kbGroupConfig[groupID].hlTileFontSizeMultiplier);
      }
        groupOffset=oskLayout_.kbGroupConfig[groupID].groupOffset;
        groupKeySpacing=oskLayout_.kbGroupConfig[groupID].groupKeySpacing;
        font.setSize(oskLayout_.textFontSize * oskLayout_.kbGroupConfig[groupID].fontScaleFactor);
        font.setEdging(SkFont::Edging::kAntiAlias);
        fontHL.setSize(oskLayout_.textHLFontSize * oskLayout_.kbGroupConfig[groupID].fontScaleFactor);
        fontHL.setEdging(SkFont::Edging::kAntiAlias);
      }
      /* Enchancement Note:To Handle Variable tile width on the same group: Need to add Tile width info in the  Layout
            and here on generating tile x,y co-ordinate, previous tiles spacing to be accumulated and considered as groupKeySpacing
         */
      hlX = oskStartpt.x()+groupOffset.x()+(( groupHLTileWidth + groupKeySpacing.x()) * groupKeyIndex);
      hlY = oskStartpt.y()+groupOffset.y()+ (( groupHLTileHeigth + groupKeySpacing.y()) * rowIndex);
      oskLayout_.keyPos->at(rowIndex).at(columnIndex).highlightTile.setXYWH(hlX,hlY,groupHLTileWidth,groupHLTileHeigth);

  //2.  Calculate text draw position
        /*Calculate Font dimenesion */
      if(oskLayout_.keyInfo->at(rowIndex).at(columnIndex).keyType == KEY_TYPE_FUNCTION) {
        FunctionKeymap :: iterator keyFunction =functionKeyMap.find(keyName);
        keyName=(char*)DRAW_FONT_FAILURE_INDICATOR;
        if(keyFunction != functionKeyMap.end()) {
          UnicharFontConfig_t uniCharConfig = keyFunction->second;
          sk_sp<SkFontMgr> mgr(SkFontMgr::RefDefault());
          sk_sp<SkTypeface> typeface(mgr->matchFamilyStyleCharacter(nullptr, SkFontStyle(), nullptr, 0, uniCharConfig.unicharValue));
          if (typeface) {
            font.setTypeface(typeface);
            fontHL.setTypeface(typeface);
            font.setSize(oskLayout_.textFontSize * uniCharConfig.fontScaleFactor);
            fontHL.setSize(oskLayout_.textHLFontSize * uniCharConfig.fontScaleFactor);
            uniChar.reset();
            uniChar.appendUnichar(uniCharConfig.unicharValue);
            keyName=(char*)uniChar.c_str();
#ifdef SHOW_UNICHAR_FAMILY_INFO
             RNS_LOG_INFO("Draw Unicahr: "<<uniChar.c_str());
             typeface->getFamilyName(&familyName);
             RNS_LOG_INFO("UniCahr Found in FAMILY :"<<familyName.c_str());
#endif/*SHOW_UNICHAR_FAMILY_INFO*/
           }
        }
      } else {
        font.setTypeface(defaultTypeface);
        fontHL.setTypeface(defaultTypeface);
        if(( oskLayout_.keyInfo->at(rowIndex).at(columnIndex).keyType == KEY_TYPE_TEXT ) && (isalpha(*keyName))) {
          char upperCase=*keyName-LOWER_TO_UPPER_CASE_OFFSET;
          fontHL.measureText(&upperCase, 1, SkTextEncoding::kUTF8, &boundsCapsHL);
          oskLayout_.keyPos->at(rowIndex).at(columnIndex).textCapsHLXY.set(
                      (oskLayout_.keyPos->at(rowIndex).at(columnIndex).highlightTile.x() + (oskLayout_.keyPos->at(rowIndex).at(columnIndex).highlightTile.width() - boundsCapsHL.width() )/2),
                      (oskLayout_.keyPos->at(rowIndex).at(columnIndex).highlightTile.y() + (oskLayout_.keyPos->at(rowIndex).at(columnIndex).highlightTile.height() + boundsCapsHL.height() )/2));
        }
      }
      font.measureText(keyName, strlen(keyName), SkTextEncoding::kUTF8, &bounds);
      fontHL.measureText(keyName, strlen(keyName), SkTextEncoding::kUTF8,&boundsHL);

      oskLayout_.keyPos->at(rowIndex).at(columnIndex).textXY.set(
             (oskLayout_.keyPos->at(rowIndex).at(columnIndex).highlightTile.x() + (oskLayout_.keyPos->at(rowIndex).at(columnIndex).highlightTile.width() - bounds.width() )/2),
             (oskLayout_.keyPos->at(rowIndex).at(columnIndex).highlightTile.y() + (oskLayout_.keyPos->at(rowIndex).at(columnIndex).highlightTile.height() + bounds.height() )/2));
      oskLayout_.keyPos->at(rowIndex).at(columnIndex).textHLXY.set(
              (oskLayout_.keyPos->at(rowIndex).at(columnIndex).highlightTile.x() + (oskLayout_.keyPos->at(rowIndex).at(columnIndex).highlightTile.width() - boundsHL.width() )/2),
              (oskLayout_.keyPos->at(rowIndex).at(columnIndex).highlightTile.y() + (oskLayout_.keyPos->at(rowIndex).at(columnIndex).highlightTile.height() + boundsHL.height() )/2));
        /* Fix : to adjust the font inside Higlight tile. Need for symobol like (|  ` } { j),
                 which has greater decend or ascend
        */
      fontHeightAdjustment=0;
      boundsHL.offset(oskLayout_.keyPos->at(rowIndex).at(columnIndex).textHLXY.x(),oskLayout_.keyPos->at(rowIndex).at(columnIndex).textHLXY.y());
      if(boundsHL.fTop < oskLayout_.keyPos->at(rowIndex).at(columnIndex).highlightTile.fTop)
        fontHeightAdjustment = floor(oskLayout_.keyPos->at(rowIndex).at(columnIndex).highlightTile.fTop - boundsHL.fTop)+2;
      if(oskLayout_.keyPos->at(rowIndex).at(columnIndex).highlightTile.fBottom < boundsHL.fBottom)
        fontHeightAdjustment = -floor((boundsHL.fBottom - oskLayout_.keyPos->at(rowIndex).at(columnIndex).highlightTile.fBottom )+2);
      if(fontHeightAdjustment != 0) {
        oskLayout_.keyPos->at(rowIndex).at(columnIndex).textHLXY.set(
        oskLayout_.keyPos->at(rowIndex).at(columnIndex).textHLXY.x() ,
        oskLayout_.keyPos->at(rowIndex).at(columnIndex).textHLXY.y()+fontHeightAdjustment);
      }
    }
  }
//3.  Calculation Navigation index
  for (unsigned int rowIndex = 0; rowIndex < oskLayout_.keyInfo->size(); rowIndex++) {
    for (unsigned int columnIndex = 0; columnIndex < oskLayout_.keyInfo->at(rowIndex).size(); columnIndex++) {
      groupID=oskLayout_.keyInfo->at(rowIndex).at(columnIndex).kbPartitionId;
      unsigned int keyCount=oskLayout_.keyInfo->at(rowIndex).size(),index;
      unsigned int rowCount=oskLayout_.keyInfo->size();

      //Predicting sibling on Left
      index =(columnIndex) ? (columnIndex-1): keyCount-1;
      oskLayout_.siblingInfo->at(rowIndex).at(columnIndex).siblingLeft.set( index,rowIndex);

      //Predicting sibling on Right
      index =(columnIndex < keyCount-1) ? (columnIndex+1): 0;
      oskLayout_.siblingInfo->at(rowIndex).at(columnIndex).siblingRight.set( index,rowIndex);

      //Predicting neighbour for Up Navigation
      index =rowIndex;
      bool siblingFound{false};
      for(unsigned int i=0;(i<rowCount) && (!siblingFound);i++ ) {
        index = (index  !=0 ) ? index -1 :rowCount-1;
        for( unsigned int i=0; i< oskLayout_.keyPos->at(index).size();i++) {
          if(groupID == oskLayout_.keyInfo->at(index).at(i).kbPartitionId) {
            if(( oskLayout_.keyPos->at(index).at(i).highlightTile.fLeft >= oskLayout_.keyPos->at(rowIndex ).at(columnIndex).highlightTile.x() ) ||
               (  oskLayout_.keyPos->at(index).at(i).highlightTile.fRight >= oskLayout_.keyPos->at(rowIndex ).at(columnIndex).highlightTile.x())) {
                oskLayout_.siblingInfo->at(rowIndex).at(columnIndex).siblingUp.set( i,index);
                siblingFound=true;
                break;
           }
         }
       }
      }

      //Predicting neighbour for down Navigation
      index =rowIndex;
      siblingFound=false;
      for(unsigned int i=0;(i<rowCount) && (!siblingFound);i++ ) {
        index = (index != (rowCount-1) ) ? index +1 :0;
         for( unsigned int i=0; i< oskLayout_.keyPos->at(index).size();i++) {
           if(groupID == oskLayout_.keyInfo->at(index).at(i).kbPartitionId) {
             if(( oskLayout_.keyPos->at(index).at(i).highlightTile.fLeft >= oskLayout_.keyPos->at(rowIndex ).at(columnIndex).highlightTile.x() ) ||
               (  oskLayout_.keyPos->at(index).at(i).highlightTile.fRight >= oskLayout_.keyPos->at(rowIndex ).at(columnIndex).highlightTile.x())) {
                  oskLayout_.siblingInfo->at(rowIndex).at(columnIndex).siblingDown.set( i,index);
                siblingFound=true;
                break;
             }
           }
         }
       }
#ifdef SHOW_SIBLING_INFO
       unsigned int indexL=oskLayout_.siblingInfo->at(rowIndex).at(columnIndex).siblingLeft.x();
       unsigned int indexR=oskLayout_.siblingInfo->at(rowIndex).at(columnIndex).siblingRight.x();
       unsigned int upRow=oskLayout_.siblingInfo->at(rowIndex).at(columnIndex).siblingUp.y();
       unsigned int upKey=oskLayout_.siblingInfo->at(rowIndex).at(columnIndex).siblingUp.x();
       unsigned int downRow=oskLayout_.siblingInfo->at(rowIndex).at(columnIndex).siblingDown.y();
       unsigned int downKey=oskLayout_.siblingInfo->at(rowIndex).at(columnIndex).siblingDown.x();

       RNS_LOG_INFO(" Sibling key for key : "<<oskLayout_.keyInfo->at(rowIndex).at(columnIndex).keyName
           <<" left Key :  "<< oskLayout_.keyInfo->at(rowIndex).at(indexL).keyName
           <<"  rigt Key :  "<< oskLayout_.keyInfo->at(rowIndex).at(indexR).keyName
           <<"  Up Key :  "<< oskLayout_.keyInfo->at(upRow).at(upKey).keyName
           <<"  down Key :  "<< oskLayout_.keyInfo->at(downRow).at(downKey).keyName);
#endif/*SHOW_SIBLING_INFO*/
    }
  }
}

void OnScreenKeyboard::windowReadyToDrawCB() {
  if((windowDelegatorCanvas != nullptr) && (oskState_== OSK_STATE_LAUNCH_INPROGRESS)) {
    oskState_=OSK_STATE_ACTIVE;
    setWindowTittle("OSK Window");
    drawOSK();
    if(oskState_== OSK_STATE_ACTIVE) commitDrawCall();
  } else oskState_=OSK_STATE_INACTIVE;
}

} // namespace sdk
} // namespace rns


