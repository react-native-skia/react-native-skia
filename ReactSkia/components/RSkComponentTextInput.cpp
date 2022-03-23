/*
 * Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkPaint.h"
#include "ReactSkia/components/RSkComponentTextInput.h"
#include "ReactSkia/components/RSkComponent.h"
#include "ReactSkia/core_modules/RSkSpatialNavigator.h"
#include "ReactSkia/sdk/RNSKeyCodeMapping.h"
#include "ReactSkia/views/common/RSkDrawUtils.h"
#include "ReactSkia/views/common/RSkConversion.h"
#include "rns_shell/compositor/layers/PictureLayer.h"

#include <string.h>
#include <iostream>
#include <queue>
#include <thread>
#include <semaphore.h>
#include <mutex>  


namespace facebook {
namespace react {

using namespace RSkDrawUtils;
using namespace skia::textlayout;

#define NUMBER_OF_LINES         1
#define FONTSIZE_MULTIPLIER     10
#define CURSOR_WIDTH 2

std::queue<rnsKey> inputQueue;
sem_t jsUpdateMutex;
std::mutex privateVarProtectorMutex;

RSkComponentTextInput::RSkComponentTextInput(const ShadowView &shadowView)
    : RSkComponent(shadowView)
    ,isInEditingMode_(false)
    ,isTextInputInFocus_(false)
    ,eventCount_(0)
    ,cursor_({0,0})
    ,paragraph_(nullptr){
  RNS_LOG_DEBUG("RSkComponentTextInput called constructor");
  cursorPaint_.setColor(SK_ColorBLUE);
  cursorPaint_.setAntiAlias(true);
  cursorPaint_.setStyle(SkPaint::kStroke_Style);
  cursorPaint_.setStrokeWidth(CURSOR_WIDTH);
  
}

void RSkComponentTextInput::drawAndSubmit(){
  layer()->client().notifyFlushBegin();
  layer()->invalidate( RnsShell::LayerPaintInvalidate);
  if (layer()->type() == RnsShell::LAYER_TYPE_PICTURE) {
    RNS_PROFILE_API_OFF(getComponentData().componentName << " getPicture :", static_cast<RnsShell::PictureLayer*>(layer().get())->setPicture(getPicture()));
  }
  layer()->client().notifyFlushRequired();
}

void RSkComponentTextInput::drawTextInput(SkCanvas *canvas,
  LayoutMetrics layout,
  const TextInputProps& props,
  struct RSkSkTextLayout &textLayout) {
  Rect frame = layout.frame;
  float yOffset;
  
  //cursor
  int position = 0;
  SkRect cursorRect;
  std::vector<TextBox> rects;

  // setParagraphStyle
  textLayout.paraStyle.setMaxLines(NUMBER_OF_LINES);
  textLayout.paraStyle.setEllipsis(u"\u2026");
  textLayout.builder->setParagraphStyle(textLayout.paraStyle);

  // buildParagraph
  paragraph_ = textLayout.builder->Build();
  paragraph_->layout(layout.getContentFrame().size.width);

  // clipRect and backgroundColor
  canvas->clipRect(SkRect::MakeXYWH(frame.origin.x, frame.origin.y, frame.size.width, frame.size.height));
  canvas->drawColor(RSkColorFromSharedColor(props.backgroundColor, SK_ColorTRANSPARENT));

  // paintParagraph
  yOffset = (layout.getContentFrame().size.height - paragraph_->getHeight()) / 2;
  paragraph_->paint(canvas, frame.origin.x + layout.contentInsets.left, frame.origin.y + layout.contentInsets.top + yOffset);

  // draw Cursor
  if (isInEditingMode_ && !caretHidden_) {
  position = cursor_.end - cursor_.locationFromEnd;
  if (cursor_.locationFromEnd == cursor_.end) {
    rects = paragraph_->getRectsForRange(0, position+1, RectHeightStyle::kTight, RectWidthStyle::kTight);
    cursorRect.fLeft =  frame.origin.x + rects[0].rect.left() + layout.contentInsets.left;
    cursorRect.fRight = cursorRect.fLeft;
  } else {
    rects = paragraph_->getRectsForRange(0, position, RectHeightStyle::kTight, RectWidthStyle::kTight);
    cursorRect.fLeft =  frame.origin.x + rects[0].rect.right() + layout.contentInsets.left;
    cursorRect.fRight = cursorRect.fLeft;
  }
  cursorRect.fTop = frame.origin.y + yOffset + layout.contentInsets.top;
  cursorRect.fBottom = cursorRect.fTop + paragraph_->getHeight();
  canvas->drawRect(cursorRect, cursorPaint_);
  }
}

void RSkComponentTextInput::OnPaint(SkCanvas *canvas) {
  auto component = getComponentData();
  auto const &textInputProps = *std::static_pointer_cast<TextInputProps const>(component.props);
  auto state = std::static_pointer_cast<TextInputShadowNode::ConcreteStateT const>(component.state);
  auto data = state->getData();
  auto borderMetrics = textInputProps.resolveBorderMetrics(component.layoutMetrics);
  Rect frame = component.layoutMetrics.frame;
  struct RSkSkTextLayout textLayout;
  TextAttributes textAttributes = textInputProps.getEffectiveTextAttributes(FONTSIZE_MULTIPLIER);
  textLayout.builder = std::static_pointer_cast<skia::textlayout::ParagraphBuilder>(
                          std::make_shared<skia::textlayout::ParagraphBuilderImpl>(
                          textLayout.paraStyle, data.layoutManager->collection_));


  if (0 == displayString_.size()) {
    textAttributes.foregroundColor = placeholderColor_;
    data.layoutManager->buildText(textLayout, textInputProps.backgroundColor, textInputProps.paragraphAttributes, textAttributes, placeholderString_, true);
  } else {
    if (secureTextEntry_) {
      std::string secureTextString(displayString_);
      data.layoutManager->buildText(textLayout, textInputProps.backgroundColor, textInputProps.paragraphAttributes, textAttributes, secureTextString.replace( secureTextString.begin(), secureTextString.end(), secureTextString.size(), '*'), true);
    } else {
      data.layoutManager->buildText(textLayout, textInputProps.backgroundColor, textInputProps.paragraphAttributes, textAttributes, displayString_, true);
    }
  }

  drawShadow(canvas, frame, borderMetrics, textInputProps.backgroundColor, layer()->shadowOpacity, layer()->shadowFilter);
  drawTextInput(canvas, component.layoutMetrics, textInputProps, textLayout);
  drawBorder(canvas, frame, borderMetrics, textInputProps.backgroundColor);
}

/*
* @brief       Handling the key Event in TextInput.
* @param       eventKeyType, is Key type
* @param[out]  StopPropogation set false if evnt should be progated furtuer (bubbling)
*              else true if you want to stop propogation of event
* @return      True if key is handled else false
*/

