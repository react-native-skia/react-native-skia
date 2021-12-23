/*
 * Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkPaint.h"
#include "ReactSkia/components/RSkComponentTextInput.h"
#include "ReactSkia/components/RSkComponent.h"
#include "ReactSkia/sdk/RNSKeyCodeMapping.h"
#include "ReactSkia/views/common/RSkDrawUtils.h"
#include "ReactSkia/views/common/RSkConversion.h"
#include "rns_shell/compositor/layers/PictureLayer.h"

#include <string.h>
#include <iostream>

namespace facebook {
namespace react {

using namespace RSkDrawUtils;
using namespace skia::textlayout;

#define NUMBER_OF_LINES         1
#define FONTSIZE_MULTIPLIER     10

RSkComponentTextInput::RSkComponentTextInput(const ShadowView &shadowView)
    : RSkComponent(shadowView)
    ,isInEditingMode_(false)
    ,eventCount_(0)
    ,cursor_({0,0}){
  RNS_LOG_DEBUG("called constructor");
  paragraph_ = nullptr;
}

void RSkComponentTextInput::drawTextInput(SkCanvas *canvas,
  LayoutMetrics layout,
  std::shared_ptr<ParagraphBuilder> &builder,
  const TextInputProps& props) {
  Rect frame = layout.frame;
  ParagraphStyle paraStyle;
  float yOffset;

  // setParagraphStyle
  paraStyle.setMaxLines(NUMBER_OF_LINES);
  paraStyle.setEllipsis(u"\u2026");
  builder->setParagraphStyle(paraStyle);

  // buildParagraph
  paragraph_ = builder->Build();
  paragraph_->layout(layout.getContentFrame().size.width);

  // clipRect and backgroundColor
  canvas->clipRect(SkRect::MakeXYWH(frame.origin.x, frame.origin.y, frame.size.width, frame.size.height));
  canvas->drawColor(RSkColorFromSharedColor(props.backgroundColor, SK_ColorTRANSPARENT));

  // paintParagraph
  yOffset = (layout.getContentFrame().size.height - paragraph_->getHeight()) / 2;
  paragraph_->paint(canvas, frame.origin.x + layout.contentInsets.left, frame.origin.y + layout.contentInsets.top + yOffset);
}

void RSkComponentTextInput::OnPaint(SkCanvas *canvas) {
  auto component = getComponentData();
  auto const &textInputProps = *std::static_pointer_cast<TextInputProps const>(component.props);
  auto state = std::static_pointer_cast<TextInputShadowNode::ConcreteStateT const>(component.state);
  auto data = state->getData();
  auto borderMetrics = textInputProps.resolveBorderMetrics(component.layoutMetrics);
  Rect frame = component.layoutMetrics.frame;
  ParagraphStyle paraStyle;
  TextShadow shadow;
  TextAttributes textAttributes = textInputProps.getEffectiveTextAttributes(FONTSIZE_MULTIPLIER);
  auto paraBuilder = std::static_pointer_cast<skia::textlayout::ParagraphBuilder>(
                          std::make_shared<skia::textlayout::ParagraphBuilderImpl>(
                          paraStyle, data.layoutManager->collection_));

  if (0 == displayString_.size()) {
    textAttributes.foregroundColor = placeholderColor_;
    data.layoutManager->buildText(textInputProps.paragraphAttributes, textAttributes, placeholderString_, shadow, true, paraBuilder);
  } else {
    data.layoutManager->buildText(textInputProps.paragraphAttributes, textAttributes, displayString_, shadow, true, paraBuilder);
  }

  drawShadow(canvas, frame, borderMetrics, textInputProps.backgroundColor, layer()->shadowOpacity, layer()->shadowFilter);
  drawTextInput(canvas, component.layoutMetrics, paraBuilder, textInputProps);
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
  std::string textString = displayString_;
  *stopPropagation = false;
  KeyPressMetrics keyPressMetrics;
  TextInputMetrics textInputMetrics;
  *stopPropagation = false;
  cursor_.end = textString.length();
  auto component = getComponentData();
  Rect frame = component.layoutMetrics.frame;
  auto textInputEventEmitter = std::static_pointer_cast<TextInputEventEmitter const>(component.eventEmitter);
  textInputMetrics.contentOffset.x = frame.origin.x;
  textInputMetrics.contentOffset.y = frame.origin.y;
  if (isInEditingMode_ == false && eventKeyType == RNS_KEY_Select ) {
    RNS_LOG_DEBUG("onfocus need to here"<<textInputMetrics.text);
    textInputMetrics.contentSize.width = paragraph_->getMaxIntrinsicWidth();
    textInputMetrics.contentSize.height = paragraph_->getHeight();
    textInputEventEmitter->onFocus(textInputMetrics);
    isInEditingMode_ = true;
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
    keyPressMetrics.text = RNSKeyMap[eventKeyType];
    if ((eventKeyType >= RNS_KEY_1 && eventKeyType <= RNS_KEY_z)) {
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
          textInputEventEmitter->onKeyPress(keyPressMetrics);
          return;
        case RNS_KEY_Right:
          if (cursor_.locationFromEnd>0){
            RNS_LOG_DEBUG("Right key pressed cursor_.locationFromEnd = "<<cursor_.locationFromEnd);
            cursor_.locationFromEnd--;
          }
          *stopPropagation = true;
          keyPressMetrics.eventCount = eventCount_;
          textInputEventEmitter->onKeyPress(keyPressMetrics);
          return;
        case RNS_KEY_Back:
        case RNS_KEY_Delete:
          if (!textString.empty() && (cursor_.end!=cursor_.locationFromEnd) )
            textString.erase(textString.begin()+(cursor_.end-cursor_.locationFromEnd-1)); //acts like a backspace.
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
          return;
        default:
         ;//noop
      }
    }
    //currently selection is not supported selectionRange length is 
    //is always 0 & selectionRange.location always end 
    textInputMetrics.selectionRange.location = cursor_.end ;
    textInputMetrics.selectionRange.length = 0;
    if (mutableFlag_ == true ){
      /*Update the display string and set the layer invalidate mask*/
      if (displayString_ != textString){
        displayString_ = textString;
        layer()->invalidate( RnsShell::LayerPaintInvalidate);
        if (layer()->type() == RnsShell::LAYER_TYPE_PICTURE) {
          RNS_PROFILE_API_OFF(component_.componentName << " getPicture :", static_cast<RnsShell::PictureLayer*>(layer().get())->setPicture(getPicture()));
        }
        layer()->client().notifyFlushRequired();
      }
    }
    eventCount_++;
    *stopPropagation = true;
    RNS_LOG_DEBUG("TextInput text " << textString);
    textInputMetrics.contentSize.width = paragraph_->getMaxIntrinsicWidth();
    textInputMetrics.contentSize.height = paragraph_->getHeight();
    textInputMetrics.text = textString;
    textInputMetrics.eventCount = eventCount_;
    textInputEventEmitter->onKeyPress(keyPressMetrics);
    textInputEventEmitter->onChange(textInputMetrics);
    textInputEventEmitter->onContentSizeChange(textInputMetrics);
    textInputEventEmitter->onSelectionChange(textInputMetrics);
  }//else if (isInEditingMode_)
}

