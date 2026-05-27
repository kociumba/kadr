#ifndef KADR_FONTS_H
#define KADR_FONTS_H

#include <imgui.h>
#include <string>
#include <unordered_map>
#include "globals.h"
#include "run_on_main.h"

namespace fonts {

inline std::unordered_map<std::string, ImFont*> font_cache;

inline void clear_cache() {
    dispatch([] {  // dispatch it to the main thread so that it runs between frames
        ImGui::GetIO().Fonts->Clear();
        font_cache.clear();
    });
}

inline ImFont* load(const std::string& path = "default") {
    if (font_cache.contains(path)) return font_cache[path];

    auto name = path.substr(path.find_last_of(sep) + 1, path.length());
    ImFontConfig c;
    size_t len = std::min(name.length(), (size_t)39);
    name.copy(c.Name, len);
    c.Name[len] = '\0';
    // c.MergeMode = true;  // actually use this
    c.OversampleH = 1;
    c.OversampleV = 1;

    ImFont* f;
    if (path == "default") {
        f = ImGui::GetIO().Fonts->AddFontDefault(&c);
    } else {
        f = ImGui::GetIO().Fonts->AddFontFromFileTTF(path.c_str(), 0.0f, &c);
    }

    if (f) {
        font_cache[path] = f;
        SDL_LogInfo(
            SDL_LOG_CATEGORY_APPLICATION, "loaded font: %s from '%s'", name.c_str(), path.c_str());
    } else {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
            "failed to load font: %s from '%s'",
            name.c_str(),
            path.c_str());
    }

    ImGui::GetIO().Fonts->CompactCache();

    return f;
}

inline ImFont* use(const std::string& path = "default", float size = 0.0f) {
    auto f = load(path);
    if (f) {
        cfg.ui_font_path = path;
        cfg.ui_font_size = (size > 0.0f) ? size : ImGui::GetStyle().FontSizeBase;
    }
    return f;
}

inline float scalar() { return ImGui::GetStyle().FontSizeBase / 18.0f; }

}  // namespace fonts

#endif  //KADR_FONTS_H
