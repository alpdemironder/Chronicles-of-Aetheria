#include "MainMenuUI.hpp"
#include <cmath>

namespace Aetheria {

MainMenuUI::MainMenuUI() {}

bool MainMenuUI::drawButton(UIRenderer* ui, float x, float y, float w, float h,
                            const std::string& text, bool primary,
                            int mouseX, int mouseY, bool clicked) {
    bool hovered = (mouseX >= x && mouseX <= x + w && mouseY >= y && mouseY <= y + h);

    Vec4 bg = primary ? (hovered ? Vec4(0.20f, 0.58f, 0.90f, 0.95f) : Vec4(0.12f, 0.38f, 0.65f, 0.90f))
                      : (hovered ? Vec4(0.24f, 0.30f, 0.40f, 0.92f) : Vec4(0.11f, 0.14f, 0.20f, 0.88f));

    Vec4 border = primary ? (hovered ? Vec4(0.70f, 0.92f, 1.0f, 1.0f) : Vec4(0.35f, 0.70f, 0.95f, 0.9f))
                          : (hovered ? Vec4(0.65f, 0.78f, 0.95f, 0.95f) : Vec4(0.25f, 0.32f, 0.44f, 0.75f));

    ui->drawRect(x, y, w, h, bg);
    ui->drawRectOutline(x, y, w, h, primary ? 2.0f : 1.5f, border);

    // Subtle corner accents for sleek look
    if (hovered) {
        ui->drawRect(x, y, 4.0f, 4.0f, {1.0f, 1.0f, 1.0f, 0.9f});
        ui->drawRect(x + w - 4.0f, y, 4.0f, 4.0f, {1.0f, 1.0f, 1.0f, 0.9f});
        ui->drawRect(x, y + h - 4.0f, 4.0f, 4.0f, {1.0f, 1.0f, 1.0f, 0.9f});
        ui->drawRect(x + w - 4.0f, y + h - 4.0f, 4.0f, 4.0f, {1.0f, 1.0f, 1.0f, 0.9f});
    }

    float textScale = primary ? 1.9f : 1.75f;
    float tx = x + (w - text.length() * 6.0f * textScale) * 0.5f;
    float ty = y + (h - 7.0f * textScale) * 0.5f;

    Vec4 textColor = primary ? (hovered ? Vec4(1.0f, 1.0f, 1.0f, 1.0f) : Vec4(0.92f, 0.96f, 1.0f, 1.0f))
                             : (hovered ? Vec4(1.0f, 0.95f, 0.65f, 1.0f) : Vec4(0.85f, 0.90f, 0.95f, 0.95f));

    ui->drawText(text, tx, ty, textScale, textColor);

    return hovered && clicked;
}

void MainMenuUI::render(UIRenderer* ui, int screenWidth, int screenHeight,
                        int mouseX, int mouseY, bool mouseLeftClicked,
                        float totalTime) {
    float sw = static_cast<float>(screenWidth);
    float sh = static_cast<float>(screenHeight);
    float cx = sw * 0.5f;

    // 1. Soft dark cinematic backdrop overlay
    ui->drawRect(0, 0, sw, sh, {0.03f, 0.05f, 0.08f, 0.45f});

    // Top and bottom cinematic dark gradient bands
    ui->drawRect(0, 0, sw, 90.0f, {0.02f, 0.03f, 0.05f, 0.65f});
    ui->drawRect(0, sh - 70.0f, sw, 70.0f, {0.02f, 0.03f, 0.05f, 0.75f});

    // 2. Title Logo & Branding
    float titleY = sh * 0.18f;

    // Subtle breathing pulse for title glow
    float pulse = 0.95f + 0.05f * std::sin(totalTime * 2.5f);

    // Title Drop Shadow
    ui->drawTextCentered("CHRONICLES OF AETHERIA", cx + 2.0f, titleY + 3.0f, 3.2f, {0.0f, 0.0f, 0.0f, 0.85f});
    // Main Title in Brilliant Gold
    ui->drawTextCentered("CHRONICLES OF AETHERIA", cx, titleY, 3.2f, {1.0f * pulse, 0.86f * pulse, 0.32f, 1.0f});

    // Subtitle
    std::string subtitle = "OPENGL 3.3 CORE RPG SANDBOX  //  365 BLOCKS & 35 BIOMES";
    ui->drawTextCentered(subtitle, cx + 1.0f, titleY + 33.0f, 1.45f, {0.0f, 0.0f, 0.0f, 0.7f});
    ui->drawTextCentered(subtitle, cx, titleY + 32.0f, 1.45f, {0.45f, 0.85f, 1.0f, 0.95f});

    // 3. Central Menu Buttons Deck
    float btnW = 380.0f;
    float btnH = 42.0f;
    float btnGap = 10.0f;
    float btnX = cx - btnW * 0.5f;
    float startY = sh * 0.36f;

    // Background frosted chassis for buttons
    float chassisPad = 16.0f;
    float chassisH = 6 * btnH + 5 * btnGap + chassisPad * 2.0f;
    ui->drawRect(btnX - chassisPad, startY - chassisPad, btnW + chassisPad * 2.0f, chassisH, {0.05f, 0.07f, 0.10f, 0.70f});
    ui->drawRectOutline(btnX - chassisPad, startY - chassisPad, btnW + chassisPad * 2.0f, chassisH, 1.5f, {0.20f, 0.35f, 0.55f, 0.5f});

    // Play Button (Primary)
    if (drawButton(ui, btnX, startY + 0 * (btnH + btnGap), btnW, btnH, "> PLAY SINGLEPLAYER <", true, mouseX, mouseY, mouseLeftClicked)) {
        if (onPlay) onPlay();
    }

    // Co-op Multiplayer Button
    if (drawButton(ui, btnX, startY + 1 * (btnH + btnGap), btnW, btnH, "> MULTIPLAYER (COK OYUNCULU) <", false, mouseX, mouseY, mouseLeftClicked)) {
        if (onOpenMultiplayer) onOpenMultiplayer();
    }

    // Update Calendar & Roadmap Button
    if (drawButton(ui, btnX, startY + 2 * (btnH + btnGap), btnW, btnH, "UPDATE CALENDAR & ROADMAP", false, mouseX, mouseY, mouseLeftClicked)) {
        if (onOpenCalendar) onOpenCalendar();
    }

    // Iris Shaders Button
    if (drawButton(ui, btnX, startY + 3 * (btnH + btnGap), btnW, btnH, "SHADER PACKS (IRIS)", false, mouseX, mouseY, mouseLeftClicked)) {
        if (onOpenShaders) onOpenShaders();
    }

    // Settings & Options Button
    if (drawButton(ui, btnX, startY + 4 * (btnH + btnGap), btnW, btnH, "SETTINGS & OPTIONS", false, mouseX, mouseY, mouseLeftClicked)) {
        if (onOpenSettings) onOpenSettings();
    }

    // Quit Button
    if (drawButton(ui, btnX, startY + 5 * (btnH + btnGap), btnW, btnH, "QUIT TO DESKTOP", false, mouseX, mouseY, mouseLeftClicked)) {
        if (onQuit) onQuit();
    }

    // 4. Footer Bar
    ui->drawText("Chronicles of Aetheria v2.4 Core", 24.0f, sh - 36.0f, 1.35f, {0.6f, 0.7f, 0.82f, 0.85f});

    std::string hint = "Double-tap [W] Sprint  |  [R] Dash  |  [SHIFT] Crouch  |  [C] Zoom  |  [K] Calendar";
    ui->drawTextCentered(hint, cx, sh - 36.0f, 1.35f, {0.85f, 0.88f, 0.95f, 0.9f});

    std::string tech = "Modern OpenGL 3.3 Core";
    float techW = tech.length() * 6.0f * 1.35f;
    ui->drawText(tech, sw - techW - 24.0f, sh - 36.0f, 1.35f, {0.4f, 0.82f, 0.95f, 0.85f});
}

} // namespace Aetheria
