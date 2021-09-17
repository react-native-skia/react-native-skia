/*
 * Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkPaint.h"

#include "ReactSkia/components/RSkComponentTextInput.h"
#include "react/renderer/components/textinput/iostextinput/TextInputShadowNode.h"
#include "ReactSkia/views/common/RSkDrawUtils.h"

namespace facebook {
namespace react {

using namespace RSkDrawUtils;

RSkComponentTextInput::RSkComponentTextInput(const ShadowView &shadowView)
    : RSkComponentView(shadowView) {}

void RSkComponentTextInput::OnPaint(SkCanvas *canvas) {

  auto component = getComponentData();
  auto const &textInputProps = *std::static_pointer_cast<TextInputProps const>(component.props);

  /*Retrieve Shadow Props*/
  ShadowMetrics shadowMetrics{};
  shadowMetrics.shadowColor=textInputProps.shadowColor;
  shadowMetrics.shadowOffset=textInputProps.shadowOffset;
  shadowMetrics.shadowOpacity=textInputProps.shadowOpacity;
  shadowMetrics.shadowRadius=textInputProps.shadowRadius;

  /* apply view style props */
  auto borderMetrics= textInputProps.resolveBorderMetrics(component.layoutMetrics);
  Rect frame = getAbsoluteFrame();

  /*Draw Order : 1. Shadow 2. BackGround 3 Border*/
  drawShadow(canvas,frame,borderMetrics,shadowMetrics);
  drawBackground(canvas,frame,borderMetrics,textInputProps.backgroundColor,textInputProps.opacity);
  drawBorder(canvas,frame,borderMetrics,textInputProps.backgroundColor,textInputProps.opacity);
  KeyPressMetrics keyPressMetrics_;
  keyPressMetrics_.text="a";
  keyPressMetrics_.eventCount=1;
  auto textInputEventEmitter = std::static_pointer_cast<TextInputEventEmitter const>(component.eventEmitter);
  textInputEventEmitter->onKeyPress(keyPressMetrics_);
}

} // namespace react
} // namespace facebook