#pragma once

#include "Noise.hpp"
#include <memory>

namespace Aetheria {

class Chunk;

class WorldGenerator {
public:
    WorldGenerator(uint32_t seed = 4242);

    void generateChunk(Chunk* chunk);

private:
    void placeBiomeTree(Chunk* chunk, int localX, int localY, int localZ, uint8_t biomeId, uint16_t logId, uint16_t leafId);
    void placeOakTree(Chunk* chunk, int localX, int localY, int localZ, uint16_t logId, uint16_t leafId);
    void placeBirchTree(Chunk* chunk, int localX, int localY, int localZ, uint16_t logId, uint16_t leafId);
    void placeSpruceTree(Chunk* chunk, int localX, int localY, int localZ, uint16_t logId, uint16_t leafId);
    void placeJungleTree(Chunk* chunk, int localX, int localY, int localZ, uint16_t logId, uint16_t leafId);
    void placeCherryTree(Chunk* chunk, int localX, int localY, int localZ, uint16_t logId, uint16_t leafId);
    void placeMapleTree(Chunk* chunk, int localX, int localY, int localZ, uint16_t logId, uint16_t leafId);
    void placeAcaciaTree(Chunk* chunk, int localX, int localY, int localZ, uint16_t logId, uint16_t leafId);
    void placeSwampWillow(Chunk* chunk, int localX, int localY, int localZ, uint16_t logId, uint16_t leafId);
    void placeGiantMushroom(Chunk* chunk, int localX, int localY, int localZ, uint16_t stemId, uint16_t capId);
    void placeBambooClump(Chunk* chunk, int localX, int localY, int localZ, uint16_t stalkId, uint16_t leafId);
    void placePalmTree(Chunk* chunk, int localX, int localY, int localZ, uint16_t logId, uint16_t leafId);

    void placeFallenLog(Chunk* chunk, int localX, int localY, int localZ, uint16_t logId);
    void placeBoulder(Chunk* chunk, int localX, int localY, int localZ, uint16_t stoneId);
    void placeCrystal(Chunk* chunk, int localX, int localY, int localZ, uint16_t crystalId);

    PerlinNoise continentalNoise;
    PerlinNoise mountainNoise;
    PerlinNoise elevationNoise;
    PerlinNoise tempNoise;
    PerlinNoise moistureNoise;
    PerlinNoise roughnessNoise;
    PerlinNoise riverNoise;
    PerlinNoise caveNoise1;
    PerlinNoise caveNoise2;
    PerlinNoise cavernNoise;
    PerlinNoise oreNoise;
    uint32_t seed = 4242;
};

} // namespace Aetheria
