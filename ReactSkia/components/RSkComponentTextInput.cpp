/*
 * Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkPaint.h"
#include "ReactSkia/components/RSkComponentTextInput.h"
#include "ReactSkia/views/common/RSkDrawUtils.h"
#include "ReactSkia/sdk/RNSKeyCodeMapping.h"
#include "react/renderer/components/textinput/TextInputShadowNode.h"

#include <string.h>

namespace facebook {
namespace react {

using namespace RSkDrawUtils;

RSkComponentTextInput::RSkComponentTextInput(const ShadowView &shadowView)
    : RSkComponent(shadowView)
    ,isInEditingMode_(false){RNS_LOG_INFO("called constructor");}

void RSkComponentTextInput::OnPaint(SkCanvas *canvas) {
  auto component = getComponentData();
  auto const &textInputProps = *std::static_pointer_cast<TextInputProps const>(component.props);
  /*Retrieve Shadow Props*/
  RNS_LOG_TODO("shadow color,offset,opacity,Radius should be taken from layer and convet into the Skia formate and update here");
  /* TODO shadow color,offset,opacity,Radius should be taken from layer and convet into the 
  * Skia formate and update here.
  ShadowMetrics shadowMetrics{};
  shadowMetrics.shadowColor=layer()->shadowColor;
  shadowMetrics.shadowOffset=layer()->shadowOffset;
  shadowMetrics.shadowOpacity=layer()->shadowOpacity;
  shadowMetrics.shadowRadius=layer()->shadowRadius;
  */
  /* apply view style props */
  auto borderMetrics= textInputProps.resolveBorderMetrics(component.layoutMetrics);
  Rect frame =  component.layoutMetrics.frame;
  /*Draw Order : 1. Shadow 2. BackGround 3 Border*/
  RNS_LOG_TODO("drawShadow & drawBackground");
  //drawShadow(canvas,frame,borderMetrics,shadowMetrics);
  //drawBackground(canvas,frame,borderMetrics,textInputProps.backgroundColor,textInputProps.opacity);
  drawBorder(canvas,frame,borderMetrics,textInputProps.backgroundColor,textInputProps.opacity);
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
void RSkComponentTextInput::updateComponentProps(const ShadowView &newShadowView,bool forceUpadate){
  // TODO
  RNS_LOG_TODO("TODO");
}

} // namespace react
} // namespace facebook
