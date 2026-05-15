#ifndef KADR_RUN_ON_MAIN_H
#define KADR_RUN_ON_MAIN_H

#include <SDL3/SDL_log.h>
#include <atomic>
#include <concepts>
#include <cstddef>
#include <functional>
#include <mutex>
#include <thread>
#include <vector>

using Task = std::move_only_function<void()>;

struct alignas(64) TaskQueue {
    std::mutex mtx;
    std::vector<Task> back;
    std::vector<Task> front;
    std::atomic<bool> dirty{false};
};

inline TaskQueue g_task_queue;
inline std::thread::id g_main_thread_id;

inline void dispatch_init() noexcept { g_main_thread_id = std::this_thread::get_id(); }

template <std::invocable F>
void dispatch(F&& f) {
    auto& q = g_task_queue;
    {
        std::lock_guard lock{q.mtx};
        q.back.emplace_back(std::forward<F>(f));
    }
    q.dirty.store(true, std::memory_order_release);

    SDL_Event e{WAKE_UP};
    SDL_PushEvent(&e);
}

inline std::size_t poll() {
#if !defined(NDEBUG)
    if (std::this_thread::get_id() != g_main_thread_id) [[unlikely]] {
        SDL_LogCritical(SDL_LOG_CATEGORY_ERROR, "task execution not on the main thread");
        std::terminate();
    }
#endif

    auto& q = g_task_queue;

    if (!q.dirty.load(std::memory_order_acquire)) [[likely]]
        return 0;

    {
        std::lock_guard lock{q.mtx};
        if (q.back.empty()) return 0;
        std::swap(q.back, q.front);
        q.dirty.store(false, std::memory_order_relaxed);
    }

    const std::size_t n = q.front.size();
    for (auto& task : q.front)
        task();
    q.front.clear();

    return n;
}

[[nodiscard]] inline bool is_main_thread() noexcept {
    return std::this_thread::get_id() == g_main_thread_id;
}

#endif  //KADR_RUN_ON_MAIN_H
