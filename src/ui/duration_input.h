#ifndef KADR_DURATION_INPUT_H
#define KADR_DURATION_INPUT_H

#include <charconv>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include "../graphics.h"

inline std::optional<size_t> parse_duration(const std::string& dur) {
    auto stoi = [](std::string_view str) -> std::optional<size_t> {
        if (str.empty()) return std::nullopt;

        size_t value = 0;
        auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), value);

        if (ec == std::errc{} && ptr == str.data() + str.size()) { return value; }
        return std::nullopt;
    };

    auto trim = [](std::string_view s) {
        while (!s.empty() && std::isspace((unsigned char)s.front()))
            s.remove_prefix(1);
        while (!s.empty() && std::isspace((unsigned char)s.back()))
            s.remove_suffix(1);
        return s;
    };

    std::string_view sv = dur;
    trim(sv);
    if (dur.empty()) return std::nullopt;

    if (sv.ends_with("ms")) { return stoi(sv.substr(0, sv.size() - 2)); }
    if (sv.ends_with("s")) {
        return stoi(sv.substr(0, sv.size() - 1)).transform([](size_t v) { return v * 1000; });
    }
    if (sv.ends_with("min")) {
        return stoi(sv.substr(0, sv.size() - 3)).transform([](size_t v) { return v * 1000 * 60; });
    }
    if (sv.ends_with("h")) {
        return stoi(sv.substr(0, sv.size() - 1)).transform([](size_t v) {
            return v * 1000 * 60 * 60;
        });
    }

    return std::nullopt;
}

inline std::string format_duration(size_t ms) {
    if (ms == 0) return "0ms";

    if (ms % (1000 * 60 * 60) == 0) { return std::to_string(ms / (1000 * 60 * 60)) + "h"; }
    if (ms % (1000 * 60) == 0) { return std::to_string(ms / (1000 * 60)) + "min"; }
    if (ms % 1000 == 0) { return std::to_string(ms / 1000) + "s"; }

    return std::to_string(ms) + "ms";
}

inline bool duration_input(const char* label, size_t* out_ms) {
    struct State {
        std::string text;
        bool initialized = false;
        bool valid = true;
    };

    ImGuiID id = ImGui::GetID(label);

    static std::unordered_map<ImGuiID, State> s_states;
    State& st = s_states[id];

    if (!st.initialized) {
        st.text = format_duration(*out_ms);  // seed from val
        st.valid = true;
        st.initialized = true;
    }

    // --- Draw the InputText ------------------------------------------------
    bool value_changed = false;

    const bool show_error = !st.valid && !st.text.empty();
    if (show_error) {
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.85f, 0.15f, 0.15f, 1.00f));
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.25f, 0.06f, 0.06f, 1.00f));
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.30f, 0.08f, 0.08f, 1.00f));
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.35f, 0.10f, 0.10f, 1.00f));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.5f);
    }

    if (ImGui::InputText(label, &st.text)) {
        if (st.text.empty()) {
            st.valid = true;
        } else {
            auto result = parse_duration(st.text);
            if (result.has_value()) {
                *out_ms = result.value();
                st.valid = true;
                value_changed = true;
            } else {
                st.valid = false;
            }
        }
    }

    if (show_error) {
        ImGui::PopStyleVar();
        ImGui::PopStyleColor(4);
    }

    // --- Tooltip on hover --------------------------------------------------
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort)) {
        ImGui::BeginTooltip();
        ImGui::TextUnformatted("Accepted formats:");
        ImGui::Separator();
        ImGui::BulletText("500ms  →  500 milliseconds");
        ImGui::BulletText("3s     →  3 seconds");
        ImGui::BulletText("2min   →  2 minutes");
        ImGui::BulletText("1h     →  1 hour");
        if (show_error) ImGui::TextColored(ImVec4(1, 0.4f, 0.4f, 1), "Invalid input!");
        ImGui::EndTooltip();
    }

    // --- Inline error label ------------------------------------------------
    if (show_error) {
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.9f, 0.2f, 0.2f, 1.0f), "!");
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort)) {
            ImGui::SetTooltip("Invalid duration. Correct examples: 500ms, 3s, 2min, 1h");
        }
    }

    return value_changed;
}

#endif  // KADR_DURATION_INPUT_H
