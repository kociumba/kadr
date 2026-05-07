#ifndef KADR_LOCK_H
#define KADR_LOCK_H

#include <string_view>
#include "globals.h"

constexpr std::string_view lock_name = "kadr.lock";

inline void remove_lock() { fs::remove(lock_name); }

#if defined(_WIN32)
#include "win_include.h"

// true - kadr is allowed to open, false - instance is already open
inline bool get_lock() {
    DWORD current_pid = GetCurrentProcessId();

    std::ifstream ifs(lock_name.data(), std::ios::binary);
    if (!ifs) {
        std::ofstream ofs(lock_name.data(), std::ios::binary);
        ofs.write((const char*)&current_pid, sizeof(current_pid));
        return true;
    }

    DWORD locked_pid = 0;
    ifs.read((char*)&locked_pid, sizeof(locked_pid));
    ifs.close();

    HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, locked_pid);
    if (!hProcess) {
        std::ofstream ofs(lock_name.data(), std::ios::binary);
        ofs.write((const char*)&current_pid, sizeof(current_pid));
        return true;
    }

    char path[MAX_PATH];
    DWORD size = MAX_PATH;
    std::string exe_name;

    if (QueryFullProcessImageNameA(hProcess, 0, path, &size)) {
        const char* name = strrchr(path, '\\');
        exe_name = name ? name + 1 : path;
    }
    CloseHandle(hProcess);

    if (exe_name != KADR_EXEC) {
        std::ofstream ofs(lock_name.data(), std::ios::binary);
        ofs.write((const char*)&current_pid, sizeof(current_pid));
        return true;
    }

    return false;
}

#elif defined(__APPLE__)
#include <libproc.h>
#include <unistd.h>

inline bool get_lock() {
    pid_t current_pid = getpid();

    std::ifstream ifs(lock_name.data(), std::ios::binary);
    if (!ifs) {
        std::ofstream ofs(lock_name.data(), std::ios::binary);
        ofs.write((const char*)&current_pid, sizeof(current_pid));
        return true;
    }

    pid_t locked_pid = 0;
    ifs.read((char*)&locked_pid, sizeof(locked_pid));
    ifs.close();

    char path[PROC_PIDPATHINFO_MAXSIZE];
    int ret = proc_pidpath(locked_pid, path, sizeof(path));

    auto take_lock = [&]() {
        std::ofstream ofs(lock_name.data(), std::ios::binary);
        ofs.write((const char*)&current_pid, sizeof(current_pid));
        return true;
    };

    if (ret <= 0) { return take_lock(); }

    const char* name = strrchr(path, '/');
    std::string exe_name = name ? name + 1 : path;

    if (exe_name != KADR_EXEC) { return take_lock(); }

    return false;
}

#else
#include <unistd.h>

inline bool get_lock() {
    pid_t current_pid = getpid();

    std::ifstream ifs(lock_name.data(), std::ios::binary);
    if (!ifs) {
        std::ofstream ofs(lock_name.data(), std::ios::binary);
        ofs.write((const char*)&current_pid, sizeof(current_pid));
        return true;
    }

    pid_t locked_pid = 0;
    ifs.read((char*)&locked_pid, sizeof(locked_pid));
    ifs.close();

    auto take_lock = [&]() {
        std::ofstream ofs(lock_name.data(), std::ios::binary);
        ofs.write((const char*)&current_pid, sizeof(current_pid));
        return true;
    };
    
    std::string comm_path = "/proc/" + std::to_string(locked_pid) + "/comm";
    std::ifstream comm_file(comm_path);
    if (!comm_file) { return take_lock(); }

    std::string exe_name;
    std::getline(comm_file, exe_name);

    std::string expected = std::string(KADR_EXEC).substr(0, 15);
    if (exe_name != expected) { return take_lock(); }

    return false;
}

#endif

#endif  //KADR_LOCK_H
