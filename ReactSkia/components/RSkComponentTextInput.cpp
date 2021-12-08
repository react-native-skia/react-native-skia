/*
 * Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkPaint.h"
#include "modules/skparagraph/include/TextStyle.h"
#include "react/renderer/components/textinput/TextInputShadowNode.h"
#include "ReactSkia/components/RSkComponentTextInput.h"
#include "ReactSkia/components/RSkComponent.h"
#include "ReactSkia/sdk/RNSKeyCodeMapping.h"
#include "ReactSkia/textlayoutmanager/RSkTextLayoutManager.h"
#include "ReactSkia/views/common/RSkDrawUtils.h"
#include "ReactSkia/views/common/RSkConversion.h"
#include <string.h>

namespace facebook {
namespace react {

using namespace RSkDrawUtils;
using namespace skia::textlayout;

#define NUMBER_OF_LINES         1
#define FONTSIZE_MULTIPLIER     10

RSkComponentTextInput::RSkComponentTextInput(const ShadowView &shadowView)
    : RSkComponent(shadowView)
    ,isInEditingMode_(false){}

void drawTextInput(SkCanvas *canvas,
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
  std::shared_ptr<Paragraph> paragraph = builder->Build();
  paragraph->layout(layout.getContentFrame().size.width);

  // clipRect and backgroundColor
  canvas->clipRect(SkRect::MakeXYWH(frame.origin.x, frame.origin.y, frame.size.width, frame.size.height));
  canvas->drawColor(RSkColorFromSharedColor(props.backgroundColor, SK_ColorTRANSPARENT));

  // paintParagraph
  yOffset = (layout.getContentFrame().size.height - paragraph->getHeight()) / 2;
  paragraph->paint(canvas, frame.origin.x + layout.contentInsets.left, frame.origin.y + layout.contentInsets.top + yOffset);
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
  drawBorder(canvas, frame, borderMetrics, textInputProps.backgroundColor, textInputProps.opacity);
}

/*
* @brief       Handling the key Event in TextInput.
* @param       eventKeyType, is Key type
* @param[out]  StopPropogation set false if evnt should be progated furtuer (bubbling)
*              else true if you want to stop propogation of event
* @return      True if key is handled else false
*/

void RSkComponentTextInput::onHandleKey(rnsKey  eventKeyType, bool* stopPropagation){
  *stopPropagation=false;
  RNS_LOG_TODO("update Keymatrics Event Count.");
  //TODO  update Keymatrics Event Count.
  KeyPressMetrics keyPressMetrics;
  TextInputMetrics textInputMetrics;
  auto component = getComponentData();
  auto textInputEventEmitter = std::static_pointer_cast<TextInputEventEmitter const>(component.eventEmitter);
  if (eventKeyType == RNS_KEY_Select){
    if(isInEditingMode_ == false){
      RNS_LOG_TODO("onfocus need to here");
      /*TODO Check onfocus need to here.
      *textInputEventEmitter->onfocus();
      */
      isInEditingMode_=true;
    }else{
      /* TODO Update textinputmatrix data and sent the event.
      textInputEventEmitter->onSubmitEditing(textInputMetrics_);
      textInputEventEmitter->onBlur(textInputMetrics_);
      textInputEventEmitter->onEndEditing(textInputMetrics_);
      */
      isInEditingMode_=false;
      *stopPropagation = true;
    }
  }else{
    if(isInEditingMode_){
      if ((eventKeyType >= RNS_KEY_1 && eventKeyType <= RNS_KEY_z)){
        keyPressMetrics.text=RNSKeyMap[eventKeyType]; 
        textInputEventEmitter->onKeyPress(keyPressMetrics);
        RNS_LOG_TODO("send onchange and onchangetext here");
        /*TODO
        *send onchange and onchangetext here.
        */
        *stopPropagation = true;
      }
    }
  }
}
RnsShell::LayerInvalidateMask  RSkComponentTextInput::updateComponentProps(const ShadowView &newShadowView,bool forceUpadate){
  auto const &textInputProps = *std::static_pointer_cast<TextInputProps const>(newShadowView.props);
  int mask = RnsShell::LayerInvalidateNone;
  std::string textString{};

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
