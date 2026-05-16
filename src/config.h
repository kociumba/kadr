#ifndef CONFIG_H
#define CONFIG_H

#include <fstream>
#include <nlohmann/json.hpp>
#include "globals.h"
#include "input.h"
#include "paths.h"

inline bool config_save(const char* path) {
    nlohmann::json j;
    j["config"] = cfg;
    j["hotkeys"] = keybinds_to_json();
    std::ofstream f(path);
    if (!f) return false;
    f << j.dump(4);

    SDL_Log("config saved to %s", path);

    return true;
}

inline bool config_save() { return config_save(cfg_path.string().c_str()); }

inline bool config_load(const char* path) {
    std::ifstream f(path);
    if (!f) return false;
    auto j = nlohmann::json::parse(f, nullptr, false);
    if (j.is_discarded()) return false;
    if (j.contains("config")) cfg = j["config"];
    if (j.contains("hotkeys")) keybinds_from_json(j["hotkeys"]);

    SDL_Log("config loaded from %s", path);

    return true;
}

inline bool config_load() { return config_load(cfg_path.string().c_str()); }

#endif  //CONFIG_H
