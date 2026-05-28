#ifndef KADR_AUTO_RUN_H
#define KADR_AUTO_RUN_H

#include <SDL3/SDL_filesystem.h>
#include "globals.h"

inline std::string get_self_path() {
    auto path = std::format("{}{}", SDL_GetBasePath(), KADR_EXEC);
    return path;
}

#if defined(_WIN32)
#include "win_include.h"

inline bool add_to_autostart() {
    HKEY hKey;
    const char* regPath = R"(Software\Microsoft\Windows\CurrentVersion\Run)";
    if (RegOpenKeyExA(HKEY_CURRENT_USER, regPath, 0, KEY_SET_VALUE, &hKey) != ERROR_SUCCESS) {
        return false;
    }

    std::string path_str = get_self_path();
    if (RegSetValueExA(
            hKey, KADR_EXEC, 0, REG_SZ, (const BYTE*)path_str.c_str(), path_str.length() + 1) !=
        ERROR_SUCCESS) {
        RegCloseKey(hKey);
        return false;
    }

    RegCloseKey(hKey);
    return true;
}

inline bool remove_from_autostart() {
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_CURRENT_USER,
            R"(Software\Microsoft\Windows\CurrentVersion\Run)",
            0,
            KEY_SET_VALUE,
            &hKey) == ERROR_SUCCESS) {
        RegDeleteValueA(hKey, KADR_EXEC);
        RegCloseKey(hKey);
    }
    return true;
}

#elif defined(__APPLE__)

inline fs::path get_plist_path(const std::string& app_name) {
    std::string home = getenv("HOME");
    return fs::path(home) / "Library/LaunchAgents" / (app_name + ".plist");
}

inline bool add_to_autostart() {
    auto p = get_plist_path(KADR_EXEC);

    std::error_code ec;
    fs::create_directories(p.parent_path(), ec);
    if (ec) return false;

    std::ofstream ofs(p);
    if (!ofs) return false;

    ofs << std::format(
        R"(<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>Label</key>
    <string>{}</string>
    <key>ProgramArguments</key>
    <array>
        <string>{}</string>
    </array>
    <key>RunAtLoad</key>
    <true/>
</dict>
</plist>)",
        KADR_EXEC,
        get_self_path());

    ofs.close();
    if (!ofs) return false;

    // could technically fail on this as well
    std::string cmd = std::format("launchctl load \"{}\"", p.string());
    std::system(cmd.c_str());
    return true;
}

inline bool remove_from_autostart() {
    auto p = get_plist_path(KADR_EXEC);

    // could technically fail on this as well
    std::string cmd = std::format("launchctl unload \"{}\"", p.string());
    std::system(cmd.c_str());

    std::error_code ec;
    fs::remove(p, ec);
    return true;
}

#else

inline fs::path get_desktop_path(const std::string& app_name) {
    const char* config_home = getenv("XDG_CONFIG_HOME");
    fs::path base = config_home ? fs::path(config_home) : fs::path(getenv("HOME")) / ".config";
    return base / "autostart" / (app_name + ".desktop");
}

inline bool add_to_autostart() {
    auto p = get_desktop_path(KADR_EXEC);
    fs::create_directories(p.parent_path());

    std::ofstream ofs(p);
    if (!ofs) return false;

    ofs << std::format(
        "[Desktop "
        "Entry]\nType=Application\nName={}\nExec={}\nHidden=false\nNoDisplay=false\nX-GNOME-"
        "Autostart-enabled=true\n",
        KADR_EXEC,
        get_self_path());
    return true;
}

inline bool remove_from_autostart() {
    std::error_code ec;
    fs::remove(get_desktop_path(KADR_EXEC), ec);
    return true;
}

#endif

#endif  //KADR_AUTO_RUN_H
