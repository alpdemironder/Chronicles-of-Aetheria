#pragma once

#include "UIRenderer.hpp"
#include "../entities/Player.hpp"
#include "../inventory/CraftingRegistry.hpp"
#include "../inventory/FurnaceManager.hpp"
#include <string>
#include <vector>

namespace Aetheria {

class Window;
class AudioEngine;

enum class InventoryUIMode {
    Backpack,   // Grounded 2 Style: 4 Armor + Backpack (27) + Hotbar (9) + Inspection & Action Deck
    Crafting,   // Grounded 2 Style: Categorized Recipe Deck + 3x3 Workbench Grid
    Furnace,    // Grounded 2 Style: Industrial Smelting Processor with Temperature Gauges
    Vitals      // Grounded 2 Style: SCA.B OS Character & Combat Vitals Overview
};

class InventoryUI {
public:
    InventoryUI();

    void toggle() {
        isOpen = !isOpen;
        if (isOpen) {
            mode = InventoryUIMode::Backpack;
            updateCraftingResult();
        }
    }

    void openPlayerInventory() {
        isOpen = true;
        mode = InventoryUIMode::Backpack;
        updateCraftingResult();
    }

    void openCraftingTable() {
        isOpen = true;
        mode = InventoryUIMode::Crafting;
        updateCraftingResult();
    }

    void openFurnace() {
        isOpen = true;
        mode = InventoryUIMode::Furnace;
    }

    void close() {
        isOpen = false;
    }

    bool getIsOpen() const { return isOpen; }
    InventoryUIMode getMode() const { return mode; }

    FurnaceManager& getFurnace() { return furnace; }

    void returnGridItemsToPlayer(Player& player);

    // Dropped items queued from UI (e.g. clicking [DROP] in action deck)
    const std::vector<ItemStack>& getPendingDrops() const { return pendingDrops; }
    void clearPendingDrops() { pendingDrops.clear(); }

    void update(float dt);
    void handleInput(const Window& window, Player& player, AudioEngine* audio);
    void render(UIRenderer* ui, int screenWidth, int screenHeight,
                Player& player, AudioEngine* audio,
                int mouseX, int mouseY,
                bool mouseDown, bool mouseClicked, bool rightClicked,
                bool shiftDown);

private:
    bool isOpen = false;
    InventoryUIMode mode = InventoryUIMode::Backpack;

    // Selection & Navigation (Grounded 2 style)
    int selectedSlotIndex = 0;   // Selected slot for right-hand Grounded inspection panel
    int hoveredSlotIndex = -1;
    std::string currentCraftCategory = "All";
    int selectedRecipeId = 0;
    int recipeScrollOffset = 0;

    // Classic 2x2 and 3x3 grids for Minecraft recipe matching
    std::vector<ItemStack> grid2x2;
    std::vector<ItemStack> grid3x3;
    ItemStack craftResult;

    // Furnace Smelting state
    FurnaceManager furnace;

    // Pending dropped items queue
    std::vector<ItemStack> pendingDrops;

    void updateCraftingResult();

    // Grounded 2 UI drawing helpers
    bool drawGroundedButton(UIRenderer* ui, float x, float y, float w, float h,
                            const std::string& text, bool active,
                            int mouseX, int mouseY, bool clicked,
                            const Vec4& accentCol = {0.0f, 0.85f, 0.95f, 1.0f},
                            const Vec4& baseBg = {0.10f, 0.13f, 0.18f, 0.92f});

    void renderSlotCard(UIRenderer* ui, float x, float y, float size,
                        const ItemStack& stack, bool isHovered, bool isSelected,
                        const std::string& placeholder = "",
                        const Vec4& borderCol = {0.25f, 0.35f, 0.45f, 0.8f},
                        bool isLocked = false, int reqLevel = 1);

    void renderInspectionPanel(UIRenderer* ui, float x, float y, float w, float h,
                               const ItemStack& stack, int slotIdx,
                               Player& player, AudioEngine* audio,
                               int mouseX, int mouseY, bool mouseClicked);

    void drawCustomCursor(UIRenderer* ui, float mx, float my);
};

} // namespace Aetheria
