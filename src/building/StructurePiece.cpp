#include "StructurePiece.hpp"
#include <iostream>

namespace Aetheria {

std::vector<StructureDef> StructureRegistry::defs;
bool StructureRegistry::initialized = false;

AABB PlacedStructure::getAABB() const {
    const auto& def = StructureRegistry::get(type);
    Vec3 halfSize = def.size * 0.5f;
    return AABB(position - halfSize, position + halfSize);
}

const StructureDef& StructureRegistry::get(StructureType type) {
    if (!initialized) init();
    size_t idx = static_cast<size_t>(type);
    if (idx < defs.size()) {
        return defs[idx];
    }
    return defs[0];
}

const std::vector<StructureDef>& StructureRegistry::getAll() {
    if (!initialized) init();
    return defs;
}

void StructureRegistry::init() {
    if (initialized) return;
    initialized = true;

    defs.clear();
    defs.resize(static_cast<size_t>(StructureType::COUNT));

    defs[static_cast<size_t>(StructureType::Foundation_Wood)] = {
        StructureType::Foundation_Wood, "Wooden Foundation", StructureCategory::Foundations,
        {2.0f, 0.4f, 2.0f}, 4.0f, 150.0f, {0.60f, 0.42f, 0.25f, 1.0f},
        { {71, 4} } // 4 Oak Logs
    };

    defs[static_cast<size_t>(StructureType::Foundation_Stone)] = {
        StructureType::Foundation_Stone, "Stone Foundation", StructureCategory::Foundations,
        {2.0f, 0.4f, 2.0f}, 6.0f, 300.0f, {0.55f, 0.55f, 0.58f, 1.0f},
        { {36, 6} } // 6 Stone
    };

    defs[static_cast<size_t>(StructureType::Foundation_Metal)] = {
        StructureType::Foundation_Metal, "Metal Foundation", StructureCategory::Foundations,
        {2.0f, 0.4f, 2.0f}, 8.0f, 600.0f, {0.70f, 0.72f, 0.78f, 1.0f},
        { {144, 4} } // 4 Iron Blocks
    };

    defs[static_cast<size_t>(StructureType::Wall_Wood)] = {
        StructureType::Wall_Wood, "Wooden Wall", StructureCategory::WallsAndRoofs,
        {2.0f, 2.0f, 0.2f}, 4.0f, 120.0f, {0.58f, 0.40f, 0.24f, 1.0f},
        { {72, 4} } // 4 Planks
    };

    defs[static_cast<size_t>(StructureType::Wall_Stone)] = {
        StructureType::Wall_Stone, "Stone Wall", StructureCategory::WallsAndRoofs,
        {2.0f, 2.0f, 0.2f}, 6.0f, 250.0f, {0.52f, 0.52f, 0.54f, 1.0f},
        { {37, 4} } // 4 Cobblestone
    };

    defs[static_cast<size_t>(StructureType::Doorway_Wood)] = {
        StructureType::Doorway_Wood, "Wooden Doorway", StructureCategory::WallsAndRoofs,
        {2.0f, 2.0f, 0.2f}, 5.0f, 120.0f, {0.54f, 0.38f, 0.22f, 1.0f},
        { {72, 5} }
    };

    defs[static_cast<size_t>(StructureType::Roof_Wood)] = {
        StructureType::Roof_Wood, "Sloped Wooden Roof", StructureCategory::WallsAndRoofs,
        {2.0f, 0.4f, 2.0f}, 4.0f, 100.0f, {0.50f, 0.35f, 0.20f, 1.0f},
        { {72, 3} }
    };

    defs[static_cast<size_t>(StructureType::Stairs_Wood)] = {
        StructureType::Stairs_Wood, "Wooden Stairs", StructureCategory::WallsAndRoofs,
        {2.0f, 2.0f, 2.0f}, 4.0f, 100.0f, {0.62f, 0.45f, 0.28f, 1.0f},
        { {72, 4} }
    };

    defs[static_cast<size_t>(StructureType::Palbox_BaseCore)] = {
        StructureType::Palbox_BaseCore, "Palbox Base Camp Core", StructureCategory::PalInfrastructure,
        {1.6f, 2.2f, 1.6f}, 10.0f, 1000.0f, {0.20f, 0.55f, 0.95f, 1.0f},
        { {71, 8}, {36, 12}, {186, 2} } // Logs, Stone, and Flame Crystal
    };

    defs[static_cast<size_t>(StructureType::Workbench_Primitive)] = {
        StructureType::Workbench_Primitive, "Primitive Workbench", StructureCategory::Production,
        {1.5f, 1.0f, 1.0f}, 3.0f, 100.0f, {0.62f, 0.45f, 0.28f, 1.0f},
        { {72, 4} }
    };

    defs[static_cast<size_t>(StructureType::Workbench_Sphere)] = {
        StructureType::Workbench_Sphere, "Sphere Taming Workbench", StructureCategory::Production,
        {1.8f, 1.2f, 1.2f}, 6.0f, 200.0f, {0.35f, 0.65f, 0.75f, 1.0f},
        { {71, 6}, {36, 8}, {192, 1} }
    };

    defs[static_cast<size_t>(StructureType::Smelting_Furnace)] = {
        StructureType::Smelting_Furnace, "Smelting Furnace", StructureCategory::Production,
        {1.4f, 1.6f, 1.4f}, 5.0f, 250.0f, {0.45f, 0.45f, 0.45f, 1.0f},
        { {37, 10}, {186, 1} }
    };

    defs[static_cast<size_t>(StructureType::Pal_Bed)] = {
        StructureType::Pal_Bed, "Straw Creature Bed", StructureCategory::PalInfrastructure,
        {2.2f, 0.5f, 1.6f}, 3.0f, 80.0f, {0.85f, 0.75f, 0.45f, 1.0f},
        { {72, 4} }
    };

    defs[static_cast<size_t>(StructureType::Feed_Box)] = {
        StructureType::Feed_Box, "Creature Feed Box", StructureCategory::PalInfrastructure,
        {1.2f, 0.8f, 1.2f}, 3.0f, 120.0f, {0.65f, 0.48f, 0.28f, 1.0f},
        { {72, 3} }
    };

    defs[static_cast<size_t>(StructureType::Storage_Chest)] = {
        StructureType::Storage_Chest, "Wooden Storage Chest", StructureCategory::StorageAndDefense,
        {1.0f, 1.0f, 1.0f}, 2.0f, 150.0f, {0.55f, 0.40f, 0.22f, 1.0f},
        { {72, 4} }
    };

    defs[static_cast<size_t>(StructureType::Campfire)] = {
        StructureType::Campfire, "Campfire", StructureCategory::Furniture,
        {1.0f, 0.6f, 1.0f}, 2.0f, 60.0f, {0.85f, 0.40f, 0.15f, 1.0f},
        { {71, 2}, {36, 4} }
    };

    // =========================================================================
    // 16 WOOD SPECIES VARIANTS: Stairs, Trapdoor, Slab, SideSlab, Chair, Bench, Table
    // =========================================================================
    static const Vec4 plankColors[16] = {
        {0.65f, 0.50f, 0.32f, 1.0f}, {0.82f, 0.75f, 0.60f, 1.0f}, {0.48f, 0.36f, 0.24f, 1.0f}, {0.52f, 0.38f, 0.25f, 1.0f},
        {0.60f, 0.44f, 0.28f, 1.0f}, {0.72f, 0.48f, 0.28f, 1.0f}, {0.35f, 0.24f, 0.16f, 1.0f}, {0.58f, 0.35f, 0.28f, 1.0f},
        {0.85f, 0.65f, 0.65f, 1.0f}, {0.72f, 0.45f, 0.25f, 1.0f}, {0.48f, 0.50f, 0.35f, 1.0f}, {0.65f, 0.38f, 0.60f, 1.0f},
        {0.55f, 0.55f, 0.58f, 1.0f}, {0.62f, 0.75f, 0.38f, 1.0f}, {0.75f, 0.58f, 0.38f, 1.0f}, {0.38f, 0.60f, 0.90f, 1.0f}
    };

    for (int sp = 0; sp < 16; ++sp) {
        std::string spName = getWoodSpeciesName(sp);
        uint16_t plankId = getWoodPlankBlockId(sp);
        Vec4 color = plankColors[sp];

        // 1. Stairs (Merdiven)
        uint16_t stairsTypeIdx = static_cast<uint16_t>(StructureType::Stairs_Oak) + sp;
        defs[stairsTypeIdx] = {
            static_cast<StructureType>(stairsTypeIdx),
            spName + " Stairs",
            StructureCategory::WallsAndRoofs,
            {2.0f, 2.0f, 2.0f}, 4.0f, 100.0f, color,
            { {plankId, 4} }
        };

        // 2. Trapdoor (Tuzak Kapısı)
        uint16_t trapdoorTypeIdx = static_cast<uint16_t>(StructureType::Trapdoor_Oak) + sp;
        defs[trapdoorTypeIdx] = {
            static_cast<StructureType>(trapdoorTypeIdx),
            spName + " Trapdoor",
            StructureCategory::WallsAndRoofs,
            {2.0f, 0.2f, 2.0f}, 3.0f, 80.0f, color,
            { {plankId, 3} }
        };

        // 3. Slab (Basamak)
        uint16_t slabTypeIdx = static_cast<uint16_t>(StructureType::Slab_Oak) + sp;
        defs[slabTypeIdx] = {
            static_cast<StructureType>(slabTypeIdx),
            spName + " Slab",
            StructureCategory::Foundations,
            {2.0f, 1.0f, 2.0f}, 3.0f, 100.0f, color,
            { {plankId, 2} }
        };

        // 4. Side Slab (Yan Basamak)
        uint16_t sideSlabTypeIdx = static_cast<uint16_t>(StructureType::SideSlab_Oak) + sp;
        defs[sideSlabTypeIdx] = {
            static_cast<StructureType>(sideSlabTypeIdx),
            spName + " Side Slab",
            StructureCategory::Foundations,
            {1.0f, 2.0f, 2.0f}, 3.0f, 100.0f, color,
            { {plankId, 2} }
        };

        // 5. Chair (Sandalye)
        uint16_t chairTypeIdx = static_cast<uint16_t>(StructureType::Chair_Oak) + sp;
        defs[chairTypeIdx] = {
            static_cast<StructureType>(chairTypeIdx),
            spName + " Chair",
            StructureCategory::Furniture,
            {0.8f, 1.3f, 0.8f}, 2.5f, 60.0f, color,
            { {plankId, 3} }
        };

        // 6. Bench (Bank)
        uint16_t benchTypeIdx = static_cast<uint16_t>(StructureType::Bench_Oak) + sp;
        defs[benchTypeIdx] = {
            static_cast<StructureType>(benchTypeIdx),
            spName + " Bench",
            StructureCategory::Furniture,
            {2.0f, 1.3f, 0.8f}, 3.5f, 90.0f, color,
            { {plankId, 5} }
        };

        // 7. Table (Masa)
        uint16_t tableTypeIdx = static_cast<uint16_t>(StructureType::Table_Oak) + sp;
        defs[tableTypeIdx] = {
            static_cast<StructureType>(tableTypeIdx),
            spName + " Table",
            StructureCategory::Furniture,
            {1.6f, 1.0f, 1.6f}, 3.5f, 100.0f, color,
            { {plankId, 4} }
        };
    }

    std::cout << "StructureRegistry initialized with " << defs.size() << " building & furniture pieces (including all 16 wood variants)!" << std::endl;
}

const char* StructureRegistry::getWoodSpeciesName(int speciesIdx) {
    static const char* sNames[16] = {
        "Oak", "Birch", "Spruce", "Pine", "Jungle", "Acacia", "Dark Oak", "Mangrove",
        "Cherry", "Maple", "Willow", "Fungal", "Petrified", "Bamboo", "Palm", "Astral"
    };
    if (speciesIdx >= 0 && speciesIdx < 16) return sNames[speciesIdx];
    return "Oak";
}

const char* StructureRegistry::getWoodVariantName(int variantIdx) {
    static const char* vNames[7] = {
        "Stairs", "Trapdoor", "Slab", "Side Slab", "Chair", "Bench", "Table"
    };
    if (variantIdx >= 0 && variantIdx < 7) return vNames[variantIdx];
    return "Piece";
}

uint16_t StructureRegistry::getWoodPlankBlockId(int speciesIdx) {
    if (speciesIdx < 0 || speciesIdx >= 16) speciesIdx = 0;
    return static_cast<uint16_t>(72 + speciesIdx * 3);
}

bool StructureRegistry::isWoodVariant(StructureType type, int& outVariant, int& outSpecies) {
    if (type == StructureType::Stairs_Wood) {
        outVariant = 0; // Stairs
        outSpecies = 0; // Oak
        return true;
    }
    uint16_t val = static_cast<uint16_t>(type);
    uint16_t start = static_cast<uint16_t>(StructureType::Stairs_Oak);
    uint16_t end = static_cast<uint16_t>(StructureType::COUNT);
    if (val >= start && val < end) {
        uint16_t offset = val - start;
        outVariant = offset / 16;
        outSpecies = offset % 16;
        return true;
    }
    outVariant = -1;
    outSpecies = -1;
    return false;
}

} // namespace Aetheria
