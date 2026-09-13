#include "MobSpawner.hpp"
#include "../world/World.hpp"
#include <cmath>
#include <cstdlib>
#include <iostream>

namespace Aetheria {

MobSpawner::MobSpawner() {}

void MobSpawner::spawnInitial(World* world, const Vec3& playerPos, std::vector<std::unique_ptr<Creature>>& creatures) {
    auto spawnMob = [&](CreatureType type, float offsetX, float offsetZ, bool tamed = false) {
        float sx = playerPos.x + offsetX;
        float sz = playerPos.z + offsetZ;
        int gy = world->getHighestBlock(static_cast<int>(std::floor(sx)), static_cast<int>(std::floor(sz)));
        float sy = static_cast<float>(gy) + 1.0f;

        if (type == CreatureType::Pixie || type == CreatureType::VoidPhantom || type == CreatureType::Harpy) {
            sy += 1.8f;
        }

        auto c = std::make_unique<Creature>(type, Vec3(sx, sy, sz));
        if (tamed) c->setTamed(true);
        creatures.push_back(std::move(c));
    };

    // Starting wildlife & monsters on surface
    spawnMob(CreatureType::Sheep, 6.0f, 6.0f);
    spawnMob(CreatureType::Boar, -8.0f, 6.0f);
    spawnMob(CreatureType::Stag, 12.0f, -8.0f);
    spawnMob(CreatureType::Wolf, -12.0f, -10.0f);
    spawnMob(CreatureType::Pixie, -4.0f, -4.0f, true); // Tamed helper
    spawnMob(CreatureType::Goblin, 16.0f, 15.0f);
    spawnMob(CreatureType::Skeleton, -16.0f, 16.0f);
    spawnMob(CreatureType::TitanGolem, 32.0f, 28.0f); // World Boss

    std::cout << "Spawned " << creatures.size() << " initial creatures safely on terrain surface!" << std::endl;
}

CreatureType MobSpawner::selectMobForBiome(uint8_t biomeId, int randomSeed) {
    int roll = randomSeed % 100;

    // Biome 0..5: Temperate Plains & Forests
    if (biomeId <= 5) {
        if (roll < 30) return CreatureType::Sheep;
        if (roll < 55) return CreatureType::Boar;
        if (roll < 75) return CreatureType::Stag;
        if (roll < 90) return CreatureType::Wolf;
        return CreatureType::Pixie;
    }
    // Biome 6..10: Arid & Deserts
    else if (biomeId <= 10) {
        if (roll < 40) return CreatureType::Camel;
        if (roll < 70) return CreatureType::Scorpion;
        if (roll < 90) return CreatureType::Skeleton;
        return CreatureType::Tortoise;
    }
    // Biome 11..15: Cold, Tundra & Taiga
    else if (biomeId <= 15) {
        if (roll < 45) return CreatureType::AlpineGoat;
        if (roll < 75) return CreatureType::Wolf;
        return CreatureType::IceGolem;
    }
    // Biome 16..19: Rainforest & Jungles
    else if (biomeId <= 19) {
        if (roll < 40) return CreatureType::Goblin;
        if (roll < 70) return CreatureType::Spider;
        if (roll < 90) return CreatureType::Harpy;
        return CreatureType::Tortoise;
    }
    // Biome 20..23: Mountainous Crags
    else if (biomeId <= 23) {
        if (roll < 45) return CreatureType::AlpineGoat;
        if (roll < 80) return CreatureType::Harpy;
        return CreatureType::TitanGolem; // Mountain boss chance
    }
    // Biome 24..26: Coastal & Aquatic
    else if (biomeId <= 26) {
        if (roll < 50) return CreatureType::Tortoise;
        if (roll < 80) return CreatureType::SwampHag;
        return CreatureType::Pixie;
    }
    // Biome 27..30: Volcanic & Magma
    else if (biomeId <= 30) {
        if (roll < 45) return CreatureType::Zombie;
        if (roll < 75) return CreatureType::Goblin;
        if (roll < 92) return CreatureType::MagmaDrake;
        return CreatureType::TitanGolem;
    }
    // Biome 31..34: Mystical & Void
    else {
        if (roll < 45) return CreatureType::Pixie;
        if (roll < 80) return CreatureType::VoidPhantom;
        return CreatureType::VoidHarbinger; // Rare celestial boss
    }
}

void MobSpawner::update(World* world, const Vec3& playerPos, std::vector<std::unique_ptr<Creature>>& creatures, float dt) {
    checkTimer += dt;
    if (checkTimer < 1.2f) return;
    checkTimer = 0.0f;

    // 1. Cleanup distant or dead creatures
    for (auto it = creatures.begin(); it != creatures.end();) {
        bool shouldRemove = false;

        // Clean dead mobs after death animation/decay
        if ((*it)->isDespawnReady()) {
            shouldRemove = true;
        }
        // Despawn non-tamed mobs wandering too far
        else if (!(*it)->getIsTamed()) {
            float distSq = ((*it)->getPosition() - playerPos).lengthSq();
            if (distSq > DESPAWN_DIST * DESPAWN_DIST) {
                shouldRemove = true;
            }
        }

        if (shouldRemove) {
            it = creatures.erase(it);
        } else {
            ++it;
        }
    }

    // 2. Spawn new creatures if population is below target
    if (creatures.size() < TARGET_MOBS) {
        int toSpawn = static_cast<int>(TARGET_MOBS - creatures.size());
        if (toSpawn > 2) toSpawn = 2; // Spawn in smooth batches

        for (int i = 0; i < toSpawn; ++i) {
            float angle = static_cast<float>(rand() % 360) * DEG2RAD;
            float dist = MIN_SPAWN_DIST + static_cast<float>(rand() % 1000) / 1000.0f * (MAX_SPAWN_DIST - MIN_SPAWN_DIST);

            float sx = playerPos.x + std::cos(angle) * dist;
            float sz = playerPos.z + std::sin(angle) * dist;

            int gy = world->getHighestBlock(static_cast<int>(std::floor(sx)), static_cast<int>(std::floor(sz)));
            if (gy > 1 && gy < 62) {
                uint8_t biomeId = world->getBiomeAt(sx, sz);
                CreatureType type = selectMobForBiome(biomeId, rand());

                float sy = static_cast<float>(gy) + 1.0f;
                if (type == CreatureType::Pixie || type == CreatureType::VoidPhantom || type == CreatureType::Harpy) {
                    sy += 1.8f;
                }

                creatures.push_back(std::make_unique<Creature>(type, Vec3(sx, sy, sz)));
            }
        }
    }
}

} // namespace Aetheria
