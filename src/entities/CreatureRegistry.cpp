#include "CreatureRegistry.hpp"
#include <iostream>

namespace Aetheria {

std::vector<CreatureDef> CreatureRegistry::defs;
bool CreatureRegistry::initialized = false;

const CreatureDef& CreatureRegistry::get(CreatureType type) {
    if (!initialized) init();
    size_t idx = static_cast<size_t>(type);
    if (idx < defs.size()) return defs[idx];
    return defs[0];
}

const std::vector<CreatureDef>& CreatureRegistry::getAll() {
    if (!initialized) init();
    return defs;
}

void CreatureRegistry::init() {
    if (initialized) return;
    initialized = true;

    defs.clear();
    defs.resize(static_cast<size_t>(CreatureType::COUNT));

    // =========================================================================
    // 1. MINECRAFT PASSIVE ANIMALS (Cow, Sheep, Pig, Chicken, Horse)
    // =========================================================================
    defs[static_cast<size_t>(CreatureType::Cow)] = {
        CreatureType::Cow, "Minecraft Cow", false, false, 45.0f, 0.0f, 3.2f, 6.0f,
        {0.9f, 1.4f, 1.3f}, {0.45f, 0.35f, 0.25f, 1.0f}, WorkSuitability::Farming,
        506, 2, 15 // Drops Leather
    };

    defs[static_cast<size_t>(CreatureType::Sheep)] = {
        CreatureType::Sheep, "Minecraft Sheep", false, false, 35.0f, 0.0f, 3.0f, 6.0f,
        {0.9f, 1.3f, 1.3f}, {0.96f, 0.96f, 0.96f, 1.0f}, WorkSuitability::Farming,
        281, 3, 12 // Drops Wool
    };

    defs[static_cast<size_t>(CreatureType::Pig)] = {
        CreatureType::Pig, "Minecraft Pig", false, false, 30.0f, 0.0f, 3.5f, 6.0f,
        {0.9f, 0.9f, 1.2f}, {0.98f, 0.70f, 0.72f, 1.0f}, WorkSuitability::Farming,
        522, 2, 12 // Drops Porkchop / Meat
    };

    defs[static_cast<size_t>(CreatureType::Chicken)] = {
        CreatureType::Chicken, "Minecraft Chicken", false, false, 20.0f, 0.0f, 3.8f, 5.0f,
        {0.5f, 0.7f, 0.5f}, {0.96f, 0.96f, 0.92f, 1.0f}, WorkSuitability::Farming,
        516, 2, 10 // Drops Feathers
    };

    defs[static_cast<size_t>(CreatureType::Horse)] = {
        CreatureType::Horse, "Minecraft Horse", false, false, 60.0f, 3.0f, 6.5f, 10.0f,
        {1.1f, 1.6f, 1.8f}, {0.55f, 0.35f, 0.20f, 1.0f}, WorkSuitability::Lumbering,
        506, 3, 25 // Drops Leather
    };

    // =========================================================================
    // 2. MINECRAFT MONSTERS (Zombie, Skeleton, Spider, Creeper, Enderman)
    // =========================================================================
    defs[static_cast<size_t>(CreatureType::Zombie)] = {
        CreatureType::Zombie, "Minecraft Zombie", true, false, 55.0f, 9.0f, 3.2f, 14.0f,
        {0.6f, 1.95f, 0.6f}, {0.32f, 0.55f, 0.32f, 1.0f}, WorkSuitability::Mining,
        501, 1, 35 // Drops Iron Ingot
    };

    defs[static_cast<size_t>(CreatureType::Skeleton)] = {
        CreatureType::Skeleton, "Minecraft Skeleton", true, false, 45.0f, 8.0f, 3.5f, 16.0f,
        {0.6f, 1.95f, 0.6f}, {0.88f, 0.88f, 0.85f, 1.0f}, WorkSuitability::Defense,
        525, 2, 40 // Drops Bone Meal
    };

    defs[static_cast<size_t>(CreatureType::Spider)] = {
        CreatureType::Spider, "Minecraft Spider", true, false, 40.0f, 8.0f, 5.5f, 14.0f,
        {1.4f, 0.65f, 1.4f}, {0.18f, 0.14f, 0.12f, 1.0f}, WorkSuitability::None,
        504, 2, 35 // Drops Silk String
    };

    defs[static_cast<size_t>(CreatureType::Creeper)] = {
        CreatureType::Creeper, "Minecraft Creeper", true, false, 45.0f, 25.0f, 3.6f, 12.0f,
        {0.6f, 1.7f, 0.6f}, {0.38f, 0.78f, 0.28f, 1.0f}, WorkSuitability::Defense,
        550, 2, 50 // Drops Gunpowder
    };

    defs[static_cast<size_t>(CreatureType::Enderman)] = {
        CreatureType::Enderman, "Minecraft Enderman", true, false, 80.0f, 14.0f, 4.8f, 20.0f,
        {0.6f, 2.9f, 0.6f}, {0.06f, 0.06f, 0.08f, 1.0f}, WorkSuitability::Handiwork,
        505, 1, 60 // Drops Ender Pearl / Aetherium
    };

    std::cout << "CreatureRegistry initialized with " << defs.size() << " creatures & monsters!" << std::endl;
}

} // namespace Aetheria
