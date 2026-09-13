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

    // Get total items in current category
    const auto& allDefs = StructureRegistry::getAll();
    int totalInCategory = 0;
    for (const auto& def : allDefs) {
        if (static_cast<int>(def.category) == currentCategory) {
            totalInCategory++;
        }
    }

    // Number keys switch categories
    for (int i = 0; i < 6; ++i) {
        if (window.isKeyPressed('1' + i)) {
            currentCategory = i;
            selectedIndex = 0;
            scrollOffset = 0;
            return;
        }
    }

    // Mouse wheel scrolling
    int wheel = window.getMouseWheelDelta();
    if (wheel > 0) {
        selectedIndex = std::max(0, selectedIndex - 1);
    } else if (wheel < 0) {
        selectedIndex = std::min(std::max(0, totalInCategory - 1), selectedIndex + 1);
    }

    // Arrow keys or W/S/A/D navigate pieces
    if (window.isKeyPressed(VK_UP) || window.isKeyPressed('W')) {
        selectedIndex = std::max(0, selectedIndex - 1);
    }
    if (window.isKeyPressed(VK_DOWN) || window.isKeyPressed('S')) {
        selectedIndex = std::min(std::max(0, totalInCategory - 1), selectedIndex + 1);
    }
    // Left/Right or A/D jump by 4 items (fast wood species navigation)
    if (window.isKeyPressed(VK_LEFT) || window.isKeyPressed('A')) {
        selectedIndex = std::max(0, selectedIndex - 4);
    }
    if (window.isKeyPressed(VK_RIGHT) || window.isKeyPressed('D')) {
        selectedIndex = std::min(std::max(0, totalInCategory - 1), selectedIndex + 4);
    }
    if (window.isKeyPressed(VK_PRIOR)) { // Page Up
        selectedIndex = std::max(0, selectedIndex - 6);
    }
    if (window.isKeyPressed(VK_NEXT)) { // Page Down
        selectedIndex = std::min(std::max(0, totalInCategory - 1), selectedIndex + 6);
    }

    // Keep scrollOffset synced with selectedIndex
    int maxVisible = 6;
    if (selectedIndex < scrollOffset) {
        scrollOffset = selectedIndex;
    } else if (selectedIndex >= scrollOffset + maxVisible) {
        scrollOffset = selectedIndex - maxVisible + 1;
    }
    if (totalInCategory > 0) {
        scrollOffset = std::max(0, std::min(scrollOffset, std::max(0, totalInCategory - maxVisible)));
        selectedIndex = std::max(0, std::min(selectedIndex, totalInCategory - 1));
    }

    // Mouse click handling for category tabs and items
    if (window.isMouseButtonPressed(0)) {
        float sw = static_cast<float>(window.getWidth());
        float sh = static_cast<float>(window.getHeight());
        float menuW = 720.0f;
        float menuH = 490.0f;
        float menuX = (sw - menuW) * 0.5f;
        float menuY = (sh - menuH) * 0.5f;
        float mx = static_cast<float>(window.getMouseX());
        float my = static_cast<float>(window.getMouseY());

        // Check category tabs click
        float tabX = menuX + 15.0f;
        float tabY = menuY + 48.0f;
        float tabW = 112.0f;
        float tabH = 28.0f;
        for (int i = 0; i < 6; ++i) {
            float tx = tabX + i * (tabW + 3);
            if (mx >= tx && mx <= tx + tabW && my >= tabY && my <= tabY + tabH) {
                currentCategory = i;
                selectedIndex = 0;
                scrollOffset = 0;
                return;
            }
        }

        // Check item click
        float listX = menuX + 20.0f;
        float listY = menuY + 86.0f;
        float itemW = menuW - 55.0f;
        float itemH = 54.0f;
        for (int vi = 0; vi < maxVisible; ++vi) {
            float iy = listY + vi * (itemH + 6);
            if (mx >= listX && mx <= listX + itemW && my >= iy && my <= iy + itemH) {
                int clickedIndex = scrollOffset + vi;
                if (clickedIndex < totalInCategory) {
                    selectedIndex = clickedIndex;
                    // Trigger selection
                    int curCount = 0;
                    for (const auto& def : allDefs) {
                        if (static_cast<int>(def.category) == currentCategory) {
                            if (curCount == selectedIndex) {
                                buildingMgr.selectStructure(def.type);
                                buildingMgr.setBuildMode(true);
                                buildingMgr.setDismantleMode(false);
                                close();
                                return;
                            }
                            curCount++;
                        }
                    }
                }
            }
        }
    }

    // Enter / Space confirms selection and activates holographic building
    if (window.isKeyPressed(VK_RETURN) || window.isKeyPressed(VK_SPACE)) {
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
}

