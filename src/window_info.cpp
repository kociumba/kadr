#include "window_info.h"

// ─── Windows ────────────────────────────────────────────────────────────────
#if defined(_WIN32)
#include "win_include.h"
#include <psapi.h>

WindowInfo get_window_info(const WindowEntry& entry) {
    WindowInfo info;

    // Title
    int len = GetWindowTextLengthW(entry.hwnd);
    if (len > 0) {
        std::wstring buf(len + 1, L'\0');
        GetWindowTextW(entry.hwnd, buf.data(), len + 1);
        buf.resize(len);
        int sz = WideCharToMultiByte(CP_UTF8, 0, buf.data(), -1, nullptr, 0, nullptr, nullptr);
        if (sz > 0) {
            info.title.resize(sz - 1);
            WideCharToMultiByte(CP_UTF8, 0, buf.data(), -1, info.title.data(), sz, nullptr, nullptr);
        }
    }

    // PID + process name
    DWORD pid = 0;
    GetWindowThreadProcessId(entry.hwnd, &pid);
    info.pid = static_cast<uint32_t>(pid);

    HANDLE proc = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (proc) {
        wchar_t path[MAX_PATH]{};
        DWORD size = MAX_PATH;
        if (QueryFullProcessImageNameW(proc, 0, path, &size)) {
            // Grab just the filename from the full path
            std::wstring full(path);
            auto slash = full.rfind(L'\\');
            std::wstring name = (slash != std::wstring::npos) ? full.substr(slash + 1) : full;
            int sz = WideCharToMultiByte(CP_UTF8, 0, name.data(), -1, nullptr, 0, nullptr, nullptr);
            if (sz > 0) {
                info.process_name.resize(sz - 1);
                WideCharToMultiByte(CP_UTF8, 0, name.data(), -1,
                                    info.process_name.data(), sz, nullptr, nullptr);
            }
        }
        CloseHandle(proc);
    }

    return info;
}

// ─── macOS ──────────────────────────────────────────────────────────────────
#elif defined(__APPLE__)
#include <CoreGraphics/CoreGraphics.h>
#include <libproc.h>

WindowInfo get_window_info(const WindowEntry& entry) {
    WindowInfo info;

    CFArrayRef list = CGWindowListCopyWindowInfo(
        kCGWindowListOptionIncludingWindow, static_cast<CGWindowID>(entry.window_id));
    if (!list) return info;

    if (CFArrayGetCount(list) > 0) {
        auto* dict = static_cast<CFDictionaryRef>(CFArrayGetValueAtIndex(list, 0));

        // Title (kCGWindowName)
        auto* name_ref = static_cast<CFStringRef>(CFDictionaryGetValue(dict, kCGWindowName));
        if (name_ref) {
            char buf[512]{};
            if (CFStringGetCString(name_ref, buf, sizeof(buf), kCFStringEncodingUTF8))
                info.title = buf;
        }

        // Owner name (kCGWindowOwnerName) — closest to process name on macOS
        auto* owner_ref = static_cast<CFStringRef>(CFDictionaryGetValue(dict, kCGWindowOwnerName));
        if (owner_ref) {
            char buf[512]{};
            if (CFStringGetCString(owner_ref, buf, sizeof(buf), kCFStringEncodingUTF8))
                info.process_name = buf;
        }

        // PID (kCGWindowOwnerPID)
        auto* pid_ref = static_cast<CFNumberRef>(CFDictionaryGetValue(dict, kCGWindowOwnerPID));
        if (pid_ref) {
            int32_t pid = 0;
            CFNumberGetValue(pid_ref, kCFNumberSInt32Type, &pid);
            info.pid = static_cast<uint32_t>(pid);
        }
    }

    CFRelease(list);
    return info;
}

// ─── Linux (X11) ────────────────────────────────────────────────────────────
#else
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <cstring>
#include <fstream>

static std::string read_proc_name(uint32_t pid) {
    // /proc/<pid>/comm is a one-liner with just the process name
    std::ifstream f("/proc/" + std::to_string(pid) + "/comm");
    std::string name;
    std::getline(f, name);
    return name;
}

WindowInfo get_window_info(const WindowEntry& entry) {
    WindowInfo info;

    Display* dpy = XOpenDisplay(nullptr);
    if (!dpy) return info;

    ::Window w = entry.xwindow;

    // Title — prefer _NET_WM_NAME (UTF-8), fall back to XFetchName
    Atom net_wm_name = XInternAtom(dpy, "_NET_WM_NAME", False);
    Atom utf8_string  = XInternAtom(dpy, "UTF8_STRING",  False);

    Atom actual_type{};
    int actual_format{};
    unsigned long nitems{}, bytes_after{};
    unsigned char* prop = nullptr;

    if (XGetWindowProperty(dpy, w, net_wm_name, 0, 1024, False, utf8_string,
                           &actual_type, &actual_format, &nitems,
                           &bytes_after, &prop) == Success && prop) {
        info.title = reinterpret_cast<char*>(prop);
        XFree(prop);
        prop = nullptr;
    } else {
        char* name = nullptr;
        if (XFetchName(dpy, w, &name) && name) {
            info.title = name;
            XFree(name);
        }
    }

    // PID via _NET_WM_PID
    Atom net_wm_pid = XInternAtom(dpy, "_NET_WM_PID", False);
    if (XGetWindowProperty(dpy, w, net_wm_pid, 0, 1, False, XA_CARDINAL,
                           &actual_type, &actual_format, &nitems,
                           &bytes_after, &prop) == Success && prop) {
        info.pid = static_cast<uint32_t>(*reinterpret_cast<unsigned long*>(prop));
        XFree(prop);
        prop = nullptr;
    }

    if (info.pid)
        info.process_name = read_proc_name(info.pid);

    XCloseDisplay(dpy);
    return info;
}
#endif
