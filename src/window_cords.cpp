#include "window_cords.h"

std::vector<WindowEntry> window_cords;

// ─── Windows ────────────────────────────────────────────────────────────────
#if defined(_WIN32)

static BOOL CALLBACK enum_windows_proc(HWND hwnd, LPARAM) {
    if (!IsWindowVisible(hwnd)) return TRUE;

    int len = GetWindowTextLengthW(hwnd);
    if (len == 0) return TRUE;

    RECT rect{};
    if (!GetWindowRect(hwnd, &rect)) return TRUE;

    WindowEntry entry{};
    entry.hwnd = hwnd;
    entry.position = {static_cast<float>(rect.left), static_cast<float>(rect.top)};
    entry.size = {
        static_cast<float>(rect.right - rect.left), static_cast<float>(rect.bottom - rect.top)};
    window_cords.push_back(entry);
    return TRUE;
}

void capture_windows() {
    window_cords.clear();
    EnumWindows(enum_windows_proc, 0);
}

// ─── macOS ──────────────────────────────────────────────────────────────────
#elif defined(__APPLE__)
#include <CoreGraphics/CoreGraphics.h>

void capture_windows() {
    window_cords.clear();

    CFArrayRef list = CGWindowListCopyWindowInfo(
        kCGWindowListOptionOnScreenOnly | kCGWindowListExcludeDesktopElements, kCGNullWindowID);
    if (!list) return;

    CFIndex count = CFArrayGetCount(list);
    for (CFIndex i = 0; i < count; ++i) {
        auto* info = static_cast<CFDictionaryRef>(CFArrayGetValueAtIndex(list, i));

        int layer = 0;
        auto* layer_ref = static_cast<CFNumberRef>(CFDictionaryGetValue(info, kCGWindowLayer));
        if (layer_ref) CFNumberGetValue(layer_ref, kCFNumberIntType, &layer);
        if (layer != 0) continue;

        auto* bounds_ref =
            static_cast<CFDictionaryRef>(CFDictionaryGetValue(info, kCGWindowBounds));
        if (!bounds_ref) continue;

        CGRect cg_rect{};
        CGRectMakeWithDictionaryRepresentation(bounds_ref, &cg_rect);

        uint32_t wid = 0;
        auto* wid_ref = static_cast<CFNumberRef>(CFDictionaryGetValue(info, kCGWindowNumber));
        if (wid_ref) CFNumberGetValue(wid_ref, kCFNumberSInt32Type, &wid);

        WindowEntry entry{};
        entry.window_id = wid;
        entry.position = {
            static_cast<float>(cg_rect.origin.x), static_cast<float>(cg_rect.origin.y)};
        entry.size = {
            static_cast<float>(cg_rect.size.width), static_cast<float>(cg_rect.size.height)};
        window_cords.push_back(entry);
    }

    CFRelease(list);
}

#else

void capture_windows() {
    window_cords.clear();

    Display* dpy = XOpenDisplay(nullptr);
    if (!dpy) return;

    ::Window root = DefaultRootWindow(dpy);
    ::Window parent{};
    ::Window* children = nullptr;
    unsigned int nchildren = 0;

    if (!XQueryTree(dpy, root, &root, &parent, &children, &nchildren)) {
        XCloseDisplay(dpy);
        return;
    }

    for (int i = static_cast<int>(nchildren) - 1; i >= 0; --i) {
        ::Window w = children[i];

        XWindowAttributes attrs{};
        if (!XGetWindowAttributes(dpy, w, &attrs)) continue;
        if (attrs.map_state != IsViewable) continue;

        int rx = 0, ry = 0;
        ::Window child_ret{};
        XTranslateCoordinates(dpy, w, root, 0, 0, &rx, &ry, &child_ret);

        WindowEntry entry{};
        entry.xwindow = w;
        entry.position = {static_cast<float>(rx), static_cast<float>(ry)};
        entry.size = {static_cast<float>(attrs.width), static_cast<float>(attrs.height)};
        window_cords.push_back(entry);
    }

    if (children) XFree(children);
    XCloseDisplay(dpy);
}
#endif

const WindowEntry* window_at(ImVec2 point) {
    for (const WindowEntry& entry : window_cords) {
        if (entry.contains(point)) return &entry;
    }
    return nullptr;
}