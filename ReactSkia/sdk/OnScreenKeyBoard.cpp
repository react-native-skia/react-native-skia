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
std::mutex conditionalLockMutex;

OnScreenKeyboard& OnScreenKeyboard::getInstance() {
  static OnScreenKeyboard oskHandle;
  return oskHandle;
}

OSKErrorCode OnScreenKeyboard::launch(OSKConfig& oskConfig) {

  OnScreenKeyboard &oskHandle=OnScreenKeyboard::getInstance();

  if((oskHandle.oskState_ == OSK_STATE_LAUNCH_INPROGRESS) || (oskHandle.oskState_ == OSK_STATE_ACTIVE)) {
      return OSK_ERROR_ANOTHER_INSTANCE_ACTIVE;
  }
  std::scoped_lock lock(oskLaunchExitCtrlMutex);
  oskHandle.oskConfig_=oskConfig;
  oskHandle.oskState_=OSK_STATE_LAUNCH_INPROGRESS;
  onScreenKeyboardEventEmit(std::string("keyboardWillShow"));

  oskHandle.launchOSKWindow();
  return OSK_LAUNCH_SUCCESS;
}

void OnScreenKeyboard::exit() {

  OnScreenKeyboard &oskHandle=OnScreenKeyboard::getInstance();

  if((oskHandle.oskState_ == OSK_STATE_EXIT_INPROGRESS) || (oskHandle.oskState_ == OSK_STATE_INACTIVE)) {
    return;
  }

  oskHandle.oskState_=OSK_STATE_EXIT_INPROGRESS;

  std::scoped_lock lock(oskLaunchExitCtrlMutex);
  onScreenKeyboardEventEmit(std::string("keyboardWillHide"));
  oskHandle.closeWindow();

  /* Stop Listening for Hw Key Event*/
  if(oskHandle.subWindowKeyEventId_ != -1) {
      NotificationCenter::subWindowCenter().removeListener(oskHandle.subWindowKeyEventId_);
      oskHandle.subWindowKeyEventId_= -1;
  }
  onScreenKeyboardEventEmit(std::string("keyboardDidHide"));

/*Resetting old values & States*/
  oskHandle.oskState_=OSK_STATE_INACTIVE;
  oskHandle.autoActivateReturnKey_=false;
  oskHandle.displayString_.clear();
  oskHandle.displayStrWidth_=0;
  oskHandle.visibleDisplayStringRange_.set(-1,-1);//Setting to invalid
  oskHandle.lastFocussIndex_.set(0,0);
  oskHandle.currentFocussIndex_.set(0,0);
  oskHandle.emittedOSKKey_=RNS_KEY_UnKnown;
#if ENABLE(FEATURE_KEY_THROTTLING)
  if(oskHandle.repeatKeyQueue_ && !oskHandle.repeatKeyQueue_->isEmpty()) {
    oskHandle.repeatKeyQueue_->clear();
  }
  // If Repeat Key Handler Thread is still Active, Exit it.
  if(oskHandle.repeatKeyHandler_.joinable()) {
    if(oskHandle.waitingForKeyConsumedSignal_) {
      //Release from waiting on response from client.
      sem_post(&oskHandle.sigKeyConsumed_);
    } else {
      //post a dummy message to avoid dead lock on waiting for key Input.
      oskHandle.repeatKeyQueue_->push(RNS_KEY_UnKnown);
    }
  }
  // Finally ensure the thread exit.
  if(oskHandle.repeatKeyHandler_.joinable()) {
    oskHandle.repeatKeyHandler_.join();
  }
  oskHandle.onKeyRepeatMode_=false;
  oskHandle.previousKey_=RNS_KEY_UnKnown;
  oskHandle.waitingForKeyConsumedSignal_=false;
  sem_destroy(&oskHandle.sigKeyConsumed_);
  oskHandle.repeatKeyQueue_=nullptr;
#endif/*ENABLE_FEATURE_KEY_THROTTLING*/
}

void  OnScreenKeyboard::updatePlaceHolderString(std::string displayString,int cursorPosition) {
  OnScreenKeyboard &oskHandle=OnScreenKeyboard::getInstance();
  oskHandle.displayString_=displayString;
  oskHandle.cursorPosition_=cursorPosition;
  if(oskHandle.oskState_ != OSK_STATE_ACTIVE) {
    return;
  }
#if ENABLE(FEATURE_KEY_THROTTLING)
  if(oskHandle.waitingForKeyConsumedSignal_) {
    sem_post(&oskHandle.sigKeyConsumed_);
  }
#endif
  oskHandle.drawPlaceHolderDisplayString();
  if(oskHandle.oskState_== OSK_STATE_ACTIVE) {
    oskHandle.commitDrawCall();

  }
  oskHandle.sendDrawCommand(DRAW_PH_STRING);
}

