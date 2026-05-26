#ifndef KADR_INPUT_H
#define KADR_INPUT_H

#include <uiohook.h>
#include <nlohmann/json.hpp>
#include <unordered_map>
#include <vector>
#include "input_boilerplate.h"

using KeyState = std::unordered_map<uint16_t, bool>;

struct KeyCombo {
    std::vector<uint16_t> required;
    std::vector<uint16_t> excluded;
};

enum class Action : uint16_t {
    NONE = 0,
    TAKE_SCREENSHOT,
    SAVE_FULLSCREEN,
    OPEN_SC_FOLDER,
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
        case Action::OPEN_SC_FOLDER:
            return "open screenshot folder";
        case Action::QUIT_KADR:
            return "quit kadr";
        default:
            return "unknown";
    }
}

struct Binding {
    uint32_t id;
    KeyCombo combo;
    Action action;
};

void init_keybinds();
void shutdown_keybinds();

void keybinds_on_event(const uiohook_event* event);

Action keybinds_poll();

const KeyState& keybinds_state();

uint32_t keybinds_bind_silent(Action action, const KeyCombo& combo);
uint32_t keybinds_bind(Action action, const KeyCombo& combo);

void keybinds_unbind_silent(Action action);
void keybinds_unbind(Action action);

void keybinds_unbind_silent(uint32_t id);
void keybinds_unbind(uint32_t id);

void keybinds_unbind_silent(Action action, const KeyCombo& combo);
void keybinds_unbind(Action action, const KeyCombo& combo);

const std::vector<Binding>& keybinds_get_bindings();

const char* keycode_name(uint16_t code);      // in input_boilerplate.h
const std::vector<uint16_t>& all_keycodes();  // in input_boilerplate.h

[[deprecated]] const KeyCombo* keybinds_get_combo(Action action);
std::vector<KeyCombo> keybinds_get_combos(Action action);

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

struct ConflictInfo {
    bool active = false;
    Action target_action = Action::NONE;
    KeyCombo proposed_combo;
    uint32_t conflict_id = 0;
    Action conflict_action = Action::NONE;
};

bool keybinds_has_conflict();
const ConflictInfo& keybinds_get_conflict();
void keybinds_conflict_accept();
void keybinds_conflict_reject();

inline KeyCombo combo(std::initializer_list<uint16_t> req) {
    return KeyCombo{std::vector(req), {}};
}
inline KeyCombo combo_ex(std::initializer_list<uint16_t> req, std::initializer_list<uint16_t> ex) {
    return KeyCombo{std::vector(req), std::vector(ex)};
}

#endif /* KADR_INPUT_H */
