#include "Audio.hpp"
#include <cstdlib>
#include <algorithm>

namespace Aetheria {

constexpr int SAMPLE_RATE = 44100;
constexpr int BUFFER_SIZE = 4096;

AudioEngine::AudioEngine() {
    for (int i = 0; i < MAX_VOICES; ++i) {
        voices[i].active = false;
    }

    WAVEFORMATEX wfx = {};
    wfx.wFormatTag = WAVE_FORMAT_PCM;
    wfx.nChannels = 1;
    wfx.nSamplesPerSec = SAMPLE_RATE;
    wfx.wBitsPerSample = 16;
    wfx.nBlockAlign = wfx.nChannels * (wfx.wBitsPerSample / 8);
    wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;
    wfx.cbSize = 0;

    if (waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL) != MMSYSERR_NOERROR) {
        return;
    }

    for (int i = 0; i < 2; ++i) {
        audioBuffers[i].resize(BUFFER_SIZE, 0);
        waveHeaders[i].lpData = reinterpret_cast<LPSTR>(audioBuffers[i].data());
        waveHeaders[i].dwBufferLength = BUFFER_SIZE * sizeof(int16_t);
        waveHeaders[i].dwFlags = 0;
        waveOutPrepareHeader(hWaveOut, &waveHeaders[i], sizeof(WAVEHDR));
    }

    isRunning = true;
    audioThread = std::thread(&AudioEngine::audioThreadFunc, this);
}

AudioEngine::~AudioEngine() {
    isRunning = false;
    if (audioThread.joinable()) {
        audioThread.join();
    }

    if (hWaveOut) {
        waveOutReset(hWaveOut);
        for (int i = 0; i < 2; ++i) {
            waveOutUnprepareHeader(hWaveOut, &waveHeaders[i], sizeof(WAVEHDR));
        }
        waveOutClose(hWaveOut);
        hWaveOut = nullptr;
    }
}

void AudioEngine::playSound(SoundID id, float pitch, float volume) {
    std::lock_guard<std::mutex> lock(voiceMutex);
    for (int i = 0; i < MAX_VOICES; ++i) {
        if (!voices[i].active) {
            voices[i].id = id;
            voices[i].position = 0.0f;
            voices[i].pitch = pitch;
            voices[i].volume = volume;
            voices[i].active = true;

            switch (id) {
            case SoundID::Footstep: voices[i].duration = 0.12f; break;
            case SoundID::BlockBreak: voices[i].duration = 0.25f; break;
            case SoundID::BlockPlace: voices[i].duration = 0.15f; break;
            case SoundID::HammerBuild: voices[i].duration = 0.2f; break;
            case SoundID::SwordSwing: voices[i].duration = 0.18f; break;
            case SoundID::CreatureHit: voices[i].duration = 0.3f; break;
            case SoundID::PlayerHurt: voices[i].duration = 0.35f; break;
            case SoundID::LevelUp: voices[i].duration = 1.0f; break;
            case SoundID::StructureComplete: voices[i].duration = 0.6f; break;
            case SoundID::AmbientDrone: voices[i].duration = 3.0f; break;
            case SoundID::ItemPickup: voices[i].duration = 0.12f; break;
            case SoundID::Dash: voices[i].duration = 0.24f; break;
            }
            break;
        }
    }
}

void AudioEngine::update(float dt) {
    ambientTimer += dt;
    if (ambientTimer > 12.0f) {
        ambientTimer = 0.0f;
        playSound(SoundID::AmbientDrone, 0.8f + (rand() % 40) / 100.0f, 0.25f);
    }
}