void OnScreenKeyboard::launchOSKWindow() {

  SkSize mainscreenSize_=RnsShell::Window::getMainWindowSize();
  if(screenSize_ != mainscreenSize_) {
    generateOSKLayout_=true;
    screenSize_=mainscreenSize_;
  } else {
      generateOSKLayout_=false;
  }

  //Set up OSk configuration
  if(oskConfig_.type == OSK_ALPHA_NUMERIC_KB) {
    oskLayout_.kbLayoutType = ALPHA_LOWERCASE_LAYOUT;
  }
  if(oskConfig_.enablesReturnKeyAutomatically) {
    autoActivateReturnKey_=true;
  }

// setting up paint objects
  // Paint object for normal text
  textPaint_.setColor((oskConfig_.theme == OSK_LIGHT_THEME) ? OSK_LIGHT_THEME_FONT_COLOR: OSK_DARK_THEME_FONT_COLOR);
  textPaint_.setAntiAlias(true);
  // Paint object for Highlighted text
  textHLPaint_.setColor(OSK_HIGHLIGHT_FONT_COLOR);
  textHLPaint_.setAntiAlias(true);
  // Paint object for Place Holder
  placeHolderPaint_.setColor ((oskConfig_.theme == OSK_LIGHT_THEME) ? OSK_LIGHT_THEME_PLACEHOLDER_COLOR: OSK_DARK_THEME_PLACEHOLDER_COLOR);
  // Paint object for OSK BackGround
  oskBGPaint_.setColor((oskConfig_.theme == OSK_LIGHT_THEME) ? OSK_LIGHT_THEME_BACKGROUND_COLOR: OSK_DARK_THEME_BACKGROUND_COLOR);
  // Paint object for inactive text
  inactiveTextPaint_.setColor((oskConfig_.theme == OSK_LIGHT_THEME) ? OSK_LIGHT_THEME_INACTIVE_FONT_COLOR: OSK_DARK_THEME_INACTIVE_FONT_COLOR);
  inactiveTextPaint_.setAntiAlias(true);
  // Paint object for cursor
  cursorPaint_.setColor(OSK_PLACEHOLDER_CURSOR_COLOR);
  cursorPaint_.setAntiAlias(true);
  cursorPaint_.setStrokeWidth(OSK_PLACEHOLDER_CURSOR_WIDTH);
  // Paint object for HighLight Tile
  highLightTilePaint_.setColor(OSK_HIGHLIGHT_BACKGROUND_COLOR);

// Calculate the OSK components dimension w.r.t screen size*/
  SkScalar XscaleFactor = screenSize_.width()/baseScreenSize.width();
   /*Font Size*/
  textFont_.setSize(OSK_FONT_SIZE *XscaleFactor);
  textHLFont_.setSize(OSK_HIGHLIGHT_FONT_SIZE *XscaleFactor);
  SkGlyphID glyphs[1];
  textFont_.textToGlyphs(" ", 1, SkTextEncoding::kUTF8, glyphs, 1);
  textFont_.getWidthsBounds(glyphs, 1, &spaceWidth_, nullptr, nullptr);
  spaceWidth_ += 1; // Adding letter spacing- hardcoded for now

   /*PlaceHolder Title */
  oskLayout_.placeHolderTitleVerticalStart=screenSize_.height()*OSK_PLACEHOLDER_NAME_VERTICAL_OFFSET;
   /*PlaceHolder dimension */
  oskLayout_.placeHolderLength=screenSize_.width()*OSK_PLACEHOLDER_LENGTH;
  oskLayout_.placeHolderHeight=textFont_.getSize()*OSK_PLACEHOLDER_HEIGHT_SCALE_FACTOR;
  oskLayout_.placeHolderVerticalStart=screenSize_.height()*OSK_PLACEHOLDER_VERTICAL_OFFSET;
  oskLayout_.placeHolderTextVerticalStart=oskLayout_.placeHolderVerticalStart + oskLayout_.placeHolderHeight - ((oskLayout_.placeHolderHeight-textFont_.getSize())/2);
   /*Key Board */
  oskLayout_.kBVerticalStart=screenSize_.height()*OSK_KB_VERTICAL_OFFSET;
   /*Common Horizontal start offset for left alligned OSK*/
  oskLayout_.horizontalStartOffset= ((screenSize_.width()-oskLayout_.placeHolderLength)/2);

//Finally Creating OSK Window
  std::function<void()> createWindowCB = std::bind(&OnScreenKeyboard::windowReadyToDrawCB,this);
   createWindow(screenSize_,createWindowCB);
}

