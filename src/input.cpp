#include "input.h"
#include <algorithm>
#include <fstream>
#include <magic_enum/magic_enum_all.hpp>
#include <optional>
#include <ranges>
#include <sstream>
#include <unordered_set>

static KeyState g_state;
static std::vector<Binding> g_bindings;
static std::unordered_set<uint32_t> g_fired;
static bool g_capturing = false;
static Action g_capture_target = Action::NONE;
static std::vector<uint16_t> g_capture_peak;
static KeybindsChangedFn g_on_changed = nullptr;
static uint32_t g_next_id = 1;
static ConflictInfo g_conflict;

void init_keybinds() {
    g_state.clear();
    g_bindings.clear();
    g_fired.clear();
    g_conflict = {};
    g_next_id = 1;
    for (uint16_t code : all_keycodes()) {
        g_state[code] = false;
    }
}

void shutdown_keybinds() {
    g_state.clear();
    g_bindings.clear();
    g_fired.clear();
    g_conflict = {};
    g_next_id = 1;
}

static void keybinds_notify_changed() {
    if (g_on_changed) g_on_changed();
}

static void prune_fired() {
    for (auto it = g_fired.begin(); it != g_fired.end();) {
        uint32_t id = *it;
        if (std::ranges::find(g_bindings, id, &Binding::id) == g_bindings.end())
            it = g_fired.erase(it);
        else
            ++it;
    }
}

static bool combos_equal(const KeyCombo& a, const KeyCombo& b) {
    if (a.required.size() != b.required.size() || a.excluded.size() != b.excluded.size())
        return false;
    auto ra = a.required, rb = b.required;
    std::ranges::sort(ra);
    std::ranges::sort(rb);
    if (ra != rb) return false;
    auto ea = a.excluded, eb = b.excluded;
    std::ranges::sort(ea);
    std::ranges::sort(eb);
    return ea == eb;
}

static std::optional<Binding> find_conflict(const KeyCombo& combo, Action target_action) {
    for (const auto& b : g_bindings) {
        // if (b.action == target_action) continue;  // this checks doesn't really make sense
        if (b.action == Action::NONE) continue;
        if (combos_equal(b.combo, combo)) return b;
    }
    return std::nullopt;
}

void keybinds_unbind_silent(Action action) {
    std::erase_if(g_bindings, [action](const Binding& b) { return b.action == action; });
    prune_fired();
}

void keybinds_unbind_silent(uint32_t id) {
    std::erase_if(g_bindings, [id](const Binding& b) { return b.id == id; });
    g_fired.erase(id);
}

void keybinds_unbind_silent(Action action, const KeyCombo& combo) {
    std::erase_if(g_bindings, [action, &combo](const Binding& b) {
        return b.action == action && b.combo.required == combo.required &&
               b.combo.excluded == combo.excluded;
    });
    prune_fired();
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

            auto conflict = find_conflict(new_combo, g_capture_target);
            if (conflict) {
                g_conflict = {true, g_capture_target, new_combo, conflict->id, conflict->action};
                g_capturing = false;
                g_capture_target = Action::NONE;
                g_capture_peak.clear();
                return;
            }

            keybinds_bind(g_capture_target, new_combo);
            g_capturing = false;
            g_capture_target = Action::NONE;
            g_capture_peak.clear();
        }

        return;  // stop hotkey input for binding
    }

    if (!pressed) {
        for (auto it = g_fired.begin(); it != g_fired.end();) {
            uint32_t id = *it;
            auto bind_it = std::ranges::find(g_bindings, id, &Binding::id);
            if (bind_it != g_bindings.end()) {
                bool should_clear = false;
                for (uint16_t req : bind_it->combo.required) {
                    if (req == code) {
                        should_clear = true;
                        break;
                    }
                }
                if (should_clear) {
                    it = g_fired.erase(it);
                    continue;
                }
            }
            ++it;
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
        if (g_fired.contains(b.id)) continue;

        bool action_already_fired = false;
        for (uint32_t fid : g_fired) {
            auto it = std::ranges::find(g_bindings, fid, &Binding::id);
            if (it != g_bindings.end() && it->action == b.action) {
                action_already_fired = true;
                break;
            }
        }
        if (action_already_fired) continue;

        if (combo_matches(b.combo, g_state)) {
            g_fired.insert(b.id);
            return b.action;
        }
    }
    return Action::NONE;
}

