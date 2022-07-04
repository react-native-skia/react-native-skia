/*
 * Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

namespace facebook {
namespace react {

using namespace rns::sdk;

inline OSKTypes RSkToSdkOSKeyboardType(KeyboardType keyBoardType) {
  switch(keyBoardType) {
    case KeyboardType::Numeric:
      return OSKTypes::OSK_NUMERIC_KB;
    default:
      return OSKTypes::OSK_ALPHA_NUMERIC_KB;
  }
}

inline OSKThemes RSkToSdkOSKeyboardTheme(KeyboardAppearance keyBoardTheme) {
  switch(keyBoardTheme) {
    case KeyboardAppearance::Light:
      return OSKThemes::OSK_LIGHT_THEME;
    default:
      return OSKThemes::OSK_DARK_THEME;
  }
}

inline OSKReturnKeyType RSkToSdkOSKReturnKeyType(ReturnKeyType returnKeyType) {
  switch(returnKeyType) {
    case ReturnKeyType::Search:
      return OSKReturnKeyType::OSK_RETURN_KEY_SEARCH;
    default:
      return OSKReturnKeyType::OSK_RETURN_KEY_DEFAULT;
  }
}

} // namespace react
} // namespace facebook
