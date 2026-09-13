#pragma once

#include <windows.h>

namespace Aetheria {

class Timer {
public:
    Timer() {
        QueryPerformanceFrequency(&frequency);
        QueryPerformanceCounter(&lastTime);
        startTime = lastTime;
    }

    void update() {
        LARGE_INTEGER currentTime;
        QueryPerformanceCounter(&currentTime);

        deltaTime = static_cast<float>(currentTime.QuadPart - lastTime.QuadPart) / static_cast<float>(frequency.QuadPart);
        totalTime = static_cast<float>(currentTime.QuadPart - startTime.QuadPart) / static_cast<float>(frequency.QuadPart);
        lastTime = currentTime;

        // FPS calculation
        frameCount++;
        fpsTimer += deltaTime;
        if (fpsTimer >= 1.0f) {
            currentFPS = frameCount / fpsTimer;
            frameCount = 0;
            fpsTimer = 0.0f;
        }

        // Clamp delta to avoid huge physics spikes on lag or window dragging
        if (deltaTime > 0.1f) deltaTime = 0.1f;
    }

    float getDeltaTime() const { return deltaTime; }
    float getTotalTime() const { return totalTime; }
    float getFPS() const { return currentFPS; }

private:
    LARGE_INTEGER frequency{};
    LARGE_INTEGER lastTime{};
    LARGE_INTEGER startTime{};
    float deltaTime = 0.016f;
    float totalTime = 0.0f;
    float fpsTimer = 0.0f;
    int frameCount = 0;
    float currentFPS = 60.0f;
};

} // namespace Aetheria
