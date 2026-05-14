#ifndef KADR_INPUT_BOILERPLATE_H
#define KADR_INPUT_BOILERPLATE_H

inline const char* keycode_name(uint16_t code) {
    switch (code) {
        case VC_ESCAPE:
            return "Esc";
        case VC_F1:
            return "F1";
        case VC_F2:
            return "F2";
        case VC_F3:
            return "F3";
        case VC_F4:
            return "F4";
        case VC_F5:
            return "F5";
        case VC_F6:
            return "F6";
        case VC_F7:
            return "F7";
        case VC_F8:
            return "F8";
        case VC_F9:
            return "F9";
        case VC_F10:
            return "F10";
        case VC_F11:
            return "F11";
        case VC_F12:
            return "F12";
        case VC_BACKQUOTE:
            return "`";
        case VC_1:
            return "1";
        case VC_2:
            return "2";
        case VC_3:
            return "3";
        case VC_4:
            return "4";
        case VC_5:
            return "5";
        case VC_6:
            return "6";
        case VC_7:
            return "7";
        case VC_8:
            return "8";
        case VC_9:
            return "9";
        case VC_0:
            return "0";
        case VC_MINUS:
            return "-";
        case VC_EQUALS:
            return "=";
        case VC_BACKSPACE:
            return "Backspace";
        case VC_TAB:
            return "Tab";
        case VC_Q:
            return "Q";
        case VC_W:
            return "W";
        case VC_E:
            return "E";
        case VC_R:
            return "R";
        case VC_T:
            return "T";
        case VC_Y:
            return "Y";
        case VC_U:
            return "U";
        case VC_I:
            return "I";
        case VC_O:
            return "O";
        case VC_P:
            return "P";
        case VC_A:
            return "A";
        case VC_S:
            return "S";
        case VC_D:
            return "D";
        case VC_F:
            return "F";
        case VC_G:
            return "G";
        case VC_H:
            return "H";
        case VC_J:
            return "J";
        case VC_K:
            return "K";
        case VC_L:
            return "L";
        case VC_Z:
            return "Z";
        case VC_X:
            return "X";
        case VC_C:
            return "C";
        case VC_V:
            return "V";
        case VC_B:
            return "B";
        case VC_N:
            return "N";
        case VC_M:
            return "M";
        case VC_SHIFT_L:
            return "LShift";
        case VC_SHIFT_R:
            return "RShift";
        case VC_CONTROL_L:
            return "LCtrl";
        case VC_CONTROL_R:
            return "RCtrl";
        case VC_ALT_L:
            return "LAlt";
        case VC_ALT_R:
            return "RAlt";
        case VC_META_L:
            return "LMeta";
        case VC_META_R:
            return "RMeta";
        case VC_SPACE:
            return "Space";
        case VC_ENTER:
            return "Enter";
        case VC_UP:
            return "Up";
        case VC_DOWN:
            return "Down";
        case VC_LEFT:
            return "Left";
        case VC_RIGHT:
            return "Right";
        case VC_INSERT:
            return "Insert";
        case VC_DELETE:
            return "Delete";
        case VC_HOME:
            return "Home";
        case VC_END:
            return "End";
        case VC_PAGE_UP:
            return "PgUp";
        case VC_PAGE_DOWN:
            return "PgDn";
        case VC_PRINTSCREEN:
            return "PrtSc";
        case VC_SCROLL_LOCK:
            return "ScrollLock";
        case VC_OPEN_BRACKET:
            return "[";
        case VC_CLOSE_BRACKET:
            return "]";
        case VC_PAUSE:
            return "Pause";
        case VC_NUM_LOCK:
            return "NumLock";
        case VC_KP_0:
            return "KP0";
        case VC_KP_1:
            return "KP1";
        case VC_KP_2:
            return "KP2";
        case VC_KP_3:
            return "KP3";
        case VC_KP_4:
            return "KP4";
        case VC_KP_5:
            return "KP5";
        case VC_KP_6:
            return "KP6";
        case VC_KP_7:
            return "KP7";
        case VC_KP_8:
            return "KP8";
        case VC_KP_9:
            return "KP9";
        case VC_KP_DIVIDE:
            return "KP/";
        case VC_KP_MULTIPLY:
            return "KP*";
        case VC_KP_SUBTRACT:
            return "KP-";
        case VC_KP_ADD:
            return "KP+";
        case VC_KP_ENTER:
            return "KPEnter";
        case VC_KP_SEPARATOR:
            return "KP.";
        case VC_VOLUME_MUTE:
            return "VolMute";
        case VC_VOLUME_DOWN:
            return "VolDown";
        case VC_VOLUME_UP:
            return "VolUp";
        case VC_MEDIA_PLAY:
            return "MediaPlay";
        case VC_MEDIA_STOP:
            return "MediaStop";
        case VC_MEDIA_PREVIOUS:
            return "MediaPrev";
        case VC_MEDIA_NEXT:
            return "MediaNext";
        default:
            return nullptr;
    }
}

