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
#define SKIA_SDK_DARK_THEME_BACKGROUND_COLOR       SkColorSetARGB(0xFF, 0x34, 0x34, 0x34) //Dark Gray
#define SKIA_SDK_DARK_THEME_FONT_COLOR             SK_ColorWHITE
// LIGHT Theme configuration
#define SKIA_SDK_LIGHT_THEME_BACKGROUND_COLOR      SkColorSetARGB(0xFF, 0x80, 0x80, 0x80) // Mild Dark Tone on SK_ColorGRAY
#define SKIA_SDK_LIGHT_THEME_FONT_COLOR            SK_ColorWHITE
//Default Text Configurations for "SCREEN RESOLUTION:720p"
#define SKIA_SDK_DEFAULT_SCREEN_RESOLUTION         "720p"
#define SKIA_SDK_DEFAULT_SCREEN_WIDTH              1280
#define SKIA_SDK_DEFAULT_SCREEN_HEIGHT             720
#define SKIA_SDK_TEXT_FONT_SIZE                    24
#define SKIA_SDK_LINE_SPACING                      30

} // sdk
} // rns
