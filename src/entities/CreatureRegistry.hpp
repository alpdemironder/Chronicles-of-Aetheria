#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include "../core/Math.hpp"

namespace Aetheria {

enum class CreatureType : uint8_t {
    // Farm Animals First
    Cow,
    Sheep,
    Pig,
    Chicken,
    Horse,

    // Monsters
    Zombie,
    Skeleton,
    Spider,
    Creeper,
    Enderman,

    COUNT
};

enum class WorkSuitability : uint8_t {
    None = 0,
    Handiwork = 1, // Helps build blueprints!
    Lumbering = 2, // Chops trees
    Mining = 3,    // Mines rocks
    Farming = 4,   // Plants/harvests
    Defense = 5    // Guards territory
};

struct CreatureDef {
    CreatureType type;
    std::string name;
    bool isHostile = false;
    bool isBoss = false;
    float maxHP = 50.0f;
    float attackDamage = 5.0f;
    float moveSpeed = 4.0f;
    float detectionRange = 12.0f;
    Vec3 size{0.8f, 0.8f, 0.8f};
    Vec4 primaryColor{0.8f, 0.8f, 0.8f, 1.0f};
    WorkSuitability workTrait = WorkSuitability::None;
    uint16_t dropItemId = 0;
    uint8_t dropCount = 1;
    uint32_t xpReward = 15;
};

class CreatureRegistry {
public:
    static void init();
    static const CreatureDef& get(CreatureType type);
    static const std::vector<CreatureDef>& getAll();

private:
    static std::vector<CreatureDef> defs;
    static bool initialized;
};

} // namespace Aetheria