void OnScreenKeyboard::drawPlaceHolderDisplayString() {

  if(oskState_!= OSK_STATE_ACTIVE) {
    return;
  }
  SkScalar textWidth{0};
/*Caculate the visible string Range to display string*/
  if(!displayString_.empty()) {

/*1. Set Visible string Range or adjust the existing one
     Cases considered on decide on Visible string Range
     1. If Visible string Range Not set, set complete string range as visible
     2. If Visible string Range set , Use it
     3. If Curosr position Is outside the visible string Range, Adjust visible string range to include cursor
*/
    int visibleRangeStart=(visibleDisplayStringRange_.x() == -1) ? 0:
                        ((cursorPosition_ > visibleDisplayStringRange_.x()) ? visibleDisplayStringRange_.x(): cursorPosition_);
    int visibleRangeEnd=((visibleDisplayStringRange_.y() ==-1 )|| (visibleDisplayStringRange_.y() > displayString_.length()-1)) ?
                           displayString_.length()-1:
                         ((cursorPosition_ <= visibleDisplayStringRange_.y()) ? visibleDisplayStringRange_.y():cursorPosition_);

/*2. Check string range decided as visible, fits in the PlaceHolder*/
    textWidth=getStringBound (displayString_,visibleRangeStart,visibleRangeEnd,textFont_);

/*3  Expand the visible string range, if  placeHolder can accomodate more*/
    if(((textWidth+OSK_PLACEHOLDER_RESERVED_LENGTH) < oskLayout_.placeHolderLength) &&
         ((visibleRangeEnd-visibleRangeStart) < (displayString_.length()-1))) {
            if(visibleRangeEnd != (displayString_.length()-1)) visibleRangeEnd = displayString_.length()-1;
            else visibleRangeStart=0;
            textWidth=getStringBound (displayString_,visibleRangeStart,visibleRangeEnd,textFont_);
    }

/*4. slide and contract the visible string range to fit the string in placeHolder Area. Anchor Visible string Start or the end w.r.t Cursor position
     and adjust the free end to fit in the placeholder Length.*/
    while((textWidth+OSK_PLACEHOLDER_RESERVED_LENGTH) >= oskLayout_.placeHolderLength) {
      if(cursorPosition_ >= (visibleRangeEnd-1)) {
        visibleRangeStart++;
      } else {
        visibleRangeEnd--;
      }
      textWidth=getStringBound (displayString_,visibleRangeStart,visibleRangeEnd,textFont_);
    }
    if(cursorPosition_ > displayString_.length()) {
     cursorPosition_=displayString_.length();
    }
    visibleDisplayStringRange_.set(visibleRangeStart,visibleRangeEnd);
    displayString_=displayString_.substr(visibleRangeStart,(visibleRangeEnd-visibleRangeStart)+1);
  }

/* Clear old String*/
  clearScreen( oskLayout_.horizontalStartOffset,oskLayout_.placeHolderVerticalStart,displayStrWidth_,oskLayout_.placeHolderHeight,placeHolderPaint_);

/*Display Current String*/
  if(!displayString_.empty()) {
    windowDelegatorCanvas->drawSimpleText(displayString_.c_str(), displayString_.length(), SkTextEncoding::kUTF8,oskLayout_.horizontalStartOffset+OSK_PLACEHOLDER_LEFT_INSET,oskLayout_.placeHolderTextVerticalStart,textFont_, textPaint_);
  }
  displayStrWidth_=((textWidth+OSK_PLACEHOLDER_RESERVED_LENGTH) < oskLayout_.placeHolderLength ) ? (textWidth +OSK_PLACEHOLDER_RESERVED_LENGTH) :oskLayout_.placeHolderLength;

/* Display Cursor*/
  if(oskConfig_.showCursor){
    textWidth=0;
    int newcursorPosition = (cursorPosition_ >= (visibleDisplayStringRange_.x()+1)) ? ( cursorPosition_- visibleDisplayStringRange_.x()):0;
    if(newcursorPosition) {
      textWidth=getStringBound (displayString_,0,newcursorPosition-1,textFont_);
    }
    textWidth +=(oskLayout_.horizontalStartOffset+OSK_PLACEHOLDER_LEFT_INSET);
    windowDelegatorCanvas->drawLine(textWidth,oskLayout_.placeHolderTextVerticalStart,textWidth,oskLayout_.placeHolderTextVerticalStart-textFont_.getSize(),cursorPaint_);
  }
#ifdef  DRAW_STRING_BOUNDING_BOX
  SkPaint paint;
  paint.setColor(SK_ColorGREEN);
  paint.setStrokeWidth(2);
  paint.setStyle(SkPaint::kStroke_Style);
  SkRect testBounds;
  textFont_.measureText(displayString_.c_str(),  displayString_.size(), SkTextEncoding::kUTF8, &testBounds);
  testBounds.offset(oskLayout_.horizontalStartOffset,oskLayout_.placeHolderTextVerticalStart);
  windowDelegatorCanvas->drawRect(testBounds,paint);
#endif /*DRAW_STRING_BOUNDING_BOX*/
}

void OnScreenKeyboard::drawOSKBackGround() {

if(oskState_!= OSK_STATE_ACTIVE) return;

/*1. Set up OSK Window background*/
  windowDelegatorCanvas->clear(oskBGPaint_.getColor());

/*2. Draw PlaceHolder Title*/
  if(!oskConfig_.placeHolderName.empty()) {
    windowDelegatorCanvas->drawSimpleText(oskConfig_.placeHolderName.c_str(),oskConfig_.placeHolderName.size(), SkTextEncoding::kUTF8,
                               oskLayout_.horizontalStartOffset,
                               oskLayout_.placeHolderTitleVerticalStart,
                               textFont_,textPaint_);
  }

/*3. Draw PLaceHolder Display String */
  clearScreen(oskLayout_.horizontalStartOffset,oskLayout_.placeHolderVerticalStart,oskLayout_.placeHolderLength,OSK_PLACEHOLDER_HEIGHT,placeHolderPaint_);

  return;
}