void RSkComponentTextInput::onHandleKey(rnsKey eventKeyType, bool *stopPropagation) {
  *stopPropagation = false;
  if (!editable_) {
    return;
  }
  bool waitForupdateProps = false; 
  privateVarProtectorMutex.lock();
  std::string textString = displayString_;
  privateVarProtectorMutex.unlock();
  KeyPressMetrics keyPressMetrics;
  TextInputMetrics textInputMetrics;
  auto component = getComponentData();
  Rect frame = component.layoutMetrics.frame;
  auto textInputEventEmitter = std::static_pointer_cast<TextInputEventEmitter const>(component.eventEmitter);
  auto const &textInputProps = *std::static_pointer_cast<TextInputProps const>(component.props);
  textInputMetrics.contentOffset.x = frame.origin.x;
  textInputMetrics.contentOffset.y = frame.origin.y;
  if (isInEditingMode_ == false && eventKeyType == RNS_KEY_Select ) {
    RNS_LOG_DEBUG("onfocus need to here"<<textInputMetrics.text);
    textInputMetrics.contentSize.width = paragraph_->getMaxIntrinsicWidth();
    textInputMetrics.contentSize.height = paragraph_->getHeight();
    textInputEventEmitter->onFocus(textInputMetrics);
    isInEditingMode_ = true;
    if (!caretHidden_) {
      drawAndSubmit();
    }
  } else if (isInEditingMode_) {
    // Logic to update the textinput string.
    // Requirement: Textinput is in Editing mode.
    // 1. Alphanumeric keys
    // case 1: appending at end
    // case 2: insert in mid
    // Navigation Keys.
    // 2. update relative location of cursor based
    // on LEFT/RIGHT navigation keys.
    // 3. Deleting character.
    // case 1: Delete last update character by default.
    // case 2: Base on absolute location of cursor.
    // Always delete the immediate left character from
    // the cursor position.
      
    if (textInputProps.value.has_value()) {
      if (!isTextInputInFocus_) {
        std::thread t(&RSkComponentTextInput::keyEventProcessingThread,this);
        t.detach();
        isTextInputInFocus_ = true;
      }
      
      //We need to Handle the StopPrpagation & select in the onHandle.
      if ((eventKeyType >= RNS_KEY_Up && eventKeyType <= RNS_KEY_Back)) {
        *stopPropagation = true;
        inputQueue.push(eventKeyType);
      } else if (eventKeyType ==  RNS_KEY_Select) {
        *stopPropagation = true;
        isTextInputInFocus_ = false;
        isInEditingMode_ = false;
        if (!caretHidden_) {
          drawAndSubmit();
        }
        privateVarProtectorMutex.lock();
        while (!inputQueue.empty()) {
          inputQueue.pop();
        } 
        privateVarProtectorMutex.unlock();
        eventCount_++;
        textInputMetrics.text = textString;
        textInputMetrics.eventCount = eventCount_;
        textInputEventEmitter->onSubmitEditing(textInputMetrics);
        textInputEventEmitter->onEndEditing(textInputMetrics);
        textInputEventEmitter->onBlur(textInputMetrics);
      } 
      return;
    }
    processEventKey(eventKeyType,stopPropagation,&waitForupdateProps,true);
  }//else if (isInEditingMode_)
}

