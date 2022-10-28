#include <X11/Xutil.h>
#include "ReactSkia/sdk/RNSKeyCodeMapping.h"
#include "WindowX11.h"
using namespace std;
rnsKey  RnsShell::WindowX11::keyIdentifierForX11KeyCode(KeySym keycode){
    switch (keycode) {
    case XK_Left: return RNS_KEY_Left;
    case XK_Right: return RNS_KEY_Right;
    case XK_Up: return RNS_KEY_Up;
    case XK_Down: return RNS_KEY_Down;
    case XK_Return:
    case XK_Select: return RNS_KEY_Select;
    case XK_Menu: return RNS_KEY_Menu;
    case XK_Pause: return RNS_KEY_AudioPause;
    case XK_BackSpace: return RNS_KEY_Back;
    case XK_0: return RNS_KEY_0;
    case XK_1: return RNS_KEY_1;
    case XK_2: return RNS_KEY_2;
    case XK_3: return RNS_KEY_3;
    case XK_4: return RNS_KEY_4;
    case XK_5: return RNS_KEY_5;
    case XK_6: return RNS_KEY_6;
    case XK_7: return RNS_KEY_7;
    case XK_8: return RNS_KEY_8;
    case XK_9: return RNS_KEY_9;
    case XK_Cancel: return RNS_KEY_Cancel;
    /*
     * KeyBoard Keys.
     */
    /*Upper case Alphabets*/
    case XK_A: return RNS_KEY_A;
    case XK_B: return RNS_KEY_B;
    case XK_C: return RNS_KEY_C;
    case XK_D: return RNS_KEY_D;
    case XK_E: return RNS_KEY_E;
    case XK_F: return RNS_KEY_F;
    case XK_G: return RNS_KEY_G;
    case XK_H: return RNS_KEY_H;
    case XK_I: return RNS_KEY_I;
    case XK_J: return RNS_KEY_J;
    case XK_K: return RNS_KEY_K;
    case XK_L: return RNS_KEY_L;
    case XK_M: return RNS_KEY_M;
    case XK_N: return RNS_KEY_N;
    case XK_O: return RNS_KEY_O;
    case XK_P: return RNS_KEY_P;
    case XK_Q: return RNS_KEY_Q;
    case XK_R: return RNS_KEY_R;
    case XK_S: return RNS_KEY_S;
    case XK_T: return RNS_KEY_T;
    case XK_U: return RNS_KEY_U;
    case XK_V: return RNS_KEY_V;
    case XK_W: return RNS_KEY_W;
    case XK_X: return RNS_KEY_X;
    case XK_Y: return RNS_KEY_Y;
    case XK_Z: return RNS_KEY_Z;
    /*Lower case Alphabets*/
    case XK_a: return RNS_KEY_a;
    case XK_b: return RNS_KEY_b;
    case XK_c: return RNS_KEY_c;
    case XK_d: return RNS_KEY_d;
    case XK_e: return RNS_KEY_e;
    case XK_f: return RNS_KEY_f;
    case XK_g: return RNS_KEY_g;
    case XK_h: return RNS_KEY_h;
    case XK_i: return RNS_KEY_i;
    case XK_j: return RNS_KEY_j;
    case XK_k: return RNS_KEY_k;
    case XK_l: return RNS_KEY_l;
    case XK_m: return RNS_KEY_m;
    case XK_n: return RNS_KEY_n;
    case XK_o: return RNS_KEY_o;
    case XK_p: return RNS_KEY_p;
    case XK_q: return RNS_KEY_q;
    case XK_r: return RNS_KEY_r;
    case XK_s: return RNS_KEY_s;
    case XK_t: return RNS_KEY_t;
    case XK_u: return RNS_KEY_u;
    case XK_v: return RNS_KEY_v;
    case XK_w: return RNS_KEY_w;
    case XK_x: return RNS_KEY_x;
    case XK_y: return RNS_KEY_y;
    case XK_z: return RNS_KEY_z;
    /*Modifiers*/
    case XK_Caps_Lock: return RNS_KEY_Caps_Lock;
    case XK_Alt_L: return RNS_KEY_Alt_L;
    case XK_Alt_R: return RNS_KEY_Alt_R;
    case XK_Shift_L: return RNS_KEY_Shift_L;
    case XK_Shift_R: return RNS_KEY_Shift_R;
    case XK_Control_L: return RNS_KEY_Control_L;
    case XK_Control_R: return RNS_KEY_Control_R;
    /* Special char */
    case XK_Tab : return RNS_KEY_Tab;
    case XK_grave: return RNS_KEY_Grave;
    case XK_asciitilde: return RNS_KEY_Asciitilde;
    case XK_exclam: return RNS_KEY_Exclam;
    case XK_at: return RNS_KEY_At;
    case XK_numbersign: return RNS_KEY_Numbersign;
    case XK_dollar: return RNS_KEY_Dollar;
    case XK_asciicircum: return RNS_KEY_Asciicircum;
    case XK_ampersand: return RNS_KEY_Ampersand;
    case XK_asterisk: return RNS_KEY_Asterisk;
    case XK_parenleft: return RNS_KEY_ParenLeft;
    case XK_parenright: return RNS_KEY_ParenRight;
    case XK_underscore: return RNS_KEY_Underscore;
    case XK_Delete: return RNS_KEY_Delete;
    case XK_Page_Up: return RNS_KEY_Page_Up;
    case XK_Page_Down: return RNS_KEY_Page_Down;
    case XK_slash: return RNS_KEY_Shash;
    case XK_period: return RNS_KEY_Period;
    case XK_comma: return RNS_KEY_Comma;
    case XK_question: return RNS_KEY_Question;
    case XK_apostrophe: return RNS_KEY_Apostrophe;
    case XK_quotedbl: return RNS_KEY_Quotedbl;
    case XK_colon: return RNS_KEY_Colon;
    case XK_semicolon: return RNS_KEY_Semicolon;
    case XK_bar: return RNS_KEY_Bar;
    case XK_backslash : return RNS_KEY_BackSlash;
    case XK_braceleft: return RNS_KEY_BraceLeft;
    case XK_braceright: return RNS_KEY_BraceRight;
    case XK_bracketleft : return RNS_KEY_BracketLeft;
    case XK_bracketright: return RNS_KEY_BracketRight;
    case XK_Escape: return RNS_KEY_Escape;
    case XK_space: return RNS_KEY_Space;
    case XK_Home: return RNS_KEY_Home;
    case XK_End: return RNS_KEY_EndKey;
    case XK_Insert: return RNS_KEY_Insert;
    /*
     * Arithematic
     */
    case XK_minus: return RNS_KEY_Minus;
    case XK_equal: return RNS_KEY_Equal;
    case XK_percent: return RNS_KEY_Percent;
    case XK_plus: return RNS_KEY_Plus;
    case XK_greater: return RNS_KEY_Greater;
    case XK_less: return RNS_KEY_Less;

    default: return RNS_KEY_UnKnown;
    }
}

