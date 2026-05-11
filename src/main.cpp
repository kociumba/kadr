#include <clip/clip.h>
#include <inttypes.h>
#include <stdio.h>
#include <atomic>
#include <chrono>
#include <filesystem>
#include <format>
#include <print>
#include <string>
#include <thread>
#include "capture.h"
#include "config.h"
#include "globals.h"
#include "graphics.h"
#include "input.h"
#include "lock.h"
#include "reflection.h"
#include "settings_ui.h"
#include "textures.h"
#include "toolbar.h"
#include "window_cords.h"

#if defined(__APPLE__)
#include <TargetConditionals.h>
#endif

namespace fs = std::filesystem;

CFG cfg = {};

static std::atomic g_open_requested_sc{false};
static std::atomic g_open_requested_settings{false};
static std::atomic g_close_requested{false};
static std::string g_last_screenshot_path;

bool ensure_dir(const std::string& path) {
    std::error_code ec;

    if (fs::exists(path, ec)) {
        if (fs::is_directory(path, ec)) {
            return true;
        } else {
            return false;
        }
    }

    if (ec && ec != std::errc::no_such_file_or_directory) { return false; }

    ec.clear();
    if (fs::create_directories(path, ec)) { return true; }

    return !ec;
}

bool logger_proc(unsigned int level, const char* format, ...) {
    bool status = false;
    va_list args;
    va_start(args, format);

    switch (level) {
        case LOG_LEVEL_INFO:
            printf("[INFO] ");
            status = vfprintf(stdout, format, args) >= 0;
            break;
        case LOG_LEVEL_WARN:
            fprintf(stderr, "[WARN] ");
            status = vfprintf(stderr, format, args) >= 0;
            break;
        case LOG_LEVEL_ERROR:
            fprintf(stderr, "[ERROR] ");
            status = vfprintf(stderr, format, args) >= 0;
            break;
    }

    va_end(args);
    return status;
}

// this is fucked, i have to track the mask myself
static void uiohook_dispatch(uiohook_event* const event) {
    keybinds_on_event(event);

    if (keybinds_is_capturing()) return;
    if (event->type == EVENT_KEY_PRESSED) {
        switch (keybinds_poll()) {
            case Action::TAKE_SCREENSHOT:
                SDL_Log("screenshot triggered\n");
                g_open_requested_sc.store(true);
                break;
            case Action::OPEN_SETTINGS:
                SDL_Log("settings opened\n");
                g_open_requested_settings.store(true);
                break;
            case Action::CLOSE_WINDOW:
                // SDL_Log("window close requested\n");
                g_close_requested.store(true);
                break;
            case Action::QUIT_KADR:
                SDL_Event quit_event;
                quit_event.type = SDL_EVENT_QUIT;
                SDL_PushEvent(&quit_event);
                break;
            default:
                break;
        }
    }
}

static void hook_thread_fn() {
    hook_set_logger_proc(&logger_proc);
    hook_set_dispatch_proc(&uiohook_dispatch);
    int status = hook_run();  // blocks until hook_stop()
    if (status != UIOHOOK_SUCCESS) { fprintf(stderr, "libuiohook error: %d\n", status); }
}

static void SetupGLAttributes() {
#if defined(__APPLE__) && TARGET_OS_OSX
    // macOS: 4.1 Core + forward compatible
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
#else
    // Windows/Linux: 4.5 Core
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
#endif

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
}

struct WindowConfig {
    const char* title;
    int w, h;
    SDL_WindowFlags flags;
    bool fullscreen_span;
};

static WindowConfig GetWindowConfig(KadrMode mode) {
    float dpi = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());

    switch (mode) {
        case SC:
            return {"kadr | screenshot",
                (int)(1280 * dpi),
                (int)(720 * dpi),
                SDL_WINDOW_OPENGL | SDL_WINDOW_HIGH_PIXEL_DENSITY | SDL_WINDOW_ALWAYS_ON_TOP |
                    SDL_WINDOW_BORDERLESS | SDL_WINDOW_TRANSPARENT | SDL_WINDOW_UTILITY |
                    SDL_WINDOW_HIDDEN,
                true};
        case SETTINGS:
            return {"kadr | settings",
                (int)(640 * dpi),
                (int)(480 * dpi),
                SDL_WINDOW_OPENGL | SDL_WINDOW_HIGH_PIXEL_DENSITY | SDL_WINDOW_BORDERLESS |
                    SDL_WINDOW_HIDDEN,
                false};
        default:
            return {};
    }
}

