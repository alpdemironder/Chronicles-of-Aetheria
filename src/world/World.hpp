#pragma once

#include "Chunk.hpp"
#include "WorldGenerator.hpp"
#include "BiomeRegistry.hpp"
#include "../core/Math.hpp"
#include <map>
#include <memory>

namespace Aetheria {

struct RaycastResult {
    bool hit = false;
    IVec3 hitBlockPos{0, 0, 0};
    IVec3 adjacentPos{0, 0, 0};
    uint16_t blockId = 0;
    float distance = 0.0f;
};

struct PlantedSapling {
    int x = 0;
    int y = 0;
    int z = 0;
    uint16_t saplingId = 0;
    float growthTimer = 20.0f; // Seconds until procedural maturation
};

class World {
public:
    World(uint32_t seed = 9999);
    ~World();

    void update(const Vec3& playerPos, float dt);
    void render();

    uint16_t getBlock(int x, int y, int z) const;
    void setBlock(int x, int y, int z, uint16_t id);
    int getHighestBlock(int x, int z) const;

    RaycastResult raycast(const Ray& ray, float maxDist = 7.0f) const;

    Chunk* getChunk(int chunkX, int chunkZ) const;
    uint8_t getBiomeAt(float worldX, float worldZ) const;

    int getLoadedChunkCount() const { return static_cast<int>(chunks.size()); }

    // Sapling & Procedural Tree Growth System
    void addSapling(int x, int y, int z, uint16_t saplingId, float timer = -1.0f);
    void removeSapling(int x, int y, int z);
    bool hasSapling(int x, int y, int z) const;
    void growTree(int x, int y, int z, uint16_t saplingId);
    static bool isSapling(uint16_t id) { return id >= 366 && id <= 369; }

    // Day/Night Cycle & Atmospheric System
    // timeOfDay in [0.0, 1.0) where 0.0=Midnight, 0.25=Dawn, 0.50=Noon, 0.75=Sunset
    float getTimeOfDay() const { return timeOfDay; }
    void setTimeOfDay(float t) {
        timeOfDay = t;
        while (timeOfDay >= 1.0f) timeOfDay -= 1.0f;
        while (timeOfDay < 0.0f) timeOfDay += 1.0f;
    }
    float getDayCycleDuration() const { return dayCycleDuration; }
    void setDayCycleDuration(float duration) { dayCycleDuration = std::max(10.0f, duration); }

    Vec3 getSunDirection() const;
    Vec3 getMoonDirection() const;
    Vec3 getActiveLightDirection() const;
    float getDaylightFactor() const;
    bool isNight() const;
    Vec3 getSkyColor(const Vec3& biomeSkyColor) const;
    Vec3 getFogColor(const Vec3& biomeFogColor) const;
    std::string getFormattedTime() const;
    std::string getTimePeriodName() const;

private:
    void loadChunksAround(int centerChunkX, int centerChunkZ, int radius);

    WorldGenerator generator;
    PerlinNoise tempNoise;
    PerlinNoise moistureNoise;
    PerlinNoise elevNoise;

    // (chunkX, chunkZ) -> Chunk
    std::map<std::pair<int, int>, std::unique_ptr<Chunk>> chunks;
    int lastPlayerChunkX = 999999;
    int lastPlayerChunkZ = 999999;

    // Planted Saplings tracking in the active world
    std::vector<PlantedSapling> saplings;

    // Day / Night Cycle (Default starts at 0.28 = ~06:45 AM morning, 12 min full cycle)
    float timeOfDay = 0.28f;
    float dayCycleDuration = 720.0f;
};

} // namespace Aetheria
