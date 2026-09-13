#include "BlockCatalogUI.hpp"
#include "ItemIconRenderer.hpp"
#include "../core/Window.hpp"
#include "../entities/Player.hpp"
#include "../world/BlockRegistry.hpp"
#include <sstream>
#include <algorithm>

namespace Aetheria {

static const char* BLOCK_CAT_NAMES[10] = {
    "Terrain", "Stone", "Wood", "Ores", "Crystals",
    "Magic", "Dungeon", "Colors", "Mech", "Exotic"
};

BlockCatalogUI::BlockCatalogUI(UIRenderer* ui) : ui(ui) {
}

void BlockCatalogUI::handleInput(const Window& window, Player& player) {
    if (!isOpen) return;

    // Number keys 1-9 & 0 switch categories
    for (int i = 0; i < 9; ++i) {
        if (window.isKeyPressed('1' + i)) {
            currentCategory = i;
            currentPage = 0;
            selectedIndex = 0;
        }
    }
    if (window.isKeyPressed('0')) {
        currentCategory = 9;
        currentPage = 0;
        selectedIndex = 0;
    }

    // Arrow keys navigate grid
    if (window.isKeyPressed(VK_LEFT) || window.isKeyPressed('A')) {
        selectedIndex = std::max(0, selectedIndex - 1);
    }
    if (window.isKeyPressed(VK_RIGHT) || window.isKeyPressed('D')) {
        selectedIndex++;
    }
    if (window.isKeyPressed(VK_UP) || window.isKeyPressed('W')) {
        selectedIndex = std::max(0, selectedIndex - 5);
    }
    if (window.isKeyPressed(VK_DOWN) || window.isKeyPressed('S')) {
        selectedIndex += 5;
    }

    // Page Up/Down
    if (window.isKeyPressed(VK_PRIOR) || window.isKeyPressed('Q')) {
        if (currentPage > 0) {
            currentPage--;
            selectedIndex = 0;
        }
    }
    if (window.isKeyPressed(VK_NEXT) || window.isKeyPressed(VK_TAB)) {
        currentPage++;
        selectedIndex = 0;
    }

    // Enter / Space assigns block to active hotbar slot
    if (window.isKeyPressed(VK_RETURN) || window.isKeyPressed(VK_SPACE) || window.isMouseButtonPressed(0)) {
        // Collect blocks in this category
        std::vector<BlockDef> catBlocks;
        for (const auto& b : BlockRegistry::getAll()) {
            if (b.id == 0) continue;
            if (static_cast<int>(b.category) == currentCategory) {
                catBlocks.push_back(b);
            }
        }

        int globalIdx = currentPage * 25 + selectedIndex;
        if (globalIdx >= 0 && globalIdx < static_cast<int>(catBlocks.size())) {
            uint16_t bId = catBlocks[globalIdx].id;
            player.setHotbarSlot(player.getSelectedHotbarIndex(), bId);
            close();
            return;
        }
    }

    // Menu toggling ('N' / ESC) is cleanly handled in main.cpp.
}

void BlockCatalogUI::render(int screenWidth, int screenHeight, const Player& player) {
    if (!isOpen) return;

    float sw = static_cast<float>(screenWidth);
    float sh = static_cast<float>(screenHeight);

    ui->drawRect(0, 0, sw, sh, {0.02f, 0.03f, 0.05f, 0.75f});

    float menuW = 760.0f;
    float menuH = 500.0f;
    float menuX = (sw - menuW) * 0.5f;
    float menuY = (sh - menuH) * 0.5f;

    ui->drawRect(menuX, menuY, menuW, menuH, {0.10f, 0.12f, 0.16f, 0.95f});
    ui->drawRectOutline(menuX, menuY, menuW, menuH, 2.0f, {0.95f, 0.75f, 0.20f, 1.0f});

    // Title
    ui->drawRect(menuX, menuY, menuW, 40, {0.25f, 0.20f, 0.08f, 1.0f});
    ui->drawText("365-BLOCK SANDBOX CATALOG [Press N or ESC to Close]", menuX + 20, menuY + 12, 1.8f, {0.98f, 0.90f, 0.35f, 1.0f});

    // 10 Category Tabs
    float tabX = menuX + 12.0f;
    float tabY = menuY + 48.0f;
    float tabW = 70.0f;
    float tabH = 24.0f;

    for (int i = 0; i < 10; ++i) {
        Vec4 tabBg = (i == currentCategory) ? Vec4(0.85f, 0.65f, 0.15f, 1.0f) : Vec4(0.18f, 0.18f, 0.22f, 0.9f);
        ui->drawRect(tabX + i * (tabW + 4), tabY, tabW, tabH, tabBg);
        ui->drawText(BLOCK_CAT_NAMES[i], tabX + i * (tabW + 4) + 4, tabY + 6, 1.1f, {0.95f, 0.95f, 0.95f, 1.0f});
    }

    // Collect blocks in category
    std::vector<BlockDef> catBlocks;
    for (const auto& b : BlockRegistry::getAll()) {
        if (b.id == 0) continue;
        if (static_cast<int>(b.category) == currentCategory) {
            catBlocks.push_back(b);
        }
    }

    // Grid: 5 columns x 5 rows = 25 items per page
    float gridX = menuX + 20.0f;
    float gridY = menuY + 84.0f;
    float slotW = 140.0f;
    float slotH = 68.0f;

    int totalPages = (static_cast<int>(catBlocks.size()) + 24) / 25;
    if (currentPage >= totalPages) currentPage = std::max(0, totalPages - 1);

    int startIdx = currentPage * 25;
    int endIdx = std::min(startIdx + 25, static_cast<int>(catBlocks.size()));

    for (int i = startIdx; i < endIdx; ++i) {
        int pageItemIdx = i - startIdx;
        int col = pageItemIdx % 5;
        int row = pageItemIdx / 5;

        float bx = gridX + col * (slotW + 6);
        float by = gridY + row * (slotH + 6);

        bool isSelected = (pageItemIdx == selectedIndex);
        Vec4 bg = isSelected ? Vec4(0.35f, 0.30f, 0.15f, 0.95f) : Vec4(0.15f, 0.16f, 0.20f, 0.9f);

        ui->drawRect(bx, by, slotW, slotH, bg);
        if (isSelected) {
            ui->drawRectOutline(bx, by, slotW, slotH, 2.0f, {0.95f, 0.85f, 0.25f, 1.0f});
        }

        const auto& block = catBlocks[i];
        // 3D Isometric Block Preview
        ui->drawRect(bx + 4, by + 4, 28, 28, {0.05f, 0.07f, 0.10f, 0.95f});
        ui->drawRectOutline(bx + 4, by + 4, 28, 28, 1.0f, {0.2f, 0.3f, 0.4f, 0.6f});
        ItemIconRenderer::drawIsometricBlock(ui, bx + 18.0f, by + 18.0f, 24.0f, block.id);

        // Block name
        ui->drawText(block.name, bx + 36, by + 6, 1.0f, {0.95f, 0.95f, 0.95f, 1.0f});

        // ID & Light
        std::stringstream ssMeta;
        ssMeta << "ID #" << block.id;
        if (block.lightEmission > 0) ssMeta << " L" << (int)block.lightEmission;
        ui->drawText(ssMeta.str(), bx + 36, by + 20, 0.9f, {0.8f, 0.8f, 0.5f, 1.0f});
    }

    // Page indicator
    std::stringstream ssPage;
    ssPage << "Page " << (currentPage + 1) << " / " << std::max(1, totalPages)
           << " (Category total: " << catBlocks.size() << " blocks)";
    ui->drawText(ssPage.str(), menuX + 24, menuY + menuH - 30, 1.3f, {0.85f, 0.85f, 0.85f, 1.0f});

    ui->drawText("[Q/E] Prev/Next Page  |  [ENTER/Click] Equip to Active Hotbar Slot",
                 menuX + 260, menuY + menuH - 30, 1.3f, {0.95f, 0.85f, 0.40f, 1.0f});
}

} // namespace Aetheria
