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

    std::cout << "StructureRegistry initialized with " << defs.size() << " Palworld building pieces!" << std::endl;
}

} // namespace Aetheria
