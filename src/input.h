#ifndef INPUT_H
#define INPUT_H

#include <uiohook.h>
#include <nlohmann/json.hpp>
#include <unordered_map>
#include <vector>

using KeyState = std::unordered_map<uint16_t, bool>;

struct KeyCombo {
    std::vector<uint16_t> required;
    std::vector<uint16_t> excluded;
};

enum class Action : uint16_t {
    NONE = 0,
    TAKE_SCREENSHOT,
    SAVE_FULLSCREEN,
    OPEN_SETTINGS,
    CLOSE_WINDOW,
    QUIT_KADR,

    COUNT,
};

inline const char* action_name(Action action) {
    switch (action) {
        case Action::TAKE_SCREENSHOT:
            return "take screenshot";
        case Action::OPEN_SETTINGS:
            return "open settings";
        case Action::CLOSE_WINDOW:
            return "close window";
        case Action::SAVE_FULLSCREEN:
            return "save fullscreen";
        case Action::QUIT_KADR:
            return "quit kadr";
        default:
            return "unknown";
    }
}

struct Binding {
    KeyCombo combo;
    Action action;
};

void init_keybinds();
void shutdown_keybinds();

void keybinds_on_event(const uiohook_event* event);

Action keybinds_poll();

const KeyState& keybinds_state();

void keybinds_bind_silent(Action action, const KeyCombo& combo);
void keybinds_bind(Action action, const KeyCombo& combo);
void keybinds_unbind_silent(Action action);
void keybinds_unbind(Action action);
const std::vector<Binding>& keybinds_get_bindings();

const char* keycode_name(uint16_t code);

const std::vector<uint16_t>& all_keycodes();

const KeyCombo* keybinds_get_combo(Action action);
const std::vector<uint16_t>& keybinds_capture_peak();

bool keybinds_is_capturing();
Action keybinds_capture_target();
void keybinds_start_capture(Action action);
void keybinds_cancel_capture();

using KeybindsChangedFn = void (*)();

void keybinds_set_changed_callback(KeybindsChangedFn fn);

[[deprecated]] bool keybinds_save(const char* path);
[[deprecated]] bool keybinds_load(const char* path);

nlohmann::json keybinds_to_json();
void keybinds_from_json(const nlohmann::json& j);

inline KeyCombo combo(std::initializer_list<uint16_t> req) {
    return KeyCombo{std::vector(req), {}};
}
inline KeyCombo combo_ex(std::initializer_list<uint16_t> req, std::initializer_list<uint16_t> ex) {
    return KeyCombo{std::vector(req), std::vector(ex)};
}

#endif /* INPUT_H */