void RSkComponentTextInput::processEventKey (rnsKey eventKeyType,bool* stopPropagation,bool *waitForupdateProps, bool updateString) {
  KeyPressMetrics keyPressMetrics;
  TextInputMetrics textInputMetrics;
  std::string textString = displayString_;
  int textLengthBeforeEdit  = textString.length();
  auto component = getComponentData();
  Rect frame = component.layoutMetrics.frame;
  auto textInputEventEmitter = std::static_pointer_cast<TextInputEventEmitter const>(component.eventEmitter);
  auto const &textInputProps = *std::static_pointer_cast<TextInputProps const>(component.props);
  keyPressMetrics.text = RNSKeyMap[eventKeyType];
    //Displayable Charector Range
    if ((eventKeyType >= RNS_KEY_1 && eventKeyType <= RNS_KEY_Less)) {
      if (cursor_.locationFromEnd != 0){
        textString.insert(cursor_.end-cursor_.locationFromEnd,keyPressMetrics.text);
      } else {
        textString = textString+keyPressMetrics.text;
      }
    } else {
      switch(eventKeyType){
        case RNS_KEY_Left:
          if(cursor_.locationFromEnd < cursor_.end ){
            RNS_LOG_DEBUG("Right key pressed cursor_.locationFromEnd = "<<cursor_.locationFromEnd);
            cursor_.locationFromEnd++; // locationFromEnd
          }
          *stopPropagation = true;
          keyPressMetrics.eventCount = eventCount_;
          if (!caretHidden_) {
            drawAndSubmit();
          }
          textInputEventEmitter->onKeyPress(keyPressMetrics);
          *waitForupdateProps = false;
          return;
        case RNS_KEY_Right:
          if (cursor_.locationFromEnd>0){
            RNS_LOG_DEBUG("Right key pressed cursor_.locationFromEnd = "<<cursor_.locationFromEnd);
            cursor_.locationFromEnd--;
          }
          *stopPropagation = true;
          keyPressMetrics.eventCount = eventCount_;
          if (!caretHidden_) {
            drawAndSubmit();
          }
          textInputEventEmitter->onKeyPress(keyPressMetrics);
          *waitForupdateProps = false;
          return;
        case RNS_KEY_Up:
        case RNS_KEY_Down:
          *stopPropagation = true;
          *waitForupdateProps = false;
          return;
        case RNS_KEY_Back:
        case RNS_KEY_Delete:
          if (!textString.empty() && (cursor_.end!=cursor_.locationFromEnd))
            textString.erase(textString.begin()+(cursor_.end-cursor_.locationFromEnd-1)); //acts like a backspace.
          else
            *waitForupdateProps = false;
          RNS_LOG_DEBUG("After removing a charector in string = "<<textString);
          break;
        case RNS_KEY_Select:
          eventCount_++;
          textInputMetrics.text = textString;
          textInputMetrics.eventCount = eventCount_;
          textInputEventEmitter->onSubmitEditing(textInputMetrics);
          textInputEventEmitter->onEndEditing(textInputMetrics);
          textInputEventEmitter->onBlur(textInputMetrics);
          isInEditingMode_ = false;
          *stopPropagation = true;
          if (!caretHidden_) {
            drawAndSubmit();
          }
          return;
        default:
          *waitForupdateProps = false;
          return;//noop
      }
    }
  //currently selection is not supported selectionRange length is
  //is always 0 & selectionRange.location always end
  textInputMetrics.selectionRange.location = cursor_.end ;
  textInputMetrics.selectionRange.length = 0;
  int textLengthAfterEdit = textString.length();
  if (updateString) {
    if (displayString_ != textString) {
      if ((maxLength_) && (textLengthAfterEdit > maxLength_) && (textLengthAfterEdit > textLengthBeforeEdit )) {
        std::copy_n(textString.begin(), textLengthBeforeEdit , displayString_.begin());
      } else {
        displayString_ = textString;
      }
    }
    cursor_.end = displayString_.length();
    drawAndSubmit();
  }
  eventCount_++;
  *stopPropagation = true;
  RNS_LOG_DEBUG("TextInput text " << textString);
  textInputMetrics.contentSize.width = paragraph_->getMaxIntrinsicWidth();
  textInputMetrics.contentSize.height = paragraph_->getHeight();
  textInputMetrics.text = textString;
  textInputMetrics.eventCount = eventCount_;
  textInputEventEmitter->onKeyPress(keyPressMetrics);
  if (!(maxLength_ && (textLengthAfterEdit > maxLength_))) {
    textInputEventEmitter->onChange(textInputMetrics);
    textInputEventEmitter->onContentSizeChange(textInputMetrics);
    textInputEventEmitter->onSelectionChange(textInputMetrics);
  }
}