RnsShell::LayerInvalidateMask  RSkComponentTextInput::updateComponentProps(const ShadowView &newShadowView,bool forceUpadate){
  auto const &textInputProps = *std::static_pointer_cast<TextInputProps const>(newShadowView.props);
  int mask = RnsShell::LayerInvalidateNone;
  std::string textString{};
  RNS_LOG_DEBUG("event count "<<textInputProps.mostRecentEventCount);
  // Fixme mutableFlag must be update in mutation command.
  if (textInputProps.value.size() == 0){
    RNS_LOG_DEBUG("Mutable flag is set.");
    mutableFlag_ = true;
  }
  if (textInputProps.value.size()) {
    textString = textInputProps.value.c_str();
  } else if (textInputProps.defaultValue.size()) {
    textString = textInputProps.defaultValue.c_str();
  }

  if(textString != displayString_) {
    displayString_ = textString;
    mask |= LayerPaintInvalidate;
  }

  if ((textInputProps.placeholder.size()) && ((placeholderString_) != (textInputProps.placeholder))) {
    placeholderString_ = textInputProps.placeholder.c_str();
    if(!displayString_.size()) {
      mask |= LayerPaintInvalidate;
    }
  }

  if(textInputProps.placeholderTextColor != placeholderColor_ ) {
    placeholderColor_ = textInputProps.placeholderTextColor;
    if(!displayString_.size()) {
      mask |= LayerPaintInvalidate;
    }
  }

  return (RnsShell::LayerInvalidateMask)mask;
}

} // namespace react
} // namespace facebook