void BuildMenuUI::render(int screenWidth, int screenHeight, const BuildingManager& buildingMgr) {
    if (!isOpen) return;

    float sw = static_cast<float>(screenWidth);
    float sh = static_cast<float>(screenHeight);

    // Dark backdrop overlay
    ui->drawRect(0, 0, sw, sh, {0.02f, 0.03f, 0.05f, 0.78f});

    float menuW = 720.0f;
    float menuH = 490.0f;
    float menuX = (sw - menuW) * 0.5f;
    float menuY = (sh - menuH) * 0.5f;

    // Main window panel
    ui->drawRect(menuX, menuY, menuW, menuH, {0.10f, 0.12f, 0.16f, 0.96f});
    ui->drawRectOutline(menuX, menuY, menuW, menuH, 2.0f, {0.25f, 0.55f, 0.85f, 1.0f});

    // Title banner
    ui->drawRect(menuX, menuY, menuW, 40, {0.15f, 0.25f, 0.40f, 1.0f});
    ui->drawText("PALWORLD MODULAR CONSTRUCTION MENU [Press B or ESC to Close]", menuX + 16, menuY + 12, 1.7f, {0.95f, 0.95f, 0.90f, 1.0f});

    // Category Tabs (Top row)
    float tabX = menuX + 15.0f;
    float tabY = menuY + 48.0f;
    float tabW = 112.0f;
    float tabH = 28.0f;

    for (int i = 0; i < 6; ++i) {
        bool isCurrent = (i == currentCategory);
        Vec4 tabBg = isCurrent ? Vec4(0.25f, 0.55f, 0.85f, 1.0f) : Vec4(0.15f, 0.18f, 0.24f, 0.9f);
        ui->drawRect(tabX + i * (tabW + 3), tabY, tabW, tabH, tabBg);
        if (isCurrent) {
            ui->drawRectOutline(tabX + i * (tabW + 3), tabY, tabW, tabH, 1.5f, {0.95f, 0.85f, 0.35f, 1.0f});
        }
        ui->drawText(CATEGORY_NAMES[i], tabX + i * (tabW + 3) + 4, tabY + 8, 1.1f, {0.95f, 0.95f, 0.95f, 1.0f});
    }

    // Structure Pieces List
    float listX = menuX + 20.0f;
    float listY = menuY + 86.0f;
    float itemW = menuW - 55.0f;
    float itemH = 54.0f;
    int maxVisible = 6;

    const auto& allDefs = StructureRegistry::getAll();

    // Collect pieces in current category
    std::vector<const StructureDef*> categoryPieces;
    for (const auto& def : allDefs) {
        if (static_cast<int>(def.category) == currentCategory) {
            categoryPieces.push_back(&def);
        }
    }

    int totalInCategory = static_cast<int>(categoryPieces.size());

    // Page & Item Counter Banner
    std::stringstream ssCounter;
    ssCounter << "Piece " << (selectedIndex + 1) << " / " << totalInCategory
              << "  [Page " << (scrollOffset / maxVisible + 1) << " of " << ((totalInCategory + maxVisible - 1) / maxVisible) << "]";
    ui->drawText(ssCounter.str(), listX + itemW - 220.0f, menuY + 12, 1.3f, {0.75f, 0.85f, 0.95f, 1.0f});

    for (int vi = 0; vi < maxVisible; ++vi) {
        int itemIndex = scrollOffset + vi;
        if (itemIndex >= totalInCategory) break;

        const StructureDef& def = *categoryPieces[itemIndex];
        float iy = listY + vi * (itemH + 6);
        bool isSelected = (itemIndex == selectedIndex);

        Vec4 itemBg = isSelected ? Vec4(0.20f, 0.35f, 0.55f, 0.95f) : Vec4(0.14f, 0.16f, 0.22f, 0.9f);
        ui->drawRect(listX, iy, itemW, itemH, itemBg);

        if (isSelected) {
            ui->drawRectOutline(listX, iy, itemW, itemH, 2.0f, {0.95f, 0.85f, 0.35f, 1.0f});
        }

        // Structure Icon preview box
        ui->drawRect(listX + 6, iy + 6, itemH - 12, itemH - 12, def.baseColor);
        ui->drawRectOutline(listX + 6, iy + 6, itemH - 12, itemH - 12, 1.0f, {0.9f, 0.9f, 0.9f, 0.5f});

        // Wood species badge or category tag
        int woodVariant = -1, woodSpecies = -1;
        if (StructureRegistry::isWoodVariant(def.type, woodVariant, woodSpecies)) {
            std::string tag = "[" + std::string(StructureRegistry::getWoodSpeciesName(woodSpecies)) + "]";
            ui->drawRect(listX + itemH + 6, iy + 6, 75.0f, 18.0f, {0.22f, 0.26f, 0.34f, 0.9f});
            ui->drawText(tag, listX + itemH + 10, iy + 9, 1.1f, def.baseColor);
            ui->drawText(def.name, listX + itemH + 86, iy + 8, 1.5f, {0.95f, 0.95f, 0.95f, 1.0f});
        } else {
            ui->drawText(def.name, listX + itemH + 6, iy + 8, 1.5f, {0.95f, 0.95f, 0.95f, 1.0f});
        }

        // Materials list
        std::stringstream ssCosts;
        ssCosts << "Cost: ";
        for (size_t c = 0; c < def.costs.size(); ++c) {
            const auto& cost = def.costs[c];
            std::string blockName = BlockRegistry::get(cost.blockOrItemId).name;
            ssCosts << blockName << " x" << cost.count;
            if (c + 1 < def.costs.size()) ssCosts << ", ";
        }
        ssCosts << " | HP: " << static_cast<int>(def.maxHealth) << " | Work: " << static_cast<int>(def.buildWorkRequired) << "s";
        ui->drawText(ssCosts.str(), listX + itemH + 6, iy + 30, 1.2f, {0.75f, 0.88f, 0.75f, 1.0f});
    }

    if (totalInCategory == 0) {
        ui->drawText("No structures in this category.", listX + 20, listY + 30, 1.5f, {0.6f, 0.6f, 0.6f, 1.0f});
    }

    // Scrollbar indicator
    if (totalInCategory > maxVisible) {
        float scrollTrackX = listX + itemW + 8.0f;
        float scrollTrackY = listY;
        float scrollTrackH = maxVisible * (itemH + 6) - 6.0f;
        float scrollTrackW = 6.0f;

        ui->drawRect(scrollTrackX, scrollTrackY, scrollTrackW, scrollTrackH, {0.18f, 0.20f, 0.25f, 0.8f});

        float thumbFraction = static_cast<float>(maxVisible) / static_cast<float>(totalInCategory);
        float thumbH = std::max(20.0f, scrollTrackH * thumbFraction);
        float maxScroll = static_cast<float>(totalInCategory - maxVisible);
        float scrollRatio = (maxScroll > 0.0f) ? (static_cast<float>(scrollOffset) / maxScroll) : 0.0f;
        float thumbY = scrollTrackY + scrollRatio * (scrollTrackH - thumbH);

        ui->drawRect(scrollTrackX, thumbY, scrollTrackW, thumbH, {0.35f, 0.65f, 0.95f, 0.9f});
    }

    // Bottom prompt
    ui->drawText("[UP/DOWN/Wheel] Scroll | [LEFT/RIGHT] Jump Wood | [ENTER/Click] Build | [R] Rotate Ghost",
                 menuX + 20, menuY + menuH - 26, 1.25f, {0.95f, 0.85f, 0.4f, 1.0f});
}

} // namespace Aetheria
