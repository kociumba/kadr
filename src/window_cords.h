#ifndef KADR_WINDOW_CORDS_H
#define KADR_WINDOW_CORDS_H

#include <optional>
#include <vector>
#include "globals.h"

#if defined(_WIN32)
#include "win_include.h"
#elif defined(__APPLE__)
#elif defined(__linux__)
#include <X11/Xlib.h>
#endif

struct WindowEntry {
    ImVec2 position;  // top-left corner in screen coordinates
    ImVec2 size;      // width and height

#if defined(_WIN32)
    HWND hwnd;
#elif defined(__APPLE__)
    // CGWindowID or NSWindow* can go here
    uint32_t window_id;
#elif defined(__linux__)
    Window xwindow;
#endif

    // Returns true if the given point falls within this window's bounds
    [[nodiscard]] bool contains(ImVec2 point) const {
        return point.x >= position.x && point.x < position.x + size.x && point.y >= position.y &&
               point.y < position.y + size.y;
    }
};

extern std::vector<WindowEntry> window_cords;

void capture_windows();

const WindowEntry* window_at(ImVec2 point);

#endif  //KADR_WINDOW_CORDS_H
