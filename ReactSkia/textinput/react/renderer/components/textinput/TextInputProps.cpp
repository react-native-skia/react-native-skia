/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 * Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "TextInputProps.h"

#include <react/renderer/attributedstring/conversions.h>
#include <react/renderer/components/textinput/propsConversions.h>
#include <react/renderer/core/propsConversions.h>
#include <react/renderer/graphics/conversions.h>

namespace facebook {
namespace react {

TextInputProps::TextInputProps(
    TextInputProps const &sourceProps,
    RawProps const &rawProps)
    : ViewProps(sourceProps, rawProps),
      BaseTextProps(sourceProps, rawProps),
      traits(convertRawProp(rawProps, sourceProps.traits, {})),
      paragraphAttributes(
          convertRawProp(rawProps, sourceProps.paragraphAttributes, {})),
      defaultValue(convertRawProp(
          rawProps,
          "defaultValue",
          sourceProps.defaultValue,
          {})),
      value(convertRawProp(rawProps, "value", sourceProps.value, {})),
      placeholder(
          convertRawProp(rawProps, "placeholder", sourceProps.placeholder, {})),
      placeholderTextColor(convertRawProp(
          rawProps,
          "placeholderTextColor",
          sourceProps.placeholderTextColor,
          {})),
      editable(convertRawProp(rawProps, "editable", sourceProps.editable, {})),
      caretHidden(convertRawProp(rawProps, "caretHidden", sourceProps.caretHidden, {})),
      secureTextEntry(convertRawProp(rawProps, "secureTextEntry", sourceProps.secureTextEntry, {})),
      maxLength(
          convertRawProp(rawProps, "maxLength", sourceProps.maxLength, {})),
      cursorColor(
          convertRawProp(rawProps, "cursorColor", sourceProps.cursorColor, {})),
      selectionColor(convertRawProp(
          rawProps,
          "selectionColor",
          sourceProps.selectionColor,
          {})),
      underlineColorAndroid(convertRawProp(
          rawProps,
          "underlineColorAndroid",
          sourceProps.underlineColorAndroid,
          {})),
      text(convertRawProp(rawProps, "text", sourceProps.text, {})),
      mostRecentEventCount(convertRawProp(
          rawProps,
          "mostRecentEventCount",
          sourceProps.mostRecentEventCount,
          {})),
      autoFocus(
          convertRawProp(rawProps, "autoFocus", sourceProps.autoFocus, {})),
      clearTextOnFocus(
          convertRawProp(rawProps, "clearTextOnFocus", sourceProps.clearTextOnFocus, {})),
      inputAccessoryViewID(convertRawProp(
          rawProps,
          "inputAccessoryViewID",
          sourceProps.inputAccessoryViewID,
          {})){};

TextAttributes TextInputProps::getEffectiveTextAttributes(
    Float fontSizeMultiplier) const {
  auto result = TextAttributes::defaultTextAttributes();
  result.fontSizeMultiplier = fontSizeMultiplier;
  result.apply(textAttributes);

  /*
   * These props are applied to `View`, therefore they must not be a part of
   * base text attributes.
   */
  result.backgroundColor = clearColor();
  result.opacity = 1;

  return result;
}

ParagraphAttributes TextInputProps::getEffectiveParagraphAttributes() const {
  auto result = paragraphAttributes;

  if (!traits.multiline) {
    result.maximumNumberOfLines = 1;
  }

  return result;
}

} // namespace react
} // namespace facebook
