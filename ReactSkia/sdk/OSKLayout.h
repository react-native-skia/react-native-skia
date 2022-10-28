 /*
 * Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */
#pragma once

#include "RNSKeyCodeMapping.h"

namespace rns {
namespace sdk {

/*
Position Configuration mentioned in this file should be for 1280*720 screen.
For other Screen Size, it will be scaled accordingly in SW w.r.t 1280*720.
*/
static SkSize baseScreenSize{1280,720};

#define LOWER_TO_UPPER_CASE_OFFSET 32

//Max Partition or Group, OSK Layout can Handle. Each Group Get it function/Purpose w.r.t Layout.
enum partitionID {
  GROUP1,
  GROUP2,
  GROUP3,
  GROUP4,
  MAX_GROUP=GROUP4
};

/* KeyBoard Layout for Alpha Numeric KeyBoard */

#define ALPHA_NUMERIC_KB_PARTITION_COUNT   4
SkPoint alphaNumericKBReturnKeyIndex{13,2};/*KeyIndex,RowIndex*/
SkPoint alphaNumericKBDefaultHLKeyIndex{7,0};/*KeyIndex,RowIndex*/
KBLayoutKeyPosContainer alphaNumericKBKeyPos;
KBLayoutSibblingInfoContainer alphaNumericKBKeySiblingInfo;
KBLayoutKeyInfoContainer alphaNumericKBKeyInfo= {
  //row 1
  {
    {"123ABC",RNS_KEY_UnKnown,KEY_TYPE_TOGGLE,GROUP1},
    {"1",RNS_KEY_1,KEY_TYPE_TEXT,GROUP2},
    {"2",RNS_KEY_2,KEY_TYPE_TEXT,GROUP2},
    {"3",RNS_KEY_3,KEY_TYPE_TEXT,GROUP2},
    {"a",RNS_KEY_a,KEY_TYPE_TEXT,GROUP3},
    {"b",RNS_KEY_b,KEY_TYPE_TEXT,GROUP3},
    {"c",RNS_KEY_c,KEY_TYPE_TEXT,GROUP3},
    {"d",RNS_KEY_d,KEY_TYPE_TEXT,GROUP3},
    {"e",RNS_KEY_e,KEY_TYPE_TEXT,GROUP3},
    {"f",RNS_KEY_f,KEY_TYPE_TEXT,GROUP3},
    {"g",RNS_KEY_g,KEY_TYPE_TEXT,GROUP3},
    {"h",RNS_KEY_h,KEY_TYPE_TEXT,GROUP3},
    {"i",RNS_KEY_i,KEY_TYPE_TEXT,GROUP3},
    {"space",RNS_KEY_Space,KEY_TYPE_FUNCTION,GROUP4}
  },
   //row 2
  {
    {"123abc",RNS_KEY_UnKnown,KEY_TYPE_TOGGLE,GROUP1},
    {"4",RNS_KEY_4,KEY_TYPE_TEXT,GROUP2},
    {"5",RNS_KEY_5,KEY_TYPE_TEXT,GROUP2},
    {"6",RNS_KEY_6,KEY_TYPE_TEXT,GROUP2},
    {"j",RNS_KEY_j,KEY_TYPE_TEXT,GROUP3},
    {"k",RNS_KEY_k,KEY_TYPE_TEXT,GROUP3},
    {"l",RNS_KEY_l,KEY_TYPE_TEXT,GROUP3},
    {"m",RNS_KEY_m,KEY_TYPE_TEXT,GROUP3},
    {"n",RNS_KEY_n,KEY_TYPE_TEXT,GROUP3},
    {"o",RNS_KEY_o,KEY_TYPE_TEXT,GROUP3},
    {"p",RNS_KEY_p,KEY_TYPE_TEXT,GROUP3},
    {"q",RNS_KEY_q,KEY_TYPE_TEXT,GROUP3},
    {"r",RNS_KEY_r,KEY_TYPE_TEXT,GROUP3},
    {"delete",RNS_KEY_Delete,KEY_TYPE_FUNCTION,GROUP4}
  },
  {
  //row 3
    {"@?#",RNS_KEY_UnKnown,KEY_TYPE_TOGGLE,GROUP1},
    {"7",RNS_KEY_7,KEY_TYPE_TEXT,GROUP2},
    {"8",RNS_KEY_8,KEY_TYPE_TEXT,GROUP2},
    {"9",RNS_KEY_9,KEY_TYPE_TEXT,GROUP2},
    {"s",RNS_KEY_s,KEY_TYPE_TEXT,GROUP3},
    {"t",RNS_KEY_t,KEY_TYPE_TEXT,GROUP3},
    {"u",RNS_KEY_u,KEY_TYPE_TEXT,GROUP3},
    {"v",RNS_KEY_v,KEY_TYPE_TEXT,GROUP3},
    {"w",RNS_KEY_w,KEY_TYPE_TEXT,GROUP3},
    {"x",RNS_KEY_x,KEY_TYPE_TEXT,GROUP3},
    {"y",RNS_KEY_y,KEY_TYPE_TEXT,GROUP3},
    {"z",RNS_KEY_z,KEY_TYPE_TEXT,GROUP3},
    {".",RNS_KEY_Period,KEY_TYPE_TEXT,GROUP3},
    {"return",RNS_KEY_Select,KEY_TYPE_FUNCTION,GROUP4}
  },
  {
    //row 4
    {"-",RNS_KEY_Minus,KEY_TYPE_TEXT,GROUP2},
    {"0",RNS_KEY_0,KEY_TYPE_TEXT,GROUP2},
    {"_",RNS_KEY_Underscore,KEY_TYPE_TEXT,GROUP2 },
    {"!",RNS_KEY_Exclam,KEY_TYPE_TEXT,GROUP3},
    {"@",RNS_KEY_At,KEY_TYPE_TEXT,GROUP3 },
    {"#",RNS_KEY_Numbersign,KEY_TYPE_TEXT,GROUP3},
    {"?",RNS_KEY_Question,KEY_TYPE_TEXT,GROUP3},
    {"*",RNS_KEY_Asterisk,KEY_TYPE_TEXT,GROUP3},
    {",",RNS_KEY_Comma,KEY_TYPE_TEXT,GROUP3},
    {";",RNS_KEY_Semicolon,KEY_TYPE_TEXT,GROUP3},
    {"right", RNS_KEY_Right,KEY_TYPE_FUNCTION,GROUP3},
    {"left", RNS_KEY_Left,KEY_TYPE_FUNCTION,GROUP3}
  }
};

keyPlacementConfig_t alphaNumericKBGroupConfig[ALPHA_NUMERIC_KB_PARTITION_COUNT]={
  //GROUP1-Toggle key
  {{20,60},{15,30},1.3,0.75,4},
  //GROUP2-Numeric key
  {{150,40},{15,15},1.25,1,1},
  //GROUP3-Alpha & SYmbol key
  {{320,40},{15,15},1.25,1,1},
  //GROUP4-Function key
  {{820,50},{15,15},1.5,1,1}
};

/* KeyBoard Layout for Symbol Key Board */

KBLayoutSibblingInfoContainer symbolKBKBKeySiblingInfo;
KBLayoutKeyPosContainer symbolKBKBKeyPos;
#define SYMBOL_KB_PARTITION_COUNT 3
SkPoint symbolKBReturnKeyIndex{11,2};/*KeyIndex,RowIndex*/
SkPoint symbolKBDefaultHLKeyIndex{7,0};/*KeyIndex,RowIndex*/
KBLayoutKeyInfoContainer symbolKBKBKeyInfo={
  {
    {"123ABC",RNS_KEY_UnKnown,KEY_TYPE_TOGGLE,GROUP1},
    {"!",RNS_KEY_Exclam,KEY_TYPE_TEXT,GROUP2},
    {"@",RNS_KEY_At,KEY_TYPE_TEXT,GROUP2},
    {"#",RNS_KEY_Numbersign,KEY_TYPE_TEXT,GROUP2},
    {"$",RNS_KEY_Dollar,KEY_TYPE_TEXT,GROUP2},
    {"^",RNS_KEY_Asciicircum,KEY_TYPE_TEXT,GROUP2},
    {"&",RNS_KEY_Ampersand,KEY_TYPE_TEXT,GROUP2},
    {"*",RNS_KEY_Asterisk,KEY_TYPE_TEXT,GROUP2},
    {"~",RNS_KEY_Asciitilde,KEY_TYPE_TEXT,GROUP2},
    {"(",RNS_KEY_ParenLeft,KEY_TYPE_TEXT,GROUP2},
    {")",RNS_KEY_ParenRight,KEY_TYPE_TEXT,GROUP2},
    {",",RNS_KEY_Comma,KEY_TYPE_TEXT,GROUP2},
    {".",RNS_KEY_Period,KEY_TYPE_TEXT,GROUP2},
    {"space",RNS_KEY_Space,KEY_TYPE_FUNCTION,GROUP3}
  },
  {
   //row 2
    {"123abc",RNS_KEY_UnKnown,KEY_TYPE_TOGGLE,GROUP1},
    {";",RNS_KEY_Semicolon,KEY_TYPE_TEXT,GROUP2},
    {":",RNS_KEY_Colon,KEY_TYPE_TEXT,GROUP2},
    {"|",RNS_KEY_Bar,KEY_TYPE_TEXT,GROUP2},
    {"\\",RNS_KEY_BackSlash,KEY_TYPE_TEXT,GROUP2 },
    {"/",RNS_KEY_Shash,KEY_TYPE_TEXT,GROUP2},
    {"{",RNS_KEY_BraceLeft,KEY_TYPE_TEXT,GROUP2},
    {"}",RNS_KEY_BraceRight,KEY_TYPE_TEXT,GROUP2},
    {"[",RNS_KEY_BracketLeft,KEY_TYPE_TEXT,GROUP2},
    {"]",RNS_KEY_BracketRight,KEY_TYPE_TEXT,GROUP2},
    {"`",RNS_KEY_Grave,KEY_TYPE_TEXT,GROUP2},
    {"'",RNS_KEY_Apostrophe,KEY_TYPE_TEXT,GROUP2},
    {"_",RNS_KEY_Underscore,KEY_TYPE_TEXT,GROUP2},
    {"delete",RNS_KEY_Delete,KEY_TYPE_FUNCTION,GROUP3}
  },
  {
  //row 3
    {"@?#",RNS_KEY_UnKnown ,KEY_TYPE_TOGGLE,GROUP1},
    {"%",RNS_KEY_Percent,KEY_TYPE_TEXT,GROUP2},
    {">",RNS_KEY_Greater,KEY_TYPE_TEXT,GROUP2},
    {"<",RNS_KEY_Less,KEY_TYPE_TEXT,GROUP2},
    {"?",RNS_KEY_Question,KEY_TYPE_TEXT,GROUP2},
    {"\"",RNS_KEY_Quotedbl,KEY_TYPE_TEXT,GROUP2},
    {"=",RNS_KEY_Equal,KEY_TYPE_TEXT,GROUP2},
    {"+",RNS_KEY_Plus,KEY_TYPE_TEXT,GROUP2},
    {"-",RNS_KEY_Minus,KEY_TYPE_TEXT,GROUP2},
    {"right", RNS_KEY_Right,KEY_TYPE_FUNCTION,GROUP2},
    {"left", RNS_KEY_Left,KEY_TYPE_FUNCTION,GROUP2},
    {"return",RNS_KEY_Select,KEY_TYPE_FUNCTION,GROUP3},
  }
};

keyPlacementConfig_t symbolKBGroupConfig[SYMBOL_KB_PARTITION_COUNT]={
  //GROUP1-Toggle key
  {{20,60},{15,30},1.3,0.75,4},
  //GROUP2-Symbol key
  {{150,40},{15,25},1.3,1,1},
  //GROUP3-Function key
  {{820,50},{30,15},1.5,1,1}
};

/* KeyBoard Layout for Numeric Key Board */

KBLayoutSibblingInfoContainer numericKBKeySiblingInfo;
KBLayoutKeyPosContainer numericKBKeyPos;
#define NUMERIC_KB_PARTITION_COUNT 1
SkPoint  numericKBReturnKeyIndex{3,3};/*KeyIndex,RowIndex*/
SkPoint numericKBDefaultHLKeyIndex{2,0};/*KeyIndex,RowIndex*/
KBLayoutKeyInfoContainer numericKBKeyKeyInfo={
  {
    //row 1
    {"1",RNS_KEY_1,KEY_TYPE_TEXT,GROUP1},
    {"2",RNS_KEY_2,KEY_TYPE_TEXT,GROUP1},
    {"3",RNS_KEY_3,KEY_TYPE_TEXT,GROUP1},
    {"-",RNS_KEY_Underscore,KEY_TYPE_TEXT,GROUP1}
  },
  {
    //row 2
    {"4",RNS_KEY_4,KEY_TYPE_TEXT,GROUP1},
    {"5",RNS_KEY_5,KEY_TYPE_TEXT,GROUP1},
    {"6",RNS_KEY_6,KEY_TYPE_TEXT,GROUP1},
    {"space",RNS_KEY_Space,KEY_TYPE_FUNCTION,GROUP1}
  },
  {
    //row 3
    {"7",RNS_KEY_7,KEY_TYPE_TEXT,GROUP1},
    {"8",RNS_KEY_8,KEY_TYPE_TEXT,GROUP1},
    {"9",RNS_KEY_9,KEY_TYPE_TEXT,GROUP1},
    {"delete",RNS_KEY_Delete,KEY_TYPE_FUNCTION,GROUP1}
  },
  {
    //row 4
    {",",RNS_KEY_Comma,KEY_TYPE_TEXT,GROUP1},
    {"0",RNS_KEY_0,KEY_TYPE_TEXT,GROUP1},
    {".",RNS_KEY_Period,KEY_TYPE_TEXT,GROUP1 },
    {"return",RNS_KEY_Select,KEY_TYPE_FUNCTION,GROUP1}
  }
};

keyPlacementConfig_t numericKBGroupConfig[NUMERIC_KB_PARTITION_COUNT] = {
  {{250,30},{40,40},1.3,1.5,1}
};


typedef std::map<std::string,KBLayoutType> ToggleKeyMap;

ToggleKeyMap toggleKeyMap = {
  {"123ABC",ALPHA_UPPERCASE_LAYOUT},
  {"123abc",ALPHA_LOWERCASE_LAYOUT},
  {"@?#",SYMBOL_LAYOUT}
};

typedef struct unicharFontConfig {
  unsigned int unicharValue;
  float fontScaleFactor;
}UnicharFontConfig_t;

typedef std::map<std::string,UnicharFontConfig_t> FunctionKeymap;

FunctionKeymap functionKeyMap = {
  {"delete",{0x232B,1}},
  {"space",{0x23B5,2}},
  {"return",{0x23CE,1.5}}, // default return key entry: "enter"
  {"enter",{0x23CE,1.5}},
  {"search",{0x2315,2.5}},
  {"right",{0x25BA,1}},
  {"left",{0x25c4,1}}
};

} // namespace sdk
} // namespace rns


