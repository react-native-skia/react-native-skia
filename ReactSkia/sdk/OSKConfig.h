 /*
 * Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

namespace rns {
namespace sdk {

// Dark Theme configuration
#define OSK_DARK_THEME_PLACEHOLDER_COLOR      SK_ColorDKGRAY
#define OSK_DARK_THEME_INACTIVE_FONT_COLOR    SkColorSetARGB(0xFF, 0x60, 0x60, 0x60) //Mild Lighter Tone of Dark Gray
// LIGHT Theme configuration
#define OSK_LIGHT_THEME_PLACEHOLDER_COLOR     SkColorSetARGB(0xFF, 0x96, 0x96, 0x96) //Mild Lighter Tone on SK_ColorGRAY
#define OSK_LIGHT_THEME_INACTIVE_FONT_COLOR   SkColorSetARGB(0xFF, 0xB3, 0xB3, 0xB3)
// OSK common styling configuration
#define OSK_FONT_SIZE                         24
#define OSK_HIGHLIGHT_FONT_SIZE               32
#define OSK_HIGHLIGHT_BACKGROUND_COLOR        SK_ColorWHITE
#define OSK_HIGHLIGHT_FONT_COLOR              SK_ColorBLACK
#define OSK_PLACEHOLDER_CURSOR_COLOR          SK_ColorBLUE
//OSK Screen & component placement confiuration
#define OSK_WINDOW_WIDTH                      1 /*100% of Screen Size*/
#define OSK_WINDOW_HEIGTH                     1

#define OSK_PLACEHOLDER_NAME_VERTICAL_OFFSET  0.18
#define OSK_PLACEHOLDER_VERTICAL_OFFSET       0.2
#define OSK_PLACEHOLDER_LENGTH                0.7 /*Total display view will be x% of the screen size*/
#define OSK_PLACEHOLDER_HEIGHT_SCALE_FACTOR   2 /* X*Font Size, will be the PlaceHolder Height */
#define OSK_KB_HORIZONTAL_OFFSET              0.1
#define OSK_KB_VERTICAL_OFFSET                0.3
// Place Holder configuration
#define OSK_PLACEHOLDER_CURSOR_WIDTH          3 /* Cursor line thickness*/
#define OSK_PLACEHOLDER_LEFT_INSET            2 /* offset from PlaceHolder Start*/
#define OSK_PLACEHOLDER_RIGHT_INSET           3 /* Safe offset to avoid overflow*/
#define OSK_PLACEHOLDER_RESERVED_LENGTH (OSK_PLACEHOLDER_CURSOR_WIDTH+OSK_PLACEHOLDER_LEFT_INSET+OSK_PLACEHOLDER_RIGHT_INSET)

//Fail Safe Configurations
#define DRAW_FONT_FAILURE_INDICATOR  "??"

} // namespace sdk
} // namespace rns


