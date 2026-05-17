#ifndef SETTINGS_UI_H
#define SETTINGS_UI_H

#include "globals.h"

void settings_ui(App* app);

static void set_theme_kadr_dark() {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    // --- 1. Sizing and Spacing (Retaining your industrial/square layout) ---
    style.WindowPadding = ImVec2(10.0f, 10.0f);
    style.FramePadding = ImVec2(6.0f, 4.0f);
    style.ItemSpacing = ImVec2(8.0f, 4.0f);
    style.ScrollbarSize = 14.0f;
    style.GrabMinSize = 12.0f;

    // --- 2. Borders & Rounding ---
    style.WindowRounding = 2.0f;
    style.FrameRounding = 2.0f;
    style.PopupRounding = 2.0f;
    style.ScrollbarRounding = 2.0f;
    style.GrabRounding = 2.0f;
    style.TabRounding = 2.0f;

    style.WindowBorderSize = 1.0f;
    style.FrameBorderSize = 1.0f;
    style.PopupBorderSize = 1.0f;

    // --- 3. Gruvbox Dark Soft — Green-Tinted Palette ---
    // Aqua: #8ec07c | Green: #b8bb26 | BG2: #504945 | BG3: #665c54

    // Text
    colors[ImGuiCol_Text] = ImVec4(0.92f, 0.86f, 0.70f, 1.00f);          // #ebdbb2
    colors[ImGuiCol_TextDisabled] = ImVec4(0.57f, 0.51f, 0.45f, 1.00f);  // #928374

    // Backgrounds
    colors[ImGuiCol_WindowBg] = ImVec4(0.20f, 0.19f, 0.18f, 1.00f);  // #32302f
    colors[ImGuiCol_ChildBg] = ImVec4(0.20f, 0.19f, 0.18f, 0.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.20f, 0.19f, 0.18f, 0.95f);

    // Borders
    colors[ImGuiCol_Border] = ImVec4(0.31f, 0.29f, 0.27f, 1.00f);  // #504945
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

    // Frames
    colors[ImGuiCol_FrameBg] = ImVec4(0.24f, 0.22f, 0.21f, 1.00f);         // #3c3836
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.31f, 0.29f, 0.27f, 1.00f);  // #504945
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.40f, 0.36f, 0.33f, 1.00f);   // #665c54

    // Title Bars
    colors[ImGuiCol_TitleBg] = ImVec4(0.20f, 0.19f, 0.18f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.24f, 0.22f, 0.21f, 1.00f);  // #3c3836
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.20f, 0.19f, 0.18f, 1.00f);

    // Menus
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.24f, 0.22f, 0.21f, 1.00f);  // #3c3836

    // Scrollbars
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.20f, 0.19f, 0.18f, 1.00f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.29f, 0.27f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.40f, 0.36f, 0.33f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.49f, 0.44f, 0.39f, 1.00f);  // #7c6f64

    // Interactables
    colors[ImGuiCol_CheckMark] = ImVec4(0.72f, 0.73f, 0.15f, 1.00f);         // #b8bb26 Green
    colors[ImGuiCol_SliderGrab] = ImVec4(0.56f, 0.75f, 0.49f, 1.00f);        // #8ec07c Aqua
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.65f, 0.85f, 0.56f, 1.00f);  // Aqua brightened

    // Buttons — replaced jarring red with a calm stepped green/aqua progression
    colors[ImGuiCol_Button] = ImVec4(
        0.31f, 0.29f, 0.27f, 1.00f);  // #504945, same as border/frame hover — just a raised surface
    colors[ImGuiCol_ButtonHovered] =
        ImVec4(0.40f, 0.38f, 0.35f, 1.00f);  // slightly lighter warm grey, clearly interactive
    colors[ImGuiCol_ButtonActive] =
        ImVec4(0.49f, 0.46f, 0.42f, 1.00f);  // one more step up on press

    // Headers (selectables, tree nodes, collapsibles)
    colors[ImGuiCol_Header] = ImVec4(0.31f, 0.29f, 0.27f, 1.00f);  // #504945
    colors[ImGuiCol_HeaderHovered] =
        ImVec4(0.38f, 0.46f, 0.33f, 1.00f);  // subtle green tint on hover
    colors[ImGuiCol_HeaderActive] = ImVec4(0.44f, 0.55f, 0.36f, 1.00f);  // green-leaning active

    // Tabs
    colors[ImGuiCol_Tab] = ImVec4(0.24f, 0.22f, 0.21f, 1.00f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.35f, 0.43f, 0.28f, 1.00f);  // green tint on hover
    colors[ImGuiCol_TabActive] = ImVec4(0.31f, 0.40f, 0.27f, 1.00f);  // active tab has green warmth
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.20f, 0.19f, 0.18f, 1.00f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.26f, 0.30f, 0.23f, 1.00f);  // faint green
    colors[ImGuiCol_TabSelectedOverline] = ImVec4(0.72f, 0.73f, 0.15f, 1.00f);

    // Misc
    colors[ImGuiCol_PlotLines] =
        ImVec4(0.98f, 0.74f, 0.18f, 1.00f);  // #fabd2f Yellow (kept, good contrast)
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.40f, 0.36f, 0.33f, 1.00f);  // #665c54
    colors[ImGuiCol_NavHighlight] = ImVec4(0.56f, 0.75f, 0.49f, 1.00f);    // Aqua instead of red

#ifdef IMGUI_HAS_DOCK
    colors[ImGuiCol_DockingPreview] = ImVec4(0.72f, 0.73f, 0.15f, 0.50f);
    colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.20f, 0.19f, 0.18f, 1.00f);
#endif
}

#endif  //SETTINGS_UI_H
