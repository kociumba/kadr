#ifndef KADR_HIJACK_H
#define KADR_HIJACK_H

#include "globals.h"

#if defined(_WIN32)
#include "win_include.h"

inline bool set_prtsc_snip(bool enabled) {
    HKEY hKey;
    const char* regPath = R"(Control Panel\Keyboard)";

    if (RegOpenKeyExA(HKEY_CURRENT_USER, regPath, 0, KEY_SET_VALUE, &hKey) != ERROR_SUCCESS)
        return false;

    DWORD value = enabled ? 1 : 0;
    if (RegSetValueExA(hKey,
            "PrintScreenKeyForSnippingEnabled",
            0,
            REG_DWORD,
            reinterpret_cast<const BYTE*>(&value),
            sizeof(value)) != ERROR_SUCCESS) {
        RegCloseKey(hKey);
        return false;
    }
    RegCloseKey(hKey);

    // RegFlushKey(HKEY_CURRENT_USER); // idk

    DWORD_PTR result = 0;
    LRESULT sent = SendMessageTimeoutA(HWND_BROADCAST,
        WM_SETTINGCHANGE,
        0,
        reinterpret_cast<LPARAM>("Keyboard"),
        SMTO_ABORTIFHUNG | SMTO_NOTIMEOUTIFNOTHUNG,
        5000,
        &result);

    if (sent == 0) { SDL_Log("failed to send setting change signal: %d", GetLastError()); }

    return true;
}

inline bool enable_prtsc_snip() { return set_prtsc_snip(true); }
inline bool disable_prtsc_snip() { return set_prtsc_snip(false); }

#else

inline bool enable_prtsc_snip() { return true; }
inline bool disable_prtsc_snip() { return true; }

#endif

#endif  //KADR_HIJACK_H
