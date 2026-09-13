#pragma once

#include <windows.h>
#include <mmsystem.h>
#include <vector>
#include <cmath>
#include <atomic>
#include <thread>
#include <mutex>

namespace Aetheria {

enum class SoundID {
    Footstep,
    BlockBreak,
    BlockPlace,
    HammerBuild,
    SwordSwing,
    CreatureHit,
    PlayerHurt,
    LevelUp,
    StructureComplete,
    AmbientDrone,
    ItemPickup,
    Dash
};

class AudioEngine {
public:
    AudioEngine();
    ~AudioEngine();

    void playSound(SoundID id, float pitch = 1.0f, float volume = 1.0f);
    void update(float dt);

private:
    struct ActiveVoice {
        SoundID id;
        float position = 0.0f;
        float duration = 0.0f;
        float pitch = 1.0f;
        float volume = 1.0f;
        bool active = false;
    };

    void audioThreadFunc();
    void renderAudio(int16_t* buffer, int numSamples);

    HWAVEOUT hWaveOut = nullptr;
    WAVEHDR waveHeaders[2] = {};
    std::vector<int16_t> audioBuffers[2];
    int currentBufferIndex = 0;

    std::atomic<bool> isRunning{false};
    std::thread audioThread;
    std::mutex voiceMutex;

    static constexpr int MAX_VOICES = 32;
    ActiveVoice voices[MAX_VOICES];

    float ambientTimer = 0.0f;
};

} // namespace Aetheria
