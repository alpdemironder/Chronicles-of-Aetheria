#include "BuildMenuUI.hpp"
#include "../core/Window.hpp"
#include "../building/BuildingManager.hpp"
#include "../world/BlockRegistry.hpp"
#include <sstream>

namespace Aetheria {

static const char* CATEGORY_NAMES[6] = {
    "1. Foundations",
    "2. Walls & Roofs",
    "3. Production",
    "4. Pal Facilities",
    "5. Storage & Defense",
    "6. Furniture"
};

BuildMenuUI::BuildMenuUI(UIRenderer* ui) : ui(ui) {
}

void BuildMenuUI::handleInput(const Window& window, BuildingManager& buildingMgr) {
    if (!isOpen) return;

    // Number keys switch categories
    for (int i = 0; i < 6; ++i) {
        if (window.isKeyPressed('1' + i)) {
            currentCategory = i;
            selectedIndex = 0;
        }
    }

    // Arrow keys or W/S navigate pieces
    if (window.isKeyPressed(VK_UP) || window.isKeyPressed('W')) {
        selectedIndex = std::max(0, selectedIndex - 1);
    }
    if (window.isKeyPressed(VK_DOWN) || window.isKeyPressed('S')) {
        selectedIndex++;
    }

    // Enter / Space / Left click confirms selection and activates holographic building
    if (window.isKeyPressed(VK_RETURN) || window.isKeyPressed(VK_SPACE) || window.isMouseButtonPressed(0)) {
        // Find structure at selectedIndex in this category
        const auto& allDefs = StructureRegistry::getAll();
        int currentCount = 0;
        for (const auto& def : allDefs) {
            if (static_cast<int>(def.category) == currentCategory) {
                if (currentCount == selectedIndex) {
                    buildingMgr.selectStructure(def.type);
                    buildingMgr.setBuildMode(true);
                    buildingMgr.setDismantleMode(false);
                    close();
                    return;
                }
                currentCount++;
            }
        }
    }
    // Category switching, item navigation, and selection are handled above.
    // Menu toggling ('B' / ESC) is managed cleanly by main.cpp.
}

void BuildMenuUI::render(int screenWidth, int screenHeight, const BuildingManager& buildingMgr) {
    if (!isOpen) return;

    float sw = static_cast<float>(screenWidth);
    float sh = static_cast<float>(screenHeight);

    // Dark backdrop overlay
    ui->drawRect(0, 0, sw, sh, {0.02f, 0.03f, 0.05f, 0.75f});

    float menuW = 680.0f;
    float menuH = 460.0f;
    float menuX = (sw - menuW) * 0.5f;
    float menuY = (sh - menuH) * 0.5f;

    // Main window panel
    ui->drawRect(menuX, menuY, menuW, menuH, {0.10f, 0.12f, 0.16f, 0.95f});
    ui->drawRectOutline(menuX, menuY, menuW, menuH, 2.0f, {0.25f, 0.55f, 0.85f, 1.0f});

    // Title banner
    ui->drawRect(menuX, menuY, menuW, 40, {0.15f, 0.25f, 0.40f, 1.0f});
    ui->drawText("PALWORLD MODULAR CONSTRUCTION MENU [Press B or ESC to Close]", menuX + 20, menuY + 12, 1.8f, {0.95f, 0.95f, 0.90f, 1.0f});

    // Category Tabs (Top row)
    float tabX = menuX + 15.0f;
    float tabY = menuY + 50.0f;
    float tabW = 105.0f;
    float tabH = 26.0f;

    for (int i = 0; i < 6; ++i) {
        Vec4 tabBg = (i == currentCategory) ? Vec4(0.25f, 0.55f, 0.85f, 1.0f) : Vec4(0.15f, 0.18f, 0.24f, 0.9f);
        ui->drawRect(tabX + i * (tabW + 4), tabY, tabW, tabH, tabBg);
        ui->drawText(CATEGORY_NAMES[i], tabX + i * (tabW + 4) + 6, tabY + 8, 1.1f, {0.95f, 0.95f, 0.95f, 1.0f});
    }

    // Structure Pieces List
    float listX = menuX + 20.0f;
    float listY = menuY + 90.0f;
    float itemW = menuW - 40.0f;
    float itemH = 50.0f;

    const auto& allDefs = StructureRegistry::getAll();
    int curIdx = 0;

    for (const auto& def : allDefs) {
        if (static_cast<int>(def.category) == currentCategory) {
            float iy = listY + curIdx * (itemH + 8);
            bool isSelected = (curIdx == selectedIndex);

            Vec4 itemBg = isSelected ? Vec4(0.20f, 0.35f, 0.55f, 0.95f) : Vec4(0.14f, 0.16f, 0.22f, 0.9f);
            ui->drawRect(listX, iy, itemW, itemH, itemBg);

            if (isSelected) {
                ui->drawRectOutline(listX, iy, itemW, itemH, 2.0f, {0.95f, 0.85f, 0.35f, 1.0f});
            }

            // Structure Icon preview
            ui->drawRect(listX + 6, iy + 6, itemH - 12, itemH - 12, def.baseColor);

            // Name
            ui->drawText(def.name, listX + itemH + 6, iy + 8, 1.6f, {0.95f, 0.95f, 0.95f, 1.0f});

            // Materials list
            std::stringstream ssCosts;
            ssCosts << "Required: ";
            for (size_t c = 0; c < def.costs.size(); ++c) {
                const auto& cost = def.costs[c];
                std::string blockName = BlockRegistry::get(cost.blockOrItemId).name;
                ssCosts << blockName << " x" << cost.count;
                if (c + 1 < def.costs.size()) ssCosts << ", ";
            }
            ui->drawText(ssCosts.str(), listX + itemH + 6, iy + 28, 1.2f, {0.75f, 0.85f, 0.75f, 1.0f});

            curIdx++;
        }
    }

    if (curIdx == 0) {
        ui->drawText("No structures in this category.", listX + 20, listY + 30, 1.5f, {0.6f, 0.6f, 0.6f, 1.0f});
    }

    // Bottom prompt
    ui->drawText("[UP/DOWN] Select Piece  |  [ENTER / Click] Place Blueprint Hologram", menuX + 30, menuY + menuH - 24, 1.3f, {0.95f, 0.85f, 0.4f, 1.0f});
}

} // namespace Aetheria
