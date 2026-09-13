#include "IrisShaderUI.hpp"
#include <algorithm>

namespace Aetheria {

IrisShaderUI::IrisShaderUI(IrisShaderManager* manager)
    : shaderManager(manager) {}

bool IrisShaderUI::drawButton(UIRenderer* ui, float x, float y, float w, float h,
                              const std::string& text, bool active,
                              int mouseX, int mouseY, bool clicked) {
    bool hovered = (mouseX >= x && mouseX <= x + w && mouseY >= y && mouseY <= y + h);

    Vec4 bg = active ? Vec4(0.18f, 0.52f, 0.85f, 0.95f) :
              (hovered ? Vec4(0.28f, 0.35f, 0.45f, 0.92f) : Vec4(0.16f, 0.19f, 0.25f, 0.88f));
    Vec4 border = active ? Vec4(0.5f, 0.85f, 1.0f, 1.0f) :
                  (hovered ? Vec4(0.7f, 0.8f, 0.95f, 1.0f) : Vec4(0.3f, 0.36f, 0.46f, 0.75f));

    ui->drawRect(x, y, w, h, bg);
    ui->drawRectOutline(x, y, w, h, 1.5f, border);

    float textX = x + (w - text.length() * 6.0f * 1.7f) * 0.5f;
    float textY = y + (h - 7.0f * 1.7f) * 0.5f;
    ui->drawText(text, textX, textY, 1.7f, active ? Vec4(1, 1, 1, 1) : (hovered ? Vec4(1, 1, 0.7f, 1) : Vec4(0.85f, 0.9f, 0.95f, 1)));

    return hovered && clicked;
}

void IrisShaderUI::drawToggle(UIRenderer* ui, float x, float y, float w, float h,
                              const std::string& label, bool value,
                              int mouseX, int mouseY, bool clicked,
                              const std::function<void(bool)>& onToggle) {
    ui->drawText(label, x, y + 5.0f, 1.7f, {0.9f, 0.92f, 0.98f, 1.0f});

    float btnW = 85.0f;
    float btnH = 26.0f;
    float btnX = x + w - btnW;
    float btnY = y;

    std::string text = value ? "[ ON ]" : "[ OFF ]";
    bool hovered = (mouseX >= btnX && mouseX <= btnX + btnW && mouseY >= btnY && mouseY <= btnY + btnH);

    Vec4 bg = value ? Vec4(0.16f, 0.55f, 0.30f, 0.92f) : Vec4(0.35f, 0.22f, 0.22f, 0.88f);
    Vec4 border = value ? Vec4(0.4f, 0.92f, 0.5f, 1.0f) : Vec4(0.65f, 0.38f, 0.38f, 0.8f);

    ui->drawRect(btnX, btnY, btnW, btnH, bg);
    ui->drawRectOutline(btnX, btnY, btnW, btnH, 1.5f, border);

    float tx = btnX + (btnW - text.length() * 6.0f * 1.7f) * 0.5f;
    float ty = btnY + (btnH - 7.0f * 1.7f) * 0.5f;
    ui->drawText(text, tx, ty, 1.7f, {1, 1, 1, 1});

    if (hovered && clicked && onToggle) {
        onToggle(!value);
    }
}

void IrisShaderUI::render(UIRenderer* ui, int screenWidth, int screenHeight,
                          int mouseX, int mouseY, bool mouseLeftDown, bool mouseLeftClicked) {
    if (!open || !shaderManager) return;

    // 1. Semi-transparent backdrop overlay
    ui->drawRect(0, 0, static_cast<float>(screenWidth), static_cast<float>(screenHeight), {0.02f, 0.03f, 0.06f, 0.82f});

    // 2. Main Dialog Frame
    float panelW = 860.0f;
    float panelH = 580.0f;
    float panelX = (screenWidth - panelW) * 0.5f;
    float panelY = (screenHeight - panelH) * 0.5f;

    // Panel background & double border
    ui->drawRect(panelX, panelY, panelW, panelH, {0.09f, 0.11f, 0.15f, 0.98f});
    ui->drawRectOutline(panelX, panelY, panelW, panelH, 2.5f, {0.25f, 0.65f, 0.95f, 0.85f});
    ui->drawRectOutline(panelX + 4, panelY + 4, panelW - 8, panelH - 8, 1.0f, {0.18f, 0.32f, 0.50f, 0.5f});

    // Title Header
    std::string title = "=== IRIS SHADER PACKS & GRAPHICS PIPELINE ===";
    float titleX = panelX + (panelW - title.length() * 6.0f * 2.1f) * 0.5f;
    ui->drawText(title, titleX, panelY + 16.0f, 2.1f, {1.0f, 0.85f, 0.25f, 1.0f});

    std::string subTitle = "Press [O] to Quick-Toggle | Place custom shaderpack folders in 'shaderpacks/'";
    float subX = panelX + (panelW - subTitle.length() * 6.0f * 1.5f) * 0.5f;
    ui->drawText(subTitle, subX, panelY + 42.0f, 1.5f, {0.65f, 0.75f, 0.88f, 1.0f});

    // Divider
    ui->drawRect(panelX + 20, panelY + 62.0f, panelW - 40, 1.5f, {0.25f, 0.35f, 0.48f, 0.7f});

    // =========================================================================
    // LEFT COLUMN: SHADERPACKS LIST
    // =========================================================================
    float col1X = panelX + 25.0f;
    float colW = 390.0f;
    float contentY = panelY + 76.0f;

    ui->drawText("AVAILABLE SHADER PACKS", col1X + 5.0f, contentY, 1.8f, {0.4f, 0.88f, 1.0f, 1.0f});

    // Shaderpack buttons list
    const auto& packs = shaderManager->getShaderpacks();
    int activeIdx = shaderManager->getActivePackIndex();

    float packBtnY = contentY + 26.0f;
    float packBtnH = 34.0f;
    float packGap = 6.0f;

    for (size_t i = 0; i < packs.size() && i < 7; ++i) {
        bool isActive = ((int)i == activeIdx);
        std::string label = (isActive ? "[ACTIVE] " : "") + packs[i].displayName;

        if (drawButton(ui, col1X, packBtnY + i * (packBtnH + packGap), colW, packBtnH, label, isActive, mouseX, mouseY, mouseLeftClicked)) {
            shaderManager->setActivePack((int)i);
        }
    }

    // Left Column Utility Buttons
    float utilY = panelY + panelH - 105.0f;
    if (drawButton(ui, col1X, utilY, colW, 32.0f, "[ OPEN SHADERPACK FOLDER ]", false, mouseX, mouseY, mouseLeftClicked)) {
        shaderManager->openShaderpacksFolder();
    }
    if (drawButton(ui, col1X, utilY + 38.0f, colW, 32.0f, "[ SCAN & RELOAD SHADERPACKS ]", false, mouseX, mouseY, mouseLeftClicked)) {
        shaderManager->reloadCurrentPack();
    }

    // Vertical Divider
    ui->drawRect(panelX + 430.0f, contentY, 1.5f, panelH - 145.0f, {0.22f, 0.28f, 0.38f, 0.7f});

    // =========================================================================
    // RIGHT COLUMN: SHADERPACK OPTIONS & PROFILES
    // =========================================================================
    float col2X = panelX + 445.0f;
    ui->drawText("SHADER CONFIGURATION & PROFILE", col2X + 5.0f, contentY, 1.8f, {0.4f, 0.88f, 1.0f, 1.0f});

    // Profile Presets row
    float profY = contentY + 28.0f;
    ui->drawText("Profile:", col2X + 5.0f, profY + 5.0f, 1.6f, {0.8f, 0.85f, 0.95f, 1.0f});

    std::string profiles[5] = { "Low", "Medium", "High", "Ultra", "Extreme" };
    float pBtnW = 60.0f;
    float pBtnH = 26.0f;
    float pStartX = col2X + 75.0f;
    for (int i = 0; i < 5; ++i) {
        bool isCur = (shaderManager->getProfile() == profiles[i]);
        if (drawButton(ui, pStartX + i * (pBtnW + 4.0f), profY, pBtnW, pBtnH, profiles[i], isCur, mouseX, mouseY, mouseLeftClicked)) {
            shaderManager->applyProfile(profiles[i]);
        }
    }

    // Option Toggles
    float optY = profY + 36.0f;
    float optGap = 29.0f;

    drawToggle(ui, col2X, optY + 0 * optGap, colW, 25.0f, "Sun God Rays (Volumetric)", shaderManager->getVolumetricLight(), mouseX, mouseY, mouseLeftClicked,
        [this](bool v) { shaderManager->setVolumetricLight(v); shaderManager->saveConfig(); });

    drawToggle(ui, col2X, optY + 1 * optGap, colW, 25.0f, "Water Reflections (SSR)", shaderManager->getWaterReflections(), mouseX, mouseY, mouseLeftClicked,
        [this](bool v) { shaderManager->setWaterReflections(v); shaderManager->saveConfig(); });

    drawToggle(ui, col2X, optY + 2 * optGap, colW, 25.0f, "Ambient Occlusion (SSAO)", shaderManager->getSSAO(), mouseX, mouseY, mouseLeftClicked,
        [this](bool v) { shaderManager->setSSAO(v); shaderManager->saveConfig(); });

    drawToggle(ui, col2X, optY + 3 * optGap, colW, 25.0f, "Dynamic Lens Bloom", shaderManager->getBloom(), mouseX, mouseY, mouseLeftClicked,
        [this](bool v) { shaderManager->setBloom(v); shaderManager->saveConfig(); });

    drawToggle(ui, col2X, optY + 4 * optGap, colW, 25.0f, "Waving Foliage & Leaves", shaderManager->getWavingFoliage(), mouseX, mouseY, mouseLeftClicked,
        [this](bool v) { shaderManager->setWavingFoliage(v); shaderManager->saveConfig(); });

    drawToggle(ui, col2X, optY + 5 * optGap, colW, 25.0f, "Cinematic Motion Blur", shaderManager->getMotionBlur(), mouseX, mouseY, mouseLeftClicked,
        [this](bool v) { shaderManager->setMotionBlur(v); shaderManager->saveConfig(); });

    // Tone Mapping Selector
    float tmY = optY + 6 * optGap + 4.0f;
    ui->drawText("HDR Color Grading:", col2X, tmY + 5.0f, 1.7f, {0.9f, 0.92f, 0.98f, 1.0f});
    std::string tmNames[4] = { "ACES Filmic", "Reinhard", "Vibrant", "Vanilla" };
    int curTM = shaderManager->getToneMapping();
    float tmBtnW = 120.0f;
    float tmBtnH = 26.0f;
    if (drawButton(ui, col2X + colW - tmBtnW, tmY, tmBtnW, tmBtnH, tmNames[curTM % 4], true, mouseX, mouseY, mouseLeftClicked)) {
        shaderManager->setToneMapping((curTM + 1) % 4);
        shaderManager->saveConfig();
    }

    // Iris Info Box
    float infoY = tmY + 34.0f;
    float infoH = 68.0f;
    ui->drawRect(col2X, infoY, colW, infoH, {0.06f, 0.08f, 0.12f, 0.9f});
    ui->drawRectOutline(col2X, infoY, colW, infoH, 1.0f, {0.2f, 0.3f, 0.45f, 0.6f});

    ui->drawText("G-Buffer Architecture: OptiFine / Iris Pipeline", col2X + 8.0f, infoY + 8.0f, 1.4f, {0.35f, 0.85f, 1.0f, 1.0f});
    ui->drawText("colortex0: 16-bit Float HDR  |  colortex1: Surface Normals", col2X + 8.0f, infoY + 26.0f, 1.4f, {0.75f, 0.8f, 0.88f, 1.0f});
    ui->drawText("depthtex0: 24-bit Depth | Cinematic Motion Blur & ACES", col2X + 8.0f, infoY + 44.0f, 1.4f, {0.75f, 0.8f, 0.88f, 1.0f});

    // =========================================================================
    // BOTTOM BAR: DONE BUTTON
    // =========================================================================
    float doneW = 260.0f;
    float doneH = 34.0f;
    float doneX = panelX + (panelW - doneW) * 0.5f;
    float doneY = panelY + panelH - 45.0f;

    if (drawButton(ui, doneX, doneY, doneW, doneH, "APPLY & CLOSE (Esc / O)", true, mouseX, mouseY, mouseLeftClicked)) {
        open = false;
    }
}

} // namespace Aetheria