void RSkComponentTextInput::keyEventProcessingThread(){
  std::string textString = {};
  bool stopPropagation = false;
  RNS_LOG_DEBUG("Creating the thread worker thread");
  KeyPressMetrics keyPressMetrics;
  TextInputMetrics textInputMetrics;
  auto component = getComponentData();
  bool waitForupdateProps = true;
  auto textInputEventEmitter = std::static_pointer_cast<TextInputEventEmitter const>(component.eventEmitter);
  while(isTextInputInFocus_) {
    if(!inputQueue.empty()) {
      privateVarProtectorMutex.lock();
      textString = displayString_;
      auto eventKeyType = inputQueue.front();
      inputQueue.pop();
      privateVarProtectorMutex.unlock();
      processEventKey(eventKeyType,&stopPropagation,&waitForupdateProps, false);
      if (waitForupdateProps)
        sem_wait(&jsUpdateMutex);
    }
    else{
      RNS_LOG_DEBUG("[KeyEventProcessingThread] ThreadAlive ");
      usleep(5000); 
    }
  }
}

RnsShell::LayerInvalidateMask  RSkComponentTextInput::updateComponentProps(const ShadowView &newShadowView,bool forceUpadate){
  auto const &textInputProps = *std::static_pointer_cast<TextInputProps const>(newShadowView.props);
  int mask = RnsShell::LayerInvalidateNone;
  std::string textString{};
  RNS_LOG_DEBUG("event count "<<textInputProps.mostRecentEventCount);
  textString = textInputProps.text;
  caretHidden_ = textInputProps.caretHidden;
  maxLength_ = textInputProps.maxLength;
  if (textString != displayString_) {
    privateVarProtectorMutex.lock();
    displayString_ = textString;
    cursor_.end = textString.length();
    privateVarProtectorMutex.unlock();
    if (isTextInputInFocus_)
      sem_post(&jsUpdateMutex);
    mask |= LayerPaintInvalidate;
  }
  if ((textInputProps.placeholder.size()) 
      && ((placeholderString_) != (textInputProps.placeholder)) 
      &&(!textInputProps.value.has_value())) {

    placeholderString_ = textInputProps.placeholder.c_str();
    if(!displayString_.size()) {
      mask |= LayerPaintInvalidate;
    }
  }

  if (secureTextEntry_ != textInputProps.secureTextEntry) {
    secureTextEntry_ = textInputProps.secureTextEntry;
    mask |= LayerPaintInvalidate;
  }

  if(textInputProps.placeholderTextColor != placeholderColor_ ) {
    placeholderColor_ = textInputProps.placeholderTextColor;
    if(!displayString_.size()) {
      mask |= LayerPaintInvalidate;
    }
  }

  if (textInputProps.selectionColor != selectionColor_) {
    selectionColor_ = textInputProps.selectionColor;
    cursorPaint_.setColor(RSkColorFromSharedColor(selectionColor_, SK_ColorBLUE));
    mask |= LayerPaintInvalidate;
  }

  if (textInputProps.editable != editable_) {
    editable_ = textInputProps.editable;
    mask |= LayerPaintInvalidate;
  }

  if (false == caretHidden_ && true == editable_){
    caretHidden_ = false;
  } else {
    caretHidden_ = true;
  }

  return (RnsShell::LayerInvalidateMask)mask;
}

