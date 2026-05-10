#ifndef KADR_WINDOW_INFO_H
#define KADR_WINDOW_INFO_H

#include <string>
#include <cstdint>
#include "window_cords.h"

struct WindowInfo {
    std::string title;
    std::string process_name;
    uint32_t    pid = 0;
};

// Returns info for the given entry. Fields may be empty if unavailable.
WindowInfo get_window_info(const WindowEntry& entry);

#endif // KADR_WINDOW_INFO_H