static bool CreateGLContext(App* app, KadrMode mode) {
    if (app->gl_context) return true;

    WindowConfig cfg = GetWindowConfig(mode);

    app->window = SDL_CreateWindow(cfg.title, cfg.w, cfg.h, cfg.flags);
    if (!app->window) {
        SDL_Log("Failed to create window: %s", SDL_GetError());
        return false;
    }

    app->gl_context = SDL_GL_CreateContext(app->window);
    if (!app->gl_context) {
        SDL_Log("Failed to create GL context: %s", SDL_GetError());
        SDL_DestroyWindow(app->window);
        app->window = nullptr;
        return false;
    }

    SDL_GL_MakeCurrent(app->window, app->gl_context);
    SDL_GL_SetSwapInterval(1);

    ImGui_ImplSDL3_InitForOpenGL(app->window, app->gl_context);
    ImGui_ImplOpenGL3_Init();

    app->mode = mode;
    logger_proc(LOG_LEVEL_INFO, "OpenGL context created and ImGui initialized.\n");
    return true;
}

static bool OpenWindow(App* app, KadrMode mode) {
    if (!app->window) {
        SDL_Log("Window does not exist. Call CreateGLContext first.");
        return false;
    }

    WindowConfig cfg = GetWindowConfig(mode);

    if (cfg.fullscreen_span) {
        int num_displays;
        SDL_DisplayID* displays = SDL_GetDisplays(&num_displays);

        if (displays && num_displays > 0) {
            int min_x = INT_MAX, min_y = INT_MAX;
            int max_x = INT_MIN, max_y = INT_MIN;

            for (int i = 0; i < num_displays; i++) {
                SDL_Rect bounds;
                if (SDL_GetDisplayBounds(displays[i], &bounds)) {
                    if (bounds.x < min_x) min_x = bounds.x;
                    if (bounds.y < min_y) min_y = bounds.y;
                    if (bounds.x + bounds.w > max_x) max_x = bounds.x + bounds.w;
                    if (bounds.y + bounds.h > max_y) max_y = bounds.y + bounds.h;
                }
            }

            app->vd_min_x = min_x;
            app->vd_min_y = min_y;
            SDL_Log("virtual space | x: %d y: %d", app->vd_min_x, app->vd_min_y);

            SDL_SetWindowPosition(app->window, min_x, min_y);
            SDL_SetWindowSize(app->window, max_x - min_x, max_y - min_y);
            SDL_free(displays);
        } else {
            SDL_SetWindowPosition(app->window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
        }
    } else {
        SDL_SetWindowPosition(app->window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
        SDL_SetWindowSize(app->window, cfg.w, cfg.h);
    }

    SDL_ShowWindow(app->window);
    SDL_RaiseWindow(app->window);

    app->mode = mode;
    std::string name = std::string(magic_enum::enum_name(mode));
    logger_proc(LOG_LEVEL_INFO, "Window opened in mode %s.\n", name.c_str());
    return true;
}

static void CloseWindow(App* app) {
    if (!app->window) return;

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();

    if (app->gl_context) {
        SDL_GL_DestroyContext(app->gl_context);
        app->gl_context = nullptr;
    }

    SDL_DestroyWindow(app->window);
    app->window = nullptr;
    app->mode = IDLE;
}

void save_screen(App* app, bool crop) {
    if (!app->shot) return;
    ensure_dir(cfg.save_path);
    auto now = std::chrono::system_clock::now();
    auto now_sec = std::chrono::floor<std::chrono::seconds>(now);
    auto stamp = std::format("{:%F_%H-%M-%S}", now_sec);
    auto path = std::format("{}kadr_screenshot_{}.png", sep, stamp);
    path = cfg.save_path + path;

    bool saved = false;

    if (crop) {
        SDL_Rect src_rect;
        src_rect.x = (int)((app->start.x < app->drag.x) ? app->start.x : app->drag.x);
        src_rect.y = (int)((app->start.y < app->drag.y) ? app->start.y : app->drag.y);
        src_rect.w = (int)abs(app->drag.x - app->start.x);
        src_rect.h = (int)abs(app->drag.y - app->start.y);

        if (src_rect.w == 0 || src_rect.h == 0) return;

        SDL_Surface* save = SDL_CreateSurface(src_rect.w, src_rect.h, app->shot->format);
        if (!save) {
            SDL_Log("Failed to create surface: %s", SDL_GetError());
            return;
        }

        if (!SDL_BlitSurface(app->shot, &src_rect, save, nullptr)) {
            SDL_Log("Screenshot cropping failed: %s", SDL_GetError());
            SDL_DestroySurface(save);
            return;
        }

        saved = SDL_SavePNG(save, path.c_str());
        if (!saved) { SDL_Log("failed to save to file: %s", SDL_GetError()); }
        SDL_DestroySurface(save);
    } else {
        saved = SDL_SavePNG(app->shot, path.c_str());
        if (!saved) { SDL_Log("failed to save to file: %s", SDL_GetError()); }
    }

    if (saved) {
        g_last_screenshot_path = path;
        SDL_Log("Screenshot saved: %s", path.c_str());
    }
}

static bool copy_surface_to_clipboard(SDL_Surface* surface) {
    if (!surface) return false;

    SDL_Surface* rgba = SDL_ConvertSurface(surface, SDL_PIXELFORMAT_RGBA32);
    if (!rgba) {
        SDL_Log("Failed to convert surface for clipboard: %s", SDL_GetError());
        return false;
    }

    clip::image_spec spec;
    spec.width = static_cast<unsigned long>(rgba->w);
    spec.height = static_cast<unsigned long>(rgba->h);
    spec.bits_per_pixel = 32;
    spec.bytes_per_row = static_cast<unsigned long>(rgba->pitch);

    SDL_PixelFormatDetails const* fmt = SDL_GetPixelFormatDetails(rgba->format);

    spec.red_mask = fmt->Rmask;
    spec.green_mask = fmt->Gmask;
    spec.blue_mask = fmt->Bmask;
    spec.alpha_mask = fmt->Amask;

    spec.red_shift = fmt->Rshift;
    spec.green_shift = fmt->Gshift;
    spec.blue_shift = fmt->Bshift;
    spec.alpha_shift = fmt->Ashift;

    clip::image img(rgba->pixels, spec);
    bool ok = clip::set_image(img);

    SDL_DestroySurface(rgba);

    if (ok) {
        SDL_Log("Copied screenshot to clipboard");
    } else {
        SDL_Log("Failed to copy screenshot to clipboard");
    }
    return ok;
}

static bool copy_screenshot_to_clipboard(App* app, bool crop) {
    if (!app->shot) return false;

    SDL_Surface* src = app->shot;
    SDL_Surface* cropped = nullptr;

    if (crop) {
        SDL_Rect src_rect;
        src_rect.x = (int)((app->start.x < app->drag.x) ? app->start.x : app->drag.x);
        src_rect.y = (int)((app->start.y < app->drag.y) ? app->start.y : app->drag.y);
        src_rect.w = (int)abs(app->drag.x - app->start.x);
        src_rect.h = (int)abs(app->drag.y - app->start.y);

        if (src_rect.w == 0 || src_rect.h == 0) return false;

        cropped = SDL_CreateSurface(src_rect.w, src_rect.h, app->shot->format);
        if (!cropped) {
            SDL_Log("Failed to create surface: %s", SDL_GetError());
            return false;
        }

        if (!SDL_BlitSurface(app->shot, &src_rect, cropped, nullptr)) {
            SDL_Log("Screenshot cropping failed: %s", SDL_GetError());
            SDL_DestroySurface(cropped);
            return false;
        }

        src = cropped;
    }

    bool result = copy_surface_to_clipboard(src);
    if (cropped) SDL_DestroySurface(cropped);
    return result;
}

void callback_quit(void* userdata, SDL_TrayEntry* entry) {
    SDL_Event event;
    event.type = SDL_EVENT_QUIT;
    SDL_PushEvent(&event);
}

static void TransitionToSC(App* app) {
    capture_windows();
    if (!app->window) CreateGLContext(app, SC);
    if (!app->shot) {
        float mx, my;
        if (cfg.hide_cursor) {
            SDL_GetGlobalMouseState(&mx, &my);
            SDL_WarpMouseGlobal(69420.0f, 69420.0f);
        }

        app->shot = Screenshotter().TakeScreenshot();
        app->shot_tex = surface_to_imgui(app->shot);

        if (cfg.hide_cursor) { SDL_WarpMouseGlobal(mx, my); }
    }
    if (app->window) OpenWindow(app, SC);
}

static void TransitionToSettings(App* app) {
    if (!app->window) CreateGLContext(app, SETTINGS);
    if (app->window) OpenWindow(app, SETTINGS);
}

int main(int, char**) {
    if (!get_lock()) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION,
            "kadr is already open",
            "an instance of kadr is already open\nto reopen kadr close the running process first",
            nullptr);
        return 69;
    }

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD)) {
        fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
        return -1;
    }

    init_keybinds();
    keybinds_set_changed_callback([] { config_save("kadr_config.json"); });

    keybinds_bind_silent(Action::TAKE_SCREENSHOT, combo({VC_ALT_L, VC_SHIFT_L, VC_S}));
    keybinds_bind_silent(Action::OPEN_SETTINGS, combo({VC_F7}));
    keybinds_bind_silent(Action::CLOSE_WINDOW, combo({VC_ESCAPE}));
    keybinds_bind_silent(Action::QUIT_KADR, combo({VC_CONTROL_L, VC_SHIFT_L, VC_E}));

    // keybinds_load("kadr.kbd");
    config_load("kadr_config.json");

    SetupGLAttributes();

    App app = {};

    app.icon = SDL_LoadPNG("assets/kadr_icon.png");

    app.tray = SDL_CreateTray(app.icon, "kadr");

    SDL_TrayMenu* menu = SDL_CreateTrayMenu(app.tray);

    SDL_TrayEntry* settings_entry =
        SDL_InsertTrayEntryAt(menu, -1, "Settings", SDL_TRAYENTRY_BUTTON);
    SDL_SetTrayEntryCallback(
        settings_entry,
        [](void* ud, SDL_TrayEntry*) {
            ((App*)ud)->mode = SETTINGS;
            TransitionToSettings((App*)ud);
        },
        &app);

    SDL_TrayEntry* reload_entry =
        SDL_InsertTrayEntryAt(menu, -1, "Reload Settings", SDL_TRAYENTRY_BUTTON);
    SDL_SetTrayEntryCallback(
        reload_entry, [](void* ud, SDL_TrayEntry*) { config_load("kadr_config.json"); }, &app);

    SDL_TrayEntry* quit_entry = SDL_InsertTrayEntryAt(menu, -1, "Quit", SDL_TRAYENTRY_BUTTON);
    SDL_SetTrayEntryCallback(quit_entry, callback_quit, NULL);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    // ImGui::StyleColorsDark();
    set_theme_kadr_dark();
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    app.hook_thread = std::thread(hook_thread_fn);

    while (app.running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (app.window) ImGui_ImplSDL3_ProcessEvent(&e);

            if (e.type == SDL_EVENT_QUIT) {
                app.running = false;
            }  // else if (e.type == SDL_EVENT_KEY_DOWN && app.window && e.key.key == SDLK_ESCAPE) {
            //     app.pending_close = true;
            // }

            if (app.mode == SC && !ImGui::GetIO().WantCaptureMouse) {
                if (cfg.sc_mode == SCMode::Region) {
                    if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
                        if (e.button.button == SDL_BUTTON_LEFT) {
                            app.start = {e.button.x, e.button.y};
                            app.dragging = true;
                        } else if (e.button.button == SDL_BUTTON_RIGHT) {
                            app.dragging = false;
                            app.start = inv_pos;
                            app.drag = inv_pos;
                        }
                    } else if (e.type == SDL_EVENT_MOUSE_BUTTON_UP &&
                               e.button.button == SDL_BUTTON_LEFT && app.dragging) {
                        app.dragging = false;
                        const bool crop = app.start != app.drag;

                        if (cfg.save_to_disk) save_screen(&app, crop);
                        if (cfg.copy_to_clipboard) copy_screenshot_to_clipboard(&app, crop);

                        app.start = inv_pos;
                        app.drag = inv_pos;

                        app.pending_close = true;
                    } else if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN &&
                               e.button.button == SDL_BUTTON_RIGHT && app.dragging) {
                        app.dragging = false;
                    }
                } else if (cfg.sc_mode == SCMode::Window) {
                    if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN &&
                        e.button.button == SDL_BUTTON_LEFT) {
                        if (app.start != inv_pos && app.drag != inv_pos) {
                            if (cfg.save_to_disk) save_screen(&app, true);
                            if (cfg.copy_to_clipboard) copy_screenshot_to_clipboard(&app, true);
                            app.pending_close = true;
                        }
                    }
                }
            }
        }

        if (g_close_requested.exchange(false)) { app.pending_close = true; }

        if (app.save_full) {
            app.dragging = false;
            app.save_full = false;

            if (cfg.save_to_disk) save_screen(&app, false);
            if (cfg.copy_to_clipboard) copy_screenshot_to_clipboard(&app, false);

            app.pending_close = true;
        }

        if (app.dragging) {
            float x, y;
            SDL_GetMouseState(&x, &y);
            app.drag = {x, y};
        }

        if (app.pending_close) {
            app.mode = IDLE;
            app.pending_close = false;
            SDL_DestroySurface(app.shot);
            app.shot = nullptr;
            app.shot_tex = 0;
            destroy_tex(app.shot_tex);
            CloseWindow(&app);
            continue;
        }

        // IMPORTANT: window needs to be opened after taking the screenshot
        if (g_open_requested_sc.exchange(false)) {
            if (app.mode == SETTINGS) CloseWindow(&app);
            app.mode = SC;
            TransitionToSC(&app);
            // if (!app.window) CreateGLContext(&app);
            // if (!app.shot) {
            //     app.shot = Screenshotter().TakeScreenshot();
            //     app.shot_tex = surface_to_imgui(app.shot);
            // }
            // if (app.window) OpenWindow(&app);
        }

        if (g_open_requested_settings.exchange(false)) {
            if (app.mode == SC) CloseWindow(&app);
            app.mode = SETTINGS;
            TransitionToSettings(&app);
        }

        if (!app.window || app.mode == IDLE) {
            SDL_Delay(20);  // idle while hidden
            continue;
        }

        if (cfg.sc_mode == SCMode::Window) {
            float mx, my;
            SDL_GetMouseState(&mx, &my);
            auto wnd = window_at({mx + app.vd_min_x, my + app.vd_min_y});
            if (wnd) {
                app.start = ImVec2(wnd->position.x - app.vd_min_x, wnd->position.y - app.vd_min_y);
                app.drag = ImVec2(wnd->position.x + wnd->size.x - app.vd_min_x,
                    wnd->position.y + wnd->size.y - app.vd_min_y);
            } else {
                app.start = inv_pos;
                app.drag = inv_pos;
            }

            if (ImGui::GetIO().WantCaptureMouse) {
                app.start = inv_pos;
                app.drag = inv_pos;
            }
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        if (app.mode == SC) {
            toolbar(&app);

            if (app.shot) {
                auto shade = IM_COL32(0, 0, 0, 80);
                auto* bg = ImGui::GetBackgroundDrawList();
                ImVec2 p_min = {0, 0};
                ImVec2 p_max = {(float)app.shot->w, (float)app.shot->h};
                bg->AddImage(ImTextureRef(app.shot_tex), p_min, p_max);

                if ((app.dragging || cfg.sc_mode == SCMode::Window) &&
                    (app.start != inv_pos || app.drag != inv_pos)) {
                    ImVec2 r_min = ImMin(app.start, app.drag);
                    ImVec2 r_max = ImMax(app.start, app.drag);

                    bg->AddRectFilled({0, 0}, {p_max.x, r_min.y}, shade);
                    bg->AddRectFilled({0, r_max.y}, {p_max.x, p_max.y}, shade);
                    bg->AddRectFilled({0, r_min.y}, {r_min.x, r_max.y}, shade);
                    bg->AddRectFilled({r_max.x, r_min.y}, {p_max.x, r_max.y}, shade);

                    if (cfg.sc_mode == SCMode::Window && !ImGui::GetIO().WantCaptureMouse) {
                        wnd_info(&app);
                    }
                } else {
                    bg->AddRectFilled(p_min, p_max, shade);
                }
            }

            if (app.dragging || cfg.sc_mode == SCMode::Window) {
                auto white = IM_COL32(255, 255, 255, 255);
                auto* fg = ImGui::GetForegroundDrawList();
                fg->AddRect(app.start, app.drag, white);
            }

            if (app.dragging && cfg.sc_mode == SCMode::Region) { drag_ui(&app); }
        } else if (app.mode == SETTINGS) {
            settings_ui(&app);
        }

        ImGui::Render();
        ImDrawData* draw_data = ImGui::GetDrawData();

        int fb_w, fb_h;
        SDL_GetWindowSizeInPixels(app.window, &fb_w, &fb_h);

        glViewport(0, 0, fb_w, fb_h);

        glClearColor(0.0f, 0.0f, 0.0f, 0.1f);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(draw_data);
        SDL_GL_SwapWindow(app.window);
    }

    hook_stop();
    // keybinds_save("kadr.kbd");
    config_save("kadr_config.json");
    shutdown_keybinds();
    if (app.hook_thread.joinable()) app.hook_thread.join();

    if (app.window) CloseWindow(&app);

    ImGui::DestroyContext();
    SDL_DestroyTray(app.tray);
    SDL_DestroySurface(app.icon);
    SDL_Quit();

    remove_lock();

    return 0;
}
