#include "SettingsUI.hpp"
#include <algorithm>
#include <sstream>
#include <iomanip>

namespace Aetheria {

SettingsUI::SettingsUI() {}

bool SettingsUI::drawButton(UIRenderer* ui, float x, float y, float w, float h,
                            const std::string& text, bool active,
                            int mouseX, int mouseY, bool clicked) {
    bool hovered = (mouseX >= x && mouseX <= x + w && mouseY >= y && mouseY <= y + h);

    Vec4 bg = active ? Vec4(0.2f, 0.6f, 0.9f, 0.95f) :
              (hovered ? Vec4(0.32f, 0.38f, 0.48f, 0.9f) : Vec4(0.2f, 0.23f, 0.29f, 0.85f));
    Vec4 border = active ? Vec4(0.6f, 0.9f, 1.0f, 1.0f) :
                  (hovered ? Vec4(0.7f, 0.75f, 0.85f, 1.0f) : Vec4(0.35f, 0.4f, 0.5f, 0.7f));

    ui->drawRect(x, y, w, h, bg);
    ui->drawRectOutline(x, y, w, h, 1.5f, border);

    float textX = x + (w - text.length() * 6.0f * 1.8f) * 0.5f;
    float textY = y + (h - 7.0f * 1.8f) * 0.5f;
    ui->drawText(text, textX, textY, 1.8f, active ? Vec4(1, 1, 1, 1) : (hovered ? Vec4(1, 1, 0.8f, 1) : Vec4(0.85f, 0.85f, 0.9f, 1)));

    return hovered && clicked;
}

void SettingsUI::drawSlider(UIRenderer* ui, float x, float y, float w, float h,
                            const std::string& label, const std::string& valText,
                            float& value, float minVal, float maxVal,
                            int mouseX, int mouseY, bool mouseDown) {
    ui->drawText(label, x, y, 1.8f, {0.9f, 0.9f, 0.95f, 1.0f});
    ui->drawText(valText, x + w - valText.length() * 6.0f * 1.8f, y, 1.8f, {0.35f, 0.85f, 1.0f, 1.0f});

    float barY = y + 20.0f;
    float barH = 14.0f;

    float progress = (value - minVal) / (maxVal - minVal);
    ui->drawProgressBar(x, barY, w, barH, progress, {0.25f, 0.65f, 0.95f, 0.95f}, {0.15f, 0.17f, 0.22f, 0.9f});

    // Slider knob
    float knobX = x + progress * (w - 10.0f);
    ui->drawRect(knobX, barY - 3.0f, 10.0f, barH + 6.0f, {0.95f, 0.95f, 1.0f, 1.0f});

    // Handle mouse drag
    if (mouseDown && mouseX >= x && mouseX <= x + w && mouseY >= y + 10.0f && mouseY <= y + 40.0f) {
        float f = std::clamp((static_cast<float>(mouseX) - x) / w, 0.0f, 1.0f);
        value = minVal + f * (maxVal - minVal);
    }
}

void SettingsUI::drawToggle(UIRenderer* ui, float x, float y, float w, float h,
                            const std::string& label, bool& value,
                            int mouseX, int mouseY, bool clicked) {
    ui->drawText(label, x, y + 6.0f, 1.8f, {0.9f, 0.9f, 0.95f, 1.0f});

    float btnW = 90.0f;
    float btnH = 26.0f;
    float btnX = x + w - btnW;
    float btnY = y;

    std::string text = value ? "[ ON ]" : "[ OFF ]";
    bool hovered = (mouseX >= btnX && mouseX <= btnX + btnW && mouseY >= btnY && mouseY <= btnY + btnH);

    Vec4 bg = value ? Vec4(0.18f, 0.58f, 0.32f, 0.9f) : Vec4(0.35f, 0.25f, 0.25f, 0.85f);
    Vec4 border = value ? Vec4(0.4f, 0.9f, 0.5f, 1.0f) : Vec4(0.6f, 0.4f, 0.4f, 0.8f);

    ui->drawRect(btnX, btnY, btnW, btnH, bg);
    ui->drawRectOutline(btnX, btnY, btnW, btnH, 1.5f, border);

    float tx = btnX + (btnW - text.length() * 6.0f * 1.8f) * 0.5f;
    float ty = btnY + (btnH - 7.0f * 1.8f) * 0.5f;
    ui->drawText(text, tx, ty, 1.8f, {1, 1, 1, 1});

    if (hovered && clicked) {
        value = !value;
    }
}

void SettingsUI::render(UIRenderer* ui, int screenWidth, int screenHeight,
                        int mouseX, int mouseY, bool mouseLeftDown, bool mouseLeftClicked) {
    if (!open) return;

    Settings& cfg = Settings::instance();

    // 1. Semi-transparent backdrop overlay
    ui->drawRect(0, 0, static_cast<float>(screenWidth), static_cast<float>(screenHeight), {0.02f, 0.03f, 0.05f, 0.78f});

    // 2. Main Dialog Frame
    float panelW = 720.0f;
    float panelH = 540.0f;
    float panelX = (screenWidth - panelW) * 0.5f;
    float panelY = (screenHeight - panelH) * 0.5f;

    // Panel background & double border
    ui->drawRect(panelX, panelY, panelW, panelH, {0.11f, 0.13f, 0.17f, 0.97f});
    ui->drawRectOutline(panelX, panelY, panelW, panelH, 2.0f, {0.35f, 0.65f, 0.95f, 0.8f});
    ui->drawRectOutline(panelX + 4, panelY + 4, panelW - 8, panelH - 8, 1.0f, {0.2f, 0.35f, 0.5f, 0.5f});

    // Title Header
    std::string title = "=== SETTINGS & CONFIGURATION ===";
    float titleX = panelX + (panelW - title.length() * 6.0f * 2.2f) * 0.5f;
    ui->drawText(title, titleX, panelY + 18.0f, 2.2f, {1.0f, 0.85f, 0.3f, 1.0f});

    // 3. Tab Buttons
    float tabY = panelY + 54.0f;
    float tabW = 150.0f;
    float tabH = 34.0f;
    float tabGap = 12.0f;
    float tabsStartX = panelX + (panelW - (4 * tabW + 3 * tabGap)) * 0.5f;

    if (drawButton(ui, tabsStartX + 0 * (tabW + tabGap), tabY, tabW, tabH, "GRAPHICS", currentTab == SettingsTab::Graphics, mouseX, mouseY, mouseLeftClicked)) {
        currentTab = SettingsTab::Graphics;
    }
    if (drawButton(ui, tabsStartX + 1 * (tabW + tabGap), tabY, tabW, tabH, "AUDIO", currentTab == SettingsTab::Audio, mouseX, mouseY, mouseLeftClicked)) {
        currentTab = SettingsTab::Audio;
    }
    if (drawButton(ui, tabsStartX + 2 * (tabW + tabGap), tabY, tabW, tabH, "CONTROLS", currentTab == SettingsTab::Controls, mouseX, mouseY, mouseLeftClicked)) {
        currentTab = SettingsTab::Controls;
    }
    if (drawButton(ui, tabsStartX + 3 * (tabW + tabGap), tabY, tabW, tabH, "GAMEPLAY", currentTab == SettingsTab::Gameplay, mouseX, mouseY, mouseLeftClicked)) {
        currentTab = SettingsTab::Gameplay;
    }

    // Divider
    ui->drawRect(panelX + 24, tabY + tabH + 12, panelW - 48, 1.5f, {0.25f, 0.3f, 0.4f, 0.7f});

    // 4. Tab Content Area
    float contentX = panelX + 45.0f;
    float contentY = tabY + tabH + 28.0f;
    float contentW = panelW - 90.0f;
    float rowSpacing = 38.0f;

    if (currentTab == SettingsTab::Graphics) {
        // Render Distance Slider
        float rd = static_cast<float>(cfg.graphics.renderDistance);
        std::string rdText = std::to_string(cfg.graphics.renderDistance) + " Chunks";
        drawSlider(ui, contentX, contentY, contentW, 34.0f, "Render Distance", rdText, rd, 2.0f, 128.0f, mouseX, mouseY, mouseLeftDown);
        cfg.graphics.renderDistance = static_cast<int>(std::round(rd));

        // Field of View Slider
        std::string fovText = std::to_string(static_cast<int>(std::round(cfg.graphics.fov))) + " deg";
        drawSlider(ui, contentX, contentY + rowSpacing * 1.0f, contentW, 34.0f, "Field of View (FOV)", fovText, cfg.graphics.fov, 60.0f, 110.0f, mouseX, mouseY, mouseLeftDown);

        // VSync Toggle
        drawToggle(ui, contentX, contentY + rowSpacing * 2.0f, contentW, 28.0f, "Vertical Sync (VSync)", cfg.graphics.vsync, mouseX, mouseY, mouseLeftClicked);

        // Ambient Occlusion Toggle
        drawToggle(ui, contentX, contentY + rowSpacing * 3.0f, contentW, 28.0f, "Voxel Ambient Occlusion (AO)", cfg.graphics.ambientOcclusion, mouseX, mouseY, mouseLeftClicked);

        // Atmospheric Fog Toggle
        drawToggle(ui, contentX, contentY + rowSpacing * 4.0f, contentW, 28.0f, "Atmospheric Distance Fog", cfg.graphics.fog, mouseX, mouseY, mouseLeftClicked);

        // Motion Blur Toggle
        drawToggle(ui, contentX, contentY + rowSpacing * 5.0f, contentW, 28.0f, "Cinematic Motion Blur", cfg.graphics.motionBlur, mouseX, mouseY, mouseLeftClicked);

        // Wireframe Debug Toggle
        drawToggle(ui, contentX, contentY + rowSpacing * 6.0f, contentW, 28.0f, "Polygon Wireframe Mode", cfg.graphics.wireframe, mouseX, mouseY, mouseLeftClicked);

        // Iris Minecraft Shaderpacks Entry Button
        if (drawButton(ui, contentX, contentY + rowSpacing * 7.0f, contentW, 32.0f, "[ SHADER PACKS (IRIS) ... ]", false, mouseX, mouseY, mouseLeftClicked)) {
            if (onOpenIris) onOpenIris();
        }
    }
    else if (currentTab == SettingsTab::Audio) {
        // Master Volume Slider
        std::string mvText = std::to_string(static_cast<int>(std::round(cfg.audio.masterVolume * 100.0f))) + "%";
        drawSlider(ui, contentX, contentY, contentW, 36.0f, "Master Volume", mvText, cfg.audio.masterVolume, 0.0f, 1.0f, mouseX, mouseY, mouseLeftDown);

        // SFX Volume Slider
        std::string sfxText = std::to_string(static_cast<int>(std::round(cfg.audio.sfxVolume * 100.0f))) + "%";
        drawSlider(ui, contentX, contentY + rowSpacing * 1.2f, contentW, 36.0f, "Sound Effects (SFX)", sfxText, cfg.audio.sfxVolume, 0.0f, 1.0f, mouseX, mouseY, mouseLeftDown);

        // Ambient Drone Synthesizer Toggle
        drawToggle(ui, contentX, contentY + rowSpacing * 2.4f, contentW, 30.0f, "Ambient Synth Atmosphere", cfg.audio.ambientDrone, mouseX, mouseY, mouseLeftClicked);
    }
    else if (currentTab == SettingsTab::Controls) {
        // Mouse Sensitivity Slider
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(2) << cfg.controls.mouseSensitivity << "x";
        drawSlider(ui, contentX, contentY, contentW, 36.0f, "Mouse Look Sensitivity", ss.str(), cfg.controls.mouseSensitivity, 0.2f, 3.0f, mouseX, mouseY, mouseLeftDown);

        // Invert Mouse Y Toggle
        drawToggle(ui, contentX, contentY + rowSpacing * 1.2f, contentW, 30.0f, "Invert Vertical Axis (Invert Y)", cfg.controls.invertY, mouseX, mouseY, mouseLeftClicked);

        // Auto Step-Up Climbing Toggle
        drawToggle(ui, contentX, contentY + rowSpacing * 2.2f, contentW, 30.0f, "Smooth Auto Step-Up Climbing", cfg.controls.autoStepUp, mouseX, mouseY, mouseLeftClicked);

        // Step-Up Height Limit Slider
        std::ostringstream ssStep;
        ssStep << std::fixed << std::setprecision(2) << cfg.controls.stepHeight << " Blocks";
        drawSlider(ui, contentX, contentY + rowSpacing * 3.2f, contentW, 36.0f, "Step-Up Height Limit", ssStep.str(), cfg.controls.stepHeight, 0.1f, 1.25f, mouseX, mouseY, mouseLeftDown);
    }
    else if (currentTab == SettingsTab::Gameplay) {
        // Camera Perspective Toggle
        drawToggle(ui, contentX, contentY, contentW, 30.0f, "Third-Person Camera (F5)", cfg.gameplay.thirdPerson, mouseX, mouseY, mouseLeftClicked);

        // Day/Night Cycle Speed Slider
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(1) << cfg.gameplay.daySpeed << "x";
        drawSlider(ui, contentX, contentY + rowSpacing * 1.1f, contentW, 36.0f, "Day/Night Cycle Speed", ss.str(), cfg.gameplay.daySpeed, 0.0f, 5.0f, mouseX, mouseY, mouseLeftDown);

        // Time of Day Presets
        ui->drawText("Time of Day Quick Presets:", contentX, contentY + rowSpacing * 2.3f, 1.8f, {0.9f, 0.9f, 0.95f, 1.0f});
        float btnW = 120.0f;
        float btnH = 28.0f;
        float btnY = contentY + rowSpacing * 2.3f + 24.0f;
        float pGap = (contentW - 4 * btnW) / 3.0f;

        if (drawButton(ui, contentX + 0 * (btnW + pGap), btnY, btnW, btnH, "SUNRISE", false, mouseX, mouseY, mouseLeftClicked)) {
            cfg.gameplay.timeOfDay = 0.0f;
        }
        if (drawButton(ui, contentX + 1 * (btnW + pGap), btnY, btnW, btnH, "NOON", false, mouseX, mouseY, mouseLeftClicked)) {
            cfg.gameplay.timeOfDay = 6000.0f;
        }
        if (drawButton(ui, contentX + 2 * (btnW + pGap), btnY, btnW, btnH, "SUNSET", false, mouseX, mouseY, mouseLeftClicked)) {
            cfg.gameplay.timeOfDay = 12000.0f;
        }
        if (drawButton(ui, contentX + 3 * (btnW + pGap), btnY, btnW, btnH, "MIDNIGHT", false, mouseX, mouseY, mouseLeftClicked)) {
            cfg.gameplay.timeOfDay = 18000.0f;
        }
    }

    // 5. Bottom Action Buttons
    float botY = panelY + panelH - 52.0f;
    float rstW = 160.0f;
    float rstH = 34.0f;

    if (drawButton(ui, contentX, botY, rstW, rstH, "RESET DEFAULTS", false, mouseX, mouseY, mouseLeftClicked)) {
        cfg.resetDefaults();
    }

    float titleBtnW = 210.0f;
    float titleBtnX = contentX + (contentW - titleBtnW) * 0.5f;
    if (drawButton(ui, titleBtnX, botY, titleBtnW, rstH, "SAVE & QUIT TO TITLE", false, mouseX, mouseY, mouseLeftClicked)) {
        cfg.save();
        open = false;
        if (onQuitToTitle) onQuitToTitle();
    }

    float closeW = 210.0f;
    float closeX = contentX + contentW - closeW;
    if (drawButton(ui, closeX, botY, closeW, rstH, "SAVE & RESUME (Esc)", true, mouseX, mouseY, mouseLeftClicked)) {
        cfg.save();
        open = false;
    }
}

} // namespace Aetheria
