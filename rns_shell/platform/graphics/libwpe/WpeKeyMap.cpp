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
    case WPE_KEY_AudioRecord: return RNS_KEY_AudioReacord;
    case 0XF062:
    case WPE_KEY_Cancel: return RNS_KEY_Cancel;
    case 0XF04E:
    case WPE_KEY_AudioMute: return RNS_KEY_Mute;
    case 0XF01B: return RNS_KEY_Guia;
    default: return RNS_KEY_UnKnown;
    }
}
