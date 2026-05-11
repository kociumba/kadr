#include "toolbar.h"

#include <magic_enum.hpp>
#include "window_cords.h"
#include "window_info.h"

void toolbar(App* app) {
    SDL_DisplayID primary = SDL_GetPrimaryDisplay();
    SDL_Rect main_bounds;

    if (!SDL_GetDisplayBounds(primary, &main_bounds)) { return; }

    constexpr float TOOLBAR_HEIGHT = 48.0f;
    const ImGuiStyle& s = ImGui::GetStyle();

    ImVec2 toolbar_pos(main_bounds.x - app->vd_min_x + main_bounds.w / 2.0f,
        main_bounds.y - app->vd_min_y + 8.0f + TOOLBAR_HEIGHT / 2);

    ImGui::SetNextWindowPos(toolbar_pos, ImGuiCond_Always, {.5, .5});
    ImGui::SetNextWindowSize({0.0, TOOLBAR_HEIGHT}, ImGuiCond_Always);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);

    ImGui::Begin("sc_toolbar",
        nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    ImGui::SetCursorPosY((TOOLBAR_HEIGHT - ImGui::GetFrameHeight()) * 0.5f);

    ImGui::SetNextItemWidth(120.0f);

    auto preview = magic_enum::enum_name(cfg.sc_mode);
    if (ImGui::BeginCombo("##sc_mode", preview.data())) {
        for (SCMode mode : magic_enum::enum_values<SCMode>()) {
            bool selected = cfg.sc_mode == mode;
            auto name = magic_enum::enum_name(mode);
            if (ImGui::Selectable(name.data(), selected)) { cfg.sc_mode = mode; }
            if (selected) { ImGui::SetItemDefaultFocus(); }
        }
        ImGui::EndCombo();
    }

    ImGui::SameLine();
    ImGui::Dummy(ImVec2(12.0f, 0.0f));
    ImGui::SameLine();

    if (ImGui::Button("All Monitors")) { app->save_full = true; }

    ImGui::SameLine();
    ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);

    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.1f, 0.1f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.5f, 0.05f, 0.05f, 1.0f));
    if (ImGui::Button("Cancel")) {
        app->pending_close = true;
        app->dragging = false;
    }
    ImGui::PopStyleColor(3);

    ImGui::End();
    ImGui::PopStyleVar(1);
}

