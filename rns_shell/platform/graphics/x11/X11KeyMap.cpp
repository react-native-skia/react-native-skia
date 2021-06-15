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
    case XK_Next: return RNS_KEY_AudioNext;
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
    default: return RNS_KEY_UnKnown;
    }
}

