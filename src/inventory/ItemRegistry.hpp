#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include "../core/Math.hpp"

namespace Aetheria {

enum class ItemCategory : uint8_t {
    Block,
    Material,
    Weapon,
    Tool,
    Armor,
    Consumable
};

enum class ArmorType : uint8_t {
    None = 0,
    Helmet,
    Chestplate,
    Leggings,
    Boots
};

enum class ToolType : uint8_t {
    None = 0,
    Sword,
    Pickaxe,
    Axe,
    Shovel,
    Bow
};

struct ItemDef {
    uint16_t id = 0;
    std::string name = "Empty";
    ItemCategory category = ItemCategory::Material;
    std::string description = "";
    Vec4 color{0.5f, 0.5f, 0.5f, 1.0f};
    uint32_t maxStack = 64;
    float attackDamage = 12.0f;
    float healAmount = 0.0f;
    float manaAmount = 0.0f;
    float staminaAmount = 0.0f;
    bool isPlaceableBlock = false;

    // Armor & Tool Specifics
    ArmorType armorType = ArmorType::None;
    float armorDefense = 0.0f;
    ToolType toolType = ToolType::None;
    int toolTier = 0; // 1=Wood, 2=Stone, 3=Iron, 4=Gold, 5=Diamond, 6=Aetherium
    float attackSpeed = 4.0f; // Attacks per second (e.g. 1.6 for sword, 0.9 for axe, 4.0 for hand)
};

class ItemRegistry {
public:
    static void init();
    static const ItemDef& get(uint16_t id);
    static const std::vector<ItemDef>& getAllCustomItems();

private:
    static void registerItem(const ItemDef& def);
    static std::vector<ItemDef> customItems;
    static bool initialized;
};

} // namespace Aetheria
