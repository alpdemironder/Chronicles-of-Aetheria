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
    // 1. FARM ANIMALS FIRST (Cow, Sheep, Pig, Chicken, Horse) - PASSIVE WILDLIFE
    // =========================================================================
    defs[static_cast<size_t>(CreatureType::Cow)] = {
        CreatureType::Cow, "Dairy Cow", false, false, 45.0f, 0.0f, 3.2f, 6.0f,
        {1.1f, 1.2f, 1.6f}, {0.92f, 0.92f, 0.92f, 1.0f}, WorkSuitability::Farming,
        506, 2, 15 // Drops Leather
    };

    defs[static_cast<size_t>(CreatureType::Sheep)] = {
        CreatureType::Sheep, "Highland Sheep", false, false, 35.0f, 0.0f, 3.0f, 6.0f,
        {0.9f, 0.95f, 1.2f}, {0.95f, 0.95f, 0.95f, 1.0f}, WorkSuitability::Farming,
        281, 3, 12 // Drops Wool
    };

    defs[static_cast<size_t>(CreatureType::Pig)] = {
        CreatureType::Pig, "Farm Pig", false, false, 30.0f, 0.0f, 3.5f, 6.0f,
        {0.85f, 0.8f, 1.2f}, {0.98f, 0.72f, 0.75f, 1.0f}, WorkSuitability::Farming,
        522, 2, 12 // Drops Meat
    };

    defs[static_cast<size_t>(CreatureType::Chicken)] = {
        CreatureType::Chicken, "Farm Chicken", false, false, 20.0f, 0.0f, 3.8f, 5.0f,
        {0.5f, 0.6f, 0.55f}, {0.96f, 0.96f, 0.92f, 1.0f}, WorkSuitability::Farming,
        516, 2, 10 // Drops Feathers
    };

    defs[static_cast<size_t>(CreatureType::Horse)] = {
        CreatureType::Horse, "Wild Steed", false, false, 60.0f, 3.0f, 6.5f, 10.0f,
        {1.1f, 1.6f, 1.8f}, {0.55f, 0.35f, 0.20f, 1.0f}, WorkSuitability::Lumbering,
        506, 3, 25 // Drops Leather
    };

    // =========================================================================
    // 2. MONSTERS (Zombie, Skeleton, Spider, Ghoul, Goblin) - HOSTILE FOES
    // =========================================================================
    defs[static_cast<size_t>(CreatureType::Zombie)] = {
        CreatureType::Zombie, "Draugr Zombie", true, false, 55.0f, 10.0f, 3.0f, 14.0f,
        {0.75f, 1.8f, 0.75f}, {0.28f, 0.48f, 0.32f, 1.0f}, WorkSuitability::Mining,
        501, 1, 35 // Drops Iron Ingot
    };

    defs[static_cast<size_t>(CreatureType::Skeleton)] = {
        CreatureType::Skeleton, "Skeleton Archer", true, false, 45.0f, 9.0f, 3.6f, 16.0f,
        {0.65f, 1.8f, 0.65f}, {0.88f, 0.88f, 0.85f, 1.0f}, WorkSuitability::Defense,
        525, 2, 40 // Drops Bone Meal
    };

    defs[static_cast<size_t>(CreatureType::Spider)] = {
        CreatureType::Spider, "Cave Spider", true, false, 40.0f, 8.0f, 5.5f, 14.0f,
        {1.3f, 0.65f, 1.3f}, {0.20f, 0.16f, 0.18f, 1.0f}, WorkSuitability::None,
        504, 2, 35 // Drops Silk String / Rope
    };

    defs[static_cast<size_t>(CreatureType::Ghoul)] = {
        CreatureType::Ghoul, "Crypt Ghoul", true, false, 70.0f, 13.0f, 4.8f, 18.0f,
        {0.8f, 1.6f, 0.8f}, {0.42f, 0.38f, 0.48f, 1.0f}, WorkSuitability::Defense,
        505, 2, 55 // Drops Cursed Hide
    };

    defs[static_cast<size_t>(CreatureType::Goblin)] = {
        CreatureType::Goblin, "Goblin Raider", true, false, 38.0f, 7.0f, 4.6f, 14.0f,
        {0.65f, 1.15f, 0.65f}, {0.35f, 0.68f, 0.22f, 1.0f}, WorkSuitability::Handiwork,
        503, 1, 30 // Drops Gold Ingot
    };

    std::cout << "CreatureRegistry initialized with " << defs.size() << " creatures & monsters!" << std::endl;
}

} // namespace Aetheria
