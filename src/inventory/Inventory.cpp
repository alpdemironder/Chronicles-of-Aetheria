#include "Inventory.hpp"
#include "ItemRegistry.hpp"
#include <algorithm>

namespace Aetheria {

Inventory::Inventory() {
    for (auto& s : slots) {
        s.clear();
    }

    // Default starter adventurer equipment in hotbar (0..8)
    slots[0] = { 514, 1, 1 };    // Iron Pickaxe
    slots[1] = { 578, 1, 1 };    // Iron Axe
    slots[2] = { 513, 1, 1 };    // Iron Broadsword
    slots[3] = { 72, 32, 64 };   // Oak Planks
    slots[4] = { 39, 32, 64 };   // Stone Bricks
    slots[5] = { 551, 16, 64 };  // Torches
    slots[6] = { 520, 4, 16 };   // Health Potions
    slots[7] = { 523, 3, 16 };   // Pal Spheres
    slots[8] = { 71, 16, 64 };   // Oak Logs

    // Starter backpack supplies in main inventory (9..35)
    slots[9]  = { 36, 32, 64 };  // Stone
    slots[10] = { 501, 16, 64 }; // Iron Ingot
    slots[11] = { 552, 24, 64 }; // Coal
    slots[12] = { 550, 32, 64 }; // Wooden Sticks
    slots[13] = { 508, 8, 32 };  // Raw Meat
    slots[14] = { 560, 6, 32 };  // Baked Bread
    slots[15] = { 506, 12, 64 }; // Tanned Leather
    slots[16] = { 554, 4, 64 };  // Diamond Gems

    // Starter equipped armor (36..39)
    slots[SLOT_HELMET] = { 534, 1, 1 }; // Iron Helmet (+3 Def)
    slots[SLOT_CHEST]  = { 535, 1, 1 }; // Iron Chestplate (+6 Def)
    slots[SLOT_LEGS]   = { 536, 1, 1 }; // Iron Leggings (+5 Def)
    slots[SLOT_BOOTS]  = { 537, 1, 1 }; // Iron Boots (+2 Def)
}

ArmorType Inventory::getSlotArmorType(int slotIdx) {
    if (slotIdx == SLOT_HELMET) return ArmorType::Helmet;
    if (slotIdx == SLOT_CHEST)  return ArmorType::Chestplate;
    if (slotIdx == SLOT_LEGS)   return ArmorType::Leggings;
    if (slotIdx == SLOT_BOOTS)  return ArmorType::Boots;
    return ArmorType::None;
}

bool Inventory::canAcceptItem(int slotIdx, const ItemStack& stack) const {
    if (!isSlotUnlocked(slotIdx)) return false;
    if (stack.isEmpty()) return true;
    if (!isArmorSlot(slotIdx)) return true;

    const auto& def = ItemRegistry::get(stack.id);
    ArmorType required = getSlotArmorType(slotIdx);
    return (def.armorType == required);
}

float Inventory::getTotalDefense() const {
    float defense = 0.0f;
    for (int i = SLOT_HELMET; i <= SLOT_BOOTS; ++i) {
        if (!slots[i].isEmpty()) {
            const auto& def = ItemRegistry::get(slots[i].id);
            defense += def.armorDefense;
        }
    }
    return defense;
}

const ItemStack& Inventory::getSlot(int index) const {
    if (index >= 0 && index < TOTAL_SLOTS) {
        return slots[index];
    }
    static ItemStack dummy;
    dummy.clear();
    return dummy;
}

ItemStack& Inventory::getSlot(int index) {
    if (index >= 0 && index < TOTAL_SLOTS) {
        return slots[index];
    }
    static ItemStack dummy;
    dummy.clear();
    return dummy;
}

void Inventory::setSlot(int index, const ItemStack& stack) {
    if (index >= 0 && index < TOTAL_SLOTS && isSlotUnlocked(index)) {
        slots[index] = stack;
    }
}

uint32_t Inventory::addItem(uint16_t id, uint32_t count) {
    if (id == 0 || count == 0) return 0;

    const auto& def = ItemRegistry::get(id);
    uint32_t maxS = def.maxStack > 0 ? def.maxStack : 64;

    // Pass 1: Try stacking into existing non-full slots in unlocked main & hotbar
    for (int i = 0; i < MAIN_AND_HOTBAR && count > 0; ++i) {
        if (!isSlotUnlocked(i)) continue;
        if (slots[i].id == id && slots[i].count < maxS) {
            uint32_t canAdd = maxS - slots[i].count;
            uint32_t toAdd = std::min(count, canAdd);
            slots[i].count += toAdd;
            slots[i].maxStack = maxS;
            count -= toAdd;
        }
    }

    // Pass 2: Fill empty slots in unlocked main & hotbar
    for (int i = 0; i < MAIN_AND_HOTBAR && count > 0; ++i) {
        if (!isSlotUnlocked(i)) continue;
        if (slots[i].isEmpty()) {
            uint32_t toAdd = std::min(count, maxS);
            slots[i].id = id;
            slots[i].count = toAdd;
            slots[i].maxStack = maxS;
            count -= toAdd;
        }
    }

    return count; // Remaining count if inventory is full
}

bool Inventory::removeItem(uint16_t id, uint32_t count) {
    if (id == 0 || count == 0) return true;
    if (countItem(id) < count) return false;

    // Deduct count across main & hotbar slots
    uint32_t remaining = count;
    for (int i = MAIN_AND_HOTBAR - 1; i >= 0 && remaining > 0; --i) {
        if (slots[i].id == id) {
            if (slots[i].count <= remaining) {
                remaining -= slots[i].count;
                slots[i].clear();
            } else {
                slots[i].count -= remaining;
                remaining = 0;
            }
        }
    }

    return true;
}

uint32_t Inventory::countItem(uint16_t id) const {
    if (id == 0) return 0;
    uint32_t total = 0;
    for (int i = 0; i < MAIN_AND_HOTBAR; ++i) {
        if (slots[i].id == id) {
            total += slots[i].count;
        }
    }
    return total;
}

void Inventory::swapSlots(int idxA, int idxB) {
    if (idxA >= 0 && idxA < TOTAL_SLOTS && idxB >= 0 && idxB < TOTAL_SLOTS) {
        if (!isSlotUnlocked(idxA) || !isSlotUnlocked(idxB)) return;
        if (!canAcceptItem(idxA, slots[idxB]) || !canAcceptItem(idxB, slots[idxA])) return;
        std::swap(slots[idxA], slots[idxB]);
    }
}

void Inventory::handleSlotClick(int slotIdx, bool isRightClick, bool isShiftClick) {
    if (slotIdx < 0 || slotIdx >= TOTAL_SLOTS) return;
    if (!isSlotUnlocked(slotIdx)) return;

    ItemStack& slot = slots[slotIdx];

    // Quick-move with Shift+Click
    if (isShiftClick) {
        if (slot.isEmpty()) return;

        const auto& def = ItemRegistry::get(slot.id);

        // If clicking armor slot, move to unlocked main inventory
        if (isArmorSlot(slotIdx)) {
            for (int i = 0; i < MAIN_AND_HOTBAR; ++i) {
                if (!isSlotUnlocked(i)) continue;
                if (slots[i].isEmpty()) {
                    slots[i] = slot;
                    slot.clear();
                    return;
                }
            }
            return;
        }

        // If clicking armor item in inventory, auto-equip to armor slot
        if (def.armorType != ArmorType::None) {
            int targetArmorSlot = -1;
            if (def.armorType == ArmorType::Helmet) targetArmorSlot = SLOT_HELMET;
            else if (def.armorType == ArmorType::Chestplate) targetArmorSlot = SLOT_CHEST;
            else if (def.armorType == ArmorType::Leggings) targetArmorSlot = SLOT_LEGS;
            else if (def.armorType == ArmorType::Boots) targetArmorSlot = SLOT_BOOTS;

            if (targetArmorSlot != -1) {
                if (slots[targetArmorSlot].isEmpty()) {
                    slots[targetArmorSlot] = slot;
                    slot.clear();
                    return;
                } else {
                    std::swap(slots[targetArmorSlot], slot);
                    return;
                }
            }
        }

        // Move between hotbar (0..8) and main backpack (9..53)
        int targetStart = (slotIdx < HOTBAR_SIZE) ? HOTBAR_SIZE : 0;
        int targetEnd   = (slotIdx < HOTBAR_SIZE) ? MAIN_AND_HOTBAR : HOTBAR_SIZE;

        // Try stacking in unlocked slots
        for (int i = targetStart; i < targetEnd && slot.count > 0; ++i) {
            if (!isSlotUnlocked(i)) continue;
            if (slots[i].id == slot.id && slots[i].count < slots[i].maxStack) {
                uint32_t space = slots[i].maxStack - slots[i].count;
                uint32_t toAdd = std::min(slot.count, space);
                slots[i].count += toAdd;
                slot.count -= toAdd;
            }
        }
        // Try placing in empty unlocked slots
        if (slot.count > 0) {
            for (int i = targetStart; i < targetEnd; ++i) {
                if (!isSlotUnlocked(i)) continue;
                if (slots[i].isEmpty()) {
                    slots[i] = slot;
                    slot.clear();
                    break;
                }
            }
        }
        if (slot.count == 0) slot.clear();
        return;
    }

    // Right-Click behavior
    if (isRightClick) {
        if (cursorStack.isEmpty()) {
            if (!slot.isEmpty()) {
                uint32_t half = (slot.count + 1) / 2;
                cursorStack = { slot.id, half, slot.maxStack };
                slot.count -= half;
                if (slot.count == 0) slot.clear();
            }
        } else {
            // Place 1 item from cursor into slot
            if (canAcceptItem(slotIdx, cursorStack)) {
                if (slot.isEmpty()) {
                    slot = { cursorStack.id, 1, cursorStack.maxStack };
                    cursorStack.count--;
                    if (cursorStack.count == 0) cursorStack.clear();
                } else if (slot.id == cursorStack.id && slot.count < slot.maxStack) {
                    slot.count++;
                    cursorStack.count--;
                    if (cursorStack.count == 0) cursorStack.clear();
                }
            }
        }
        return;
    }

    // Left-Click behavior
    if (cursorStack.isEmpty()) {
        if (!slot.isEmpty()) {
            cursorStack = slot;
            slot.clear();
        }
    } else {
        if (!canAcceptItem(slotIdx, cursorStack)) return;

        if (slot.isEmpty()) {
            slot = cursorStack;
            cursorStack.clear();
        } else if (slot.id == cursorStack.id) {
            uint32_t space = slot.maxStack - slot.count;
            uint32_t toAdd = std::min(cursorStack.count, space);
            slot.count += toAdd;
            cursorStack.count -= toAdd;
            if (cursorStack.count == 0) cursorStack.clear();
        } else {
            // Check if cursor can accept current slot item before swapping
            if (isArmorSlot(slotIdx) && !canAcceptItem(slotIdx, cursorStack)) return;
            std::swap(slot, cursorStack);
        }
    }
}

} // namespace Aetheria
