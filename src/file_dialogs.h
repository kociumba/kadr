#ifndef KADR_FILE_DIALOGS_H
#define KADR_FILE_DIALOGS_H

#include <SDL3/SDL.h>
#include <functional>
#include <span>
#include <string>
#include <vector>

namespace dialog {

struct filter_t {
    const char* name;
    const char* pattern;
};

using callback_t = std::function<void(std::span<const std::string>)>;

namespace detail {

struct State {
    callback_t cb;
    std::vector<SDL_DialogFileFilter> filters;
};

inline void SDLCALL sdl_callback(void* userdata, const char* const* filelist, int) {
    std::unique_ptr<State> state{static_cast<State*>(userdata)};

    std::vector<std::string> paths;
    if (filelist)
        for (int i = 0; filelist[i]; ++i)
            paths.emplace_back(filelist[i]);

    state->cb(paths);
}

inline std::unique_ptr<State> make_state(callback_t cb, std::span<const filter_t> filters) {
    auto state = std::make_unique<State>(std::move(cb));
    state->filters.reserve(filters.size());
    for (auto& f : filters)
        state->filters.push_back({f.name, f.pattern});
    return state;
}

}  // namespace detail

inline void open_file(SDL_Window* window,
    std::initializer_list<filter_t> filters,
    callback_t cb,
    const char* default_location = nullptr) {
    auto s = detail::make_state(std::move(cb), filters);
    auto* raw = s.release();
    SDL_ShowOpenFileDialog(detail::sdl_callback,
        raw,
        window,
        raw->filters.data(),
        (int)raw->filters.size(),
        default_location,
        false);
}

inline void open_files(SDL_Window* window,
    std::initializer_list<filter_t> filters,
    callback_t cb,
    const char* default_location = nullptr) {
    auto s = detail::make_state(std::move(cb), filters);
    auto* raw = s.release();
    SDL_ShowOpenFileDialog(detail::sdl_callback,
        raw,
        window,
        raw->filters.data(),
        (int)raw->filters.size(),
        default_location,
        true);
}

inline void open_folder(SDL_Window* window, callback_t cb, const char* default_location = nullptr) {
    auto s = detail::make_state(std::move(cb), {});
    SDL_ShowOpenFolderDialog(detail::sdl_callback, s.release(), window, default_location, false);
}

inline void save_file(SDL_Window* window,
    std::initializer_list<filter_t> filters,
    const char* default_name,
    callback_t cb,
    const char* default_location = nullptr) {
    auto s = detail::make_state(std::move(cb), filters);
    auto* raw = s.release();
    SDL_ShowSaveFileDialog(detail::sdl_callback,
        raw,
        window,
        raw->filters.data(),
        (int)raw->filters.size(),
        default_location);
}

}  // namespace dialog

#endif  //KADR_FILE_DIALOGS_H