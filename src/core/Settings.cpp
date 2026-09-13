#include "Settings.hpp"
#include <fstream>
#include <iostream>

namespace Aetheria {

Settings& Settings::instance() {
    static Settings inst;
    return inst;
}

void Settings::resetDefaults() {
    graphics.renderDistance = 6;
    graphics.fov = 75.0f;
    graphics.vsync = true;
    graphics.ambientOcclusion = true;
    graphics.fog = true;
    graphics.wireframe = false;
    graphics.fullscreen = false;
    graphics.motionBlur = true;
    graphics.motionBlurStrength = 0.60f;

    audio.masterVolume = 0.8f;
    audio.sfxVolume = 0.8f;
    audio.ambientDrone = true;

    controls.mouseSensitivity = 1.0f;
    controls.invertY = false;
    controls.autoStepUp = true;
    controls.stepHeight = 0.5f;

    gameplay.thirdPerson = false;
    gameplay.timeOfDay = 6000.0f;
    gameplay.daySpeed = 1.0f;
    gameplay.peacefulMode = false;
}

void Settings::load() {
    // Attempt reading simple settings.cfg if present
    std::ifstream file("settings.cfg");
    if (!file.is_open()) return;

    std::string key;
    while (file >> key) {
        if (key == "renderDistance") file >> graphics.renderDistance;
        else if (key == "fov") file >> graphics.fov;
        else if (key == "vsync") file >> graphics.vsync;
        else if (key == "ambientOcclusion") file >> graphics.ambientOcclusion;
        else if (key == "fog") file >> graphics.fog;
        else if (key == "wireframe") file >> graphics.wireframe;
        else if (key == "motionBlur") file >> graphics.motionBlur;
        else if (key == "motionBlurStrength") file >> graphics.motionBlurStrength;
        else if (key == "masterVolume") file >> audio.masterVolume;
        else if (key == "sfxVolume") file >> audio.sfxVolume;
        else if (key == "mouseSensitivity") file >> controls.mouseSensitivity;
        else if (key == "invertY") file >> controls.invertY;
        else if (key == "autoStepUp") file >> controls.autoStepUp;
        else if (key == "stepHeight") file >> controls.stepHeight;
        else if (key == "thirdPerson") file >> gameplay.thirdPerson;
    }
}

void Settings::save() {
    std::ofstream file("settings.cfg");
    if (!file.is_open()) return;

    file << "renderDistance " << graphics.renderDistance << "\n";
    file << "fov " << graphics.fov << "\n";
    file << "vsync " << graphics.vsync << "\n";
    file << "ambientOcclusion " << graphics.ambientOcclusion << "\n";
    file << "fog " << graphics.fog << "\n";
    file << "wireframe " << graphics.wireframe << "\n";
    file << "motionBlur " << graphics.motionBlur << "\n";
    file << "motionBlurStrength " << graphics.motionBlurStrength << "\n";
    file << "masterVolume " << audio.masterVolume << "\n";
    file << "sfxVolume " << audio.sfxVolume << "\n";
    file << "mouseSensitivity " << controls.mouseSensitivity << "\n";
    file << "invertY " << controls.invertY << "\n";
    file << "autoStepUp " << controls.autoStepUp << "\n";
    file << "stepHeight " << controls.stepHeight << "\n";
    file << "thirdPerson " << gameplay.thirdPerson << "\n";
}

} // namespace Aetheria
