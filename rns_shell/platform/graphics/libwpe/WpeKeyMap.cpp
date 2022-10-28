#include <wpe/wpe.h>
#include "ReactSkia/sdk/RNSKeyCodeMapping.h"
#include "WindowLibWPE.h"
using namespace std;
rnsKey  RnsShell::WindowLibWPE::keyIdentifierForWPEKeyCode(int keyCode){
    switch (keyCode) {
    case WPE_KEY_Left: return RNS_KEY_Left;
    case WPE_KEY_Right: return RNS_KEY_Right;
    case WPE_KEY_Up: return RNS_KEY_Up;
    case WPE_KEY_Down: return RNS_KEY_Down;
    case WPE_KEY_Select:
    case WPE_KEY_ISO_Enter:
    case WPE_KEY_Return: return RNS_KEY_Select;
    case WPE_KEY_PowerOff: return RNS_KEY_PowerOff;
    case WPE_KEY_AudioMedia: return RNS_KEY_AudioMedia;
    case 0XF067:
    case WPE_KEY_Menu: return RNS_KEY_Menu;
    case 0xF04D:
    case WPE_KEY_AudioLowerVolume: return RNS_KEY_AudioLowerVolume;
    case 0xF04C:
    case WPE_KEY_AudioRaiseVolume: return RNS_KEY_AudioRaiseVolume;
    case 0XF046: return RNS_KEY_ChannelUp;
    case 0XF047: return RNS_KEY_ChannelDown;
    case 0XF014: return RNS_KEY_Help;
    case WPE_KEY_Refresh: return RNS_KEY_Refresh;
    case 0xF052:
    case WPE_KEY_AudioStop: return RNS_KEY_AudioStop;
    case 0XF051:
    case WPE_KEY_AudioPlay: return RNS_KEY_AudioPlay;
    case 0XFF13:
    case WPE_KEY_AudioPause: return RNS_KEY_AudioPause;
    case WPE_KEY_Back: return RNS_KEY_Back;
    case 0XF05C:
    case WPE_KEY_AudioNext: return RNS_KEY_AudioNext;
    case 0xF059:
    case WPE_KEY_AudioRewind: return RNS_KEY_AudioRewind;
    case 0xF05A:
    case WPE_KEY_AudioForward: return RNS_KEY_AudioForward;
    case 0XF05B:
    case WPE_KEY_AudioPrev: return RNS_KEY_AudioPrev;
    case 0XF043:
    case WPE_KEY_Green: return RNS_KEY_Green;
    case 0XF042:
    case WPE_KEY_Red: return RNS_KEY_Red;
    case 0XF045:
    case WPE_KEY_Blue: return RNS_KEY_Blue;
    case 0XF044:
    case WPE_KEY_Yellow: return RNS_KEY_Yellow;
    case WPE_KEY_1: return RNS_KEY_1;
    case WPE_KEY_2: return RNS_KEY_2;
    case WPE_KEY_3: return RNS_KEY_3;
    case WPE_KEY_4: return RNS_KEY_4;
    case WPE_KEY_5: return RNS_KEY_5;
    case WPE_KEY_6: return RNS_KEY_6;
    case WPE_KEY_7: return RNS_KEY_7;
    case WPE_KEY_8: return RNS_KEY_8;
    case WPE_KEY_9: return RNS_KEY_9;
    case WPE_KEY_0: return RNS_KEY_0;
    case 0xF02F: return RNS_KEY_TvRad;
    case 0xF037: return RNS_KEY_Aud;
    case 0XF056:
    case WPE_KEY_AudioRecord: return RNS_KEY_AudioRecord;
    case 0XF062:
    case WPE_KEY_Cancel: return RNS_KEY_Cancel;
    case 0XF04E:
    case WPE_KEY_AudioMute: return RNS_KEY_Mute;
    case 0XF01B: return RNS_KEY_Guia;
    /*Upper Case Alpha */
    case WPE_KEY_A: return RNS_KEY_A;
    case WPE_KEY_B: return RNS_KEY_B;
    case WPE_KEY_C: return RNS_KEY_C;
    case WPE_KEY_D: return RNS_KEY_D;
    case WPE_KEY_E: return RNS_KEY_E;
    case WPE_KEY_F: return RNS_KEY_F;
    case WPE_KEY_G: return RNS_KEY_G;
    case WPE_KEY_H: return RNS_KEY_H;
    case WPE_KEY_I: return RNS_KEY_I;
    case WPE_KEY_J: return RNS_KEY_J;
    case WPE_KEY_K: return RNS_KEY_K;
    case WPE_KEY_L: return RNS_KEY_L;
    case WPE_KEY_M: return RNS_KEY_M;
    case WPE_KEY_N: return RNS_KEY_N;
    case WPE_KEY_O: return RNS_KEY_O;
    case WPE_KEY_P: return RNS_KEY_P;
    case WPE_KEY_Q: return RNS_KEY_Q;
    case WPE_KEY_R: return RNS_KEY_R;
    case WPE_KEY_S: return RNS_KEY_S;
    case WPE_KEY_T: return RNS_KEY_T;
    case WPE_KEY_U: return RNS_KEY_U;
    case WPE_KEY_V: return RNS_KEY_V;
    case WPE_KEY_W: return RNS_KEY_W;
    case WPE_KEY_X: return RNS_KEY_X;
    case WPE_KEY_Y: return RNS_KEY_Y;
    case WPE_KEY_Z: return RNS_KEY_Z;
    /*Lower Case Alpha*/
    case WPE_KEY_a: return RNS_KEY_a;
    case WPE_KEY_b: return RNS_KEY_b;
    case WPE_KEY_c: return RNS_KEY_c;
    case WPE_KEY_d: return RNS_KEY_d;
    case WPE_KEY_e: return RNS_KEY_e;
    case WPE_KEY_f: return RNS_KEY_f;
    case WPE_KEY_g: return RNS_KEY_g;
    case WPE_KEY_h: return RNS_KEY_h;
    case WPE_KEY_i: return RNS_KEY_i;
    case WPE_KEY_j: return RNS_KEY_j;
    case WPE_KEY_k: return RNS_KEY_k;
    case WPE_KEY_l: return RNS_KEY_l;
    case WPE_KEY_m: return RNS_KEY_m;
    case WPE_KEY_n: return RNS_KEY_n;
    case WPE_KEY_o: return RNS_KEY_o;
    case WPE_KEY_p: return RNS_KEY_p;
    case WPE_KEY_q: return RNS_KEY_q;
    case WPE_KEY_r: return RNS_KEY_r;
    case WPE_KEY_s: return RNS_KEY_s;
    case WPE_KEY_t: return RNS_KEY_t;
    case WPE_KEY_u: return RNS_KEY_u;
    case WPE_KEY_v: return RNS_KEY_v;
    case WPE_KEY_w: return RNS_KEY_w;
    case WPE_KEY_x: return RNS_KEY_x;
    case WPE_KEY_y: return RNS_KEY_y;
    case WPE_KEY_z: return RNS_KEY_z;
    /*Modifier keys */
    case WPE_KEY_Caps_Lock : return RNS_KEY_Caps_Lock;
    case WPE_KEY_Alt_L: return RNS_KEY_Alt_L;
    case WPE_KEY_Alt_R: return RNS_KEY_Alt_R;
    case WPE_KEY_Shift_L: return RNS_KEY_Shift_L;
    case WPE_KEY_Shift_R: return RNS_KEY_Shift_R;
    case WPE_KEY_Control_L: return RNS_KEY_Control_L;
    case WPE_KEY_Control_R: return RNS_KEY_Control_R;
    /*special case */
    case WPE_KEY_Tab: return RNS_KEY_Tab;
    case WPE_KEY_grave: return RNS_KEY_Grave;
    case WPE_KEY_BackSpace: return RNS_KEY_Back;
    case WPE_KEY_asciitilde: return RNS_KEY_Asciitilde;
    case WPE_KEY_exclam: return RNS_KEY_Exclam;
    case WPE_KEY_at : return RNS_KEY_At;
    case WPE_KEY_numbersign: return RNS_KEY_Numbersign;
    case WPE_KEY_dollar: return RNS_KEY_Dollar;
    case WPE_KEY_asciicircum: return RNS_KEY_Asciicircum;
    case WPE_KEY_ampersand: return RNS_KEY_Ampersand;
    case WPE_KEY_asterisk: return RNS_KEY_Asterisk;
    case WPE_KEY_braceleft: return RNS_KEY_BraceLeft;
    case WPE_KEY_braceright: return RNS_KEY_BraceRight;
    case WPE_KEY_underscore: return RNS_KEY_Underscore;
    case WPE_KEY_Delete: return RNS_KEY_Delete;
    case WPE_KEY_Page_Up: return RNS_KEY_Page_Up;
    case WPE_KEY_Page_Down: return RNS_KEY_Page_Down;
    case WPE_KEY_slash: return RNS_KEY_Shash;
    case WPE_KEY_period: return RNS_KEY_Period;
    case WPE_KEY_comma: return RNS_KEY_Comma;
    case WPE_KEY_question: return RNS_KEY_Question;
    case WPE_KEY_apostrophe: return RNS_KEY_Apostrophe;
    case WPE_KEY_quotedbl: return RNS_KEY_Quotedbl;
    case WPE_KEY_semicolon: return RNS_KEY_Semicolon;
    case WPE_KEY_colon: return RNS_KEY_Colon;
    case WPE_KEY_bar: return RNS_KEY_Bar;
    case WPE_KEY_backslash: return RNS_KEY_BackSlash;
    case WPE_KEY_parenleft: return   RNS_KEY_ParenLeft;
    case WPE_KEY_parenright: return RNS_KEY_ParenRight;
    case WPE_KEY_bracketleft: return RNS_KEY_BracketLeft;
    case WPE_KEY_bracketright: return RNS_KEY_BracketRight;
    case WPE_KEY_Escape: return RNS_KEY_Escape;
    case WPE_KEY_space: return RNS_KEY_Space;
    case WPE_KEY_Home: return RNS_KEY_Home;
    case WPE_KEY_End : return RNS_KEY_EndKey;
    case WPE_KEY_Insert: return RNS_KEY_Insert;
    /*Arthimatic*/
    case WPE_KEY_minus: return RNS_KEY_Minus;
    case WPE_KEY_equal: return RNS_KEY_Equal;
    case WPE_KEY_percent: return RNS_KEY_Percent;
    case WPE_KEY_plus: return RNS_KEY_Plus;
    case WPE_KEY_greater: return RNS_KEY_Greater;
    case WPE_KEY_less : return RNS_KEY_Less;
    /*UnKnownKey*/
    default: return RNS_KEY_UnKnown;
    }
}
