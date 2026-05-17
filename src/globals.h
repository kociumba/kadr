#ifndef GLOBALS_H
#define GLOBALS_H

#include <filesystem>
#include <nlohmann/json.hpp>
#include <string>
#include <thread>

#include "graphics.h"
#include "paths.h"
#include "version.h"

#if !defined(KADR_EXEC)
#define KADR_EXEC ""
#endif

constexpr ImVec2 inv_pos = {FLT_MAX, FLT_MAX};
static const std::string sep_str(1, fs::path::preferred_separator);
static const char* sep = sep_str.c_str();

enum KadrMode { IDLE, SC, SETTINGS };

struct App {
    KadrMode mode = IDLE;

    SDL_Window* window = nullptr;
    SDL_GLContext gl_context = nullptr;
    bool running = true;
    bool pending_close = false;
    bool save_full = false;
    std::thread hook_thread;
    SDL_Surface* shot = nullptr;
    ImTextureID shot_tex = 0;
    SDL_Tray* tray = nullptr;
    SDL_Surface* icon = nullptr;

    int vd_min_x = 0;
    int vd_min_y = 0;

    ImVec2 start, drag = inv_pos;
    bool dragging = false;
};

enum class SCMode { Region, Window };
inline uint32_t WAKE_UP;  // sdl user event for waking up the main thread

struct CFG {
    bool copy_to_clipboard = true;
    bool save_to_disk = true;
    std::string save_path = sc_path.string();
    bool start_on_login = false;
    SCMode sc_mode = SCMode::Region;
    bool hide_cursor = false;
    bool hijack_prtsc = false;
    bool play_capture_sound = true;
    std::string capture_sound_path = shutter_path.string();

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(CFG,
        copy_to_clipboard,
        save_to_disk,
        save_path,
        start_on_login,
        sc_mode,
        hide_cursor,
        hijack_prtsc,
        play_capture_sound,
        capture_sound_path)
};

extern CFG cfg;

#endif  //GLOBALS_H