const KeyState& keybinds_state() { return g_state; }

uint32_t keybinds_bind_silent(Action action, const KeyCombo& combo) {
    uint32_t id = g_next_id++;
    g_bindings.push_back(Binding{id, combo, action});
    return id;
}

uint32_t keybinds_bind(Action action, const KeyCombo& combo) {
    const auto id = keybinds_bind_silent(action, combo);
    keybinds_notify_changed();
    return id;
}

void keybinds_unbind(Action action) {
    size_t before = g_bindings.size();
    keybinds_unbind_silent(action);
    if (g_bindings.size() < before) keybinds_notify_changed();
}

void keybinds_unbind(uint32_t id) {
    size_t before = g_bindings.size();
    keybinds_unbind_silent(id);
    if (g_bindings.size() < before) keybinds_notify_changed();
}

void keybinds_unbind(Action action, const KeyCombo& combo) {
    size_t before = g_bindings.size();
    keybinds_unbind_silent(action, combo);
    if (g_bindings.size() < before) keybinds_notify_changed();
}

const std::vector<Binding>& keybinds_get_bindings() { return g_bindings; }

const KeyCombo* keybinds_get_combo(Action action) {
    for (const auto& b : g_bindings) {
        if (b.action == action) return &b.combo;
    }
    return nullptr;
}

std::vector<KeyCombo> keybinds_get_combos(Action action) {
    std::vector<KeyCombo> result;
    for (const auto& b : g_bindings) {
        if (b.action == action) result.push_back(b.combo);
    }
    return result;
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
    g_next_id = 1;

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

        g_bindings.push_back(Binding{g_next_id++, c, static_cast<Action>(act_idx)});
    }
    return true;
}

nlohmann::json keybinds_to_json() {
    nlohmann::json j;
    for (const auto& b : g_bindings) {
        std::string name(magic_enum::enum_name(b.action));
        nlohmann::json combo_json;
        combo_json["required"] = b.combo.required;
        if (!b.combo.excluded.empty()) combo_json["excluded"] = b.combo.excluded;
        j[name].push_back(combo_json);
    }
    return j;
}

void keybinds_from_json(const nlohmann::json& j) {
    for (auto& [key, val] : j.items()) {
        auto action = magic_enum::enum_cast<Action>(key);
        if (!action) continue;
        keybinds_unbind_silent(*action);
    }

    auto load_combo = [](const nlohmann::json& obj) -> KeyCombo {
        KeyCombo c;
        c.required = obj.value("required", std::vector<uint16_t>{});
        c.excluded = obj.value("excluded", std::vector<uint16_t>{});
        return c;
    };

    for (auto& [key, val] : j.items()) {
        auto action = magic_enum::enum_cast<Action>(key);
        if (!action) continue;

        if (val.is_array()) {
            for (const auto& combo_json : val) {
                g_bindings.push_back(Binding{g_next_id++, load_combo(combo_json), *action});
            }
        } else if (val.is_object()) {
            g_bindings.push_back(Binding{g_next_id++, load_combo(val), *action});
        }
    }

    g_fired.clear();
}

bool keybinds_has_conflict() { return g_conflict.active; }
const ConflictInfo& keybinds_get_conflict() { return g_conflict; }

void keybinds_conflict_accept() {
    if (!g_conflict.active) return;
    // keybinds_unbind_silent(g_conflict.conflict_id);  // for now leave the conflicting bind
    keybinds_bind_silent(g_conflict.target_action, g_conflict.proposed_combo);
    keybinds_notify_changed();
    g_conflict.active = false;
}

void keybinds_conflict_reject() { g_conflict.active = false; }
