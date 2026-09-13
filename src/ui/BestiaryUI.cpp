#include "BestiaryUI.hpp"
#include "../core/Window.hpp"
#include <sstream>
#include <algorithm>

namespace Aetheria {

static const char* WORK_TRAIT_NAMES[6] = {
    "None", "Handiwork (Builds Blueprints)", "Lumbering (Logs Trees)",
    "Mining (Extracts Stone/Ores)", "Farming (Grows Crops)", "Base Defense (Guards Camp)"
};

BestiaryUI::BestiaryUI(UIRenderer* ui) : ui(ui) {
}

void BestiaryUI::handleInput(const Window& window) {
    if (!isOpen) return;

    if (window.isKeyPressed(VK_UP) || window.isKeyPressed('W')) {
        selectedIndex = std::max(0, selectedIndex - 1);
    }
    if (window.isKeyPressed(VK_DOWN) || window.isKeyPressed('S')) {
        const auto& all = CreatureRegistry::getAll();
        selectedIndex = std::min(static_cast<int>(all.size()) - 1, selectedIndex + 1);
    }
    // Creature navigation is handled above.
    // Menu toggling ('M' / ESC) is cleanly handled in main.cpp.
}

void BestiaryUI::render(int screenWidth, int screenHeight) {
    if (!isOpen) return;

    float sw = static_cast<float>(screenWidth);
    float sh = static_cast<float>(screenHeight);

    ui->drawRect(0, 0, sw, sh, {0.02f, 0.03f, 0.05f, 0.75f});

    float menuW = 720.0f;
    float menuH = 480.0f;
    float menuX = (sw - menuW) * 0.5f;
    float menuY = (sh - menuH) * 0.5f;

    ui->drawRect(menuX, menuY, menuW, menuH, {0.12f, 0.14f, 0.18f, 0.95f});
    ui->drawRectOutline(menuX, menuY, menuW, menuH, 2.0f, {0.85f, 0.35f, 0.25f, 1.0f});

    // Title
    ui->drawRect(menuX, menuY, menuW, 40, {0.35f, 0.15f, 0.12f, 1.0f});
    ui->drawText("CREATURE BESTIARY & PAL CODEX [Press M or ESC to Close]", menuX + 20, menuY + 12, 1.8f, {0.98f, 0.90f, 0.85f, 1.0f});

    // Left column: Creature List
    float listX = menuX + 20.0f;
    float listY = menuY + 55.0f;
    float listW = 280.0f;
    float itemH = 34.0f;

    const auto& creatures = CreatureRegistry::getAll();

    int startIdx = std::clamp(selectedIndex - 5, 0, std::max(0, static_cast<int>(creatures.size()) - 11));
    int endIdx = std::min(startIdx + 11, static_cast<int>(creatures.size()));

    for (int i = startIdx; i < endIdx; ++i) {
        float iy = listY + (i - startIdx) * (itemH + 4);
        bool isSelected = (i == selectedIndex);

        Vec4 bg = isSelected ? Vec4(0.45f, 0.20f, 0.18f, 0.95f) : Vec4(0.18f, 0.20f, 0.24f, 0.9f);
        ui->drawRect(listX, iy, listW, itemH, bg);

        if (isSelected) {
            ui->drawRectOutline(listX, iy, listW, itemH, 2.0f, {0.95f, 0.85f, 0.35f, 1.0f});
        }

        const auto& c = creatures[i];
        ui->drawRect(listX + 4, iy + 4, 26, 26, c.primaryColor);
        ui->drawText(c.name, listX + 36, iy + 8, 1.3f, {0.95f, 0.95f, 0.95f, 1.0f});
    }

    // Right column: Detailed Stats Card
    float detailsX = menuX + 320.0f;
    float detailsY = menuY + 55.0f;
    float detailsW = menuW - 340.0f;
    float detailsH = menuH - 80.0f;

    ui->drawRect(detailsX, detailsY, detailsW, detailsH, {0.15f, 0.18f, 0.22f, 0.95f});
    ui->drawRectOutline(detailsX, detailsY, detailsW, detailsH, 1.5f, {0.35f, 0.40f, 0.45f, 1.0f});

    if (selectedIndex >= 0 && selectedIndex < static_cast<int>(creatures.size())) {
        const auto& c = creatures[selectedIndex];

        // Header name
        ui->drawText(c.name, detailsX + 20, detailsY + 20, 2.0f, {0.98f, 0.85f, 0.30f, 1.0f});

        // Classification
        std::string classification = c.isBoss ? "WORLD BOSS - EXTREME THREAT" : (c.isHostile ? "Hostile Monster" : "Docile Wildlife / Pal");
        Vec4 classColor = c.isBoss ? Vec4(1.0f, 0.2f, 0.2f, 1.0f) : (c.isHostile ? Vec4(0.9f, 0.5f, 0.2f, 1.0f) : Vec4(0.3f, 0.9f, 0.4f, 1.0f));
        ui->drawText(classification, detailsX + 20, detailsY + 46, 1.3f, classColor);

        // Stats
        std::stringstream ssHP, ssAtk, ssSpd, ssXP;
        ssHP << "Max Health: " << static_cast<int>(c.maxHP) << " HP";
        ssAtk << "Attack Power: " << static_cast<int>(c.attackDamage) << " DMG";
        ssSpd << "Movement Speed: " << c.moveSpeed << " m/s";
        ssXP << "XP Reward: +" << c.xpReward << " XP";

        ui->drawText(ssHP.str(), detailsX + 20, detailsY + 80, 1.4f, {0.95f, 0.95f, 0.95f, 1.0f});
        ui->drawText(ssAtk.str(), detailsX + 20, detailsY + 105, 1.4f, {0.95f, 0.95f, 0.95f, 1.0f});
        ui->drawText(ssSpd.str(), detailsX + 20, detailsY + 130, 1.4f, {0.95f, 0.95f, 0.95f, 1.0f});
        ui->drawText(ssXP.str(), detailsX + 20, detailsY + 155, 1.4f, {0.95f, 0.85f, 0.35f, 1.0f});

        // Pal Work Trait
        ui->drawText("Pal Base Suitability:", detailsX + 20, detailsY + 195, 1.5f, {0.4f, 0.8f, 1.0f, 1.0f});
        std::string traitStr = WORK_TRAIT_NAMES[static_cast<int>(c.workTrait)];
        ui->drawText(traitStr, detailsX + 20, detailsY + 220, 1.3f, {0.85f, 0.95f, 0.85f, 1.0f});

        // 3D Model preview box
        ui->drawRect(detailsX + 20, detailsY + 260, 80, 80, c.primaryColor);
        ui->drawRectOutline(detailsX + 20, detailsY + 260, 80, 80, 2.0f, {0.9f, 0.9f, 0.9f, 0.9f});
        ui->drawText("[3D Voxel Model Preview]", detailsX + 115, detailsY + 295, 1.3f, {0.7f, 0.7f, 0.7f, 1.0f});
    }

    ui->drawText("[UP/DOWN] Browse Creatures  |  [ESC] Close Bestiary", menuX + 30, menuY + menuH - 24, 1.3f, {0.95f, 0.85f, 0.40f, 1.0f});
}

} // namespace Aetheria
