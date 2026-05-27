#ifndef KADR_SOUND_H
#define KADR_SOUND_H

#include <SDL3_mixer/SDL_mixer.h>
#include <string>
#include <unordered_map>

inline MIX_Mixer* g_mixer = nullptr;
inline std::unordered_map<std::string, MIX_Audio*> g_sound_cache;

inline void audio_error(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    SDL_LogMessageV(SDL_LOG_CATEGORY_AUDIO, SDL_LOG_PRIORITY_ERROR, fmt, args);

    va_end(args);
}

inline void clear_sound_cache() {
    for (auto& [_, audio] : g_sound_cache) {
        MIX_DestroyAudio(audio);
    }
    g_sound_cache.clear();
}

inline bool sound_init() {
    if (!MIX_Init()) {
        audio_error("failed to init sound: %s", SDL_GetError());
        return false;
    }
    g_mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr);
    if (!g_mixer) {
        audio_error("failed to open default sound device: %s", SDL_GetError());
        return false;
    }
    return true;
}

inline void play_sound_file(const std::string& path) {
    if (!g_mixer || path.empty()) return;

    auto it = g_sound_cache.find(path);
    if (it == g_sound_cache.end()) {
        MIX_Audio* audio = MIX_LoadAudio(g_mixer, path.c_str(), false);
        if (!audio) {
            audio_error("failed to load sound '%s': %s", path.c_str(), SDL_GetError());
            return;
        }
        it = g_sound_cache.emplace(path, audio).first;
    }

    if (!MIX_PlayAudio(g_mixer, it->second)) {
        audio_error("failed to play sound: %s", SDL_GetError());
    }
}

inline void sound_quit() {
    clear_sound_cache();
    if (g_mixer) {
        MIX_DestroyMixer(g_mixer);
        g_mixer = nullptr;
    }
    MIX_Quit();
}

#endif  //KADR_SOUND_H
