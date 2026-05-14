#include "settings_ui.h"
#include <magic_enum_all.hpp>
#include <ranges>
#include "auto_run.h"
#include "config.h"
#include "hijack.h"
#include "input.h"

static std::string format_combo(const KeyCombo* combo, bool capturing) {
    if (capturing) {
        const auto& peak = keybinds_capture_peak();
        if (peak.empty()) return "[press keys...]";
        std::string s;
        for (size_t i = 0; i < peak.size(); ++i) {
            if (i) s += " + ";
            const char* name = keycode_name(peak[i]);
            s += name ? name : "?";
        }
        return s;
    }
    if (!combo || combo->required.empty()) return "[click to bind]";
    std::string s;
    for (size_t i = 0; i < combo->required.size(); ++i) {
        if (i) s += " + ";
        const char* name = keycode_name(combo->required[i]);
        s += name ? name : "?";
    }
    return s;
}

#ifdef _WIN32
static HWND sdl3_get_hwnd(SDL_Window* window) {
    SDL_PropertiesID props = SDL_GetWindowProperties(window);
    if (!props) return nullptr;
    return (HWND)SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr);
}

static void notify_wm_drag_start(SDL_Window* window) {
    if (HWND hwnd = sdl3_get_hwnd(window)) {
        NotifyWinEvent(EVENT_SYSTEM_MOVESIZESTART, hwnd, OBJID_WINDOW, CHILDID_SELF);
    }
}

static void notify_wm_drag_end(SDL_Window* window) {
    if (HWND hwnd = sdl3_get_hwnd(window)) {
        NotifyWinEvent(EVENT_SYSTEM_MOVESIZEEND, hwnd, OBJID_WINDOW, CHILDID_SELF);
    }
}

static void notify_wm_moved(SDL_Window* window) {
    if (HWND hwnd = sdl3_get_hwnd(window)) {
        NotifyWinEvent(EVENT_OBJECT_LOCATIONCHANGE, hwnd, OBJID_WINDOW, CHILDID_SELF);
    }
}
#endif

