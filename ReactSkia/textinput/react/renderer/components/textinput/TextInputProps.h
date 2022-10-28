/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 * Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <react/renderer/attributedstring/ParagraphAttributes.h>
#include <react/renderer/attributedstring/TextAttributes.h>
#include <react/renderer/components/textinput/conversions.h>
#include <react/renderer/components/textinput/primitives.h>
#include <react/renderer/components/text/BaseTextProps.h>
#include <react/renderer/components/view/ViewProps.h>
#include <react/renderer/core/Props.h>
#include <react/renderer/core/propsConversions.h>
#include <react/renderer/graphics/Color.h>
#include <react/renderer/imagemanager/primitives.h>
#include <vector>

namespace facebook {
namespace react {

class TextInputProps final : public ViewProps, public BaseTextProps {
 public:
  TextInputProps() = default;
  TextInputProps(TextInputProps const &sourceProps, RawProps const &rawProps);

#pragma mark - Props

  TextInputTraits const traits{};
  ParagraphAttributes const paragraphAttributes{};

  better::optional<std::string> const defaultValue{};
  better::optional<std::string> value{};
  std::string const placeholder{};
  SharedColor const placeholderTextColor{};
  int maxLength{};

  /*
   * Tint colors
   */
  SharedColor const cursorColor{};
  SharedColor const selectionColor{};
  // TODO: Rename to `tintColor` and make universal.
  better::optional<SharedColor> underlineColorAndroid{};

  /*
   * "Private" (only used by TextInput.js) props
   */
  std::string const text{};
  int const mostRecentEventCount{0};

  bool autoFocus{false};

  std::string const inputAccessoryViewID{};

  /*
   * Accessors
   */
  TextAttributes getEffectiveTextAttributes(Float fontSizeMultiplier) const;
  ParagraphAttributes getEffectiveParagraphAttributes() const;

#ifdef ANDROID
  folly::dynamic getDynamic() const;
#endif
};

} // namespace react
} // namespace facebook