void AudioEngine::audioThreadFunc() {
    while (isRunning) {
        WAVEHDR& currentHeader = waveHeaders[currentBufferIndex];

        if ((currentHeader.dwFlags & WHDR_DONE) || !(currentHeader.dwFlags & WHDR_PREPARED)) {
            renderAudio(audioBuffers[currentBufferIndex].data(), BUFFER_SIZE);
            currentHeader.dwFlags &= ~WHDR_DONE;
            waveOutWrite(hWaveOut, &currentHeader, sizeof(WAVEHDR));
            currentBufferIndex = 1 - currentBufferIndex;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void AudioEngine::renderAudio(int16_t* buffer, int numSamples) {
    std::fill_n(buffer, numSamples, 0);

    std::lock_guard<std::mutex> lock(voiceMutex);
    float sampleDuration = 1.0f / SAMPLE_RATE;

    for (int i = 0; i < MAX_VOICES; ++i) {
        if (!voices[i].active) continue;

        ActiveVoice& v = voices[i];

        for (int s = 0; s < numSamples; ++s) {
            float t = v.position;
            if (t >= v.duration) {
                v.active = false;
                break;
            }

            float sample = 0.0f;
            float progress = t / v.duration;

            switch (v.id) {
            case SoundID::Footstep: {
                float freq = 120.0f * v.pitch * (1.0f - progress);
                float env = 1.0f - progress;
                sample = std::sin(2.0f * 3.14159f * freq * t) * env;
                sample += ((rand() % 100) / 50.0f - 1.0f) * env * 0.3f;
                break;
            }
            case SoundID::BlockBreak: {
                float noise = ((rand() % 200) / 100.0f - 1.0f);
                float env = 1.0f - progress;
                sample = noise * env * 0.8f;
                break;
            }
            case SoundID::BlockPlace: {
                float freq = 160.0f * v.pitch * (1.0f - progress * 0.5f);
                float env = (1.0f - progress) * (1.0f - progress);
                sample = std::sin(2.0f * 3.14159f * freq * t) * env;
                break;
            }
            case SoundID::HammerBuild: {
                // Metallic ring + click
                float freq = 880.0f * v.pitch;
                float env = std::exp(-progress * 8.0f);
                sample = std::sin(2.0f * 3.14159f * freq * t) * env * 0.7f;
                sample += ((rand() % 100) / 50.0f - 1.0f) * env * 0.2f;
                break;
            }
            case SoundID::SwordSwing: {
                float noise = ((rand() % 200) / 100.0f - 1.0f);
                float env = std::sin(progress * 3.14159f);
                sample = noise * env * 0.6f;
                break;
            }
            case SoundID::CreatureHit: {
                float freq = 220.0f * (1.0f - progress * 0.8f);
                float env = 1.0f - progress;
                sample = (std::sin(2.0f * 3.14159f * freq * t) + ((rand() % 100) / 50.0f - 1.0f) * 0.5f) * env;
                break;
            }
            case SoundID::PlayerHurt: {
                float freq = 150.0f * (1.0f - progress * 0.6f);
                float env = 1.0f - progress;
                sample = std::sin(2.0f * 3.14159f * freq * t) * env;
                break;
            }
            case SoundID::LevelUp: {
                // Arpeggio C-E-G-C
                float notes[] = { 261.63f, 329.63f, 392.00f, 523.25f };
                int noteIdx = std::min(3, static_cast<int>(progress * 4.0f));
                float freq = notes[noteIdx] * v.pitch;
                sample = std::sin(2.0f * 3.14159f * freq * t) * (1.0f - progress * 0.3f) * 0.6f;
                break;
            }
            case SoundID::StructureComplete: {
                // Fanfare chord
                float freq1 = 440.0f * v.pitch;
                float freq2 = 554.37f * v.pitch;
                float env = 1.0f - progress;
                sample = (std::sin(2.0f * 3.14159f * freq1 * t) + std::sin(2.0f * 3.14159f * freq2 * t)) * 0.4f * env;
                break;
            }
            case SoundID::AmbientDrone: {
                float freq = 65.41f * v.pitch; // Low C drone
                float env = std::sin(progress * 3.14159f);
                sample = (std::sin(2.0f * 3.14159f * freq * t) + 0.5f * std::sin(2.0f * 3.14159f * freq * 1.5f * t)) * env * 0.3f;
                break;
            }
            case SoundID::ItemPickup: {
                // Crisp two-tone bell chime
                float freq = (880.0f + progress * 440.0f) * v.pitch;
                float env = 1.0f - progress;
                sample = std::sin(2.0f * 3.14159f * freq * t) * env * 0.65f;
                break;
            }
            case SoundID::Dash: {
                // Dynamic wind whoosh swoosh effect
                float noise = ((rand() % 200) / 100.0f - 1.0f);
                float env = std::sin(progress * 3.14159f); // arch envelope
                float freq = 360.0f * (1.3f - progress * 0.75f) * v.pitch;
                sample = (std::sin(2.0f * 3.14159f * freq * t) * 0.45f + noise * 0.55f) * env * 0.85f;
                break;
            }
            }

            int32_t val = static_cast<int32_t>(buffer[s]) + static_cast<int32_t>(sample * v.volume * 16000.0f);
            buffer[s] = static_cast<int16_t>(std::clamp(val, -32767, 32767));
            v.position += sampleDuration;
        }
    }
}

} // namespace Aetheria