void settings_ui(App* app) {
    constexpr float TITLE_H = 32.0f;
    ImVec2 win_size = ImGui::GetIO().DisplaySize;
    auto* bg = ImGui::GetBackgroundDrawList();

    // --- titlebar background ---
    bg->AddRectFilled({0, 0},
        {win_size.x, TITLE_H},
        ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[ImGuiCol_MenuBarBg]));
    bg->AddText({12, (TITLE_H - ImGui::GetTextLineHeight()) * 0.5f},
        IM_COL32(220, 220, 220, 255),
        "kadr | settings");

    ImGui::SetNextWindowPos({0, 0});
    ImGui::SetNextWindowSize({win_size.x, TITLE_H});
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0, 0});
    ImGui::Begin("##titlebar",
        nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse |
            ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoBringToFrontOnFocus);

    ImGui::SetCursorPos({0, 0});
    ImGui::InvisibleButton("##drag_handle", {win_size.x - TITLE_H, TITLE_H});

    static int dragOffsetX, dragOffsetY;
    static bool was_dragging = false;

    if (ImGui::IsItemHovered()) { ImGui::SetMouseCursor(ImGuiMouseCursor_Hand); }

    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && ImGui::IsItemHovered()) {
        float mouseInWindowX, mouseInWindowY;
        SDL_GetMouseState(&mouseInWindowX, &mouseInWindowY);
        dragOffsetX = (int)mouseInWindowX;
        dragOffsetY = (int)mouseInWindowY;

#ifdef _WIN32
        notify_wm_drag_start(app->window);
#endif
    }

    bool is_dragging = ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left);

    if (is_dragging) {
        float globalMouseX, globalMouseY;
        SDL_GetGlobalMouseState(&globalMouseX, &globalMouseY);

        SDL_SetWindowPosition(
            app->window, (int)globalMouseX - dragOffsetX, (int)globalMouseY - dragOffsetY);

        was_dragging = true;

#ifdef _WIN32
        notify_wm_moved(app->window);
#endif
    } else if (was_dragging) {
        was_dragging = false;
#ifdef _WIN32
        notify_wm_drag_end(app->window);
#endif
    }

    // close button — top right, fires a clean close without the SC teardown path
    ImGui::SetCursorPos({win_size.x - TITLE_H, 0});
    ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(196, 43, 28, 255));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(160, 30, 20, 255));
    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(220, 220, 220, 255));
    if (ImGui::Button("x", {TITLE_H, TITLE_H})) { app->pending_close = true; }
    ImGui::PopStyleColor(4);

    ImGui::End();
    ImGui::PopStyleVar();

    // --- settings content ---
    ImGui::SetNextWindowPos({0, TITLE_H});
    ImGui::SetNextWindowSize({win_size.x, win_size.y - TITLE_H});
    ImGui::SetNextWindowBgAlpha(1.0f);
    ImGui::Begin("##settings_content",
        nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus);

    ImGui::SeparatorText("capture");
    if (ImGui::Checkbox("copy to clipboard after capture", &cfg.copy_to_clipboard))
        config_save("kadr_config.json");
    if (ImGui::Checkbox("save to disk after capture", &cfg.save_to_disk))
        config_save("kadr_config.json");

    if (cfg.copy_to_clipboard == false && cfg.save_to_disk == false) {
        ImGui::TextColored(ImGui::GetStyle().Colors[ImGuiCol_ButtonHovered],
            "You WILL lose all screenshots taken !!!");
    }

    ImGui::SeparatorText("output");
    ImGui::SetNextItemWidth(300);
    if (ImGui::InputText("save folder", &cfg.save_path)) config_save("kadr_config.json");

    ImGui::SeparatorText("hotkeys");

    for (auto action : magic_enum::enum_values<Action>()) {
        if (action == Action::NONE || action == Action::COUNT || action == Action::CLOSE_WINDOW)
            continue;

        ImGui::PushID(static_cast<int>(action));
        ImGui::Text("%s", action_name(action));

        std::vector<KeyCombo> combos = keybinds_get_combos(action);

        bool any_capturing = keybinds_is_capturing() && keybinds_capture_target() == action;

        const auto& all_bindings = keybinds_get_bindings();
        int slot_index = 0;
        for (const auto& binding : all_bindings) {
            if (binding.action != action) continue;

            ImGui::PushID(binding.id);

            bool is_this_capturing = any_capturing && slot_index == 0;
            std::string label = format_combo(&binding.combo, false);

            ImGui::SameLine(slot_index == 0 ? 180.0f : 0.0f);
            if (ImGui::Button(label.c_str(), {200, 0})) {
                keybinds_unbind(binding.id);
                keybinds_start_capture(action);
            }
            ImGui::SameLine();
            if (ImGui::SmallButton("x")) { keybinds_unbind(binding.id); }

            ++slot_index;
            ImGui::PopID();
        }

        if (!any_capturing) {
            ImGui::SameLine(slot_index == 0 ? 180.0f : 0.0f);
            ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(50, 50, 50, 180));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(70, 70, 70, 200));
            if (ImGui::Button("[press keys...]", {200, 0})) { keybinds_start_capture(action); }
            ImGui::PopStyleColor(2);
        } else {
            ImGui::SameLine(slot_index == 0 ? 180.0f : 0.0f);
            ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(80, 120, 80, 255));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(100, 150, 100, 255));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(80, 120, 80, 255));
            std::string capture_label = format_combo(nullptr, true);
            if (ImGui::Button(capture_label.c_str(), {200, 0})) { keybinds_cancel_capture(); }
            ImGui::PopStyleColor(3);
            ImGui::SameLine();
            ImGui::TextDisabled("[press combo, release to set]");
        }

        ImGui::PopID();
    }

    if (keybinds_is_capturing() && ImGui::IsKeyPressed(ImGuiKey_Escape)) {
        keybinds_cancel_capture();
    }

    ImGui::SeparatorText("behaviour");
    static bool fail_warning = false;
    if (ImGui::Checkbox("start on login", &cfg.start_on_login)) {
        fail_warning = false;
        bool ok = cfg.start_on_login ? add_to_autostart() : remove_from_autostart();
        if (ok) {
            config_save("kadr_config.json");
        } else {
            cfg.start_on_login = !cfg.start_on_login;
            fail_warning = true;
        }
    }
    if (fail_warning) {
        ImGui::TextColored(
            {0.984f, 0.286f, 0.204f, 1.00f}, "failed to add/remove kadr from startup");
    }

    ImGui::Spacing();

    ImGui::SeparatorText("advanced");
    ImGui::PushStyleColor(ImGuiCol_Text, {0.98f, 0.74f, 0.18f, 1.00f});
    bool exp_open = ImGui::TreeNodeEx("Experimental features");
    ImGui::PopStyleColor();

    if (exp_open) {
        ImGui::TextColored({0.996f, 0.502f, 0.098f, 0.90f},
            "These options may be unstable or behave unexpectedly.");

        ImGui::Spacing();

        if (ImGui::Checkbox("hide mouse cursor in screenshots", &cfg.hide_cursor))
            config_save("kadr_config.json");

#ifdef _WIN32
        constexpr bool can_hijack_prtsc = true;
#else
        constexpr bool can_hijack_prtsc = false;
#endif

        if (!can_hijack_prtsc) ImGui::BeginDisabled();

        static bool hijack_change = false;
        static bool hijack_fail = false;
        if (ImGui::Checkbox("hijack the print screen key", &cfg.hijack_prtsc)) {
            hijack_fail = false;
            hijack_change = false;
            bool ok = cfg.hijack_prtsc ? disable_prtsc_snip() : enable_prtsc_snip();
            if (ok) {
                config_save("kadr_config.json");
                hijack_change = true;
            } else {
                cfg.hijack_prtsc = !cfg.hijack_prtsc;
                hijack_fail = true;
            }
        }

        if (!can_hijack_prtsc) {
            ImGui::EndDisabled();
            ImGui::SameLine();
            ImGui::TextDisabled("(Windows only)");
        }

        if (cfg.hijack_prtsc && can_hijack_prtsc) {
            ImGui::Text("you can now use print screen in your bindings");
        }

        if (hijack_change) {
            ImGui::TextColored({0.984f, 0.286f, 0.204f, 1.00f},
                "restart explorer or the system if changes don't apply\n"
                "ignore if already applied");
        }

        if (hijack_fail) {
            ImGui::TextColored(
                {0.984f, 0.286f, 0.204f, 1.00f}, "failed to hijack/release print screen");
        }

        ImGui::TreePop();
    }

    ImGui::End();
}
