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

    defs[static_cast<size_t>(CreatureType::Sheep)] = {
        CreatureType::Sheep, "Mountain Sheep", false, false, 30.0f, 0.0f, 3.0f, 6.0f,
        {0.8f, 0.9f, 1.2f}, {0.92f, 0.92f, 0.95f, 1.0f}, WorkSuitability::Farming,
        281, 2, 10
    };

    defs[static_cast<size_t>(CreatureType::Boar)] = {
        CreatureType::Boar, "Wild Boar", false, false, 45.0f, 4.0f, 4.2f, 8.0f,
        {0.9f, 0.8f, 1.3f}, {0.52f, 0.35f, 0.22f, 1.0f}, WorkSuitability::Mining,
        2, 2, 15
    };

    defs[static_cast<size_t>(CreatureType::Wolf)] = {
        CreatureType::Wolf, "Forest Wolf", false, false, 50.0f, 8.0f, 5.5f, 10.0f,
        {0.7f, 0.9f, 1.4f}, {0.60f, 0.60f, 0.62f, 1.0f}, WorkSuitability::Defense,
        71, 1, 25
    };

    defs[static_cast<size_t>(CreatureType::Stag)] = {
        CreatureType::Stag, "Royal Highland Stag", false, false, 60.0f, 3.0f, 6.0f, 12.0f,
        {0.9f, 1.6f, 1.5f}, {0.65f, 0.45f, 0.28f, 1.0f}, WorkSuitability::Lumbering,
        72, 2, 30
    };

    defs[static_cast<size_t>(CreatureType::Camel)] = {
        CreatureType::Camel, "Desert Camel", false, false, 80.0f, 2.0f, 3.5f, 8.0f,
        {1.2f, 1.8f, 2.0f}, {0.85f, 0.72f, 0.48f, 1.0f}, WorkSuitability::Farming,
        11, 4, 35
    };

    defs[static_cast<size_t>(CreatureType::AlpineGoat)] = {
        CreatureType::AlpineGoat, "Alpine Mountain Goat", false, false, 40.0f, 4.0f, 4.5f, 8.0f,
        {0.8f, 1.0f, 1.2f}, {0.88f, 0.85f, 0.82f, 1.0f}, WorkSuitability::Mining,
        36, 3, 20
    };

    defs[static_cast<size_t>(CreatureType::Pixie)] = {
        CreatureType::Pixie, "Luminescent Pixie", false, false, 25.0f, 0.0f, 5.0f, 10.0f,
        {0.5f, 0.6f, 0.5f}, {0.45f, 0.85f, 0.98f, 1.0f}, WorkSuitability::Handiwork, // Handiwork builder!
        192, 1, 40 // Drops Arcane Mana Crystal
    };

    defs[static_cast<size_t>(CreatureType::Tortoise)] = {
        CreatureType::Tortoise, "Ancient Tortoise", false, false, 120.0f, 2.0f, 1.8f, 6.0f,
        {1.4f, 0.8f, 1.6f}, {0.35f, 0.52f, 0.32f, 1.0f}, WorkSuitability::Defense,
        37, 5, 45
    };

    defs[static_cast<size_t>(CreatureType::Goblin)] = {
        CreatureType::Goblin, "Goblin Skirmisher", true, false, 40.0f, 7.0f, 4.5f, 14.0f,
        {0.7f, 1.2f, 0.7f}, {0.35f, 0.70f, 0.25f, 1.0f}, WorkSuitability::None,
        124, 1, 35
    };

    defs[static_cast<size_t>(CreatureType::Skeleton)] = {
        CreatureType::Skeleton, "Skeleton Marksman", true, false, 45.0f, 9.0f, 3.8f, 16.0f,
        {0.6f, 1.8f, 0.6f}, {0.85f, 0.85f, 0.85f, 1.0f}, WorkSuitability::None,
        248, 1, 40
    };

    defs[static_cast<size_t>(CreatureType::Zombie)] = {
        CreatureType::Zombie, "Draugr Zombie", true, false, 65.0f, 10.0f, 2.8f, 12.0f,
        {0.7f, 1.8f, 0.7f}, {0.25f, 0.45f, 0.35f, 1.0f}, WorkSuitability::None,
        124, 2, 45
    };

    defs[static_cast<size_t>(CreatureType::Spider)] = {
        CreatureType::Spider, "Cave Spider", true, false, 40.0f, 8.0f, 6.0f, 14.0f,
        {1.2f, 0.6f, 1.2f}, {0.20f, 0.15f, 0.15f, 1.0f}, WorkSuitability::None,
        339, 1, 35
    };

    defs[static_cast<size_t>(CreatureType::Scorpion)] = {
        CreatureType::Scorpion, "Sand Scorpion", true, false, 55.0f, 11.0f, 4.5f, 12.0f,
        {1.3f, 0.8f, 1.5f}, {0.65f, 0.45f, 0.20f, 1.0f}, WorkSuitability::None,
        196, 1, 50
    };

    defs[static_cast<size_t>(CreatureType::IceGolem)] = {
        CreatureType::IceGolem, "Frost Ice Golem", true, false, 150.0f, 16.0f, 2.5f, 12.0f,
        {1.6f, 2.4f, 1.6f}, {0.55f, 0.80f, 0.98f, 1.0f}, WorkSuitability::None,
        187, 2, 90 // Frost Crystal
    };

    defs[static_cast<size_t>(CreatureType::MagmaDrake)] = {
        CreatureType::MagmaDrake, "Magma Drake", true, false, 130.0f, 18.0f, 4.8f, 15.0f,
        {1.8f, 1.6f, 2.2f}, {0.95f, 0.35f, 0.10f, 1.0f}, WorkSuitability::None,
        186, 2, 100 // Flame Crystal
    };

    defs[static_cast<size_t>(CreatureType::SwampHag)] = {
        CreatureType::SwampHag, "Swamp Hag", true, false, 75.0f, 12.0f, 3.5f, 15.0f,
        {0.7f, 1.7f, 0.7f}, {0.42f, 0.28f, 0.48f, 1.0f}, WorkSuitability::None,
        196, 1, 65
    };

    defs[static_cast<size_t>(CreatureType::VoidPhantom)] = {
        CreatureType::VoidPhantom, "Void Phantom", true, false, 90.0f, 14.0f, 5.2f, 16.0f,
        {1.0f, 1.8f, 1.0f}, {0.18f, 0.05f, 0.30f, 1.0f}, WorkSuitability::None,
        191, 1, 80 // Shadow Void Crystal
    };

    defs[static_cast<size_t>(CreatureType::Harpy)] = {
        CreatureType::Harpy, "Mountain Harpy", true, false, 70.0f, 13.0f, 5.8f, 18.0f,
        {1.1f, 1.6f, 1.1f}, {0.70f, 0.55f, 0.45f, 1.0f}, WorkSuitability::None,
        73, 2, 70
    };

    // --- BOSSES ---
    defs[static_cast<size_t>(CreatureType::TitanGolem)] = {
        CreatureType::TitanGolem, "Ancient Titan Golem", true, true, 600.0f, 30.0f, 2.2f, 25.0f,
        {2.8f, 4.2f, 2.8f}, {0.45f, 0.50f, 0.55f, 1.0f}, WorkSuitability::None,
        356, 1, 500 // Drops Titan Core!
    };

    defs[static_cast<size_t>(CreatureType::VoidHarbinger)] = {
        CreatureType::VoidHarbinger, "Void Harbinger", true, true, 850.0f, 38.0f, 4.0f, 30.0f,
        {3.2f, 4.8f, 3.2f}, {0.12f, 0.02f, 0.22f, 1.0f}, WorkSuitability::None,
        357, 1, 800 // Drops Void Singularity!
    };

    std::cout << "CreatureRegistry initialized with " << defs.size() << " creatures & bosses!" << std::endl;
}

} // namespace Aetheria
