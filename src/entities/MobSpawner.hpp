#pragma once

#include "Creature.hpp"
#include <vector>
#include <memory>

namespace Aetheria {

class World;
class Player;
class AudioEngine;

class MobSpawner {
public:
    MobSpawner();
    ~MobSpawner() = default;

    // Spawns initial creatures safely on top of terrain around player spawn
    void spawnInitial(World* world, const Vec3& playerPos, std::vector<std::unique_ptr<Creature>>& creatures);

    // Dynamic spawner: maintains healthy mob population around player, despawns distant mobs, matches biomes
    void update(World* world, const Vec3& playerPos, std::vector<std::unique_ptr<Creature>>& creatures, float dt);

private:
    CreatureType selectMobForBiome(uint8_t biomeId, int randomSeed);

    float checkTimer = 0.0f;
    const size_t TARGET_MOBS = 20;
    const float DESPAWN_DIST = 85.0f;
    const float MIN_SPAWN_DIST = 22.0f;
    const float MAX_SPAWN_DIST = 60.0f;
};

} // namespace Aetheria
