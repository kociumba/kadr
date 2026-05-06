#include "input.h"
#include <algorithm>
#include <fstream>
#include <magic_enum/magic_enum_all.hpp>
#include <sstream>
#include <unordered_set>

static KeyState g_state;
static std::vector<Binding> g_bindings;
static std::unordered_set<uint8_t> g_fired;
static bool g_capturing = false;
static Action g_capture_target = Action::NONE;
static std::vector<uint16_t> g_capture_peak;
static KeybindsChangedFn g_on_changed = nullptr;

void init_keybinds() {
    g_state.clear();
    g_bindings.clear();
    g_fired.clear();
    for (uint16_t code : all_keycodes()) {
        g_state[code] = false;
    }
}

void shutdown_keybinds() {
    g_state.clear();
    g_bindings.clear();
    g_fired.clear();
}

static void keybinds_notify_changed() {
    if (g_on_changed) g_on_changed();
}

static void keybinds_unbind_silent(Action action) {
    std::erase_if(g_bindings, [action](const Binding& b) { return b.action == action; });
}

void keybinds_on_event(const uiohook_event* event) {
    if (event->type != EVENT_KEY_PRESSED && event->type != EVENT_KEY_RELEASED) return;

    uint16_t code = event->data.keyboard.keycode;
    bool pressed = event->type == EVENT_KEY_PRESSED;
    g_state[code] = pressed;

    if (g_capturing) {
        if (pressed) {
            if (std::ranges::find(g_capture_peak, code) == g_capture_peak.end())
                g_capture_peak.push_back(code);
        }

        bool any_down = false;
        for (const auto& [c, down] : g_state) {
            if (down) {
                any_down = true;
                break;
            }
        }

        if (!any_down && !g_capture_peak.empty()) {
            KeyCombo new_combo;
            new_combo.required = g_capture_peak;
            keybinds_bind(g_capture_target, new_combo);
            g_capturing = false;
            g_capture_target = Action::NONE;
            g_capture_peak.clear();
        }

        return;  // stop hotkey input for binding
    }

    if (!pressed) {
        for (auto it = g_fired.begin(); it != g_fired.end();) {
            auto act = static_cast<Action>(*it);
            bool should_clear = false;
            for (const auto& b : g_bindings) {
                if (b.action != act) continue;
                for (uint16_t req : b.combo.required) {
                    if (req == code) {
                        should_clear = true;
                        break;
                    }
                }
                if (should_clear) break;
            }
            if (should_clear) {
                it = g_fired.erase(it);
            } else {
                ++it;
            }
        }
    }
}

static bool combo_matches(const KeyCombo& c, const KeyState& state) {
    for (uint16_t code : c.required) {
        auto it = state.find(code);
        if (it == state.end() || !it->second) return false;
    }
    for (uint16_t code : c.excluded) {
        auto it = state.find(code);
        if (it != state.end() && it->second) return false;
    }
    return true;
}

Action keybinds_poll() {
    for (const auto& b : g_bindings) {
        auto act_idx = static_cast<uint16_t>(b.action);
        if (g_fired.contains(act_idx)) continue;
        if (combo_matches(b.combo, g_state)) {
            g_fired.insert(act_idx);
            return b.action;
        }
    }
    return Action::NONE;
}

const KeyState& keybinds_state() { return g_state; }

void keybinds_bind_silent(Action action, const KeyCombo& combo) {
    keybinds_unbind_silent(action);
    g_bindings.push_back(Binding{combo, action});
}

void keybinds_bind(Action action, const KeyCombo& combo) {
    keybinds_bind_silent(action, combo);
    keybinds_notify_changed();
}

void keybinds_unbind(Action action) {
    size_t before = g_bindings.size();
    keybinds_unbind_silent(action);
    if (g_bindings.size() < before) keybinds_notify_changed();
}

const std::vector<Binding>& keybinds_get_bindings() { return g_bindings; }

const char* keycode_name(uint16_t code) {
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

const std::vector<uint16_t>& all_keycodes() {
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

const KeyCombo* keybinds_get_combo(Action action) {
    for (const auto& b : g_bindings) {
        if (b.action == action) return &b.combo;
    }
    return nullptr;
}

const std::vector<uint16_t>& keybinds_capture_peak() { return g_capture_peak; }

bool keybinds_is_capturing() { return g_capturing; }
Action keybinds_capture_target() { return g_capture_target; }

void keybinds_start_capture(Action action) {
    g_capturing = true;
    g_capture_target = action;
    g_capture_peak.clear();
}

void keybinds_cancel_capture() {
    g_capturing = false;
    g_capture_target = Action::NONE;
    g_capture_peak.clear();
}

void keybinds_set_changed_callback(KeybindsChangedFn fn) { g_on_changed = fn; }

bool keybinds_save(const char* path) {
    std::ofstream f(path);
    if (!f) return false;
    for (const auto& b : g_bindings) {
        f << static_cast<int>(b.action);
        f << " ";
        for (size_t i = 0; i < b.combo.required.size(); ++i) {
            if (i) f << ",";
            f << b.combo.required[i];
        }
        if (!b.combo.excluded.empty()) {
            f << " !";
            for (size_t i = 0; i < b.combo.excluded.size(); ++i) {
                if (i) f << ",";
                f << b.combo.excluded[i];
            }
        }
        f << "\n";
    }
    return true;
}

bool keybinds_load(const char* path) {
    std::ifstream f(path);
    if (!f) return false;

    g_bindings.clear();
    g_fired.clear();

    std::string line;
    while (std::getline(f, line)) {
        std::istringstream iss(line);
        int act_idx;
        iss >> act_idx;

        std::string req_str, ex_str;
        iss >> req_str;
        size_t bang = req_str.find('!');
        if (bang != std::string::npos) {
            ex_str = req_str.substr(bang + 1);
            req_str = req_str.substr(0, bang);
        }

        KeyCombo c;
        std::stringstream req_ss(req_str);
        std::string token;
        while (std::getline(req_ss, token, ',')) {
            c.required.push_back(static_cast<uint16_t>(std::stoi(token)));
        }

        if (!ex_str.empty()) {
            std::stringstream ex_ss(ex_str);
            while (std::getline(ex_ss, token, ',')) {
                c.excluded.push_back(static_cast<uint16_t>(std::stoi(token)));
            }
        }

        g_bindings.push_back(Binding{c, static_cast<Action>(act_idx)});
    }
    return true;
}

nlohmann::json keybinds_to_json() {
    nlohmann::json j;
    for (const auto& [combo, action] : g_bindings) {
        auto* name = magic_enum::enum_name(action).data();
        j[name]["required"] = combo.required;
        if (!combo.excluded.empty()) j[name]["excluded"] = combo.excluded;
    }
    return j;
}

void keybinds_from_json(const nlohmann::json& j) {
    g_bindings.clear();
    g_fired.clear();
    for (auto& [key, val] : j.items()) {
        auto action = magic_enum::enum_cast<Action>(key);
        if (!action) continue;
        KeyCombo c;
        c.required = val.value("required", std::vector<uint16_t>{});
        c.excluded = val.value("excluded", std::vector<uint16_t>{});
        g_bindings.push_back({c, *action});
    }
}