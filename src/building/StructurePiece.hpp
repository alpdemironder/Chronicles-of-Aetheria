#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include "../core/Math.hpp"

namespace Aetheria {

enum class StructureCategory : uint8_t {
    Foundations,
    WallsAndRoofs,
    Production,
    PalInfrastructure,
    StorageAndDefense,
    Furniture
};

enum class WoodPieceVariant : uint8_t {
    Stairs = 0,
    Trapdoor = 1,
    Slab = 2,
    SideSlab = 3,
    Chair = 4,
    Bench = 5,
    Table = 6,
    COUNT = 7
};

enum class StructureType : uint16_t {
    Foundation_Wood,
    Foundation_Stone,
    Foundation_Metal,
    Wall_Wood,
    Wall_Stone,
    Doorway_Wood,
    Roof_Wood,
    Stairs_Wood,
    Palbox_BaseCore,
    Workbench_Primitive,
    Workbench_Sphere,
    Smelting_Furnace,
    Pal_Bed,
    Feed_Box,
    Storage_Chest,
    Campfire,

    // --- 16 WOOD STAIRS (MERDIVEN) ---
    Stairs_Oak, Stairs_Birch, Stairs_Spruce, Stairs_Pine,
    Stairs_Jungle, Stairs_Acacia, Stairs_DarkOak, Stairs_Mangrove,
    Stairs_Cherry, Stairs_Maple, Stairs_Willow, Stairs_Fungal,
    Stairs_Petrified, Stairs_Bamboo, Stairs_Palm, Stairs_Astral,

    // --- 16 WOOD TRAPDOORS (TUZAK KAPISI) ---
    Trapdoor_Oak, Trapdoor_Birch, Trapdoor_Spruce, Trapdoor_Pine,
    Trapdoor_Jungle, Trapdoor_Acacia, Trapdoor_DarkOak, Trapdoor_Mangrove,
    Trapdoor_Cherry, Trapdoor_Maple, Trapdoor_Willow, Trapdoor_Fungal,
    Trapdoor_Petrified, Trapdoor_Bamboo, Trapdoor_Palm, Trapdoor_Astral,

    // --- 16 WOOD SLABS (BASAMAK) ---
    Slab_Oak, Slab_Birch, Slab_Spruce, Slab_Pine,
    Slab_Jungle, Slab_Acacia, Slab_DarkOak, Slab_Mangrove,
    Slab_Cherry, Slab_Maple, Slab_Willow, Slab_Fungal,
    Slab_Petrified, Slab_Bamboo, Slab_Palm, Slab_Astral,

    // --- 16 WOOD SIDE SLABS (YAN BASAMAK) ---
    SideSlab_Oak, SideSlab_Birch, SideSlab_Spruce, SideSlab_Pine,
    SideSlab_Jungle, SideSlab_Acacia, SideSlab_DarkOak, SideSlab_Mangrove,
    SideSlab_Cherry, SideSlab_Maple, SideSlab_Willow, SideSlab_Fungal,
    SideSlab_Petrified, SideSlab_Bamboo, SideSlab_Palm, SideSlab_Astral,

    // --- 16 WOOD CHAIRS (SANDALYE) ---
    Chair_Oak, Chair_Birch, Chair_Spruce, Chair_Pine,
    Chair_Jungle, Chair_Acacia, Chair_DarkOak, Chair_Mangrove,
    Chair_Cherry, Chair_Maple, Chair_Willow, Chair_Fungal,
    Chair_Petrified, Chair_Bamboo, Chair_Palm, Chair_Astral,

    // --- 16 WOOD BENCHES (BANK) ---
    Bench_Oak, Bench_Birch, Bench_Spruce, Bench_Pine,
    Bench_Jungle, Bench_Acacia, Bench_DarkOak, Bench_Mangrove,
    Bench_Cherry, Bench_Maple, Bench_Willow, Bench_Fungal,
    Bench_Petrified, Bench_Bamboo, Bench_Palm, Bench_Astral,

    // --- 16 WOOD TABLES (MASA) ---
    Table_Oak, Table_Birch, Table_Spruce, Table_Pine,
    Table_Jungle, Table_Acacia, Table_DarkOak, Table_Mangrove,
    Table_Cherry, Table_Maple, Table_Willow, Table_Fungal,
    Table_Petrified, Table_Bamboo, Table_Palm, Table_Astral,

    COUNT
};

struct MaterialCost {
    uint16_t blockOrItemId;
    uint16_t count;
};

struct StructureDef {
    StructureType type;
    std::string name;
    StructureCategory category;
    Vec3 size{2.0f, 2.0f, 2.0f};
    float buildWorkRequired = 5.0f; // Seconds to hammer
    float maxHealth = 100.0f;
    Vec4 baseColor{0.6f, 0.4f, 0.2f, 1.0f};
    std::vector<MaterialCost> costs;
};

struct PlacedStructure {
    uint32_t id = 0;
    StructureType type = StructureType::Foundation_Wood;
    Vec3 position{0, 0, 0};
    float rotationY = 0.0f; // Radians
    float buildProgress = 0.0f; // 0.0f to 1.0f
    bool isCompleted = false;
    float currentHealth = 100.0f;
    AABB getAABB() const;
};

class StructureRegistry {
public:
    static void init();
    static const StructureDef& get(StructureType type);
    static const std::vector<StructureDef>& getAll();

    static bool isWoodVariant(StructureType type, int& outVariant, int& outSpecies);
    static const char* getWoodSpeciesName(int speciesIdx);
    static const char* getWoodVariantName(int variantIdx);
    static uint16_t getWoodPlankBlockId(int speciesIdx);

private:
    static std::vector<StructureDef> defs;
    static bool initialized;
};

} // namespace Aetheria
