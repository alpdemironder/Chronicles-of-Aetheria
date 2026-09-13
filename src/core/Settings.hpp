#pragma once
#include <string>

namespace Aetheria {

struct GraphicsSettings {
    int renderDistance = 8;     // Chunks radius (2 to 128)
    float fov = 75.0f;          // Field of view (60 to 110)
    bool vsync = true;          // Vertical synchronization
    bool ambientOcclusion = true; // Voxel AO shading
    bool fog = true;            // Atmospheric distance fog
    bool wireframe = false;     // Debug polygon wireframe
    bool fullscreen = false;    // Fullscreen window
    bool motionBlur = true;     // Camera & velocity motion blur
    float motionBlurStrength = 0.60f; // Blur strength (0.0 to 1.5)
};

struct AudioSettings {
    float masterVolume = 0.8f;  // 0.0 to 1.0
    float sfxVolume = 0.8f;     // 0.0 to 1.0
    bool ambientDrone = true;   // Ambient procedural drone
};

struct ControlsSettings {
    float mouseSensitivity = 1.0f; // 0.2 to 3.0
    bool invertY = false;
    bool autoStepUp = true;     // Smooth voxel stair climbing
    float stepHeight = 0.5f;    // Step-up height limit (default: 0.5 blocks)
};

struct GameplaySettings {
    bool thirdPerson = false;
    float timeOfDay = 6000.0f;  // 0 to 24000 (6000 = noon)
    float daySpeed = 1.0f;      // Cycle multiplier
    bool peacefulMode = false;
};

struct AccountSettings {
    std::string username = "Alp";
    std::string characterClass = "Savasci";
    std::string race = "Insan";
    bool rememberMe = true;
    bool isLoggedIn = false;
};

class Settings {
public:
    static Settings& instance();

    GraphicsSettings graphics;
    AudioSettings audio;
    ControlsSettings controls;
    GameplaySettings gameplay;
    AccountSettings account;

    void resetDefaults();
    void load();
    void save();

private:
    Settings() { resetDefaults(); }
};

} // namespace Aetheria
