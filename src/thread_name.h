#ifndef KADR_THREAD_NAME_H
#define KADR_THREAD_NAME_H

#include <string>
#include <thread>

#if defined(_WIN32)
#include "win_include.h"

#include <processthreadsapi.h>

namespace detail {
inline HANDLE handle_from_id(std::thread::id id) {
    DWORD native = 0;
    static_assert(sizeof(id) >= sizeof(DWORD), "unexpected std::thread::id size");
    std::memcpy(&native, &id, sizeof(DWORD));
    return OpenThread(THREAD_SET_LIMITED_INFORMATION, FALSE, native);
}
}  // namespace detail

inline bool set_thread_name(std::thread::id id, const std::string& name) {
    HANDLE h = detail::handle_from_id(id);
    if (!h || h == INVALID_HANDLE_VALUE) return false;

    int wlen = MultiByteToWideChar(CP_UTF8, 0, name.c_str(), -1, nullptr, 0);
    if (wlen <= 0) {
        CloseHandle(h);
        return false;
    }

    std::wstring wname(wlen, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, name.c_str(), -1, wname.data(), wlen);

    HRESULT hr = SetThreadDescription(h, wname.c_str());
    CloseHandle(h);
    return SUCCEEDED(hr);
}

#elif defined(__APPLE__)
#include <pthread.h>

inline bool set_thread_name(std::thread::id id, const std::string& name) {
    if (id == std::this_thread::get_id()) { return pthread_setname_np(name.c_str()) == 0; }

    pthread_t pt;
    std::memcpy(&pt, &id, sizeof(pt));

    (void)pt;
    return false;
}

#else
#include <pthread.h>

namespace detail {
inline pthread_t pthread_from_id(std::thread::id id) {
    pthread_t pt{};
    static_assert(sizeof(id) >= sizeof(pt), "unexpected std::thread::id size");
    std::memcpy(&pt, &id, sizeof(pt));
    return pt;
}
}  // namespace detail

inline bool set_thread_name(std::thread::id id, const std::string& name) {
    pthread_t pt = detail::pthread_from_id(id);
    return pthread_setname_np(pt, name.c_str()) == 0;
}

#endif

inline bool set_current_thread_name(const std::string& name) {
    return set_thread_name(std::this_thread::get_id(), name);
}

#endif  // KADR_THREAD_NAME_H
