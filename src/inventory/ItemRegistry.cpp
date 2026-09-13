#include "ItemRegistry.hpp"
#include "../world/BlockRegistry.hpp"
#include <unordered_map>
#include <iostream>

namespace Aetheria {

std::vector<ItemDef> ItemRegistry::customItems;
bool ItemRegistry::initialized = false;
static std::unordered_map<uint16_t, ItemDef> customItemMap;
static ItemDef emptyItemDef{0, "Empty", ItemCategory::Material, "Empty slot", {0.2f, 0.2f, 0.2f, 0.0f}, 64, 0, 0, 0, 0, false, ArmorType::None, 0.0f, ToolType::None, 0, 4.0f};

void ItemRegistry::registerItem(const ItemDef& inDef) {
    ItemDef def = inDef;
    // Configure Minecraft 1.9+ attack speeds
    if (def.category == ItemCategory::Weapon) {
        if (def.toolType == ToolType::Sword) {
            def.attackSpeed = 1.6f; // Swift military & practice swords
        } else if (def.toolType == ToolType::Bow) {
            def.attackSpeed = 1.0f;
        }
    } else if (def.category == ItemCategory::Tool) {
        if (def.toolType == ToolType::Axe) {
            def.attackSpeed = 0.9f; // Heavy, slower crushing strikes
        } else if (def.toolType == ToolType::Pickaxe) {
            def.attackSpeed = 1.2f; // Utility strikes
        } else if (def.toolType == ToolType::Shovel) {
            def.attackSpeed = 1.0f; // Earth strikes
        }
    }

    customItems.push_back(def);
    customItemMap[def.id] = def;
}

const ItemDef& ItemRegistry::get(uint16_t id) {
    if (!initialized) init();

    if (id == 0) return emptyItemDef;

    // 1. Check if it's one of the registered voxel blocks (1..369)
    if (id >= 1 && id <= 369) {
        static thread_local ItemDef blockItemCache;
        const BlockDef& bDef = BlockRegistry::get(id);
        blockItemCache.id = bDef.id;
        blockItemCache.name = bDef.name;
        blockItemCache.category = ItemCategory::Block;
        blockItemCache.description = "Voxel Block: " + bDef.name + " (#" + std::to_string(bDef.id) + "). Right-click to place into terrain.";
        blockItemCache.color = bDef.color;
        blockItemCache.maxStack = 64;
        blockItemCache.attackDamage = 10.0f;
        blockItemCache.healAmount = 0.0f;
        blockItemCache.manaAmount = 0.0f;
        blockItemCache.staminaAmount = 0.0f;
        blockItemCache.isPlaceableBlock = true;
        blockItemCache.armorType = ArmorType::None;
        blockItemCache.armorDefense = 0.0f;
        blockItemCache.toolType = ToolType::None;
        blockItemCache.toolTier = 0;
        blockItemCache.attackSpeed = 4.0f;
        return blockItemCache;
    }

    // 2. Check custom RPG items / tools / materials
    auto it = customItemMap.find(id);
    if (it != customItemMap.end()) {
        return it->second;
    }

    return emptyItemDef;
}

const std::vector<ItemDef>& ItemRegistry::getAllCustomItems() {
    if (!initialized) init();
    return customItems;
}

void ItemRegistry::init() {
    if (initialized) return;
    initialized = true;

    customItems.clear();
    customItemMap.clear();

    // ----------------------------------------------------
    // 1. RAW MATERIALS & REFINED INGOTS
    // ----------------------------------------------------
    registerItem({
        501, "Iron Ingot", ItemCategory::Material,
        "Smelted refined iron metal ingot. Fundamental for forging tools, armor, and structures.",
        {0.78f, 0.82f, 0.88f, 1.0f}, 64, 12.0f, 0, 0, 0, false, ArmorType::None, 0.0f, ToolType::None, 0
    });

    registerItem({
        502, "Copper Ingot", ItemCategory::Material,
        "Conductive reddish-bronze copper ingot. Used in mechanical contraptions and alloys.",
        {0.88f, 0.55f, 0.35f, 1.0f}, 64, 12.0f, 0, 0, 0, false, ArmorType::None, 0.0f, ToolType::None, 0
    });

    registerItem({
        503, "Gold Ingot", ItemCategory::Material,
        "Gleaming refined precious auric ingot. Required for high-tier magic and electronics.",
        {1.0f, 0.85f, 0.22f, 1.0f}, 64, 12.0f, 0, 0, 0, false, ArmorType::None, 0.0f, ToolType::None, 0
    });

    registerItem({
        504, "Plant Fiber Rope", ItemCategory::Material,
        "Braided durable fiber twine. Used for bindings, tools, bows, and scaffolds.",
        {0.55f, 0.65f, 0.35f, 1.0f}, 64, 12.0f, 0, 0, 0, false, ArmorType::None, 0.0f, ToolType::None, 0
    });

    registerItem({
        505, "Animal Pelt", ItemCategory::Material,
        "Soft warm fur pelt harvested from wilderness wildlife. Used for beds and leather.",
        {0.62f, 0.44f, 0.28f, 1.0f}, 64, 12.0f, 0, 0, 0, false, ArmorType::None, 0.0f, ToolType::None, 0
    });

    registerItem({
        506, "Tanned Leather", ItemCategory::Material,
        "Cured tough leather hide. Essential for crafting leather armor.",
        {0.50f, 0.30f, 0.15f, 1.0f}, 64, 12.0f, 0, 0, 0, false, ArmorType::None, 0.0f, ToolType::None, 0
    });

    registerItem({
        507, "Golem Core", ItemCategory::Material,
        "Pulsing ancient mechanical power core harvested from Titan Golems.",
        {0.20f, 0.85f, 0.95f, 1.0f}, 16, 12.0f, 0, 0, 0, false, ArmorType::None, 0.0f, ToolType::None, 0
    });

    registerItem({
        508, "Raw Meat", ItemCategory::Material,
        "Fresh game meat from wild beasts. Cook in a Furnace or campfire to make a Feast.",
        {0.85f, 0.28f, 0.28f, 1.0f}, 32, 12.0f, 0, 0, 0, false, ArmorType::None, 0.0f, ToolType::None, 0
    });

    registerItem({
        509, "Bone Shards", ItemCategory::Material,
        "Hard calcified skeletal fragments dropped by undead and scavengers.",
        {0.92f, 0.92f, 0.88f, 1.0f}, 64, 12.0f, 0, 0, 0, false, ArmorType::None, 0.0f, ToolType::None, 0
    });

    registerItem({
        524, "Ancient Technology", ItemCategory::Material,
        "Rare relic micro-components found in boss domains. Unlocks Aetherium gear.",
        {0.85f, 0.35f, 0.95f, 1.0f}, 32, 12.0f, 0, 0, 0, false, ArmorType::None, 0.0f, ToolType::None, 0
    });

    registerItem({
        550, "Wooden Stick", ItemCategory::Material,
        "Whittled wooden handle rod. Essential handle component for all tools and torches.",
        {0.55f, 0.40f, 0.25f, 1.0f}, 64, 8.0f, 0, 0, 0, false, ArmorType::None, 0.0f, ToolType::None, 0
    });

    registerItem({
        551, "Torch", ItemCategory::Material,
        "Bright burning wooden torch. Lights the path in deep caverns and night.",
        {1.0f, 0.75f, 0.2f, 1.0f}, 64, 8.0f, 0, 0, 0, false, ArmorType::None, 0.0f, ToolType::None, 0
    });

    registerItem({
        552, "Coal", ItemCategory::Material,
        "Combustible black mineral lump. Burns for 80 seconds in a Furnace.",
        {0.18f, 0.18f, 0.20f, 1.0f}, 64, 8.0f, 0, 0, 0, false, ArmorType::None, 0.0f, ToolType::None, 0
    });

    registerItem({
        553, "Charcoal", ItemCategory::Material,
        "Smelted wood carbon fuel. Identical burning properties to geological coal.",
        {0.25f, 0.22f, 0.20f, 1.0f}, 64, 8.0f, 0, 0, 0, false, ArmorType::None, 0.0f, ToolType::None, 0
    });

    registerItem({
        554, "Diamond Gem", ItemCategory::Material,
        "Pure glittering brilliant-cut diamond. Forges highest tier masterwork equipment.",
        {0.35f, 0.95f, 1.0f, 1.0f}, 64, 15.0f, 0, 0, 0, false, ArmorType::None, 0.0f, ToolType::None, 0
    });

    registerItem({
        555, "Emerald Gem", ItemCategory::Material,
        "Lustrous deep forest green emerald. Prized currency and jewelcrafting component.",
        {0.15f, 0.95f, 0.45f, 1.0f}, 64, 12.0f, 0, 0, 0, false, ArmorType::None, 0.0f, ToolType::None, 0
    });

    registerItem({
        556, "Silk String", ItemCategory::Material,
        "Strong spiderweb thread. Used to string hunting bows and weave fishing lines.",
        {0.90f, 0.90f, 0.92f, 1.0f}, 64, 5.0f, 0, 0, 0, false, ArmorType::None, 0.0f, ToolType::None, 0
    });

    registerItem({
        557, "Feather", ItemCategory::Material,
        "Light buoyant avian pinion. Essential for arrow fletching and delicate craft.",
        {0.95f, 0.95f, 0.95f, 1.0f}, 64, 4.0f, 0, 0, 0, false, ArmorType::None, 0.0f, ToolType::None, 0
    });

    registerItem({
        558, "Flint Shard", ItemCategory::Material,
        "Conchoidally fractured hard flint. Arrowheads and fire starters.",
        {0.35f, 0.35f, 0.38f, 1.0f}, 64, 10.0f, 0, 0, 0, false, ArmorType::None, 0.0f, ToolType::None, 0
    });

    registerItem({
        559, "Wheat Sheaf", ItemCategory::Material,
        "Golden ripe agricultural stalk. Mill 3 sheaves into nutritious fresh Bread.",
        {0.88f, 0.78f, 0.35f, 1.0f}, 64, 5.0f, 0, 0, 0, false, ArmorType::None, 0.0f, ToolType::None, 0
    });

    registerItem({
        560, "Baked Bread", ItemCategory::Consumable,
        "Warm golden crust loaf. Right-click to restore 25 Health and 40 Stamina.",
        {0.78f, 0.55f, 0.25f, 1.0f}, 32, 5.0f, 25.0f, 0, 40.0f, false, ArmorType::None, 0.0f, ToolType::None, 0
    });

    registerItem({
        561, "Red Apple", ItemCategory::Consumable,
        "Crisp juicy wild orchard fruit. Right-click to restore 15 Health.",
        {0.92f, 0.18f, 0.18f, 1.0f}, 32, 5.0f, 15.0f, 0, 20.0f, false, ArmorType::None, 0.0f, ToolType::None, 0
    });

    registerItem({
        562, "Golden Apple", ItemCategory::Consumable,
        "Enchanted apple wrapped in pure gold. Restores 100 Health and 100 Mana.",
        {1.0f, 0.85f, 0.15f, 1.0f}, 16, 10.0f, 100.0f, 100.0f, 50.0f, false, ArmorType::None, 0.0f, ToolType::None, 0
    });

    registerItem({
        563, "Iron Bucket", ItemCategory::Tool,
        "Forged iron liquid canister for transporting fluids and molten lava.",
        {0.75f, 0.78f, 0.82f, 1.0f}, 16, 10.0f, 0, 0, 0, false, ArmorType::None, 0.0f, ToolType::None, 0
    });

    // ----------------------------------------------------
    // 2. WEAPONS (ALL TIERS)
    // ----------------------------------------------------
    registerItem({
        510, "Wooden Sword", ItemCategory::Weapon,
        "Carved oak practice blade. Basic self-defense. [Attack: 18]",
        {0.58f, 0.40f, 0.22f, 1.0f}, 1, 18.0f, 0, 0, 0, false, ArmorType::None, 0.0f, ToolType::Sword, 1
    });

    registerItem({
        570, "Stone Sword", ItemCategory::Weapon,
        "Chiseled cobblestone broadblade. Sturdy starting weapon. [Attack: 26]",
        {0.62f, 0.62f, 0.65f, 1.0f}, 1, 26.0f, 0, 0, 0, false, ArmorType::None, 0.0f, ToolType::Sword, 2
    });

    registerItem({
        513, "Iron Broadsword", ItemCategory::Weapon,
        "Keen forged steel broadsword. Reliable military blade. [Attack: 38]",
        {0.85f, 0.88f, 0.95f, 1.0f}, 1, 38.0f, 0, 0, 0, false, ArmorType::None, 0.0f, ToolType::Sword, 3
    });

    registerItem({
        571, "Golden Sword", ItemCategory::Weapon,
        "Enchanted gilded ceremonial rapier. Fast and graceful. [Attack: 32]",
        {1.0f, 0.85f, 0.20f, 1.0f}, 1, 32.0f, 0, 0, 0, false, ArmorType::None, 0.0f, ToolType::Sword, 4
    });

    registerItem({
        572, "Diamond Sword", ItemCategory::Weapon,
        "Razor-sharp crystalline diamond blade. Lethal critical strikes. [Attack: 52]",
        {0.35f, 0.95f, 1.0f, 1.0f}, 1, 52.0f, 0, 0, 0, false, ArmorType::None, 0.0f, ToolType::Sword, 5
    });

    registerItem({
        517, "Aetherium Greatsword", ItemCategory::Weapon,
        "Mythical prismatic greatsword forged with Ancient Tech. [Attack: 70]",
        {1.0f, 0.35f, 0.95f, 1.0f}, 1, 70.0f, 0, 0, 0, false, ArmorType::None, 0.0f, ToolType::Sword, 6
    });

    registerItem({
        515, "Hunter Bow", ItemCategory::Weapon,
        "Flexible recurve bow for long range combat. [Attack: 32]",
        {0.65f, 0.45f, 0.25f, 1.0f}, 1, 32.0f, 0, 0, 0, false, ArmorType::None, 0.0f, ToolType::Bow, 2
    });

    registerItem({
        516, "Wooden Arrows", ItemCategory::Weapon,
        "Fletched arrows with flint points. Bow ammunition.",
        {0.70f, 0.60f, 0.40f, 1.0f}, 128, 15.0f, 0, 0, 0, false, ArmorType::None, 0.0f, ToolType::None, 0
    });

    // ----------------------------------------------------
    // 3. TOOLS (PICKAXES, AXES, SHOVELS)
    // ----------------------------------------------------
    registerItem({
        573, "Wooden Pickaxe", ItemCategory::Tool,
        "Primitive wooden pickaxe. Breaks soft stone strata. [Tier: 1, Attack: 12]",
        {0.55f, 0.38f, 0.20f, 1.0f}, 1, 12.0f, 0, 0, 0, false, ArmorType::None, 0.0f, ToolType::Pickaxe, 1
    });

    registerItem({
        512, "Stone Pickaxe", ItemCategory::Tool,
        "Chiseled stone pickaxe. Excavates iron and copper veins. [Tier: 2, Attack: 16]",
        {0.55f, 0.55f, 0.58f, 1.0f}, 1, 16.0f, 0, 0, 0, false, ArmorType::None, 0.0f, ToolType::Pickaxe, 2
    });

    registerItem({
        514, "Iron Pickaxe", ItemCategory::Tool,
        "Hardened steel pickaxe. Mines diamonds and gold veins. [Tier: 3, Attack: 24]",
        {0.80f, 0.82f, 0.90f, 1.0f}, 1, 24.0f, 0, 0, 0, false, ArmorType::None, 0.0f, ToolType::Pickaxe, 3
    });

    registerItem({
        574, "Golden Pickaxe", ItemCategory::Tool,
        "Lightweight auric pickaxe with rapid mining speed. [Tier: 4, Attack: 20]",
        {1.0f, 0.85f, 0.25f, 1.0f}, 1, 20.0f, 0, 0, 0, false, ArmorType::None, 0.0f, ToolType::Pickaxe, 4
    });

    registerItem({
        575, "Diamond Pickaxe", ItemCategory::Tool,
        "Indestructible diamond pickaxe. Fractures obsidian bedrock. [Tier: 5, Attack: 36]",
        {0.35f, 0.95f, 1.0f, 1.0f}, 1, 36.0f, 0, 0, 0, false, ArmorType::None, 0.0f, ToolType::Pickaxe, 5
    });

    registerItem({
        576, "Aetherium Pickaxe", ItemCategory::Tool,
        "Cosmic resonating pickaxe. Cleaves all terrain instantly. [Tier: 6, Attack: 50]",
        {0.95f, 0.45f, 1.0f, 1.0f}, 1, 50.0f, 0, 0, 0, false, ArmorType::None, 0.0f, ToolType::Pickaxe, 6
    });

    registerItem({
        577, "Wooden Axe", ItemCategory::Tool,
        "Basic timber axe for felling trees. [Attack: 16]",
        {0.55f, 0.38f, 0.20f, 1.0f}, 1, 16.0f, 0, 0, 0, false, ArmorType::None, 0.0f, ToolType::Axe, 1
    });

    registerItem({
        511, "Stone Axe", ItemCategory::Tool,
        "Flint hatchet. Chops logs efficiently. [Attack: 22]",
        {0.60f, 0.60f, 0.62f, 1.0f}, 1, 22.0f, 0, 0, 0, false, ArmorType::None, 0.0f, ToolType::Axe, 2
    });

    registerItem({
        578, "Iron Axe", ItemCategory::Tool,
        "Heavy steel woodchopper axe. [Attack: 32]",
        {0.80f, 0.82f, 0.90f, 1.0f}, 1, 32.0f, 0, 0, 0, false, ArmorType::None, 0.0f, ToolType::Axe, 3
    });

    registerItem({
        579, "Diamond Axe", ItemCategory::Tool,
        "Diamond battle axe. Slices timber and armor. [Attack: 44]",
        {0.35f, 0.95f, 1.0f, 1.0f}, 1, 44.0f, 0, 0, 0, false, ArmorType::None, 0.0f, ToolType::Axe, 5
    });

    registerItem({
        580, "Wooden Shovel", ItemCategory::Tool,
        "Oak spade for digging soil, gravel, and sand. [Attack: 10]",
        {0.55f, 0.38f, 0.20f, 1.0f}, 1, 10.0f, 0, 0, 0, false, ArmorType::None, 0.0f, ToolType::Shovel, 1
    });

    registerItem({
        581, "Stone Shovel", ItemCategory::Tool,
        "Cobblestone spade for earth excavation. [Attack: 14]",
        {0.60f, 0.60f, 0.62f, 1.0f}, 1, 14.0f, 0, 0, 0, false, ArmorType::None, 0.0f, ToolType::Shovel, 2
    });

    registerItem({
        582, "Iron Shovel", ItemCategory::Tool,
        "Forged steel trench shovel. [Attack: 20]",
        {0.80f, 0.82f, 0.90f, 1.0f}, 1, 20.0f, 0, 0, 0, false, ArmorType::None, 0.0f, ToolType::Shovel, 3
    });

    registerItem({
        583, "Diamond Shovel", ItemCategory::Tool,
        "Diamond excavation spade. Clears earth in seconds. [Attack: 28]",
        {0.35f, 0.95f, 1.0f, 1.0f}, 1, 28.0f, 0, 0, 0, false, ArmorType::None, 0.0f, ToolType::Shovel, 5
    });

    // ----------------------------------------------------
    // 4. ARMOR SETS (HELMET, CHEST, LEGS, BOOTS)
    // ----------------------------------------------------
    // Leather Armor (Total Defense: 7)
    registerItem({
        530, "Leather Cap", ItemCategory::Armor,
        "Soft cured leather helm. Light protection against wild beasts. [Defense: +1]",
        {0.52f, 0.32f, 0.18f, 1.0f}, 1, 0, 0, 0, 0, false, ArmorType::Helmet, 1.0f, ToolType::None, 0
    });
    registerItem({
        531, "Leather Tunic", ItemCategory::Armor,
        "Stitched animal hide vest. [Defense: +3]",
        {0.52f, 0.32f, 0.18f, 1.0f}, 1, 0, 0, 0, 0, false, ArmorType::Chestplate, 3.0f, ToolType::None, 0
    });
    registerItem({
        532, "Leather Pants", ItemCategory::Armor,
        "Flexible buckskin trousers. [Defense: +2]",
        {0.52f, 0.32f, 0.18f, 1.0f}, 1, 0, 0, 0, 0, false, ArmorType::Leggings, 2.0f, ToolType::None, 0
    });
    registerItem({
        533, "Leather Boots", ItemCategory::Armor,
        "Warm leather riding boots. [Defense: +1]",
        {0.52f, 0.32f, 0.18f, 1.0f}, 1, 0, 0, 0, 0, false, ArmorType::Boots, 1.0f, ToolType::None, 0
    });

    // Iron Armor (Total Defense: 16)
    registerItem({
        534, "Iron Helmet", ItemCategory::Armor,
        "Heavy riveted steel armet with eye visor. [Defense: +3]",
        {0.78f, 0.82f, 0.88f, 1.0f}, 1, 0, 0, 0, 0, false, ArmorType::Helmet, 3.0f, ToolType::None, 0
    });
    registerItem({
        535, "Iron Chestplate", ItemCategory::Armor,
        "Solid steel breastplate. Guards heart against mortal blows. [Defense: +6]",
        {0.78f, 0.82f, 0.88f, 1.0f}, 1, 0, 0, 0, 0, false, ArmorType::Chestplate, 6.0f, ToolType::None, 0
    });
    registerItem({
        536, "Iron Leggings", ItemCategory::Armor,
        "Articulated steel greaves and culet. [Defense: +5]",
        {0.78f, 0.82f, 0.88f, 1.0f}, 1, 0, 0, 0, 0, false, ArmorType::Leggings, 5.0f, ToolType::None, 0
    });
    registerItem({
        537, "Iron Boots", ItemCategory::Armor,
        "Plated iron sabatons. [Defense: +2]",
        {0.78f, 0.82f, 0.88f, 1.0f}, 1, 0, 0, 0, 0, false, ArmorType::Boots, 2.0f, ToolType::None, 0
    });

    // Gold Armor (Total Defense: 11)
    registerItem({
        538, "Golden Helmet", ItemCategory::Armor,
        "Gleaming gilded coronet imbued with magic protection. [Defense: +2]",
        {1.0f, 0.85f, 0.22f, 1.0f}, 1, 0, 0, 0, 0, false, ArmorType::Helmet, 2.0f, ToolType::None, 0
    });
    registerItem({
        539, "Golden Chestplate", ItemCategory::Armor,
        "Ornate auric breastplate inlaid with sun motifs. [Defense: +5]",
        {1.0f, 0.85f, 0.22f, 1.0f}, 1, 0, 0, 0, 0, false, ArmorType::Chestplate, 5.0f, ToolType::None, 0
    });
    registerItem({
        540, "Golden Leggings", ItemCategory::Armor,
        "Gilded chausses of radiant beauty. [Defense: +3]",
        {1.0f, 0.85f, 0.22f, 1.0f}, 1, 0, 0, 0, 0, false, ArmorType::Leggings, 3.0f, ToolType::None, 0
    });
    registerItem({
        541, "Golden Boots", ItemCategory::Armor,
        "Polished auric sabatons. [Defense: +1]",
        {1.0f, 0.85f, 0.22f, 1.0f}, 1, 0, 0, 0, 0, false, ArmorType::Boots, 1.0f, ToolType::None, 0
    });

    // Diamond Armor (Total Defense: 21)
    registerItem({
        542, "Diamond Helmet", ItemCategory::Armor,
        "Masterwork helm forged from cut diamond facets. [Defense: +4]",
        {0.35f, 0.95f, 1.0f, 1.0f}, 1, 0, 0, 0, 0, false, ArmorType::Helmet, 4.0f, ToolType::None, 0
    });
    registerItem({
        543, "Diamond Chestplate", ItemCategory::Armor,
        "Impenetrable crystalline cuirass reflecting hostile blows. [Defense: +8]",
        {0.35f, 0.95f, 1.0f, 1.0f}, 1, 0, 0, 0, 0, false, ArmorType::Chestplate, 8.0f, ToolType::None, 0
    });
    registerItem({
        544, "Diamond Leggings", ItemCategory::Armor,
        "Faceted diamond plate greaves. [Defense: +6]",
        {0.35f, 0.95f, 1.0f, 1.0f}, 1, 0, 0, 0, 0, false, ArmorType::Leggings, 6.0f, ToolType::None, 0
    });
    registerItem({
        545, "Diamond Boots", ItemCategory::Armor,
        "Diamond-encrusted boots granting unwavering stance. [Defense: +3]",
        {0.35f, 0.95f, 1.0f, 1.0f}, 1, 0, 0, 0, 0, false, ArmorType::Boots, 3.0f, ToolType::None, 0
    });

    // Aetherium Armor (Total Defense: 26)
    registerItem({
        546, "Aetherium Crown", ItemCategory::Armor,
        "Cosmic relic circlet crackling with astral energy. [Defense: +5]",
        {0.95f, 0.45f, 1.0f, 1.0f}, 1, 0, 0, 0, 0, false, ArmorType::Helmet, 5.0f, ToolType::None, 0
    });
    registerItem({
        547, "Aetherium Cuirass", ItemCategory::Armor,
        "Celestial battlegarb forged with Ancient Tech. [Defense: +10]",
        {0.95f, 0.45f, 1.0f, 1.0f}, 1, 0, 0, 0, 0, false, ArmorType::Chestplate, 10.0f, ToolType::None, 0
    });
    registerItem({
        548, "Aetherium Greaves", ItemCategory::Armor,
        "Astral leg armor deflecting catastrophic attacks. [Defense: +7]",
        {0.95f, 0.45f, 1.0f, 1.0f}, 1, 0, 0, 0, 0, false, ArmorType::Leggings, 7.0f, ToolType::None, 0
    });
    registerItem({
        549, "Aetherium Boots", ItemCategory::Armor,
        "Floating relic boots granting stride upon starlight. [Defense: +4]",
        {0.95f, 0.45f, 1.0f, 1.0f}, 1, 0, 0, 0, 0, false, ArmorType::Boots, 4.0f, ToolType::None, 0
    });

    // ----------------------------------------------------
    // 5. CONSUMABLES & PAL TECH
    // ----------------------------------------------------
    registerItem({
        520, "Health Potion", ItemCategory::Consumable,
        "Brewed herbal elixir. Right-click to restore 60 Health instantly.",
        {0.95f, 0.18f, 0.22f, 1.0f}, 16, 0, 60.0f, 0, 0, false, ArmorType::None, 0.0f, ToolType::None, 0
    });

    registerItem({
        521, "Mana Elixir", ItemCategory::Consumable,
        "Infused celestial sapphire tincture. Right-click to restore 60 Mana instantly.",
        {0.20f, 0.50f, 1.0f, 1.0f}, 16, 0, 0, 60.0f, 0, false, ArmorType::None, 0.0f, ToolType::None, 0
    });

    registerItem({
        522, "Cooked Feast", ItemCategory::Consumable,
        "Savory fire-roasted steak feast. Restores 40 Health and 100 Stamina.",
        {0.75f, 0.45f, 0.20f, 1.0f}, 16, 0, 40.0f, 0, 100.0f, false, ArmorType::None, 0.0f, ToolType::None, 0
    });

    registerItem({
        523, "Pal Sphere", ItemCategory::Consumable,
        "Technological capture sphere. Right-click to throw with arc physics to capture creatures (1.0x power)!",
        {0.20f, 0.85f, 0.95f, 1.0f}, 16, 0, 0, 0, 0, false, ArmorType::None, 0.0f, ToolType::None, 0
    });

    registerItem({
        526, "Mega Sphere", ItemCategory::Consumable,
        "High-grade capture sphere with enhanced magnetic containment (2.0x power). Tames resilient beasts!",
        {0.12f, 0.88f, 0.65f, 1.0f}, 16, 0, 0, 0, 0, false, ArmorType::None, 0.0f, ToolType::None, 0
    });

    registerItem({
        527, "Giga Sphere", ItemCategory::Consumable,
        "Masterwork celestial capture sphere (3.5x power). High capture rate capable of taming World Bosses!",
        {0.72f, 0.20f, 0.92f, 1.0f}, 16, 0, 0, 0, 0, false, ArmorType::None, 0.0f, ToolType::None, 0
    });

    registerItem({
        525, "Bone Meal", ItemCategory::Material,
        "Crushed organic bone fertilizer. Right-click planted saplings to instantly mature them into full trees!",
        {0.92f, 0.92f, 0.88f, 1.0f}, 64, 4.0f, 0, 0, 0, false, ArmorType::None, 0.0f, ToolType::None, 0
    });

    std::cout << "ItemRegistry initialized with " << customItems.size() << " custom RPG items + 369 blocks!" << std::endl;
}

} // namespace Aetheria
