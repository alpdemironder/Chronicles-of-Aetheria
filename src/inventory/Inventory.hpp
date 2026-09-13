#pragma once

#include "ItemStack.hpp"
#include "ItemRegistry.hpp"
#include <array>
#include <cstdint>

namespace Aetheria {

class Inventory {
public:
    static constexpr int HOTBAR_SIZE = 9;
    static constexpr int MAIN_ROWS = 5;
    static constexpr int MAIN_COLS = 9;
    static constexpr int MAIN_SIZE = MAIN_ROWS * MAIN_COLS; // 45
    static constexpr int MAIN_AND_HOTBAR = HOTBAR_SIZE + MAIN_SIZE; // 54

    // Dedicated Armor Slots
    static constexpr int SLOT_HELMET = 54;
    static constexpr int SLOT_CHEST = 55;
    static constexpr int SLOT_LEGS = 56;
    static constexpr int SLOT_BOOTS = 57;
    static constexpr int ARMOR_SLOTS = 4;

    static constexpr int TOTAL_SLOTS = MAIN_AND_HOTBAR + ARMOR_SLOTS; // 58

    Inventory();

    // Level-Based Storage Capacity
    bool isSlotUnlocked(int slotIdx) const {
        if (slotIdx < 0 || slotIdx >= TOTAL_SLOTS) return false;
        if (isArmorSlot(slotIdx)) return true;
        if (slotIdx < HOTBAR_SIZE) return true;
        return static_cast<uint32_t>(slotIdx) < unlockedSlots;
    }

    uint32_t getUnlockedSlotCount() const { return unlockedSlots; }
    void setUnlockedSlotCount(uint32_t count) {
        unlockedSlots = std::clamp(count, static_cast<uint32_t>(HOTBAR_SIZE), static_cast<uint32_t>(MAIN_AND_HOTBAR));
    }
    static int getRequiredLevelForSlot(int slotIdx) {
        if (slotIdx < 18) return 1;
        if (slotIdx >= MAIN_AND_HOTBAR) return 1;
        return 1 + (slotIdx - 18) / 3 + 1;
    }

    const ItemStack& getSlot(int index) const;
    ItemStack& getSlot(int index);
    void setSlot(int index, const ItemStack& stack);

    // Armor queries
    static bool isArmorSlot(int slotIdx) { return slotIdx >= SLOT_HELMET && slotIdx <= SLOT_BOOTS; }
    static ArmorType getSlotArmorType(int slotIdx);
    bool canAcceptItem(int slotIdx, const ItemStack& stack) const;
    float getTotalDefense() const;

    const ItemStack& getHelmet() const { return slots[SLOT_HELMET]; }
    const ItemStack& getChestplate() const { return slots[SLOT_CHEST]; }
    const ItemStack& getLeggings() const { return slots[SLOT_LEGS]; }
    const ItemStack& getBoots() const { return slots[SLOT_BOOTS]; }

    // Adds items to inventory; returns remaining count if inventory full
    uint32_t addItem(uint16_t id, uint32_t count = 1);

    // Removes items; returns true if successful, false if insufficient items
    bool removeItem(uint16_t id, uint32_t count = 1);

    // Queries total count across main & hotbar slots
    uint32_t countItem(uint16_t id) const;
    bool hasItem(uint16_t id, uint32_t count = 1) const { return countItem(id) >= count; }

    void swapSlots(int idxA, int idxB);

    // Mouse cursor drag & drop stack
    const ItemStack& getCursorStack() const { return cursorStack; }
    ItemStack& getCursorStack() { return cursorStack; }
    void setCursorStack(const ItemStack& stack) { cursorStack = stack; }

    // Unified click interaction
    void handleSlotClick(int slotIdx, bool isRightClick, bool isShiftClick);

private:
    std::array<ItemStack, TOTAL_SLOTS> slots;
    ItemStack cursorStack;
    uint32_t unlockedSlots = 18;
};

} // namespace Aetheria
