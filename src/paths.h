#ifndef KADR_PATHS_H
#define KADR_PATHS_H

#include <SDL3/SDL_filesystem.h>
#include <filesystem>

namespace fs = std::filesystem;

[[nodiscard]] inline fs::path base_path() {
    const char* base = SDL_GetBasePath();
    if (!base) { return {}; }

    std::filesystem::path path{base};
    return path;
}

inline fs::path base;
inline fs::path cfg_path;
inline fs::path icon_path;
inline fs::path sc_path;
inline fs::path shutter_path;
inline fs::path lock_path;
inline fs::path font_path;

inline void init_paths() {
    base = base_path();
    cfg_path = base / "kadr_config.json";
    icon_path = base / "assets" / "kadr_icon.png";
    sc_path = base / "screenshots";
    shutter_path = base / "assets" / "shutter.wav";
    lock_path = base / "kadr.lock";
    font_path = base / "assets" / "MonaSans-Regular.ttf";
}

#endif  //KADR_PATHS_H