void drag_ui(App* app) {
    auto m = ImGui::GetMousePos();
    if (!app->shot) return;

    const float zoom_level = 15.0f;
    const float zoom_radius = 8.0f;
    const float gap = 8.0f;
    const float mouse_pad = 15.0f;

    ImVec2 tex_size = ImVec2((float)app->shot->w, (float)app->shot->h);
    ImVec2 zoom_size = ImVec2(zoom_radius * zoom_level * 2.0f, zoom_radius * zoom_level * 2.0f);

    bool dragging_right = app->drag.x >= app->start.x;

    ImVec2 zoom_pos;
    if (dragging_right) {
        zoom_pos.x = m.x + mouse_pad;
    } else {
        zoom_pos.x = m.x - mouse_pad - zoom_size.x;
    }
    zoom_pos.y = m.y - (zoom_size.y / 2);

    ImVec2 zoom_max = ImVec2(zoom_pos.x + zoom_size.x, zoom_pos.y + zoom_size.y);
    ImVec2 c = ImVec2(zoom_pos.x + zoom_size.x * 0.5f, zoom_pos.y + zoom_size.y * 0.5f);

    auto* fg = ImGui::GetForegroundDrawList();
    if (!fg) return;

    ImVec2 center_uv = ImVec2((m.x + 0.5f) / tex_size.x, (m.y + 0.5f) / tex_size.y);
    ImVec2 half_size_uv = ImVec2(zoom_radius / tex_size.x, zoom_radius / tex_size.y);

    ImVec2 uv0 = ImVec2(center_uv.x - half_size_uv.x, center_uv.y - half_size_uv.y);
    ImVec2 uv1 = ImVec2(center_uv.x + half_size_uv.x, center_uv.y + half_size_uv.y);

    uv0 = ImVec2(ImClamp(uv0.x, 0.0f, 1.0f), ImClamp(uv0.y, 0.0f, 1.0f));
    uv1 = ImVec2(ImClamp(uv1.x, 0.0f, 1.0f), ImClamp(uv1.y, 0.0f, 1.0f));

    fg->AddRectFilled(zoom_pos, zoom_max, IM_COL32(0, 0, 0, 180), 8.0f);
    fg->AddImageRounded(
        app->shot_tex, zoom_pos, zoom_max, uv0, uv1, IM_COL32(255, 255, 255, 255), 8.0f);

    const uint8_t alpha_max = 190;
    const float max_dist = zoom_size.x * 0.5f;
    const float half_cell = zoom_level * 0.5f;
    const ImU32 clr = IM_COL32(0, 0, 0, 0);

    for (int i = -zoom_radius; i <= zoom_radius; ++i) {
        float x = c.x + i * zoom_level + half_cell;
        if (x < zoom_pos.x || x > zoom_max.x) continue;

        float tx = fabsf(x - c.x) / max_dist;
        uint8_t ax = (uint8_t)(alpha_max * (1.0f - tx * tx));
        ImU32 col = IM_COL32(0, 0, 0, ax);
        fg->AddRectFilledMultiColor(
            ImVec2(x, zoom_pos.y), ImVec2(x + 1.0f, c.y), clr, clr, col, col);

        fg->AddRectFilledMultiColor(
            ImVec2(x, c.y), ImVec2(x + 1.0f, zoom_max.y), col, col, clr, clr);
    }

    for (int j = -zoom_radius; j <= zoom_radius; ++j) {
        float y = c.y + j * zoom_level + half_cell;
        if (y < zoom_pos.y || y > zoom_max.y) continue;

        float ty = fabsf(y - c.y) / max_dist;
        uint8_t ay = (uint8_t)(alpha_max * (1.0f - ty * ty));
        ImU32 col = IM_COL32(0, 0, 0, ay);

        fg->AddRectFilledMultiColor(
            ImVec2(zoom_pos.x, y), ImVec2(c.x, y + 1.0f), clr, col, col, clr);

        fg->AddRectFilledMultiColor(
            ImVec2(c.x, y), ImVec2(zoom_max.x, y + 1.0f), col, clr, clr, col);
    }

    ImVec2 cp_min = ImVec2(c.x - zoom_level * 0.5f, c.y - zoom_level * 0.5f);
    ImVec2 cp_max = ImVec2(c.x + zoom_level * 0.5f, c.y + zoom_level * 0.5f);
    fg->AddRectFilled(cp_min, cp_max, IM_COL32(255, 255, 255, 40));
    fg->AddRect(cp_min, cp_max, IM_COL32(255, 255, 255, 230), 0.0f, 0, 1.5f);

    fg->AddRect(zoom_pos, zoom_max, IM_COL32(255, 255, 255, 220), 8.0f, 0, 1.5f);

    ImVec2 top = ImVec2(zoom_pos.x + gap, zoom_pos.y + gap);
    auto inf = std::format("x:{:.0f}, y:{:.0f}", m.x, m.y);
    fg->AddText(top, IM_COL32(255, 255, 255, 255), inf.c_str());
    fg->AddText(ImVec2(top.x, top.y + ImGui::GetTextLineHeightWithSpacing()),
        IM_COL32(255, 255, 255, 255),
        "right click to cancel dragging");
}

void wnd_info(App* app) {
    float mx, my;
    SDL_GetMouseState(&mx, &my);
    auto wnd = window_at({mx + app->vd_min_x, my + app->vd_min_y});
    if (!wnd) return;

    auto info = get_window_info(*wnd);

    float margin = 8.0f;
    float info_h = 50.0f;

    float rel_top = wnd->position.y - app->vd_min_y;
    float rel_bottom = rel_top + wnd->size.y;
    float wnd_h = wnd->size.y;

    float info_y;
    if (rel_top - margin - info_h >= 0) {
        info_y = rel_top - margin - info_h / 2;
    } else if (wnd_h >= info_h + margin * 2) {
        info_y = rel_top + margin + info_h / 2;
    } else {
        info_y = rel_bottom + margin + info_h / 2;
    }

    ImVec2 mid = {
        wnd->position.x - app->vd_min_x + wnd->size.x / 2,
        info_y,
    };

    ImGui::SetNextWindowPos(mid, ImGuiCond_Always, {0.5, 0.5});

    ImGui::Begin("wnd_info",
        nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoMouseInputs | ImGuiWindowFlags_NoInputs);

    ImGui::Text("%s (%s) : %d", info.title.c_str(), info.process_name.c_str(), info.pid);

    ImGui::End();
}