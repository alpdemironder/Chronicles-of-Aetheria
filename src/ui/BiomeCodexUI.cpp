#include "BiomeCodexUI.hpp"
#include "../core/Window.hpp"
#include "../world/BlockRegistry.hpp"
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace Aetheria {

BiomeCodexUI::BiomeCodexUI(UIRenderer* ui) : ui(ui) {
}

void BiomeCodexUI::handleInput(const Window& window) {
    if (!isOpen) return;

    if (window.isKeyPressed(VK_UP) || window.isKeyPressed('W')) {
        selectedIndex = std::max(1, selectedIndex - 1);
    }
    if (window.isKeyPressed(VK_DOWN) || window.isKeyPressed('S')) {
        selectedIndex = std::min(35, selectedIndex + 1);
    }
    // Biome navigation is handled above.
    // Menu toggling ('J' / ESC) is cleanly handled in main.cpp.
}

void BiomeCodexUI::render(int screenWidth, int screenHeight) {
    if (!isOpen) return;

    float sw = static_cast<float>(screenWidth);
    float sh = static_cast<float>(screenHeight);

    ui->drawRect(0, 0, sw, sh, {0.02f, 0.03f, 0.05f, 0.75f});

    float menuW = 760.0f;
    float menuH = 490.0f;
    float menuX = (sw - menuW) * 0.5f;
    float menuY = (sh - menuH) * 0.5f;

    ui->drawRect(menuX, menuY, menuW, menuH, {0.10f, 0.14f, 0.16f, 0.95f});
    ui->drawRectOutline(menuX, menuY, menuW, menuH, 2.0f, {0.20f, 0.75f, 0.55f, 1.0f});

    // Header
    ui->drawRect(menuX, menuY, menuW, 40, {0.12f, 0.32f, 0.25f, 1.0f});
    ui->drawText("35 PROCEDURAL BIOMES EXPLORATION CODEX [Press J or ESC to Close]", menuX + 20, menuY + 12, 1.8f, {0.90f, 0.98f, 0.90f, 1.0f});

    // Left List (35 biomes)
    float listX = menuX + 20.0f;
    float listY = menuY + 52.0f;
    float listW = 310.0f;
    float itemH = 32.0f;

    int startIdx = std::clamp(selectedIndex - 5, 1, 24);
    int endIdx = std::min(startIdx + 12, 36);

    for (int i = startIdx; i < endIdx; ++i) {
        float iy = listY + (i - startIdx) * (itemH + 3);
        bool isSelected = (i == selectedIndex);

        Vec4 bg = isSelected ? Vec4(0.20f, 0.45f, 0.35f, 0.95f) : Vec4(0.14f, 0.18f, 0.20f, 0.9f);
        ui->drawRect(listX, iy, listW, itemH, bg);

        if (isSelected) {
            ui->drawRectOutline(listX, iy, listW, itemH, 2.0f, {0.95f, 0.85f, 0.35f, 1.0f});
        }

        const auto& b = BiomeRegistry::get(static_cast<uint8_t>(i));

        // Color indicator
        ui->drawRect(listX + 4, iy + 4, 24, 24, b.fogColor);

        std::stringstream ssName;
        ssName << i << ". " << b.name;
        ui->drawText(ssName.str(), listX + 34, iy + 8, 1.2f, {0.95f, 0.95f, 0.95f, 1.0f});
    }

    // Right Details Card
    float detailsX = menuX + 350.0f;
    float detailsY = menuY + 52.0f;
    float detailsW = menuW - 370.0f;
    float detailsH = menuH - 80.0f;

    ui->drawRect(detailsX, detailsY, detailsW, detailsH, {0.14f, 0.18f, 0.20f, 0.95f});
    ui->drawRectOutline(detailsX, detailsY, detailsW, detailsH, 1.5f, {0.25f, 0.45f, 0.40f, 1.0f});

    if (selectedIndex >= 1 && selectedIndex <= 35) {
        const auto& b = BiomeRegistry::get(static_cast<uint8_t>(selectedIndex));

        ui->drawText(b.name, detailsX + 20, detailsY + 20, 1.9f, {0.35f, 0.95f, 0.65f, 1.0f});
        ui->drawText(b.description, detailsX + 20, detailsY + 48, 1.2f, {0.85f, 0.90f, 0.85f, 1.0f});

        // Climate Specs
        std::stringstream ssTemp, ssMoist, ssElev;
        ssTemp << "Temperature: " << std::fixed << std::setprecision(2) << b.temperature;
        ssMoist << "Moisture: " << std::fixed << std::setprecision(2) << b.moisture;
        ssElev << "Base Elevation: " << static_cast<int>(b.baseElevation) << "m (Roughness: " << static_cast<int>(b.roughness) << ")";

        ui->drawText(ssTemp.str(), detailsX + 20, detailsY + 95, 1.3f, {0.95f, 0.95f, 0.95f, 1.0f});
        ui->drawText(ssMoist.str(), detailsX + 20, detailsY + 120, 1.3f, {0.95f, 0.95f, 0.95f, 1.0f});
        ui->drawText(ssElev.str(), detailsX + 20, detailsY + 145, 1.3f, {0.95f, 0.95f, 0.95f, 1.0f});

        // Surface Block
        const auto& surfBlock = BlockRegistry::get(b.surfaceBlock);
        ui->drawText("Surface Block:", detailsX + 20, detailsY + 185, 1.4f, {0.95f, 0.85f, 0.4f, 1.0f});
        ui->drawRect(detailsX + 20, detailsY + 210, 24, 24, surfBlock.color);
        ui->drawText(surfBlock.name, detailsX + 52, detailsY + 214, 1.3f, {0.95f, 0.95f, 0.95f, 1.0f});

        // Atmospheric Palette
        ui->drawText("Sky & Fog Atmosphere:", detailsX + 20, detailsY + 255, 1.4f, {0.95f, 0.85f, 0.4f, 1.0f});
        ui->drawRect(detailsX + 20, detailsY + 280, 50, 30, b.skyColor);
        ui->drawText("Sky Tint", detailsX + 24, detailsY + 315, 1.0f, {0.8f, 0.8f, 0.8f, 1.0f});

        ui->drawRect(detailsX + 85, detailsY + 280, 50, 30, b.fogColor);
        ui->drawText("Fog Tint", detailsX + 89, detailsY + 315, 1.0f, {0.8f, 0.8f, 0.8f, 1.0f});
    }

    ui->drawText("[UP/DOWN] Browse 35 Biomes  |  [ESC] Close Codex", menuX + 30, menuY + menuH - 24, 1.3f, {0.95f, 0.85f, 0.40f, 1.0f});
}

} // namespace Aetheria
