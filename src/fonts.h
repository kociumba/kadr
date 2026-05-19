#ifndef KADR_FONTS_H
#define KADR_FONTS_H

#include <string>
#include <unordered_map>
#include "graphics.h"

namespace fonts {

inline std::unordered_map<std::string, std::unordered_map<size_t, ImFont*>> font_cache;

inline void clear_cache() { font_cache.clear(); }

inline ImFont* load(const std::string& path = "default", size_t size = 13) {
    if (font_cache.contains(path)) {
        if (font_cache[path].contains(size)) { return font_cache[path][size]; }
    }

    auto name = path.substr(path.find_last_of(sep) + 1, path.length());
    ImFontConfig c;
    size_t len = std::min(name.length(), (size_t)39);
    name.copy(c.Name, len);
    c.Name[len] = '\0';
    c.SizePixels = size;
    // cfg.MergeMode = true;  // actually use this
    c.OversampleH = 1;
    c.OversampleV = 1;

    ImFont* f;
    if (path == "default") {
        f = ImGui::GetIO().Fonts->AddFontDefault(&c);
    } else {
        f = ImGui::GetIO().Fonts->AddFontFromFileTTF(path.c_str(), size, &c);
    }

    if (f) {
        font_cache[path][size] = f;
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
            "loaded font: %s at size %zu from '%s'",
            name.c_str(),
            size,
            path.c_str());
    } else {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
            "failed to load font %s at size %zu from '%s'",
            name.c_str(),
            size,
            path.c_str());
    }

    return f;
}

inline ImFont* use(const std::string& path = "default", size_t size = 13) {
    auto f = load(path, size);

    if (f) {
        cfg.ui_font_path = path;
        cfg.ui_font_size = size;
    }

    return f;
}

}  // namespace fonts

#endif  //KADR_FONTS_H