inline const std::vector<uint16_t>& all_keycodes() {
    static const std::vector<uint16_t> codes = {
        VC_ESCAPE,
        VC_F1,
        VC_F2,
        VC_F3,
        VC_F4,
        VC_F5,
        VC_F6,
        VC_F7,
        VC_F8,
        VC_F9,
        VC_F10,
        VC_F11,
        VC_F12,
        VC_BACKSPACE,
        VC_1,
        VC_2,
        VC_3,
        VC_4,
        VC_5,
        VC_6,
        VC_7,
        VC_8,
        VC_9,
        VC_0,
        VC_MINUS,
        VC_EQUALS,
        VC_BACKSPACE,
        VC_TAB,
        VC_Q,
        VC_W,
        VC_E,
        VC_R,
        VC_T,
        VC_Y,
        VC_U,
        VC_I,
        VC_O,
        VC_P,
        VC_A,
        VC_S,
        VC_D,
        VC_F,
        VC_G,
        VC_H,
        VC_J,
        VC_K,
        VC_L,
        VC_Z,
        VC_X,
        VC_C,
        VC_V,
        VC_B,
        VC_N,
        VC_M,
        VC_SHIFT_L,
        VC_SHIFT_R,
        VC_CONTROL_L,
        VC_CONTROL_R,
        VC_ALT_L,
        VC_ALT_R,
        VC_META_L,
        VC_META_R,
        VC_SPACE,
        VC_ENTER,
        VC_UP,
        VC_DOWN,
        VC_LEFT,
        VC_RIGHT,
        VC_INSERT,
        VC_DELETE,
        VC_HOME,
        VC_END,
        VC_PAGE_UP,
        VC_PAGE_DOWN,
        VC_PRINTSCREEN,
        VC_SCROLL_LOCK,
        VC_PAUSE,
        VC_NUM_LOCK,
        VC_OPEN_BRACKET,
        VC_CLOSE_BRACKET,
        VC_KP_0,
        VC_KP_1,
        VC_KP_2,
        VC_KP_3,
        VC_KP_4,
        VC_KP_5,
        VC_KP_6,
        VC_KP_7,
        VC_KP_8,
        VC_KP_9,
        VC_KP_DIVIDE,
        VC_KP_MULTIPLY,
        VC_KP_SUBTRACT,
        VC_KP_ADD,
        VC_KP_ENTER,
        VC_KP_SEPARATOR,
        VC_VOLUME_MUTE,
        VC_VOLUME_DOWN,
        VC_VOLUME_UP,
        VC_MEDIA_PLAY,
        VC_MEDIA_STOP,
        VC_MEDIA_PREVIOUS,
        VC_MEDIA_NEXT,
    };
    return codes;
}

#endif  //KADR_INPUT_BOILERPLATE_H
