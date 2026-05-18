#ifndef KADR_RESIZABLE_BORDERLESS_H
#define KADR_RESIZABLE_BORDERLESS_H

#ifdef _WIN32
#include "win_include.h"

static LRESULT CALLBACK
BorderlessWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR, DWORD_PTR) {
    if (msg == WM_NCCALCSIZE && wParam == TRUE) { return 0; }

    if (msg == WM_NCHITTEST) {
        POINT pt = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
        RECT rc;
        GetWindowRect(hwnd, &rc);

        const int border = 8;

        if (pt.x < rc.left + border) {
            if (pt.y < rc.top + border) return HTTOPLEFT;
            if (pt.y >= rc.bottom - border) return HTBOTTOMLEFT;
            return HTLEFT;
        }
        if (pt.x >= rc.right - border) {
            if (pt.y < rc.top + border) return HTTOPRIGHT;
            if (pt.y >= rc.bottom - border) return HTBOTTOMRIGHT;
            return HTRIGHT;
        }
        if (pt.y < rc.top + border) return HTTOP;
        if (pt.y >= rc.bottom - border) return HTBOTTOM;
    }

    // NOTE: I really don't want to implement drag rendering here

    return DefSubclassProc(hwnd, msg, wParam, lParam);
}

inline void make_borderless_resizable(SDL_Window* window) {
    SDL_PropertiesID props = SDL_GetWindowProperties(window);
    auto hwnd = (HWND)SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr);
    if (!hwnd) return;

    LONG style = GetWindowLongW(hwnd, GWL_STYLE);
    style |= WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
    SetWindowLongW(hwnd, GWL_STYLE, style);

    SetWindowSubclass(hwnd, BorderlessWndProc, 1, 0);

    SetWindowPos(
        hwnd, nullptr, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
}

inline void make_borderless_unresizable(SDL_Window* window) {
    SDL_PropertiesID props = SDL_GetWindowProperties(window);
    auto hwnd = (HWND)SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr);
    if (!hwnd) return;

    RemoveWindowSubclass(hwnd, BorderlessWndProc, 1);

    LONG style = GetWindowLongW(hwnd, GWL_STYLE);
    style &= ~(WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX);
    SetWindowLongW(hwnd, GWL_STYLE, style);

    SetWindowPos(
        hwnd, nullptr, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
}

#else

inline void make_borderless_resizable(SDL_Window* window) { (void)window; }
inline void make_borderless_unresizable(SDL_Window* window) { (void)window; }

#endif

#endif  // KADR_RESIZABLE_BORDERLESS_H
