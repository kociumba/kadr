#ifndef KADR_CAPTURE_H
#define KADR_CAPTURE_H

#include <SDL3/SDL.h>
#include <ScreenCapture.h>
#include <algorithm>
#include <chrono>
#include <mutex>
#include <unordered_map>
#include <vector>

struct Screenshotter {
    SDL_Surface* TakeScreenshot() {
        auto monitors = SL::Screen_Capture::GetMonitors();
        if (monitors.empty()) return nullptr;

        struct MonitorBuf {
            std::vector<uint8_t> pixels;
            int width = 0;
            int height = 0;
            bool ready = false;
        };

        std::unordered_map<int, MonitorBuf> bufs;
        for (auto& mon : monitors) {
            auto& b = bufs[mon.Id];
            b.width = mon.Width;
            b.height = mon.Height;
            b.pixels.resize(static_cast<size_t>(mon.Width) * mon.Height * 4, 0);
        }

        std::mutex mtx;
        int framesReceived = 0;
        const int framesNeeded = static_cast<int>(monitors.size());

        auto capture =
            SL::Screen_Capture::CreateCaptureConfiguration([&monitors] { return monitors; })
                ->onNewFrame([&](const SL::Screen_Capture::Image& img,
                                 const SL::Screen_Capture::Monitor& mon) {
                    std::lock_guard lock(mtx);

                    auto it = bufs.find(mon.Id);
                    if (it == bufs.end() || it->second.ready) return;

                    auto& b = it->second;
                    const int srcStride = SL::Screen_Capture::Width(img) * 4;
                    const int dstStride = b.width * 4;
                    const auto* src = SL::Screen_Capture::StartSrc(img);

                    const int h = std::min(SL::Screen_Capture::Height(img), b.height);
                    for (int row = 0; row < h; ++row) {
                        memcpy(b.pixels.data() + row * dstStride,
                            reinterpret_cast<const uint8_t*>(src) + row * srcStride,
                            dstStride);
                    }

                    b.ready = true;
                    ++framesReceived;
                })
                ->start_capturing();

        while (framesReceived < framesNeeded) {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
        capture.reset();

        int minX = std::numeric_limits<int>::max();
        int minY = std::numeric_limits<int>::max();
        int maxX = std::numeric_limits<int>::min();
        int maxY = std::numeric_limits<int>::min();

        for (auto& mon : monitors) {
            minX = std::min(minX, mon.OffsetX);
            minY = std::min(minY, mon.OffsetY);
            maxX = std::max(maxX, mon.OffsetX + mon.Width);
            maxY = std::max(maxY, mon.OffsetY + mon.Height);
        }

        const int totalWidth = maxX - minX;
        const int totalHeight = maxY - minY;

        SDL_Surface* stitched = SDL_CreateSurface(totalWidth, totalHeight, SDL_PIXELFORMAT_BGRA32);
        if (!stitched) return nullptr;

        SDL_ClearSurface(stitched, 0.0f, 0.0f, 0.0f, 1.0f);
        SDL_LockSurface(stitched);

        auto* dst = static_cast<uint8_t*>(stitched->pixels);

        for (auto& mon : monitors) {
            auto& b = bufs[mon.Id];
            if (!b.ready) continue;

            // Normalize coordinates so (0,0) is top-left of the bounding box
            const int dstX = mon.OffsetX - minX;
            const int dstY = mon.OffsetY - minY;
            const int monPitch = b.width * 4;

            for (int row = 0; row < b.height; ++row) {
                const int dstRow = dstY + row;
                if (dstRow < 0 || dstRow >= totalHeight) continue;

                uint8_t* dstRowPtr = dst + dstRow * stitched->pitch + dstX * 4;
                const uint8_t* srcRowPtr = b.pixels.data() + row * monPitch;

                memcpy(dstRowPtr, srcRowPtr, monPitch);
            }
        }

        SDL_UnlockSurface(stitched);
        return stitched;
    }
};

#endif