void RSkComponentTextInput::handleCommand(std::string commandName,folly::dynamic args){
  if (commandName == "setTextAndSelection") {
    RNS_LOG_DEBUG("Calling Dyanic args"<<args[1].getString());
    privateVarProtectorMutex.lock();
    displayString_ = args[1].getString();
    cursor_.end = displayString_.length();
    privateVarProtectorMutex.unlock();
    drawAndSubmit();
    if(isTextInputInFocus_)
      sem_post(&jsUpdateMutex);
  }else if (commandName == "focus") {
    RNS_LOG_DEBUG("commandName === "<< commandName);
    requestForEditingMode();
  }else if (commandName == "blur") {
    RNS_LOG_DEBUG("commandName === "<< commandName);
    resignFromEditingMode();
  } else {
    RNS_LOG_NOT_IMPL;
  }
}

void RSkComponentTextInput::requestForEditingMode(){
  RNS_LOG_DEBUG("[requestForEditingMode] ENTRY");
  // check if textinput is already in Editing
  if ( isInEditingMode_ )
    return;
  RNS_LOG_TODO("[requestForEditingMode] Launch OnScreen Keyboard");
  auto spatialNavigator =  SpatialNavigator::RSkSpatialNavigator::sharedSpatialNavigator();
  auto previousFocusedCandidate = spatialNavigator->getCurrentFocusElement();
  auto candidateToFocus = getComponentData();
  auto textInputEventEmitter = std::static_pointer_cast<TextInputEventEmitter const>(candidateToFocus.eventEmitter);
  auto const &textInputProps = *std::static_pointer_cast<TextInputProps const>(candidateToFocus.props);
  TextInputMetrics textInputMetrics;
  //TODO
  //check layout event expects absolute frame or relative frame
  Rect frame = candidateToFocus.layoutMetrics.frame;
  textInputMetrics.contentOffset.x = frame.origin.x;
  textInputMetrics.contentOffset.y = frame.origin.y;
  textInputMetrics.contentSize.width = paragraph_->getMaxIntrinsicWidth();
  textInputMetrics.contentSize.height = paragraph_->getHeight();
  // Resign the editing mode
  if(previousFocusedCandidate)
    ((RSkComponentTextInput*)previousFocusedCandidate)->resignFromEditingMode();
  //SpatialNavigtor API which is responsible
  //for changing focus to respective textinput.
  spatialNavigator->updateFocusCandidate(this);
  isInEditingMode_ = true;
  textInputEventEmitter->onFocus(textInputMetrics);
  if (!caretHidden_) {
    drawAndSubmit();
  }
  RNS_LOG_DEBUG("[requestForEditingMode] END");
}

void RSkComponentTextInput::resignFromEditingMode() {
  RNS_LOG_DEBUG("[requestFromEditingMode] ENTER");
  if (!this->isInEditingMode_)
    return;
  RNS_LOG_TODO("[requestForEditingMode] Exit OnScreen Keyboard");
  std::string componentName = "TextInput";
  TextInputMetrics textInputMetrics;
  auto component = this->getComponentData();
  if ( component.componentName == componentName ) {
    textInputMetrics.text = this->displayString_;
    textInputMetrics.eventCount = this->eventCount_;
    this->isInEditingMode_ = false;
    auto textInputEventEmitter = std::static_pointer_cast<TextInputEventEmitter const>(component.eventEmitter);
    textInputEventEmitter->onSubmitEditing(textInputMetrics);
    textInputEventEmitter->onEndEditing(textInputMetrics);
    textInputEventEmitter->onBlur(textInputMetrics);
    if (!caretHidden_) {
      drawAndSubmit();
    }
  }
  RNS_LOG_DEBUG("[requestForEditingMode] END");
}

} // namespace react
} // namespace facebook