void OnScreenKeyboard::drawKBLayout(OSKTypes oskType) {

  if(oskState_!= OSK_STATE_ACTIVE) return;

  RNS_PROFILE_START(OSKLayoutCreate)
  createOSKLayout(oskType);
  currentFocussIndex_=oskLayout_.defaultFocussIndex;
  RNS_PROFILE_END("OSk Layout Create Done : ",OSKLayoutCreate)
  RNS_PROFILE_START(OSKDraw)
//clear KeyBoard Area
  clearScreen( oskLayout_.horizontalStartOffset,oskLayout_.kBVerticalStart,
               screenSize_.width(),(screenSize_.height()- oskLayout_.kBVerticalStart),
               oskBGPaint_);
  if(oskLayout_.keyInfo && oskLayout_.keyPos) {
    /*1. Draw Keys */
    for (unsigned int rowIndex = 0; rowIndex < oskLayout_.keyInfo->size(); rowIndex++) {
      for (unsigned int keyIndex = 0; keyIndex < oskLayout_.keyInfo->at(rowIndex).size(); keyIndex++) {
        if(oskState_== OSK_STATE_ACTIVE) {
          drawKBKeyFont({keyIndex,rowIndex});
        } else { return;}
      }
    }
    /*2. Draw KB partition*/
    if(oskConfig_.type != OSK_NUMERIC_KB) {
      SkPaint paint;
      paint.setColor(placeHolderPaint_.getColor());
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
  }
  RNS_PROFILE_END("OSk Draw completion : ",OSKDraw)
}

inline void OnScreenKeyboard::drawKBKeyFont(SkPoint index,bool onHLTile) {

  if(oskState_!= OSK_STATE_ACTIVE) return;

  KeyInfo_t &keyInfo=oskLayout_.keyInfo->at(index.y()).at(index.x());
  keyPosition_t &keyPos=oskLayout_.keyPos->at(index.y()).at(index.x());

  if(keyInfo.keyName ) {
    SkPaint textPaint;
    SkFont font;
    SkString uniChar;
    char *keyName=(char*)keyInfo.keyName;
    char upperCase[2];
    unsigned int textX,textY,groupID;

    groupID=keyInfo.kbPartitionId;

    if(onHLTile) {
      font.setSize(keyPos.fontHLSize);
      textPaint.setColor(textHLPaint_.getColor());
      textX=keyPos.textHLXY.x();
      textY=keyPos.textHLXY.y();
    } else {
      textPaint.setColor(textPaint_.getColor());
      font.setSize(keyPos.fontSize);
      textX=keyPos.textXY.x();
      textY=keyPos.textXY.y();
      if (keyInfo.keyType == KEY_TYPE_TOGGLE) {
        ToggleKeyMap :: iterator keyFunction =toggleKeyMap.find(keyName);
        if(keyFunction != toggleKeyMap.end()) {
           if(keyFunction->second != oskLayout_.kbLayoutType) {
             textPaint.setColor(inactiveTextPaint_.getColor());
           }
        }
      }
    }
    if(( keyInfo.keyType == KEY_TYPE_TEXT ) &&
     (oskLayout_.kbLayoutType == ALPHA_UPPERCASE_LAYOUT) && (isalpha(*keyName))) {
      upperCase[0] = *keyName-LOWER_TO_UPPER_CASE_OFFSET;
      upperCase[1]='\0';
      keyName=upperCase;
      if(onHLTile) {
        textX=keyPos.textCapsHLXY.x();
        textY=keyPos.textCapsHLXY.y();
     }
    }

    bool needsCanvasRestore{false};
    if(keyInfo.keyType == KEY_TYPE_FUNCTION) {
     /*TempFix: Search Icon Presented properly only by "DejaVu Sans Mono/monospace" family. So hardcoding it
       NOTE : When more icon needs custom handling, custom options to be specified in functionKeyMap to handle in a genric way
     */
      char* fontFamily=nullptr;
      if(!strcmp(keyName,"return")) {
     /* If autoActivateReturnKey_ is set. Return key to be presented in disabled/In-Active look until TI was empty.*/
        if(autoActivateReturnKey_ && displayString_.empty()) {
          textPaint.setColor(inactiveTextPaint_.getColor());
        }
        if(oskConfig_.returnKeyLabel == OSK_RETURN_KEY_SEARCH) {
          keyName=(char*)"search";
          fontFamily =(char *)"DejaVu Sans Mono";
        } else {
          keyName=(char*)"enter";
        }
      }
      FunctionKeymap :: iterator keyFunction =functionKeyMap.find(keyName);
      if(keyFunction != functionKeyMap.end()) {
        UnicharFontConfig_t uniCharConfig = keyFunction->second;
        sk_sp<SkFontMgr> mgr(SkFontMgr::RefDefault());
        sk_sp<SkTypeface> typeface(mgr->matchFamilyStyleCharacter(fontFamily, SkFontStyle(), nullptr, 0, uniCharConfig.unicharValue));

        if (typeface) {
          uniChar.appendUnichar(uniCharConfig.unicharValue);
          font.setTypeface(typeface);
          /*Current Search icon used needs a flip to be alligned with rest of the icons*/
          if(!strcmp(keyName,"search")) {
            SkRect bounds;
            /*NOTE:: TBD-On extending supports for return keys, decision to be taken whether
                     draw co-ordiantes for any other return keys (other than deafult) to be done
                     here while drawing or can calculate during LAyout creation and maintained seperately
            */
            if(onHLTile) {
              font.setSize(textHLFont_.getSize() * uniCharConfig.fontScaleFactor);
            } else {
              font.setSize(textFont_.getSize() * uniCharConfig.fontScaleFactor);
            }
            font.measureText(uniChar.c_str(), uniChar.size(), SkTextEncoding::kUTF8, &bounds);
            textX = keyPos.highlightTile.x() + ((keyPos.highlightTile.width() - bounds.width() )/2);
            textY = keyPos.highlightTile.y() + ((keyPos.highlightTile.height() + bounds.height()) /2);
            textX-=5;textY+=5;// For allignment
            windowDelegatorCanvas->save();
            SkMatrix transformMatrix;
            //to Flip on x-axis - anchor @ center
            transformMatrix.preRotate(270,textX+(bounds.width()/2),textY-(bounds.height()/2));
            transformMatrix.postTranslate(bounds.width()/2,0);
            windowDelegatorCanvas->concat(transformMatrix);
            needsCanvasRestore=true;
          }
 #ifdef DISPLAY_FONT_FAMILY_INFO
              SkString familyName;
              typeface->getFamilyName(&familyName);
              RNS_LOG_INFO(" Font Family Name :"<<familyName.c_str() <<" Key Name : "<<keyName);
#endif/*DISPLAY_FONT_FAMILY_INFO*/
          keyName=(char *)uniChar.c_str();
        } else {
           keyName=(char *)DRAW_FONT_FAILURE_INDICATOR;
        }
      } else {
         keyName=(char *)DRAW_FONT_FAILURE_INDICATOR;
      }
    }
    windowDelegatorCanvas->drawSimpleText(keyName, strlen(keyName), SkTextEncoding::kUTF8,textX,textY,font, textPaint);
    if(needsCanvasRestore) windowDelegatorCanvas->restore();
#ifdef SHOW_FONT_PLACING_ON_HLTILE
    SkPaint textPaint;
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
    windowDelegatorCanvas->drawRect(keyPos.highlightTile,textPaint);
    textPaint.setColor(SK_ColorYELLOW);
    textPaint.setStrokeWidth(1);
    //HighLightTile Centre
    windowDelegatorCanvas->drawLine(keyPos.highlightTile.fLeft,
                         keyPos.highlightTile.fTop+(keyPos.highlightTile.height()/2),
                         keyPos.highlightTile.fRight,
                         keyPos.highlightTile.fTop+(keyPos.highlightTile.height()/2),
                         textPaint);
    windowDelegatorCanvas->drawLine(keyPos.highlightTile.fLeft+(keyPos.highlightTile.width()/2),
                         keyPos.highlightTile.fTop,
                         keyPos.highlightTile.fLeft+(keyPos.highlightTile.width()/2),
                         keyPos.highlightTile.fBottom,
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

  unsigned int rowIndex=index.y(),keyIndex=index.x();
  unsigned int lastRowIndex=lastFocussIndex_.y(),lastKeyIndex=lastFocussIndex_.x();

  RNS_PROFILE_START(HighlightOSKKey)
  //reset last focussed item
  windowDelegatorCanvas->drawRect(oskLayout_.keyPos->at(lastRowIndex).at(lastKeyIndex).highlightTile,oskBGPaint_);
  drawKBKeyFont({lastKeyIndex,lastRowIndex});

  //Hight current focussed item
  windowDelegatorCanvas->drawRect(oskLayout_.keyPos->at(rowIndex).at(keyIndex).highlightTile,highLightTilePaint_);
  drawKBKeyFont({keyIndex,rowIndex},true);
  RNS_PROFILE_END(" Highlight Completion : ",HighlightOSKKey)
}

void OnScreenKeyboard::onHWkeyHandler(rnsKey keyValue, rnsKeyAction eventKeyAction) {
  RNS_LOG_DEBUG("rnsKey: "<<RNSKeyMap[keyValue]<<" rnsKeyAction: "<<((eventKeyAction ==0) ? "RNS_KEY_Press ": "RNS_KEY_Release ") );

  if(eventKeyAction == RNS_KEY_Release) {
#if ENABLE(FEATURE_KEY_THROTTLING)
    if(onKeyRepeatMode_) {
      if(!repeatKeyQueue_->isEmpty()) {
        repeatKeyQueue_->clear();
      }
    }
    previousKey_ = RNS_KEY_UnKnown;
    onKeyRepeatMode_ = false;
#endif /*ENABLE_FEATURE_KEY_THROTTLING*/
    if(emittedOSKKey_ != RNS_KEY_UnKnown) {
      NotificationCenter::subWindowCenter().emit("onOSKKeyEvent", emittedOSKKey_, RNS_KEY_Release);
    } else {
      NotificationCenter::subWindowCenter().emit("onOSKKeyEvent", keyValue, RNS_KEY_Release);
    }
    return;
  }

  if(oskState_ != OSK_STATE_ACTIVE) { return;}
#if ENABLE(FEATURE_KEY_THROTTLING)
  if(previousKey_ == keyValue  && eventKeyAction == RNS_KEY_Press) {
    onKeyRepeatMode_ = true;
    repeatKeyQueue_->push(keyValue);
  } else
#endif /*ENABLE_FEATURE_KEY_THROTTLING*/
  {
    processKey(keyValue);
  }
#if ENABLE(FEATURE_KEY_THROTTLING)
  previousKey_=keyValue;
#endif/*ENABLE_FEATURE_KEY_THROTTLING*/
}

inline void OnScreenKeyboard::processKey(rnsKey keyValue) {
  if(keyValue == RNS_KEY_UnKnown){
    return;
  }
  if(oskState_ != OSK_STATE_ACTIVE) return;
  
  SkPoint hlCandidate;
  hlCandidate=lastFocussIndex_=currentFocussIndex_;
  rnsKey OSKkeyValue{RNS_KEY_UnKnown};
  unsigned int rowIndex=currentFocussIndex_.y(),keyIndex=currentFocussIndex_.x();
  RNS_LOG_DEBUG("KEY RECEIVED : "<<RNSKeyMap[keyValue]);
  switch( keyValue ) {
/* Case  1: Process Navigation Keys*/
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
/*Case 2: handle Enter/selection key*/
    case RNS_KEY_Select:
    {
      bool proceedToKeyEmit=false;
      if(oskLayout_.keyInfo->at(rowIndex).at(keyIndex).keyValue == RNS_KEY_Select) {
        proceedToKeyEmit=true; /* If selection on select/return Key needs to be emitted to client*/
      } else if (oskLayout_.keyInfo->at(rowIndex).at(keyIndex).keyType == KEY_TYPE_TOGGLE){
        ToggleKeyMap :: iterator keyFunction =toggleKeyMap.find(oskLayout_.keyInfo->at(rowIndex).at(keyIndex).keyName);
        if((keyFunction != toggleKeyMap.end()) && (keyFunction->second != oskLayout_.kbLayoutType)) {
          oskLayout_.kbLayoutType=keyFunction->second;
          sendDrawCommand(DRAW_KB);
          sendDrawCommand(DRAW_HL);//Set default Focuss
        }
      }else {
        OSKkeyValue=oskLayout_.keyInfo->at(rowIndex).at(keyIndex).keyValue;
        if(( oskLayout_.keyInfo->at(rowIndex).at(keyIndex).keyType == KEY_TYPE_TEXT ) &&
           (oskLayout_.kbLayoutType == ALPHA_UPPERCASE_LAYOUT) &&
           (isalpha(*oskLayout_.keyInfo->at(rowIndex).at(keyIndex).keyName))) {
          OSKkeyValue = static_cast<rnsKey>(OSKkeyValue-26);
        }
      }
      if(!proceedToKeyEmit)break;
    }
/*Case 3: Emit back other known keys*/
    default:
    {
      bool keyFound=false;
      /*Process only KB keys*/
      if(( keyValue == RNS_KEY_Select) ||
         ((keyValue < RNS_KEY_UnKnown ) && (keyValue >= RNS_KEY_1))) {
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

  RNS_LOG_DEBUG("OSK KEY VALUE RECEIVED : "<<RNSKeyMap[OSKkeyValue]);

  /* Enable Return Key on Valid Key Event if disabled*/
  if((OSKkeyValue != RNS_KEY_UnKnown) && autoActivateReturnKey_) {
    autoActivateReturnKey_=false;
    currentFocussIndex_=oskLayout_.returnKeyIndex;
    sendDrawCommand(DRAW_HL);
  }
  if((lastFocussIndex_ != hlCandidate)) {
/* To reduce the draw call and to avoid the frequency of swap buffer issue in openGL backend,
   trigger commit only if there is no key emit to client. As in the other case commit will be
   taken care as part of update string call from client , followed by key emit
*/
    currentFocussIndex_=hlCandidate;
    sendDrawCommand(DRAW_HL);
   }

  /* Emit only known keys to client*/
  if(OSKkeyValue != RNS_KEY_UnKnown) {
#if ENABLE(FEATURE_KEY_THROTTLING)
    if(onKeyRepeatMode_) {
      waitingForKeyConsumedSignal_=true;
    }
#endif
    emittedOSKKey_=OSKkeyValue;
    NotificationCenter::subWindowCenter().emit("onOSKKeyEvent", OSKkeyValue, RNS_KEY_Press);
  }
}

inline void OnScreenKeyboard::clearScreen(SkScalar x,SkScalar y,SkScalar width,SkScalar height,SkPaint & paintObj) {
    SkRect rect=SkRect::MakeXYWH( x,y,width,height);
    windowDelegatorCanvas->drawRect(rect,paintObj);
}

inline SkScalar OnScreenKeyboard:: getStringBound (const std::string & stringToMeasure,
                                                      unsigned int startIndex,unsigned int endIndex,
                                                      SkFont & stringFont) {
  unsigned int spaceCount{0};
  SkScalar width{0};
  if(!stringToMeasure.empty() ) {
  /* Fix: Skfont ignores white spaces in the begining and end of the string, while calculating bounds.
     So adding width of space seperately*/
    if(!stringToMeasure.compare(0,1," ")) { spaceCount++; startIndex++; }
    if(!stringToMeasure.compare(stringToMeasure.size()-1,1," ")) { spaceCount++; endIndex--; }
    if(startIndex >= 0) {
      width=stringFont.measureText(&stringToMeasure.c_str()[startIndex],(endIndex-startIndex)+1, SkTextEncoding::kUTF8);
    }
    if(spaceCount) width +=(spaceWidth_*spaceCount);
  }
  return width;
}

void OnScreenKeyboard::createOSKLayout(OSKTypes oskType) {

  if(oskType == OSK_NUMERIC_KB) {
    RNS_LOG_DEBUG("DRAW call for Numeric KB");
    oskLayout_.keyInfo=&numericKBKeyKeyInfo;
    oskLayout_.keyPos=&numericKBKeyPos;
    oskLayout_.siblingInfo=&numericKBKeySiblingInfo;
    oskLayout_.kbGroupConfig=numericKBGroupConfig;
    oskLayout_.returnKeyIndex=numericKBReturnKeyIndex;
    oskLayout_.defaultFocussIndex=numericKBDefaultHLKeyIndex;
  } else {
    if(oskLayout_.kbLayoutType == SYMBOL_LAYOUT) {
      RNS_LOG_DEBUG("DRAW call for AlphaNumeric-symbol KB");
      oskLayout_.keyInfo=&symbolKBKBKeyInfo;
      oskLayout_.keyPos=&symbolKBKBKeyPos;
      oskLayout_.siblingInfo=&symbolKBKBKeySiblingInfo;
      oskLayout_.kbGroupConfig=symbolKBGroupConfig;
      oskLayout_.returnKeyIndex=symbolKBReturnKeyIndex;
      oskLayout_.defaultFocussIndex=symbolKBDefaultHLKeyIndex;
    } else {
      RNS_LOG_DEBUG("DRAW call for AlphaNumeric KB : "<<((oskLayout_.kbLayoutType == ALPHA_UPPERCASE_LAYOUT)? "UpperCase" : "LowerCase"));
      oskLayout_.keyInfo=&alphaNumericKBKeyInfo;
      oskLayout_.keyPos=&alphaNumericKBKeyPos;
      oskLayout_.siblingInfo=&alphaNumericKBKeySiblingInfo;
      oskLayout_.kbGroupConfig=alphaNumericKBGroupConfig;
      oskLayout_.returnKeyIndex=alphaNumericKBReturnKeyIndex;
      oskLayout_.defaultFocussIndex=alphaNumericKBDefaultHLKeyIndex;
    }
  }

  // Generate key Position info based on screen Size
  // Not needed, if screen Size is not changed & Layout already created
  if((!oskLayout_.keyPos->empty()) && (! generateOSKLayout_ )) { return;}

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
  SkScalar XscaleFactor = screenSize_.width()/baseScreenSize.width();
  SkScalar YscaleFactor = screenSize_.height()/baseScreenSize.height();

  unsigned int rowSize=oskLayout_.keyInfo->size();
  oskLayout_.keyPos->resize(rowSize);
  oskLayout_.siblingInfo->resize(rowSize);

  for (unsigned int rowIndex = 0; rowIndex < rowSize; rowIndex++) {

    unsigned int columnSize=oskLayout_.keyInfo->at(rowIndex).size();
    oskLayout_.keyPos->at(rowIndex).resize(columnSize);
    oskLayout_.siblingInfo->at(rowIndex).resize(columnSize);

    for (unsigned int columnIndex = 0; columnIndex < columnSize; columnIndex++) {
      KeyInfo_t &keyInfo=oskLayout_.keyInfo->at(rowIndex).at(columnIndex);
      keyPosition_t &keyPos=oskLayout_.keyPos->at(rowIndex).at(columnIndex);

  //1.  Calculate Highlight Tile
      groupID=keyInfo.kbPartitionId;
      groupKeyIndex=(columnIndex !=0) ?((groupID != oskLayout_.keyInfo->at(rowIndex).at(columnIndex-1).kbPartitionId) ? 0: (groupKeyIndex+1)): columnIndex;
      keyName=(char*)keyInfo.keyName;

      RNS_LOG_DEBUG("Group Index : "<<groupKeyIndex<<"Partition Id : "<<groupID);
      RNS_LOG_DEBUG("Group Offset : x "<<oskLayout_.kbGroupConfig[groupID].groupOffset.x()<<"y : "<<oskLayout_.kbGroupConfig[groupID].groupOffset.y());
      RNS_LOG_DEBUG("Group Key Spacing : x "<<oskLayout_.kbGroupConfig[groupID].groupKeySpacing.x()<<"y : "<<oskLayout_.kbGroupConfig[groupID].groupKeySpacing.x());

      if(!groupKeyIndex) {
        groupHLTileWidth=groupHLTileHeigth =(textHLFont_.getSize() * oskLayout_.kbGroupConfig[groupID].maxTextLength* oskLayout_.kbGroupConfig[groupID].fontScaleFactor) * oskLayout_.kbGroupConfig[groupID].hlTileFontSizeMultiplier;
        if(oskLayout_.kbGroupConfig[groupID].maxTextLength) {
          groupHLTileHeigth =(textHLFont_.getSize() * oskLayout_.kbGroupConfig[groupID].fontScaleFactor * oskLayout_.kbGroupConfig[groupID].hlTileFontSizeMultiplier);
      }
        groupOffset.set(oskLayout_.kbGroupConfig[groupID].groupOffset.x()*XscaleFactor,
                        oskLayout_.kbGroupConfig[groupID].groupOffset.y()*YscaleFactor);
        groupKeySpacing.set(oskLayout_.kbGroupConfig[groupID].groupKeySpacing.x()*XscaleFactor,
                            oskLayout_.kbGroupConfig[groupID].groupKeySpacing.y()*YscaleFactor);
        font.setSize(textFont_.getSize() * oskLayout_.kbGroupConfig[groupID].fontScaleFactor);
        font.setEdging(SkFont::Edging::kAntiAlias);
        fontHL.setSize(textHLFont_.getSize() * oskLayout_.kbGroupConfig[groupID].fontScaleFactor);
        fontHL.setEdging(SkFont::Edging::kAntiAlias);
      }
      /* Enchancement Note:To Handle Variable tile width on the same group: Need to add Tile width info in the  Layout
            and here on generating tile x,y co-ordinate, previous tiles spacing to be accumulated and considered as groupKeySpacing
         */
      hlX = oskLayout_.horizontalStartOffset+groupOffset.x()+(( groupHLTileWidth + groupKeySpacing.x()) * groupKeyIndex);
      hlY = oskLayout_.kBVerticalStart+groupOffset.y()+ (( groupHLTileHeigth + groupKeySpacing.y()) * rowIndex);
      keyPos.highlightTile.setXYWH(hlX,hlY,groupHLTileWidth,groupHLTileHeigth);

  //2.  Calculate text draw position
        /*Calculate Font dimenesion */
      if(keyInfo.keyType == KEY_TYPE_FUNCTION) {
        FunctionKeymap :: iterator keyFunction =functionKeyMap.find(keyName);
        keyName=(char*)DRAW_FONT_FAILURE_INDICATOR;
        if(keyFunction != functionKeyMap.end()) {
          UnicharFontConfig_t uniCharConfig = keyFunction->second;
          sk_sp<SkFontMgr> mgr(SkFontMgr::RefDefault());
          sk_sp<SkTypeface> typeface(mgr->matchFamilyStyleCharacter(nullptr, SkFontStyle(), nullptr, 0, uniCharConfig.unicharValue));
          if (typeface) {
            font.setTypeface(typeface);
            fontHL.setTypeface(typeface);
            font.setSize(textFont_.getSize() * uniCharConfig.fontScaleFactor);
            fontHL.setSize(textHLFont_.getSize() * uniCharConfig.fontScaleFactor);
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
        if(( keyInfo.keyType == KEY_TYPE_TEXT ) && (isalpha(*keyName))) {
          char upperCase=*keyName-LOWER_TO_UPPER_CASE_OFFSET;
          fontHL.measureText(&upperCase, 1, SkTextEncoding::kUTF8, &boundsCapsHL);
          keyPos.textCapsHLXY.set((keyPos.highlightTile.x() + (keyPos.highlightTile.width() - boundsCapsHL.width() )/2),
                                   (keyPos.highlightTile.y() + (keyPos.highlightTile.height() + boundsCapsHL.height() )/2));
        }
      }
      font.measureText(keyName, strlen(keyName), SkTextEncoding::kUTF8, &bounds);
      fontHL.measureText(keyName, strlen(keyName), SkTextEncoding::kUTF8,&boundsHL);

      keyPos.fontSize=font.getSize();
      keyPos.fontHLSize=fontHL.getSize();
      keyPos.textXY.set((keyPos.highlightTile.x() + (keyPos.highlightTile.width() - bounds.width() )/2),
                         (keyPos.highlightTile.y() + (keyPos.highlightTile.height() + bounds.height() )/2));
      keyPos.textHLXY.set((keyPos.highlightTile.x() + (keyPos.highlightTile.width() - boundsHL.width() )/2),
                           (keyPos.highlightTile.y() + (keyPos.highlightTile.height() + boundsHL.height() )/2));
        /* Fix : to adjust the font inside Higlight tile. Need for symobol like (|  ` } { j),
                 which has greater decend or ascend
        */
      fontHeightAdjustment=0;
      boundsHL.offset(keyPos.textHLXY.x(),keyPos.textHLXY.y());
      if(boundsHL.fTop < keyPos.highlightTile.fTop) {
        fontHeightAdjustment = floor(keyPos.highlightTile.fTop - boundsHL.fTop)+2;
      }
      if(keyPos.highlightTile.fBottom < boundsHL.fBottom) {
        fontHeightAdjustment = -floor((boundsHL.fBottom - keyPos.highlightTile.fBottom )+2);
      }
      if(fontHeightAdjustment != 0) {
        keyPos.textHLXY.set(keyPos.textHLXY.x() , keyPos.textHLXY.y()+fontHeightAdjustment);
      }
    }
  }
#ifdef SHOW_KEYPOSITION_INFO
  RNS_LOG_INFO("KEY NAME : "<<keyName);
  RNS_LOG_INFO("Text X : "<<keyPos.textXY.x()<<"  Y : "<<keyPos.textXY.y());
  RNS_LOG_INFO("Text HL X : "<<keyPos.textHLXY.x()<<"  Y : "<<keyPos.textHLXY.y());
#endif/*SHOW_KEYPOSITION_INFO*/

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
  if(oskState_== OSK_STATE_LAUNCH_INPROGRESS) {
    oskState_=OSK_STATE_ACTIVE;
    setWindowTittle("OSK Window");
    sendDrawCommand(DRAW_OSK_BG);
    sendDrawCommand(DRAW_PH_STRING);
    sendDrawCommand(DRAW_KB);
    sendDrawCommand(DRAW_HL);
 
    if(oskState_== OSK_STATE_ACTIVE) {
#if ENABLE(FEATURE_KEY_THROTTLING)
    /*create Queue & KeyHandler for Repeat key Processing */
      sem_init(&sigKeyConsumed_, 0, 0);
      repeatKeyQueue_ =  std::make_unique<ThreadSafeQueue<rnsKey>>();
      repeatKeyHandler_ = std::thread(&OnScreenKeyboard::repeatKeyProcessingThread, this);
#endif
      /*Listen for  Key Press event */
      if((oskState_== OSK_STATE_ACTIVE) && (subWindowKeyEventId_ == -1) ) {
        std::function<void(rnsKey, rnsKeyAction)> handler = std::bind(&OnScreenKeyboard::onHWkeyHandler,this,
                                                                       std::placeholders::_1,
                                                                       std::placeholders::_2);
        subWindowKeyEventId_ = NotificationCenter::subWindowCenter().addListener("onHWKeyEvent", handler);
      }
      onScreenKeyboardEventEmit(std::string("keyboardDidShow"));
  } else {
    oskState_=OSK_STATE_INACTIVE;
  }
}

void OnScreenKeyboard::onScreenKeyboardEventEmit(std::string eventType){
  NotificationCenter::subWindowCenter().emit("onScreenKeyboardEvent",eventType);
}

#if ENABLE(FEATURE_KEY_THROTTLING)
void OnScreenKeyboard::repeatKeyProcessingThread(){
  rnsKey eventKeyType;
  while(oskState_ == OSK_STATE_ACTIVE) {
    if(waitingForKeyConsumedSignal_) {
      sem_wait(&sigKeyConsumed_);
      waitingForKeyConsumedSignal_=false;
    }
    if(oskState_ == OSK_STATE_ACTIVE) {
      repeatKeyQueue_->pop(eventKeyType);
      processKey(eventKeyType);
    }
  }
}
#endif

void OnScreenKeyboard::sendDrawCommand(DrawCommands commands) {
   std::scoped_lock lock(conditionalLockMutex);
   SkPictureRecorder pictureRecorder_;
   std::string commandKey;
   pictureCanvas_ = pictureRecorder_.beginRecording(SkRect::MakeXYWH(0, 0, screenSize_.width(), screenSize_.height()));
   switch(commands) {
     case DRAW_OSK_BG:
     RNS_LOG_INFO("@@@ Got Task to work:DRAW_OSK_BG@@");
     drawOSKBackGround();
     commandKey="OSKBackGround";
     setBasePicCommand(commandKey);
     break;
     case DRAW_PH_STRING:
       RNS_LOG_INFO("@@@ Got Task to work:DRAW_PH_STRING@@");
       drawPlaceHolderDisplayString();
       commandKey="EmbededTIString";
       break;
      case DRAW_HL:
        RNS_LOG_INFO("@@@ Got Task to work:DRAW_HL@@");
        commandKey="HighLight";
        drawHighLightOnKey(currentFocussIndex_);
      break;
     case DRAW_KB:
       RNS_LOG_INFO("@@@ Got Task to work:DRAW_KB@@");
       commandKey="KeyBoardLayout";
       drawKBLayout(oskConfig_.type);
      break;
     default:
     break;
   }
   auto pic = pictureRecorder_.finishRecordingAsPicture();
   if(pic.get()) {
     RNS_LOG_INFO("SkPicture ( "  << pic << " )For " <<
     pic.get()->approximateOpCount() << " operations and size : " << pic.get()->approximateBytesUsed());
   }
   if(oskState_== OSK_STATE_ACTIVE) commitDrawCall(commandKey,pic);
}

} // namespace sdk
} // namespace rns


