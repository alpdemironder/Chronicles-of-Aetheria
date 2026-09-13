#include "InventoryUI.hpp"
#include "ItemIconRenderer.hpp"
#include "../core/Window.hpp"
#include "../core/Audio.hpp"
#include "../world/BlockRegistry.hpp"
#include <algorithm>
#include <sstream>
#include <iomanip>

namespace Aetheria {

InventoryUI::InventoryUI() {
    grid2x2.resize(4);
    grid3x3.resize(9);
    for (auto& s : grid2x2) s.clear();
    for (auto& s : grid3x3) s.clear();
    craftResult.clear();
}

void InventoryUI::returnGridItemsToPlayer(Player& player) {
    for (auto& s : grid2x2) {
        if (!s.isEmpty()) {
            player.getInventory().addItem(s.id, s.count);
            s.clear();
        }
    }
    for (auto& s : grid3x3) {
        if (!s.isEmpty()) {
            player.getInventory().addItem(s.id, s.count);
            s.clear();
        }
    }
    ItemStack& cursor = player.getInventory().getCursorStack();
    if (!cursor.isEmpty()) {
        player.getInventory().addItem(cursor.id, cursor.count);
        cursor.clear();
    }
    craftResult.clear();
}

void InventoryUI::update(float dt) {
    furnace.update(dt);
}

void InventoryUI::updateCraftingResult() {
    if (mode == InventoryUIMode::Backpack) {
        craftResult = CraftingRegistry::matchGrid(grid2x2, 2, 2);
    } else if (mode == InventoryUIMode::Crafting) {
        craftResult = CraftingRegistry::matchGrid(grid3x3, 3, 3);
    } else {
        craftResult.clear();
    }
}

bool InventoryUI::drawGroundedButton(UIRenderer* ui, float x, float y, float w, float h,
                                    const std::string& text, bool active,
                                    int mouseX, int mouseY, bool clicked,
                                    const Vec4& accentCol,
                                    const Vec4& baseBg) {
    bool hovered = (mouseX >= x && mouseX <= x + w && mouseY >= y && mouseY <= y + h);

    Vec4 bg = active ? Vec4(accentCol.x * 0.35f, accentCol.y * 0.35f, accentCol.z * 0.35f, 0.95f) :
              (hovered ? Vec4(0.14f, 0.22f, 0.32f, 0.95f) : baseBg);
    Vec4 border = active ? accentCol :
                  (hovered ? Vec4(0.90f, 0.98f, 1.0f, 1.0f) : Vec4(0.24f, 0.32f, 0.44f, 0.75f));

    // Edgy chamfered button
    ui->drawEdgyPanel(x, y, w, h, 4.0f, bg, border, active ? 2.0f : (hovered ? 1.5f : 1.0f));

    // Left accent notch & tech brackets on hover
    if (active || hovered) {
        ui->drawRect(x + 2.0f, y + 2.0f, 3.5f, h - 4.0f, accentCol);
        ui->drawTechBracket(x, y, w, h, 6.0f, 1.2f, accentCol);
    }

    float fontScale = 1.2f;
    float textX = x + (w - text.length() * 6.0f * fontScale) * 0.5f;
    float textY = y + (h - 7.0f * fontScale) * 0.5f;
    ui->drawText(text, textX, textY, fontScale, active ? Vec4(1, 1, 1, 1) : (hovered ? Vec4(1, 1, 0.9f, 1) : Vec4(0.85f, 0.90f, 0.95f, 1)));

    return hovered && clicked;
}

void InventoryUI::renderSlotCard(UIRenderer* ui, float x, float y, float size,
                                const ItemStack& stack, bool isHovered, bool isSelected,
                                const std::string& placeholder,
                                const Vec4& borderCol,
                                bool isLocked, int reqLevel) {
    if (isLocked) {
        Vec4 slotBg = isHovered ? Vec4(0.09f, 0.06f, 0.08f, 0.95f) : Vec4(0.04f, 0.04f, 0.06f, 0.92f);
        Vec4 outlineCol = isHovered ? Vec4(0.95f, 0.35f, 0.35f, 0.95f) : Vec4(0.22f, 0.16f, 0.20f, 0.7f);
        float outlineThickness = isHovered ? 1.8f : 1.0f;
        ui->drawEdgyPanel(x, y, size, size, 4.0f, slotBg, outlineCol, outlineThickness);

        // Padlock icon: shackle & lock body
        float cx = x + size * 0.5f;
        float cy = y + size * 0.36f;
        Vec4 metalCol = isHovered ? Vec4(1.0f, 0.45f, 0.45f, 0.95f) : Vec4(0.65f, 0.55f, 0.45f, 0.75f);
        Vec4 bodyCol  = isHovered ? Vec4(0.75f, 0.20f, 0.20f, 0.95f) : Vec4(0.35f, 0.22f, 0.22f, 0.85f);
        ui->drawRectOutline(cx - 5.0f, cy - 6.0f, 10.0f, 7.0f, 1.4f, metalCol);
        ui->drawRect(cx - 7.0f, cy, 14.0f, 9.0f, bodyCol);
        ui->drawRect(cx - 1.0f, cy + 3.0f, 2.0f, 3.0f, {0.1f, 0.1f, 0.1f, 0.95f});

        // Level text requirement: "LV.{reqLevel}"
        std::string lvlTag = "LV." + std::to_string(reqLevel);
        float tW = lvlTag.length() * 5.0f;
        ui->drawText(lvlTag, cx - tW * 0.5f, y + size - 12.0f, 0.95f, isHovered ? Vec4(1.0f, 0.7f, 0.7f, 1.0f) : Vec4(0.65f, 0.55f, 0.55f, 0.8f));
        return;
    }

    Vec4 slotBg = isSelected ? Vec4(0.08f, 0.22f, 0.34f, 0.95f) :
                  (isHovered ? Vec4(0.14f, 0.20f, 0.28f, 0.92f) : Vec4(0.06f, 0.08f, 0.12f, 0.92f));

    Vec4 outlineCol = isSelected ? Vec4(0.0f, 0.95f, 1.0f, 1.0f) :
                      (isHovered ? Vec4(0.95f, 0.95f, 1.0f, 1.0f) : borderCol);
    float outlineThickness = isSelected ? 2.5f : (isHovered ? 2.0f : 1.0f);

    // Sleek chamfered slot card
    ui->drawEdgyPanel(x, y, size, size, 4.0f, slotBg, outlineCol, outlineThickness);
    if (isSelected || isHovered) {
        ui->drawTechBracket(x, y, size, size, 6.0f, 1.2f, isSelected ? Vec4(0.0f, 0.95f, 1.0f, 1.0f) : Vec4(1, 1, 1, 0.9f));
    }

    if (stack.isEmpty()) {
        if (!placeholder.empty()) {
            float textW = placeholder.length() * 6.0f * 1.0f;
            ui->drawText(placeholder, x + (size - textW) * 0.5f, y + (size - 7.0f) * 0.5f, 1.0f, {0.35f, 0.42f, 0.50f, 0.7f});
        }
        return;
    }

    const auto& def = ItemRegistry::get(stack.id);

    // Fresh 3D Isometric block or stylized RPG icon preview
    ItemIconRenderer::drawItem(ui, x + size * 0.5f, y + size * 0.5f, size - 8.0f, stack.id);

    // Item count badge
    if (stack.count > 1) {
        std::string countStr = std::to_string(stack.count);
        float cW = countStr.length() * 6.0f * 1.15f;
        ui->drawEdgyPanel(x + size - cW - 6.0f, y + size - 14.0f, cW + 6.0f, 13.0f, 3.0f, {0.04f, 0.06f, 0.09f, 0.92f}, {0.18f, 0.28f, 0.38f, 0.8f}, 1.0f);
        ui->drawText(countStr, x + size - cW - 3.0f, y + size - 12.0f, 1.15f, {1.0f, 1.0f, 0.85f, 1.0f});
    }
}

void InventoryUI::drawCustomCursor(UIRenderer* ui, float mx, float my) {
    // Sleek Tactical Edgy Pointer with Drop Shadow
    ui->addTriangle(mx + 1, my + 1, mx + 16, my + 11, mx + 10, my + 16, {0.0f, 0.0f, 0.0f, 0.6f});
    ui->addTriangle(mx, my, mx + 14, my + 10, mx + 9, my + 14, {0.04f, 0.08f, 0.12f, 0.98f});
    ui->drawLine(mx, my, mx + 14, my + 10, 1.8f, {0.0f, 0.95f, 1.0f, 1.0f});
    ui->drawLine(mx, my, mx + 9, my + 14, 1.8f, {0.0f, 0.95f, 1.0f, 1.0f});
    ui->drawLine(mx + 14, my + 10, mx + 9, my + 14, 1.5f, {0.0f, 0.70f, 0.85f, 1.0f});
    ui->drawRect(mx + 4, my + 4, 2, 2, {1.0f, 1.0f, 1.0f, 1.0f});
}

void InventoryUI::handleInput(const Window& window, Player& player, AudioEngine* audio) {
    if (!isOpen) return;

    // Number keys 1..9 quick-swap hovered slot with hotbar
    if (hoveredSlotIndex >= 0 && hoveredSlotIndex < Inventory::TOTAL_SLOTS) {
        for (int i = 0; i < 9; ++i) {
            if (window.isKeyPressed('1' + i)) {
                player.getInventory().swapSlots(hoveredSlotIndex, i);
                selectedSlotIndex = i;
                if (audio) audio->playSound(SoundID::ItemPickup, 1.0f, 1.0f);
                break;
            }
        }
    }

    // Hotkey 'Q' to drop from selected slot
    if (window.isKeyPressed('Q')) {
        ItemStack& sl = player.getInventory().getSlot(selectedSlotIndex);
        if (!sl.isEmpty()) {
            pendingDrops.push_back({sl.id, 1, sl.maxStack});
            sl.count--;
            if (sl.count == 0) sl.clear();
            if (audio) audio->playSound(SoundID::ItemPickup, 0.8f, 0.7f);
        }
    }
}

void InventoryUI::renderInspectionPanel(UIRenderer* ui, float x, float y, float w, float h,
                                       const ItemStack& stack, int slotIdx,
                                       Player& player, AudioEngine* audio,
                                       int mouseX, int mouseY, bool mouseClicked) {
    // Grounded 2 SCA.B OS Inspection Chassis (Edgy Panel)
    ui->drawEdgyPanelTopCut(x, y, w, h, 10.0f, {0.05f, 0.07f, 0.11f, 0.96f}, {0.0f, 0.85f, 0.95f, 0.85f}, 1.5f);
    ui->drawTechBracket(x, y, w, h, 14.0f, 1.5f, {0.0f, 0.95f, 1.0f, 0.95f});

    // Header strip
    ui->drawEdgyPanel(x + 4.0f, y + 4.0f, w - 8.0f, 26.0f, 5.0f, {0.08f, 0.12f, 0.18f, 0.95f}, {0.0f, 0.85f, 0.95f, 0.5f}, 1.0f);
    ui->drawHexBadge(x + 18.0f, y + 17.0f, 6.0f, {0.0f, 0.95f, 1.0f, 1.0f}, {1, 1, 1, 1});
    ui->drawText("SCA.B // ITEM INSPECTOR", x + 30.0f, y + 10.0f, 1.25f, {0.0f, 0.95f, 1.0f, 1.0f});

    if (stack.isEmpty()) {
        ui->drawTextCentered("NO ITEM SELECTED", x + w * 0.5f, y + 80.0f, 1.4f, {0.45f, 0.55f, 0.65f, 0.8f});
        ui->drawTextCentered("Click any inventory slot", x + w * 0.5f, y + 110.0f, 1.15f, {0.35f, 0.45f, 0.55f, 0.7f});
        ui->drawTextCentered("to inspect stats & actions.", x + w * 0.5f, y + 128.0f, 1.15f, {0.35f, 0.45f, 0.55f, 0.7f});

        // Quick Controls Info Card (Edgy Chassis)
        float qcY = y + 180.0f;
        ui->drawEdgyPanelTopCut(x + 12.0f, qcY, w - 24.0f, 170.0f, 8.0f, {0.04f, 0.06f, 0.09f, 0.90f}, {0.18f, 0.28f, 0.38f, 0.7f}, 1.2f);
        ui->drawTechBracket(x + 12.0f, qcY, w - 24.0f, 170.0f, 10.0f, 1.2f, {0.0f, 0.85f, 0.95f, 0.85f});
        ui->drawText("QUICK CONTROLS", x + 20.0f, qcY + 8.0f, 1.2f, {0.0f, 0.85f, 0.95f, 1.0f});

        ui->drawText("[TAB / E] Toggle Inventory", x + 20.0f, qcY + 30.0f, 1.05f, {0.8f, 0.85f, 0.9f, 0.9f});
        ui->drawText("[L-CLICK] Select / Move", x + 20.0f, qcY + 50.0f, 1.05f, {0.8f, 0.85f, 0.9f, 0.9f});
        ui->drawText("[R-CLICK] Split Half", x + 20.0f, qcY + 70.0f, 1.05f, {0.8f, 0.85f, 0.9f, 0.9f});
        ui->drawText("[SHIFT+CLICK] Fast Move", x + 20.0f, qcY + 90.0f, 1.05f, {0.8f, 0.85f, 0.9f, 0.9f});
        ui->drawText("[1-9] Hotbar Quick Swap", x + 20.0f, qcY + 110.0f, 1.05f, {0.8f, 0.85f, 0.9f, 0.9f});
        ui->drawText("[Q] Drop Selected Item", x + 20.0f, qcY + 130.0f, 1.05f, {0.8f, 0.85f, 0.9f, 0.9f});
        return;
    }

    const auto& def = ItemRegistry::get(stack.id);

    // Large preview icon (high-detail 3D isometric block or stylized RPG item)
    float iconSz = 48.0f;
    float iconX = x + 16.0f;
    float iconY = y + 40.0f;
    ui->drawEdgyPanel(iconX, iconY, iconSz, iconSz, 6.0f, {0.04f, 0.06f, 0.09f, 0.95f}, {0.0f, 0.85f, 0.95f, 0.85f}, 1.5f);
    ui->drawTechBracket(iconX, iconY, iconSz, iconSz, 6.0f, 1.2f, {0.0f, 0.95f, 1.0f, 0.95f});
    ItemIconRenderer::drawItem(ui, iconX + iconSz * 0.5f, iconY + iconSz * 0.5f, iconSz - 4.0f, stack.id);

    // Title & Category
    ui->drawText(def.name, x + 72.0f, y + 40.0f, 1.4f, {1.0f, 1.0f, 0.95f, 1.0f});

    std::string catName = "[MATERIAL]";
    if (def.category == ItemCategory::Weapon) catName = "[WEAPON]";
    else if (def.category == ItemCategory::Armor) catName = "[ARMOR PIECE]";
    else if (def.category == ItemCategory::Tool) catName = "[TOOL: TIER " + std::to_string(def.toolTier) + "]";
    else if (def.category == ItemCategory::Consumable) catName = "[CONSUMABLE]";
    else if (def.category == ItemCategory::Block) catName = "[VOXEL BLOCK #" + std::to_string(def.id) + "]";

    ui->drawText(catName, x + 72.0f, y + 58.0f, 1.15f, {0.0f, 0.85f, 0.95f, 1.0f});
    ui->drawText("Qty: " + std::to_string(stack.count) + " / " + std::to_string(stack.maxStack),
                 x + 72.0f, y + 74.0f, 1.1f, {0.75f, 0.82f, 0.88f, 0.85f});

    // Divider line
    float divY = y + 98.0f;
    ui->drawRect(x + 12.0f, divY, w - 24.0f, 1.0f, {0.18f, 0.28f, 0.38f, 0.8f});

    // Stats breakdown
    float statY = divY + 10.0f;
    if (def.category == ItemCategory::Weapon || def.category == ItemCategory::Tool) {
        ui->drawText("ATTACK DAMAGE", x + 16.0f, statY, 1.2f, {1.0f, 0.75f, 0.25f, 1.0f});
        float frac = std::clamp(def.attackDamage / 60.0f, 0.05f, 1.0f);
        ui->drawProgressBar(x + 16.0f, statY + 16.0f, w - 32.0f, 8.0f, frac, {1.0f, 0.65f, 0.15f, 1.0f}, {0.12f, 0.15f, 0.2f, 0.8f});
        std::stringstream ssAtkStats;
        ssAtkStats << "+" << static_cast<int>(def.attackDamage) << " Dmg (" << std::fixed << std::setprecision(1) << def.attackSpeed << " Attacks/s)";
        ui->drawText(ssAtkStats.str(), x + 16.0f, statY + 28.0f, 1.15f, {0.9f, 0.9f, 0.9f, 1.0f});
        statY += 48.0f;
    } else if (def.category == ItemCategory::Armor) {
        ui->drawText("ARMOR PROTECTION", x + 16.0f, statY, 1.2f, {0.25f, 0.85f, 1.0f, 1.0f});
        float frac = std::clamp(def.armorDefense / 20.0f, 0.05f, 1.0f);
        ui->drawProgressBar(x + 16.0f, statY + 16.0f, w - 32.0f, 8.0f, frac, {0.0f, 0.85f, 1.0f, 1.0f}, {0.12f, 0.15f, 0.2f, 0.8f});
        int resPct = static_cast<int>(std::min(80.0f, def.armorDefense * 3.5f));
        ui->drawText("+" + std::to_string(static_cast<int>(def.armorDefense)) + " Def (" + std::to_string(resPct) + "% Res)", x + 16.0f, statY + 28.0f, 1.15f, {0.9f, 0.9f, 0.9f, 1.0f});
        statY += 48.0f;
    } else if (def.category == ItemCategory::Consumable) {
        ui->drawText("HEALTH RESTORATION", x + 16.0f, statY, 1.2f, {0.35f, 0.95f, 0.45f, 1.0f});
        ui->drawProgressBar(x + 16.0f, statY + 16.0f, w - 32.0f, 8.0f, 0.5f, {0.2f, 0.9f, 0.35f, 1.0f}, {0.12f, 0.15f, 0.2f, 0.8f});
        ui->drawText("+25 HP & Sustenance", x + 16.0f, statY + 28.0f, 1.15f, {0.9f, 0.9f, 0.9f, 1.0f});
        statY += 48.0f;
    }

    // Description / Lore Box
    float descH = 68.0f;
    ui->drawRect(x + 12.0f, statY, w - 24.0f, descH, {0.06f, 0.08f, 0.12f, 0.75f});
    ui->drawRectOutline(x + 12.0f, statY, w - 24.0f, descH, 1.0f, {0.18f, 0.25f, 0.35f, 0.5f});
    std::string desc = def.description;
    if (desc.length() > 34) {
        ui->drawText(desc.substr(0, 34), x + 18.0f, statY + 10.0f, 1.1f, {0.8f, 0.85f, 0.9f, 0.9f});
        ui->drawText(desc.substr(34, 34), x + 18.0f, statY + 28.0f, 1.1f, {0.8f, 0.85f, 0.9f, 0.9f});
        if (desc.length() > 68) {
            ui->drawText(desc.substr(68, 34), x + 18.0f, statY + 46.0f, 1.1f, {0.8f, 0.85f, 0.9f, 0.9f});
        }
    } else {
        ui->drawText(desc, x + 18.0f, statY + 16.0f, 1.15f, {0.8f, 0.85f, 0.9f, 0.9f});
    }

    // Interactive Action Buttons
    float btnW = (w - 32.0f) * 0.5f;
    float btnH = 32.0f;
    float btnY1 = y + h - 86.0f;
    float btnY2 = y + h - 46.0f;

    // Button 1: [EQUIP / USE]
    std::string b1Text = (def.category == ItemCategory::Armor) ? "[EQUIP GEAR]" :
                         (def.category == ItemCategory::Consumable) ? "[CONSUME / EAT]" : "[TO HOTBAR]";
    if (drawGroundedButton(ui, x + 12.0f, btnY1, btnW, btnH, b1Text, false, mouseX, mouseY, mouseClicked, {0.0f, 0.95f, 0.65f, 1.0f})) {
        Inventory& inv = player.getInventory();
        if (def.category == ItemCategory::Armor) {
            // Auto equip to armor slot
            int armSlot = -1;
            std::string nLow = def.name;
            for (auto& c : nLow) c = tolower(c);
            if (nLow.find("helm") != std::string::npos || nLow.find("cap") != std::string::npos) armSlot = Inventory::SLOT_HELMET;
            else if (nLow.find("chest") != std::string::npos || nLow.find("tunic") != std::string::npos) armSlot = Inventory::SLOT_CHEST;
            else if (nLow.find("leg") != std::string::npos || nLow.find("pant") != std::string::npos) armSlot = Inventory::SLOT_LEGS;
            else if (nLow.find("boot") != std::string::npos || nLow.find("shoe") != std::string::npos) armSlot = Inventory::SLOT_BOOTS;

            if (armSlot >= 0 && slotIdx >= 0 && slotIdx < Inventory::TOTAL_SLOTS) {
                inv.swapSlots(slotIdx, armSlot);
                selectedSlotIndex = armSlot;
                if (audio) audio->playSound(SoundID::ItemPickup, 1.2f, 0.8f);
            }
        } else if (def.category == ItemCategory::Consumable) {
            player.heal(25.0f);
            ItemStack& sl = inv.getSlot(slotIdx);
            sl.count--;
            if (sl.count == 0) sl.clear();
            if (audio) audio->playSound(SoundID::ItemPickup, 1.0f, 1.2f);
        } else {
            // Swap to hotbar slot 0
            if (slotIdx >= 0 && slotIdx < Inventory::TOTAL_SLOTS) {
                inv.swapSlots(slotIdx, player.getSelectedHotbarIndex());
                selectedSlotIndex = player.getSelectedHotbarIndex();
                if (audio) audio->playSound(SoundID::ItemPickup, 1.0f, 1.0f);
            }
        }
    }

    // Button 2: [QUICK MOVE]
    if (drawGroundedButton(ui, x + 20.0f + btnW, btnY1, btnW, btnH, "[FAST MOVE]", false, mouseX, mouseY, mouseClicked, {0.0f, 0.85f, 1.0f, 1.0f})) {
        Inventory& inv = player.getInventory();
        if (slotIdx >= 0 && slotIdx < Inventory::TOTAL_SLOTS) {
            if (slotIdx < 9) {
                // Hotbar -> Backpack
                for (int d = 9; d < 36; ++d) {
                    if (inv.getSlot(d).isEmpty()) {
                        inv.swapSlots(slotIdx, d);
                        selectedSlotIndex = d;
                        if (audio) audio->playSound(SoundID::ItemPickup, 1.0f, 1.0f);
                        break;
                    }
                }
            } else {
                // Backpack/Armor -> Hotbar
                for (int d = 0; d < 9; ++d) {
                    if (inv.getSlot(d).isEmpty()) {
                        inv.swapSlots(slotIdx, d);
                        selectedSlotIndex = d;
                        if (audio) audio->playSound(SoundID::ItemPickup, 1.0f, 1.0f);
                        break;
                    }
                }
            }
        }
    }

    // Button 3: [SPLIT (1/2)]
    if (drawGroundedButton(ui, x + 12.0f, btnY2, btnW, btnH, "[SPLIT (1/2)]", false, mouseX, mouseY, mouseClicked, {0.85f, 0.75f, 0.25f, 1.0f})) {
        Inventory& inv = player.getInventory();
        ItemStack& cursor = inv.getCursorStack();
        ItemStack& sl = inv.getSlot(slotIdx);
        if (cursor.isEmpty() && sl.count > 1) {
            uint32_t half = (sl.count + 1) / 2;
            cursor = { sl.id, half, sl.maxStack };
            sl.count -= half;
            if (sl.count == 0) sl.clear();
            if (audio) audio->playSound(SoundID::ItemPickup, 1.0f, 1.1f);
        }
    }

    // Button 4: [DROP ITEM (Q)]
    if (drawGroundedButton(ui, x + 20.0f + btnW, btnY2, btnW, btnH, "[DROP (Q)]", false, mouseX, mouseY, mouseClicked, {0.95f, 0.35f, 0.35f, 1.0f})) {
        Inventory& inv = player.getInventory();
        ItemStack& sl = inv.getSlot(slotIdx);
        if (!sl.isEmpty()) {
            pendingDrops.push_back({sl.id, 1, sl.maxStack});
            sl.count--;
            if (sl.count == 0) sl.clear();
            if (audio) audio->playSound(SoundID::ItemPickup, 0.8f, 0.7f);
        }
    }
}

void InventoryUI::render(UIRenderer* ui, int screenWidth, int screenHeight,
                        Player& player, AudioEngine* audio,
                        int mouseX, int mouseY,
                        bool mouseDown, bool mouseClicked, bool rightClicked,
                        bool shiftDown) {
    if (!isOpen) return;

    hoveredSlotIndex = -1;

    // 1. Grounded 2 Backdrop Tint (Dark Slate Translucent)
    ui->drawRect(0, 0, static_cast<float>(screenWidth), static_cast<float>(screenHeight), {0.02f, 0.04f, 0.06f, 0.78f});

    float panelW = 920.0f;
    float panelH = 580.0f;
    float panelX = (screenWidth - panelW) * 0.5f;
    float panelY = (screenHeight - panelH) * 0.5f;

    // Outer Chassis Frame (High-Tech Edgy Chamfered Panel)
    ui->drawEdgyPanel(panelX, panelY, panelW, panelH, 12.0f, {0.06f, 0.08f, 0.12f, 0.98f}, {0.0f, 0.85f, 0.95f, 0.9f}, 2.0f);
    ui->drawTechBracket(panelX, panelY, panelW, panelH, 22.0f, 2.0f, {0.0f, 0.95f, 1.0f, 1.0f});

    // Top Navigation / Header Bar
    ui->drawEdgyPanel(panelX + 4.0f, panelY + 4.0f, panelW - 8.0f, 40.0f, 6.0f, {0.05f, 0.07f, 0.10f, 0.98f}, {0.18f, 0.28f, 0.38f, 0.7f}, 1.0f);

    // SCA.B Green Power Beacon (Hex Badge)
    ui->drawHexBadge(panelX + 22.0f, panelY + 24.0f, 8.0f, {0.2f, 0.95f, 0.35f, 1.0f}, {1.0f, 1.0f, 1.0f, 0.9f});
    ui->drawText("SCA.B OS // v2.4", panelX + 36.0f, panelY + 18.0f, 1.35f, {0.2f, 0.95f, 0.35f, 1.0f});

    // 4 Grounded Top Tabs
    float tabX = panelX + 180.0f;
    float tabY = panelY + 9.0f;
    float tabW = 150.0f;
    float tabH = 30.0f;

    if (drawGroundedButton(ui, tabX, tabY, tabW, tabH, "[1] BACKPACK", mode == InventoryUIMode::Backpack, mouseX, mouseY, mouseClicked)) {
        returnGridItemsToPlayer(player);
        mode = InventoryUIMode::Backpack;
        updateCraftingResult();
    }
    if (drawGroundedButton(ui, tabX + 158.0f, tabY, tabW, tabH, "[2] WORKBENCH", mode == InventoryUIMode::Crafting, mouseX, mouseY, mouseClicked)) {
        returnGridItemsToPlayer(player);
        mode = InventoryUIMode::Crafting;
        updateCraftingResult();
    }
    if (drawGroundedButton(ui, tabX + 316.0f, tabY, tabW, tabH, "[3] SMELTER", mode == InventoryUIMode::Furnace, mouseX, mouseY, mouseClicked)) {
        returnGridItemsToPlayer(player);
        mode = InventoryUIMode::Furnace;
    }
    if (drawGroundedButton(ui, tabX + 474.0f, tabY, tabW, tabH, "[4] VITALS", mode == InventoryUIMode::Vitals, mouseX, mouseY, mouseClicked)) {
        returnGridItemsToPlayer(player);
        mode = InventoryUIMode::Vitals;
    }

    // Close button on far right
    if (drawGroundedButton(ui, panelX + panelW - 74.0f, tabY, 60.0f, tabH, "[X] ESC", false, mouseX, mouseY, mouseClicked, {0.95f, 0.35f, 0.35f, 1.0f})) {
        returnGridItemsToPlayer(player);
        close();
        return;
    }

    Inventory& inv = player.getInventory();
    ItemStack& cursor = inv.getCursorStack();
    const ItemStack* hoveredStack = nullptr;

    float slotSz = 44.0f;
    float gap = 5.0f;

    // =========================================================================
    // TAB 0: BACKPACK & GEAR + INSPECTION DECK
    // =========================================================================
    if (mode == InventoryUIMode::Backpack) {
        // --- 1. LEFT COLUMN: ARMOR & FIELD CRAFT ---
        float leftX = panelX + 22.0f;
        float gearY = panelY + 62.0f;

        ui->drawText("GEAR & ARMOR", leftX, gearY, 1.35f, {0.0f, 0.85f, 0.95f, 1.0f});

        std::string armorLabels[] = { "[HELM]", "[CHEST]", "[LEGS]", "[BOOTS]" };
        for (int i = 0; i < 4; ++i) {
            int slotIdx = Inventory::SLOT_HELMET + i;
            float sy = gearY + 22.0f + i * (slotSz + 6.0f);
            bool hov = (mouseX >= leftX && mouseX <= leftX + slotSz && mouseY >= sy && mouseY <= sy + slotSz);

            if (hov) {
                hoveredSlotIndex = slotIdx;
                hoveredStack = &inv.getSlot(slotIdx);
                if (mouseClicked || rightClicked) {
                    selectedSlotIndex = slotIdx;
                    inv.handleSlotClick(slotIdx, rightClicked, shiftDown);
                    if (audio) audio->playSound(SoundID::ItemPickup, 1.0f, 1.0f);
                }
            }

            renderSlotCard(ui, leftX, sy, slotSz, inv.getSlot(slotIdx), hov, selectedSlotIndex == slotIdx, armorLabels[i], {0.0f, 0.75f, 0.95f, 0.7f});
        }

        // Total Armor Defense Badge
        float defY = gearY + 22.0f + 4 * (slotSz + 6.0f) + 6.0f;
        ui->drawRect(leftX, defY, 130.0f, 32.0f, {0.06f, 0.08f, 0.12f, 0.85f});
        ui->drawRectOutline(leftX, defY, 130.0f, 32.0f, 1.0f, {0.2f, 0.45f, 0.65f, 0.6f});
        int resPct = static_cast<int>(std::min(80.0f, player.getDefense() * 3.5f));
        ui->drawText("DEFENSE: +" + std::to_string(static_cast<int>(player.getDefense())), leftX + 8.0f, defY + 5.0f, 1.15f, {0.0f, 0.85f, 1.0f, 1.0f});
        ui->drawText("RESISTANCE: " + std::to_string(resPct) + "%", leftX + 8.0f, defY + 18.0f, 1.05f, {0.75f, 0.85f, 0.95f, 0.9f});

        // 2x2 Field Crafting Matrix
        float craftY = defY + 44.0f;
        ui->drawText("FIELD CRAFT (2x2)", leftX, craftY, 1.25f, {0.95f, 0.85f, 0.35f, 1.0f});

        for (int r = 0; r < 2; ++r) {
            for (int c = 0; c < 2; ++c) {
                int gIdx = r * 2 + c;
                float sx = leftX + c * (slotSz + gap);
                float sy = craftY + 18.0f + r * (slotSz + gap);
                bool hov = (mouseX >= sx && mouseX <= sx + slotSz && mouseY >= sy && mouseY <= sy + slotSz);

                if (hov) {
                    hoveredStack = &grid2x2[gIdx];
                    if (mouseClicked || rightClicked) {
                        if (shiftDown) {
                            if (!grid2x2[gIdx].isEmpty()) {
                                uint32_t rem = inv.addItem(grid2x2[gIdx].id, grid2x2[gIdx].count);
                                if (rem == 0) grid2x2[gIdx].clear();
                                else grid2x2[gIdx].count = rem;
                                updateCraftingResult();
                                if (audio) audio->playSound(SoundID::ItemPickup, 1.0f, 1.0f);
                            }
                        } else if (rightClicked) {
                            if (cursor.isEmpty()) {
                                if (!grid2x2[gIdx].isEmpty()) {
                                    uint32_t half = (grid2x2[gIdx].count + 1) / 2;
                                    cursor = { grid2x2[gIdx].id, half, grid2x2[gIdx].maxStack };
                                    grid2x2[gIdx].count -= half;
                                    if (grid2x2[gIdx].count == 0) grid2x2[gIdx].clear();
                                }
                            } else {
                                if (grid2x2[gIdx].isEmpty()) {
                                    grid2x2[gIdx] = { cursor.id, 1, cursor.maxStack };
                                    cursor.count--;
                                    if (cursor.count == 0) cursor.clear();
                                } else if (grid2x2[gIdx].id == cursor.id && grid2x2[gIdx].count < grid2x2[gIdx].maxStack) {
                                    grid2x2[gIdx].count++;
                                    cursor.count--;
                                    if (cursor.count == 0) cursor.clear();
                                }
                            }
                            updateCraftingResult();
                            if (audio) audio->playSound(SoundID::ItemPickup, 1.0f, 1.0f);
                        } else {
                            if (cursor.isEmpty()) {
                                cursor = grid2x2[gIdx];
                                grid2x2[gIdx].clear();
                            } else {
                                if (grid2x2[gIdx].isEmpty()) {
                                    grid2x2[gIdx] = cursor;
                                    cursor.clear();
                                } else if (grid2x2[gIdx].id == cursor.id) {
                                    uint32_t space = grid2x2[gIdx].maxStack - grid2x2[gIdx].count;
                                    uint32_t toAdd = std::min(cursor.count, space);
                                    grid2x2[gIdx].count += toAdd;
                                    cursor.count -= toAdd;
                                    if (cursor.count == 0) cursor.clear();
                                } else {
                                    std::swap(grid2x2[gIdx], cursor);
                                }
                            }
                            updateCraftingResult();
                            if (audio) audio->playSound(SoundID::ItemPickup, 1.0f, 1.0f);
                        }
                    }
                }
                renderSlotCard(ui, sx, sy, slotSz, grid2x2[gIdx], hov, false);
            }
        }

        // Result slot
        float resX = leftX + 2 * (slotSz + gap) + 12.0f;
        float resY = craftY + 18.0f + (slotSz * 0.5f);
        float resSz = slotSz * 1.15f;
        ui->drawText("->", resX - 8.0f, resY + 14.0f, 1.4f, {0.0f, 0.85f, 0.95f, 1.0f});

        bool resHov = (mouseX >= resX + 12.0f && mouseX <= resX + 12.0f + resSz && mouseY >= resY && mouseY <= resY + resSz);
        if (resHov) {
            hoveredStack = &craftResult;
            if ((mouseClicked || rightClicked) && !craftResult.isEmpty()) {
                if (shiftDown) {
                    while (!craftResult.isEmpty()) {
                        uint32_t rem = inv.addItem(craftResult.id, craftResult.count);
                        if (rem > 0) break;
                        CraftingRegistry::consumeGrid(grid2x2);
                        updateCraftingResult();
                    }
                    if (audio) audio->playSound(SoundID::LevelUp, 1.2f, 0.7f);
                } else if (cursor.isEmpty()) {
                    cursor = craftResult;
                    CraftingRegistry::consumeGrid(grid2x2);
                    updateCraftingResult();
                    if (audio) audio->playSound(SoundID::LevelUp, 1.2f, 0.7f);
                } else if (cursor.id == craftResult.id && cursor.count + craftResult.count <= cursor.maxStack) {
                    cursor.count += craftResult.count;
                    CraftingRegistry::consumeGrid(grid2x2);
                    updateCraftingResult();
                    if (audio) audio->playSound(SoundID::LevelUp, 1.2f, 0.7f);
                }
            }
        }
        renderSlotCard(ui, resX + 12.0f, resY, resSz, craftResult, resHov, false, "[OUT]", {0.95f, 0.85f, 0.25f, 0.9f});

        // --- 2. CENTER COLUMN: BACKPACK (45 SLOTS) & HOTBAR (9 SLOTS) ---
        float centerStartX = panelX + 175.0f;
        float bpStartY = panelY + 54.0f;
        float bpSlotSz = 40.0f;
        float bpGap = 4.0f;

        uint32_t unlockedTotal = inv.getUnlockedSlotCount();
        uint32_t bpUnlocked = (unlockedTotal > 9) ? (unlockedTotal - 9) : 0;
        ui->drawText("BACKPACK STORAGE (" + std::to_string(bpUnlocked) + "/45 UNLOCKED)", centerStartX, bpStartY, 1.25f, {0.90f, 0.92f, 0.95f, 1.0f});

        // 5 Rows x 9 Cols (Slots 9..53)
        for (int r = 0; r < 5; ++r) {
            for (int c = 0; c < 9; ++c) {
                int slotIdx = 9 + r * 9 + c;
                float sx = centerStartX + c * (bpSlotSz + bpGap);
                float sy = bpStartY + 20.0f + r * (bpSlotSz + bpGap);

                bool unlocked = inv.isSlotUnlocked(slotIdx);
                bool hov = (mouseX >= sx && mouseX <= sx + bpSlotSz && mouseY >= sy && mouseY <= sy + bpSlotSz);
                if (hov) {
                    hoveredSlotIndex = slotIdx;
                    hoveredStack = unlocked ? &inv.getSlot(slotIdx) : nullptr;
                    if (unlocked && (mouseClicked || rightClicked)) {
                        selectedSlotIndex = slotIdx;
                        inv.handleSlotClick(slotIdx, rightClicked, shiftDown);
                        if (audio) audio->playSound(SoundID::ItemPickup, 1.0f, 1.0f);
                    }
                }
                renderSlotCard(ui, sx, sy, bpSlotSz, inv.getSlot(slotIdx), hov, selectedSlotIndex == slotIdx,
                               "", {0.25f, 0.35f, 0.45f, 0.8f}, !unlocked, inv.getRequiredLevelForSlot(slotIdx));
            }
        }

        // Equipped Hotbar (Slots 0..8)
        float hotbarY = bpStartY + 20.0f + 5 * (bpSlotSz + bpGap) + 12.0f;
        ui->drawText("EQUIPPED HOTBAR (QUICK SLOTS)", centerStartX, hotbarY, 1.25f, {1.0f, 0.85f, 0.35f, 1.0f});

        for (int c = 0; c < 9; ++c) {
            int slotIdx = c;
            float sx = centerStartX + c * (bpSlotSz + bpGap);
            float sy = hotbarY + 20.0f;

            bool hov = (mouseX >= sx && mouseX <= sx + bpSlotSz && mouseY >= sy && mouseY <= sy + bpSlotSz);
            if (hov) {
                hoveredSlotIndex = slotIdx;
                hoveredStack = &inv.getSlot(slotIdx);
                if (mouseClicked || rightClicked) {
                    selectedSlotIndex = slotIdx;
                    inv.handleSlotClick(slotIdx, rightClicked, shiftDown);
                    if (audio) audio->playSound(SoundID::ItemPickup, 1.0f, 1.0f);
                }
            }
            renderSlotCard(ui, sx, sy, bpSlotSz, inv.getSlot(slotIdx), hov, selectedSlotIndex == slotIdx, std::to_string(c + 1), {0.2f, 0.65f, 0.95f, 0.85f});
        }

        // --- 3. RIGHT COLUMN: GROUNDED 2 INSPECTION & ACTION DECK ---
        float inspectX = panelX + 580.0f;
        float inspectY = panelY + 54.0f;
        float inspectW = 320.0f;
        float inspectH = 470.0f;

        renderInspectionPanel(ui, inspectX, inspectY, inspectW, inspectH,
                              inv.getSlot(selectedSlotIndex), selectedSlotIndex,
                              player, audio, mouseX, mouseY, mouseClicked);
    }
    // =========================================================================
    // TAB 1: WORKBENCH (CATEGORIZED RECIPE DECK + 3x3 MATRIX)
    // =========================================================================
    else if (mode == InventoryUIMode::Crafting) {
        // --- LEFT COLUMN: CATEGORIZED RECIPE BROWSER ---
        float recX = panelX + 22.0f;
        float recY = panelY + 62.0f;
        float recW = 390.0f;

        ui->drawText("SCA.B RECIPE MATRIX", recX, recY, 1.35f, {0.0f, 0.85f, 0.95f, 1.0f});

        // Category buttons
        std::string categories[] = { "All", "Tools", "Weapons", "Armor", "Survival" };
        float cBtnW = 74.0f;
        for (int i = 0; i < 5; ++i) {
            float bx = recX + i * (cBtnW + 4.0f);
            if (drawGroundedButton(ui, bx, recY + 22.0f, cBtnW, 26.0f, categories[i], currentCraftCategory == categories[i], mouseX, mouseY, mouseClicked)) {
                currentCraftCategory = categories[i];
            }
        }

        // Recipe card list
        std::vector<Recipe> list = (currentCraftCategory == "All") ? CraftingRegistry::getAll() : CraftingRegistry::getByCategory(currentCraftCategory);
        float cardY = recY + 56.0f;
        int maxVisible = 6;

        for (size_t i = 0; i < list.size() && static_cast<int>(i) < maxVisible; ++i) {
            const auto& r = list[i];
            float cy = cardY + i * 56.0f;

            bool canMake = r.canCraft(inv);
            ui->drawRect(recX, cy, recW, 50.0f, canMake ? Vec4(0.08f, 0.14f, 0.18f, 0.95f) : Vec4(0.06f, 0.08f, 0.11f, 0.85f));
            ui->drawRectOutline(recX, cy, recW, 50.0f, 1.0f, canMake ? Vec4(0.2f, 0.75f, 0.65f, 0.85f) : Vec4(0.2f, 0.25f, 0.35f, 0.5f));

            // Result preview icon
            const auto& rDef = ItemRegistry::get(r.result.id);
            ui->drawRect(recX + 6.0f, cy + 6.0f, 38.0f, 38.0f, {0.05f, 0.07f, 0.10f, 0.92f});
            ui->drawRectOutline(recX + 6.0f, cy + 6.0f, 38.0f, 38.0f, 1.0f, canMake ? Vec4(0.2f, 0.75f, 0.65f, 0.85f) : Vec4(0.2f, 0.25f, 0.35f, 0.5f));
            ItemIconRenderer::drawItem(ui, recX + 25.0f, cy + 25.0f, 32.0f, r.result.id);

            // Name
            ui->drawText(r.name, recX + 50.0f, cy + 6.0f, 1.25f, canMake ? Vec4(1, 1, 1, 1) : Vec4(0.6f, 0.65f, 0.7f, 0.9f));

            // Ingredients
            std::stringstream ssIng;
            for (size_t j = 0; j < r.ingredients.size(); ++j) {
                if (j > 0) ssIng << ", ";
                const auto& ingDef = ItemRegistry::get(r.ingredients[j].itemId);
                ssIng << r.ingredients[j].count << "x " << ingDef.name;
            }
            std::string ingStr = ssIng.str();
            if (ingStr.length() > 32) ingStr = ingStr.substr(0, 32) + "...";
            ui->drawText(ingStr, recX + 50.0f, cy + 24.0f, 1.05f, canMake ? Vec4(0.4f, 0.95f, 0.65f, 1.0f) : Vec4(0.7f, 0.45f, 0.45f, 0.9f));

            // Craft button
            if (canMake) {
                if (drawGroundedButton(ui, recX + recW - 74.0f, cy + 10.0f, 68.0f, 30.0f, "[CRAFT]", false, mouseX, mouseY, mouseClicked, {0.1f, 0.95f, 0.5f, 1.0f})) {
                    if (CraftingRegistry::craft(r.id, inv)) {
                        if (audio) audio->playSound(SoundID::LevelUp, 1.2f, 0.8f);
                    }
                }
            } else {
                ui->drawRect(recX + recW - 74.0f, cy + 10.0f, 68.0f, 30.0f, {0.12f, 0.14f, 0.18f, 0.8f});
                ui->drawTextCentered("LOCKED", recX + recW - 40.0f, cy + 19.0f, 1.05f, {0.5f, 0.55f, 0.6f, 0.7f});
            }
        }

        // --- RIGHT COLUMN: 3x3 MATRIX & BACKPACK ---
        float gridX = panelX + 440.0f;
        float gridY = panelY + 62.0f;

        ui->drawText("3x3 WORKBENCH CRAFTING MATRIX", gridX, gridY, 1.35f, {1.0f, 0.85f, 0.35f, 1.0f});

        for (int r = 0; r < 3; ++r) {
            for (int c = 0; c < 3; ++c) {
                int gIdx = r * 3 + c;
                float sx = gridX + c * (slotSz + gap);
                float sy = gridY + 22.0f + r * (slotSz + gap);
                bool hov = (mouseX >= sx && mouseX <= sx + slotSz && mouseY >= sy && mouseY <= sy + slotSz);

                if (hov) {
                    hoveredStack = &grid3x3[gIdx];
                    if (mouseClicked || rightClicked) {
                        if (shiftDown) {
                            if (!grid3x3[gIdx].isEmpty()) {
                                uint32_t rem = inv.addItem(grid3x3[gIdx].id, grid3x3[gIdx].count);
                                if (rem == 0) grid3x3[gIdx].clear();
                                else grid3x3[gIdx].count = rem;
                                updateCraftingResult();
                                if (audio) audio->playSound(SoundID::ItemPickup, 1.0f, 1.0f);
                            }
                        } else if (rightClicked) {
                            if (cursor.isEmpty()) {
                                if (!grid3x3[gIdx].isEmpty()) {
                                    uint32_t half = (grid3x3[gIdx].count + 1) / 2;
                                    cursor = { grid3x3[gIdx].id, half, grid3x3[gIdx].maxStack };
                                    grid3x3[gIdx].count -= half;
                                    if (grid3x3[gIdx].count == 0) grid3x3[gIdx].clear();
                                }
                            } else {
                                if (grid3x3[gIdx].isEmpty()) {
                                    grid3x3[gIdx] = { cursor.id, 1, cursor.maxStack };
                                    cursor.count--;
                                    if (cursor.count == 0) cursor.clear();
                                } else if (grid3x3[gIdx].id == cursor.id && grid3x3[gIdx].count < grid3x3[gIdx].maxStack) {
                                    grid3x3[gIdx].count++;
                                    cursor.count--;
                                    if (cursor.count == 0) cursor.clear();
                                }
                            }
                            updateCraftingResult();
                            if (audio) audio->playSound(SoundID::ItemPickup, 1.0f, 1.0f);
                        } else {
                            if (cursor.isEmpty()) {
                                cursor = grid3x3[gIdx];
                                grid3x3[gIdx].clear();
                            } else {
                                if (grid3x3[gIdx].isEmpty()) {
                                    grid3x3[gIdx] = cursor;
                                    cursor.clear();
                                } else if (grid3x3[gIdx].id == cursor.id) {
                                    uint32_t space = grid3x3[gIdx].maxStack - grid3x3[gIdx].count;
                                    uint32_t toAdd = std::min(cursor.count, space);
                                    grid3x3[gIdx].count += toAdd;
                                    cursor.count -= toAdd;
                                    if (cursor.count == 0) cursor.clear();
                                } else {
                                    std::swap(grid3x3[gIdx], cursor);
                                }
                            }
                            updateCraftingResult();
                            if (audio) audio->playSound(SoundID::ItemPickup, 1.0f, 1.0f);
                        }
                    }
                }
                renderSlotCard(ui, sx, sy, slotSz, grid3x3[gIdx], hov, false);
            }
        }

        // Arrow & 3x3 Result Slot
        float arrowX = gridX + 3 * (slotSz + gap) + 16.0f;
        float arrowY = gridY + 22.0f + slotSz;
        ui->drawText("-->", arrowX, arrowY + 12.0f, 1.8f, {0.0f, 0.85f, 0.95f, 1.0f});

        float resX = arrowX + 44.0f;
        float resY = arrowY - 4.0f;
        float resSz = slotSz * 1.3f;
        bool resHov = (mouseX >= resX && mouseX <= resX + resSz && mouseY >= resY && mouseY <= resY + resSz);
        if (resHov) {
            hoveredStack = &craftResult;
            if ((mouseClicked || rightClicked) && !craftResult.isEmpty()) {
                if (shiftDown) {
                    while (!craftResult.isEmpty()) {
                        uint32_t rem = inv.addItem(craftResult.id, craftResult.count);
                        if (rem > 0) break;
                        CraftingRegistry::consumeGrid(grid3x3);
                        updateCraftingResult();
                    }
                    if (audio) audio->playSound(SoundID::LevelUp, 1.2f, 0.8f);
                } else if (cursor.isEmpty()) {
                    cursor = craftResult;
                    CraftingRegistry::consumeGrid(grid3x3);
                    updateCraftingResult();
                    if (audio) audio->playSound(SoundID::LevelUp, 1.2f, 0.8f);
                } else if (cursor.id == craftResult.id && cursor.count + craftResult.count <= cursor.maxStack) {
                    cursor.count += craftResult.count;
                    CraftingRegistry::consumeGrid(grid3x3);
                    updateCraftingResult();
                    if (audio) audio->playSound(SoundID::LevelUp, 1.2f, 0.8f);
                }
            }
        }
        renderSlotCard(ui, resX, resY, resSz, craftResult, resHov, false, "[OUT]", {0.95f, 0.85f, 0.25f, 0.95f});

        // Player Inventory underneath 3x3 matrix (5 rows)
        float invY = gridY + 22.0f + 3 * (slotSz + gap) + 14.0f;
        float wbSlotSz = slotSz * 0.82f;
        float wbGap = 3.0f;
        uint32_t unlockedTotalWb = inv.getUnlockedSlotCount();
        uint32_t bpUnlockedWb = (unlockedTotalWb > 9) ? (unlockedTotalWb - 9) : 0;
        ui->drawText("PLAYER BACKPACK (" + std::to_string(bpUnlockedWb) + "/45 UNLOCKED)", gridX, invY, 1.2f, {0.85f, 0.9f, 0.95f, 1.0f});

        for (int r = 0; r < 5; ++r) {
            for (int c = 0; c < 9; ++c) {
                int slotIdx = 9 + r * 9 + c;
                float sx = gridX + c * (wbSlotSz + wbGap);
                float sy = invY + 16.0f + r * (wbSlotSz + wbGap);

                bool unlocked = inv.isSlotUnlocked(slotIdx);
                bool hov = (mouseX >= sx && mouseX <= sx + wbSlotSz && mouseY >= sy && mouseY <= sy + wbSlotSz);
                if (hov) {
                    hoveredSlotIndex = slotIdx;
                    hoveredStack = unlocked ? &inv.getSlot(slotIdx) : nullptr;
                    if (unlocked && (mouseClicked || rightClicked)) {
                        inv.handleSlotClick(slotIdx, rightClicked, shiftDown);
                        if (audio) audio->playSound(SoundID::ItemPickup, 1.0f, 1.0f);
                    }
                }
                renderSlotCard(ui, sx, sy, wbSlotSz, inv.getSlot(slotIdx), hov, selectedSlotIndex == slotIdx,
                               "", {0.25f, 0.35f, 0.45f, 0.8f}, !unlocked, inv.getRequiredLevelForSlot(slotIdx));
            }
        }
    }
    // =========================================================================
    // TAB 2: SMELTER (INDUSTRIAL THERMAL PROCESSOR)
    // =========================================================================
    else if (mode == InventoryUIMode::Furnace) {
        float smX = panelX + 36.0f;
        float smY = panelY + 62.0f;

        ui->drawText("SCA.B THERMAL SMELTING PROCESSOR", smX, smY, 1.4f, {1.0f, 0.65f, 0.2f, 1.0f});

        // Temperature display & chamber status
        float statusY = smY + 24.0f;
        ui->drawRect(smX, statusY, 340.0f, 36.0f, {0.08f, 0.10f, 0.14f, 0.9f});
        ui->drawRectOutline(smX, statusY, 340.0f, 36.0f, 1.0f, {0.3f, 0.4f, 0.5f, 0.6f});

        if (furnace.isBurning()) {
            ui->drawText("COMBUSTION: 1250°C [ACTIVE]", smX + 12.0f, statusY + 10.0f, 1.25f, {1.0f, 0.65f, 0.15f, 1.0f});
        } else {
            ui->drawText("COMBUSTION: AMBIENT 24°C [IDLE]", smX + 12.0f, statusY + 10.0f, 1.25f, {0.45f, 0.65f, 0.85f, 1.0f});
        }

        // Input Slot (Top)
        float inX = smX + 40.0f;
        float inY = statusY + 54.0f;
        bool inHov = (mouseX >= inX && mouseX <= inX + slotSz && mouseY >= inY && mouseY <= inY + slotSz);
        if (inHov) {
            hoveredStack = &furnace.getInputSlot();
            if (mouseClicked || rightClicked) {
                if (shiftDown && !furnace.getInputSlot().isEmpty()) {
                    uint32_t rem = inv.addItem(furnace.getInputSlot().id, furnace.getInputSlot().count);
                    if (rem == 0) furnace.getInputSlot().clear();
                    else furnace.getInputSlot().count = rem;
                    if (audio) audio->playSound(SoundID::ItemPickup, 1.0f, 1.0f);
                } else if (cursor.isEmpty()) {
                    cursor = furnace.getInputSlot();
                    furnace.getInputSlot().clear();
                    if (audio) audio->playSound(SoundID::ItemPickup, 1.0f, 1.0f);
                } else {
                    if (furnace.getInputSlot().isEmpty()) {
                        furnace.getInputSlot() = cursor;
                        cursor.clear();
                    } else if (furnace.getInputSlot().id == cursor.id) {
                        uint32_t sp = furnace.getInputSlot().maxStack - furnace.getInputSlot().count;
                        uint32_t add = std::min(cursor.count, sp);
                        furnace.getInputSlot().count += add;
                        cursor.count -= add;
                        if (cursor.count == 0) cursor.clear();
                    } else {
                        std::swap(furnace.getInputSlot(), cursor);
                    }
                    if (audio) audio->playSound(SoundID::ItemPickup, 1.0f, 1.0f);
                }
            }
        }
        renderSlotCard(ui, inX, inY, slotSz, furnace.getInputSlot(), inHov, false, "[ORE]", {0.7f, 0.75f, 0.85f, 0.8f});

        // Flame indicator
        float flameX = inX + (slotSz - 28.0f) * 0.5f;
        float flameY = inY + slotSz + 6.0f;
        float flameW = 28.0f;
        float flameH = 24.0f;

        ui->drawRect(flameX, flameY, flameW, flameH, {0.15f, 0.10f, 0.08f, 0.85f});
        if (furnace.isBurning()) {
            float bFrac = furnace.getBurnProgress();
            float fillH = flameH * bFrac;
            ui->drawRect(flameX + 2, flameY + flameH - fillH, flameW - 4, fillH, {1.0f, 0.45f, 0.10f, 0.95f});
            ui->drawText("FIRE", flameX + 2, flameY + 7.0f, 1.05f, {1.0f, 0.95f, 0.35f, 1.0f});
        } else {
            ui->drawText("OFF", flameX + 4, flameY + 7.0f, 1.05f, {0.5f, 0.5f, 0.55f, 0.8f});
        }

        // Fuel Slot (Bottom)
        float fuelX = inX;
        float fuelY = flameY + flameH + 6.0f;
        bool fuelHov = (mouseX >= fuelX && mouseX <= fuelX + slotSz && mouseY >= fuelY && mouseY <= fuelY + slotSz);
        if (fuelHov) {
            hoveredStack = &furnace.getFuelSlot();
            if (mouseClicked || rightClicked) {
                if (shiftDown && !furnace.getFuelSlot().isEmpty()) {
                    uint32_t rem = inv.addItem(furnace.getFuelSlot().id, furnace.getFuelSlot().count);
                    if (rem == 0) furnace.getFuelSlot().clear();
                    else furnace.getFuelSlot().count = rem;
                    if (audio) audio->playSound(SoundID::ItemPickup, 1.0f, 1.0f);
                } else if (cursor.isEmpty()) {
                    cursor = furnace.getFuelSlot();
                    furnace.getFuelSlot().clear();
                    if (audio) audio->playSound(SoundID::ItemPickup, 1.0f, 1.0f);
                } else {
                    if (furnace.getFuelSlot().isEmpty()) {
                        furnace.getFuelSlot() = cursor;
                        cursor.clear();
                    } else if (furnace.getFuelSlot().id == cursor.id) {
                        uint32_t sp = furnace.getFuelSlot().maxStack - furnace.getFuelSlot().count;
                        uint32_t add = std::min(cursor.count, sp);
                        furnace.getFuelSlot().count += add;
                        cursor.count -= add;
                        if (cursor.count == 0) cursor.clear();
                    } else {
                        std::swap(furnace.getFuelSlot(), cursor);
                    }
                    if (audio) audio->playSound(SoundID::ItemPickup, 1.0f, 1.0f);
                }
            }
        }
        renderSlotCard(ui, fuelX, fuelY, slotSz, furnace.getFuelSlot(), fuelHov, false, "[COAL]", {1.0f, 0.65f, 0.2f, 0.85f});

        // Cooking Progress Bar & Output
        float cookX = inX + slotSz + 24.0f;
        float cookY = inY + 36.0f;
        float cookW = 80.0f;
        float cookH = 16.0f;

        ui->drawProgressBar(cookX, cookY, cookW, cookH, furnace.getCookProgress(), {1.0f, 0.85f, 0.25f, 0.95f}, {0.15f, 0.18f, 0.22f, 0.85f});
        ui->drawText("SMELT", cookX + 16.0f, cookY + 3.0f, 1.15f, {1, 1, 1, 1});

        float outX = cookX + cookW + 24.0f;
        float outY = inY + 22.0f;
        float outSz = slotSz * 1.35f;
        bool outHov = (mouseX >= outX && mouseX <= outX + outSz && mouseY >= outY && mouseY <= outY + outSz);
        if (outHov) {
            hoveredStack = &furnace.getOutputSlot();
            if ((mouseClicked || rightClicked) && !furnace.getOutputSlot().isEmpty()) {
                if (shiftDown) {
                    uint32_t rem = inv.addItem(furnace.getOutputSlot().id, furnace.getOutputSlot().count);
                    if (rem == 0) furnace.getOutputSlot().clear();
                    else furnace.getOutputSlot().count = rem;
                    if (audio) audio->playSound(SoundID::LevelUp, 1.2f, 0.8f);
                } else if (cursor.isEmpty()) {
                    cursor = furnace.getOutputSlot();
                    furnace.getOutputSlot().clear();
                    if (audio) audio->playSound(SoundID::LevelUp, 1.2f, 0.8f);
                } else if (cursor.id == furnace.getOutputSlot().id && cursor.count + furnace.getOutputSlot().count <= cursor.maxStack) {
                    cursor.count += furnace.getOutputSlot().count;
                    furnace.getOutputSlot().clear();
                    if (audio) audio->playSound(SoundID::LevelUp, 1.2f, 0.8f);
                }
            }
        }
        renderSlotCard(ui, outX, outY, outSz, furnace.getOutputSlot(), outHov, false, "[OUT]", {0.95f, 0.85f, 0.25f, 0.95f});

        // Fuel Efficiency Card
        float guideY = fuelY + slotSz + 18.0f;
        ui->drawRect(smX, guideY, 340.0f, 100.0f, {0.06f, 0.08f, 0.12f, 0.85f});
        ui->drawRectOutline(smX, guideY, 340.0f, 100.0f, 1.0f, {0.2f, 0.35f, 0.45f, 0.6f});
        ui->drawText("THERMAL EFFICIENCY GUIDE", smX + 12.0f, guideY + 8.0f, 1.2f, {0.0f, 0.85f, 0.95f, 1.0f});
        ui->drawText("- Coal: 80s burn (8 smelt operations)", smX + 12.0f, guideY + 30.0f, 1.05f, {0.85f, 0.9f, 0.95f, 0.9f});
        ui->drawText("- Planks: 15s burn (1.5 smelt operations)", smX + 12.0f, guideY + 48.0f, 1.05f, {0.85f, 0.9f, 0.95f, 0.9f});
        ui->drawText("- Wood Logs: 10s burn (1 smelt operation)", smX + 12.0f, guideY + 66.0f, 1.05f, {0.85f, 0.9f, 0.95f, 0.9f});

        // Right side: Player inventory with easy Shift-Click transfer into furnace
        float bpX = panelX + 410.0f;
        float bpY = smY + 16.0f;
        float smSlotSz = 38.0f;
        float smGap = 4.0f;

        uint32_t unlockedTotalSm = inv.getUnlockedSlotCount();
        uint32_t bpUnlockedSm = (unlockedTotalSm > 9) ? (unlockedTotalSm - 9) : 0;
        ui->drawText("PLAYER BACKPACK (" + std::to_string(bpUnlockedSm) + "/45 UNLOCKED)", bpX, bpY - 14.0f, 1.25f, {0.85f, 0.92f, 0.98f, 1.0f});

        for (int r = 0; r < 5; ++r) {
            for (int c = 0; c < 9; ++c) {
                int slotIdx = 9 + r * 9 + c;
                float sx = bpX + c * (smSlotSz + smGap);
                float sy = bpY + r * (smSlotSz + smGap);

                bool unlocked = inv.isSlotUnlocked(slotIdx);
                bool hov = (mouseX >= sx && mouseX <= sx + smSlotSz && mouseY >= sy && mouseY <= sy + smSlotSz);
                if (hov) {
                    hoveredSlotIndex = slotIdx;
                    hoveredStack = unlocked ? &inv.getSlot(slotIdx) : nullptr;
                    if (unlocked && (mouseClicked || rightClicked)) {
                        if (shiftDown && !inv.getSlot(slotIdx).isEmpty()) {
                            ItemStack& item = inv.getSlot(slotIdx);
                            if (FurnaceManager::findRecipe(item.id)) {
                                if (furnace.getInputSlot().isEmpty()) {
                                    furnace.getInputSlot() = item;
                                    item.clear();
                                } else if (furnace.getInputSlot().id == item.id) {
                                    uint32_t sp = furnace.getInputSlot().maxStack - furnace.getInputSlot().count;
                                    uint32_t add = std::min(item.count, sp);
                                    furnace.getInputSlot().count += add;
                                    item.count -= add;
                                    if (item.count == 0) item.clear();
                                }
                                if (audio) audio->playSound(SoundID::ItemPickup, 1.0f, 1.0f);
                            } else if (FurnaceManager::getFuelDuration(item.id) > 0.0f) {
                                if (furnace.getFuelSlot().isEmpty()) {
                                    furnace.getFuelSlot() = item;
                                    item.clear();
                                } else if (furnace.getFuelSlot().id == item.id) {
                                    uint32_t sp = furnace.getFuelSlot().maxStack - furnace.getFuelSlot().count;
                                    uint32_t add = std::min(item.count, sp);
                                    furnace.getFuelSlot().count += add;
                                    item.count -= add;
                                    if (item.count == 0) item.clear();
                                }
                                if (audio) audio->playSound(SoundID::ItemPickup, 1.0f, 1.0f);
                            } else {
                                inv.handleSlotClick(slotIdx, rightClicked, true);
                            }
                        } else {
                            inv.handleSlotClick(slotIdx, rightClicked, shiftDown);
                        }
                    }
                }
                renderSlotCard(ui, sx, sy, smSlotSz, inv.getSlot(slotIdx), hov, selectedSlotIndex == slotIdx,
                               "", {0.25f, 0.35f, 0.45f, 0.8f}, !unlocked, inv.getRequiredLevelForSlot(slotIdx));
            }
        }

        // Hotbar
        float hbY = bpY + 5 * (smSlotSz + smGap) + 12.0f;
        ui->drawText("EQUIPPED HOTBAR", bpX, hbY - 12.0f, 1.15f, {1.0f, 0.85f, 0.35f, 1.0f});
        for (int c = 0; c < 9; ++c) {
            int slotIdx = c;
            float sx = bpX + c * (smSlotSz + smGap);
            float sy = hbY;

            bool hov = (mouseX >= sx && mouseX <= sx + smSlotSz && mouseY >= sy && mouseY <= sy + smSlotSz);
            if (hov) {
                hoveredSlotIndex = slotIdx;
                hoveredStack = &inv.getSlot(slotIdx);
                if (mouseClicked || rightClicked) {
                    if (shiftDown && !inv.getSlot(slotIdx).isEmpty()) {
                        ItemStack& item = inv.getSlot(slotIdx);
                        if (FurnaceManager::findRecipe(item.id)) {
                            if (furnace.getInputSlot().isEmpty()) {
                                furnace.getInputSlot() = item;
                                item.clear();
                            } else if (furnace.getInputSlot().id == item.id) {
                                uint32_t sp = furnace.getInputSlot().maxStack - furnace.getInputSlot().count;
                                uint32_t add = std::min(item.count, sp);
                                furnace.getInputSlot().count += add;
                                item.count -= add;
                                if (item.count == 0) item.clear();
                            }
                            if (audio) audio->playSound(SoundID::ItemPickup, 1.0f, 1.0f);
                        } else if (FurnaceManager::getFuelDuration(item.id) > 0.0f) {
                            if (furnace.getFuelSlot().isEmpty()) {
                                furnace.getFuelSlot() = item;
                                item.clear();
                            } else if (furnace.getFuelSlot().id == item.id) {
                                uint32_t sp = furnace.getFuelSlot().maxStack - furnace.getFuelSlot().count;
                                uint32_t add = std::min(item.count, sp);
                                furnace.getFuelSlot().count += add;
                                item.count -= add;
                                if (item.count == 0) item.clear();
                            }
                            if (audio) audio->playSound(SoundID::ItemPickup, 1.0f, 1.0f);
                        } else {
                            inv.handleSlotClick(slotIdx, rightClicked, true);
                        }
                    } else {
                        inv.handleSlotClick(slotIdx, rightClicked, shiftDown);
                    }
                }
            }
            renderSlotCard(ui, sx, sy, smSlotSz, inv.getSlot(slotIdx), hov, selectedSlotIndex == slotIdx, std::to_string(c + 1), {0.2f, 0.65f, 0.95f, 0.85f});
        }
    }
    // =========================================================================
    // TAB 3: VITALS (SCA.B BIOMETRIC & COMBAT MONITOR)
    // =========================================================================
    else if (mode == InventoryUIMode::Vitals) {
        float vitX = panelX + 40.0f;
        float vitY = panelY + 62.0f;

        ui->drawText("SCA.B // BIOMETRIC & COMBAT VITALS", vitX, vitY, 1.45f, {0.0f, 0.95f, 0.85f, 1.0f});

        // Left Panel: Vitals & Progression
        float p1W = 400.0f;
        float p1H = 440.0f;
        ui->drawRect(vitX, vitY + 24.0f, p1W, p1H, {0.08f, 0.10f, 0.14f, 0.92f});
        ui->drawRectOutline(vitX, vitY + 24.0f, p1W, p1H, 1.5f, {0.18f, 0.35f, 0.50f, 0.8f});

        float vy = vitY + 40.0f;
        ui->drawText("ADVENTURER LEVEL: " + std::to_string(player.getLevel()), vitX + 16.0f, vy, 1.4f, {1.0f, 0.85f, 0.35f, 1.0f});

        vy += 24.0f;
        ui->drawText("EXPERIENCE POINTS", vitX + 16.0f, vy, 1.15f, {0.85f, 0.88f, 0.95f, 1.0f});
        float xpFrac = (player.getXPToNext() > 0) ? std::min(1.0f, static_cast<float>(player.getXP()) / player.getXPToNext()) : 0.0f;
        ui->drawProgressBar(vitX + 16.0f, vy + 16.0f, p1W - 32.0f, 10.0f, xpFrac, {0.2f, 0.85f, 0.45f, 1.0f}, {0.12f, 0.15f, 0.20f, 0.8f});
        ui->drawText(std::to_string(static_cast<int>(player.getXP())) + " / " + std::to_string(static_cast<int>(player.getXPToNext())) + " XP", vitX + 16.0f, vy + 28.0f, 1.05f, {0.7f, 0.8f, 0.9f, 0.9f});

        vy += 54.0f;
        ui->drawText("HEALTH", vitX + 16.0f, vy, 1.15f, {1.0f, 0.35f, 0.35f, 1.0f});
        float hpFrac = (player.getMaxHealth() > 0) ? std::clamp(player.getHealth() / player.getMaxHealth(), 0.0f, 1.0f) : 0.0f;
        ui->drawProgressBar(vitX + 16.0f, vy + 16.0f, p1W - 32.0f, 14.0f, hpFrac, {0.95f, 0.25f, 0.25f, 1.0f}, {0.2f, 0.08f, 0.08f, 0.8f});
        ui->drawText(std::to_string(static_cast<int>(player.getHealth())) + " / " + std::to_string(static_cast<int>(player.getMaxHealth())) + " HP", vitX + 24.0f, vy + 17.0f, 1.15f, {1, 1, 1, 1});

        vy += 48.0f;
        ui->drawText("MANA / FOCUS", vitX + 16.0f, vy, 1.15f, {0.25f, 0.65f, 1.0f, 1.0f});
        float mpFrac = (player.getMaxMana() > 0) ? std::clamp(player.getMana() / player.getMaxMana(), 0.0f, 1.0f) : 0.0f;
        ui->drawProgressBar(vitX + 16.0f, vy + 16.0f, p1W - 32.0f, 14.0f, mpFrac, {0.2f, 0.65f, 1.0f, 1.0f}, {0.08f, 0.12f, 0.2f, 0.8f});
        ui->drawText(std::to_string(static_cast<int>(player.getMana())) + " / " + std::to_string(static_cast<int>(player.getMaxMana())) + " MP", vitX + 24.0f, vy + 17.0f, 1.15f, {1, 1, 1, 1});

        vy += 48.0f;
        ui->drawText("STAMINA / ENDURANCE", vitX + 16.0f, vy, 1.15f, {0.35f, 0.95f, 0.45f, 1.0f});
        float staFrac = (player.getMaxStamina() > 0) ? std::clamp(player.getStamina() / player.getMaxStamina(), 0.0f, 1.0f) : 0.0f;
        ui->drawProgressBar(vitX + 16.0f, vy + 16.0f, p1W - 32.0f, 14.0f, staFrac, {0.25f, 0.90f, 0.40f, 1.0f}, {0.08f, 0.18f, 0.10f, 0.8f});
        ui->drawText(std::to_string(static_cast<int>(player.getStamina())) + " / " + std::to_string(static_cast<int>(player.getMaxStamina())) + " STA (+15 / Lvl)", vitX + 24.0f, vy + 17.0f, 1.15f, {1, 1, 1, 1});

        vy += 48.0f;
        ui->drawText("STORAGE CAPACITY", vitX + 16.0f, vy, 1.15f, {0.95f, 0.75f, 0.25f, 1.0f});
        uint32_t unlocked = inv.getUnlockedSlotCount();
        float invFrac = std::clamp(static_cast<float>(unlocked) / static_cast<float>(Inventory::MAIN_AND_HOTBAR), 0.0f, 1.0f);
        ui->drawProgressBar(vitX + 16.0f, vy + 16.0f, p1W - 32.0f, 14.0f, invFrac, {0.95f, 0.75f, 0.25f, 1.0f}, {0.18f, 0.14f, 0.08f, 0.8f});
        ui->drawText(std::to_string(unlocked) + " / " + std::to_string(Inventory::MAIN_AND_HOTBAR) + " Slots Unlocked (+3 / Lvl)", vitX + 24.0f, vy + 17.0f, 1.15f, {1, 1, 1, 1});

        // Right Panel: Combat Performance & Survival Conditions
        float p2X = vitX + p1W + 24.0f;
        float p2W = 415.0f;
        ui->drawRect(p2X, vitY + 24.0f, p2W, p1H, {0.08f, 0.10f, 0.14f, 0.92f});
        ui->drawRectOutline(p2X, vitY + 24.0f, p2W, p1H, 1.5f, {0.18f, 0.35f, 0.50f, 0.8f});

        float cY = vitY + 40.0f;
        ui->drawText("COMBAT ATTRIBUTES", p2X + 16.0f, cY, 1.35f, {0.0f, 0.85f, 0.95f, 1.0f});

        cY += 28.0f;
        ui->drawText("Total Attack Power: +" + std::to_string(static_cast<int>(player.getAttackPower())), p2X + 16.0f, cY, 1.3f, {1.0f, 0.8f, 0.3f, 1.0f});
        cY += 24.0f;
        int defRes = static_cast<int>(std::min(80.0f, player.getDefense() * 3.5f));
        ui->drawText("Armor Defense Rating: +" + std::to_string(static_cast<int>(player.getDefense())) + " (" + std::to_string(defRes) + "% Damage Reduction)", p2X + 16.0f, cY, 1.25f, {0.2f, 0.85f, 1.0f, 1.0f});

        cY += 36.0f;
        ui->drawText("SURVIVAL PERKS & CONDITIONS", p2X + 16.0f, cY, 1.35f, {0.2f, 0.95f, 0.45f, 1.0f});
        cY += 24.0f;
        ui->drawText("- Level Growth: +15 Max Stamina & +3 Slots per level", p2X + 16.0f, cY, 1.1f, {0.4f, 0.95f, 0.65f, 1.0f});
        cY += 20.0f;
        ui->drawText("- Well Rested: Movement speed optimal (+0%)", p2X + 16.0f, cY, 1.1f, {0.85f, 0.9f, 0.95f, 0.9f});
        cY += 20.0f;
        ui->drawText("- Oxygen Saturation: 100% [OPTIMAL]", p2X + 16.0f, cY, 1.1f, {0.85f, 0.9f, 0.95f, 0.9f});
        cY += 20.0f;
        ui->drawText("- Pal Sphere Matrix: Ready for creature taming", p2X + 16.0f, cY, 1.1f, {0.85f, 0.9f, 0.95f, 0.9f});
        cY += 20.0f;
        ui->drawText("- Habitat Adaptation: Safe ambient temperature", p2X + 16.0f, cY, 1.1f, {0.85f, 0.9f, 0.95f, 0.9f});
    }

    // =========================================================================
    // BOTTOM BAR: GROUNDED 2 SYSTEM KEYS GUIDE
    // =========================================================================
    float btmY = panelY + panelH - 30.0f;
    ui->drawRect(panelX, btmY, panelW, 30.0f, {0.06f, 0.08f, 0.11f, 0.98f});
    ui->drawRectOutline(panelX, btmY, panelW, 30.0f, 1.0f, {0.18f, 0.28f, 0.38f, 0.6f});

    std::string btmTips = "[TAB/E] Close  [L-CLICK] Select/Move  [R-CLICK] Split Half  [SHIFT+CLICK] Fast Transfer  [1-9] Hotbar Swap  [Q] Drop";
    ui->drawText(btmTips, panelX + 16.0f, btmY + 8.0f, 1.12f, {0.0f, 0.85f, 0.95f, 0.9f});

    // =========================================================================
    // HOVER TOOLTIP CARD (When mouse hovers over an item slot or locked slot)
    // =========================================================================
    if (hoveredSlotIndex >= 0 && !inv.isSlotUnlocked(hoveredSlotIndex)) {
        float tipW = 240.0f;
        float tipH = 54.0f;
        float tipX = mouseX + 14.0f;
        float tipY = mouseY + 14.0f;
        if (tipX + tipW > screenWidth - 10.0f) tipX = mouseX - tipW - 14.0f;
        if (tipY + tipH > screenHeight - 10.0f) tipY = mouseY - tipH - 14.0f;

        ui->drawRect(tipX, tipY, tipW, tipH, {0.08f, 0.04f, 0.05f, 0.96f});
        ui->drawRectOutline(tipX, tipY, tipW, tipH, 1.5f, {0.95f, 0.35f, 0.35f, 0.9f});
        int reqLvl = inv.getRequiredLevelForSlot(hoveredSlotIndex);
        ui->drawText("[SLOT LOCKED]", tipX + 8.0f, tipY + 8.0f, 1.25f, {1.0f, 0.4f, 0.4f, 1.0f});
        ui->drawText("Requires Adventurer Level " + std::to_string(reqLvl), tipX + 8.0f, tipY + 28.0f, 1.1f, {0.85f, 0.85f, 0.85f, 0.9f});
    } else if (hoveredStack && !hoveredStack->isEmpty()) {
        const auto& def = ItemRegistry::get(hoveredStack->id);

        float tipW = 260.0f;
        float tipH = 90.0f;
        float tipX = mouseX + 14.0f;
        float tipY = mouseY + 14.0f;

        if (tipX + tipW > screenWidth - 10.0f) tipX = mouseX - tipW - 14.0f;
        if (tipY + tipH > screenHeight - 10.0f) tipY = mouseY - tipH - 14.0f;

        ui->drawRect(tipX, tipY, tipW, tipH, {0.05f, 0.07f, 0.10f, 0.96f});
        ui->drawRectOutline(tipX, tipY, tipW, tipH, 1.5f, def.color);

        ui->drawText(def.name, tipX + 8.0f, tipY + 8.0f, 1.4f, {1.0f, 1.0f, 0.9f, 1.0f});

        std::stringstream ssCat;
        if (def.category == ItemCategory::Weapon) {
            ssCat << "[Weapon] Atk: +" << static_cast<int>(def.attackDamage) << " (" << std::fixed << std::setprecision(1) << def.attackSpeed << " Spd)";
        } else if (def.category == ItemCategory::Armor) {
            ssCat << "[Armor] Defense: +" << static_cast<int>(def.armorDefense);
        } else if (def.category == ItemCategory::Tool) {
            ssCat << "[Tool: T" << def.toolTier << "] Atk: +" << static_cast<int>(def.attackDamage) << " (" << std::fixed << std::setprecision(1) << def.attackSpeed << " Spd)";
        } else if (def.category == ItemCategory::Consumable) {
            ssCat << "[Consumable]";
        } else if (def.category == ItemCategory::Block) {
            ssCat << "[Voxel Block #" << def.id << "]";
        } else {
            ssCat << "[Material]";
        }

        ui->drawText(ssCat.str(), tipX + 8.0f, tipY + 28.0f, 1.15f, {0.0f, 0.85f, 0.95f, 1.0f});

        std::string desc = def.description;
        if (desc.length() > 36) {
            ui->drawText(desc.substr(0, 36), tipX + 8.0f, tipY + 48.0f, 1.05f, {0.8f, 0.85f, 0.9f, 0.9f});
            ui->drawText(desc.substr(36), tipX + 8.0f, tipY + 64.0f, 1.05f, {0.8f, 0.85f, 0.9f, 0.9f});
        } else {
            ui->drawText(desc, tipX + 8.0f, tipY + 50.0f, 1.1f, {0.8f, 0.85f, 0.9f, 0.9f});
        }
    }

    // =========================================================================
    // CURSOR FLOATING ITEM STACK
    // =========================================================================
    if (!cursor.isEmpty()) {
        const auto& def = ItemRegistry::get(cursor.id);
        float cs = 38.0f;
        float cx = mouseX - cs * 0.5f;
        float cy = mouseY - cs * 0.5f;

        ui->drawRect(cx, cy, cs, cs, {0.05f, 0.08f, 0.12f, 0.85f});
        ui->drawRectOutline(cx, cy, cs, cs, 1.5f, {0.0f, 0.95f, 1.0f, 0.95f});
        ItemIconRenderer::drawItem(ui, static_cast<float>(mouseX), static_cast<float>(mouseY), cs - 6.0f, cursor.id);
        if (cursor.count > 1) {
            std::string cStr = std::to_string(cursor.count);
            float cW = cStr.length() * 6.0f * 1.25f;
            ui->drawRect(cx + cs - cW - 4.0f, cy + cs - 13.0f, cW + 4.0f, 12.0f, {0.05f, 0.06f, 0.08f, 0.9f});
            ui->drawText(cStr, cx + cs - cW - 2.0f, cy + cs - 11.0f, 1.25f, {1.0f, 1.0f, 0.85f, 1.0f});
        }
    }

    // High-visibility Custom Software Cursor
    drawCustomCursor(ui, static_cast<float>(mouseX), static_cast<float>(mouseY));
}

} // namespace Aetheria
