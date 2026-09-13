#include "WorldGenerator.hpp"
#include "Chunk.hpp"
#include "BiomeRegistry.hpp"
#include "StructureGenerator.hpp"
#include <algorithm>
#include <cstdlib>
#include <cmath>

namespace Aetheria {

static inline float localHash(int x, int z, uint32_t seed) {
    uint32_t n = static_cast<uint32_t>(x * 374761393) ^ static_cast<uint32_t>(z * 668265263) ^ seed;
    n = (n ^ (n >> 13)) * 1274126177;
    return static_cast<float>(n % 10000) / 10000.0f;
}

static inline bool isValidTreeSoil(uint16_t bId, uint8_t biomeId) {
    // Dirt, grass, podzol, mycelium, turf, moss
    if (bId == 1 || bId == 2 || bId == 3 || bId == 4 || 
        bId == 5 || bId == 6 || bId == 7 || bId == 26) {
        return true;
    }
    // Sand for Palm Trees in coastal / desert / oasis biomes
    if ((bId == 11 || bId == 12) && (biomeId == 15 || biomeId == 27 || biomeId == 10 || biomeId == 14)) {
        return true;
    }
    return false;
}

WorldGenerator::WorldGenerator(uint32_t seed)
    : continentalNoise(seed + 11),
      mountainNoise(seed + 77),
      elevationNoise(seed),
      tempNoise(seed + 101),
      moistureNoise(seed + 202),
      roughnessNoise(seed + 303),
      riverNoise(seed + 404),
      caveNoise1(seed + 505),
      caveNoise2(seed + 606),
      cavernNoise(seed + 707),
      oreNoise(seed + 808),
      seed(seed) {
}

void WorldGenerator::generateChunk(Chunk* chunk) {
    int cx = chunk->getChunkX();
    int cz = chunk->getChunkZ();

    int heights[CHUNK_X][CHUNK_Z];
    uint8_t biomeMap[CHUNK_X][CHUNK_Z];
    bool isRiverBank[CHUNK_X][CHUNK_Z];
    constexpr int SEA_LEVEL = 26;

    // -------------------------------------------------------------------------
    // 1. HEIGHTMAP & BIOME EVALUATION
    // -------------------------------------------------------------------------
    for (int x = 0; x < CHUNK_X; ++x) {
        for (int z = 0; z < CHUNK_Z; ++z) {
            float wx = static_cast<float>(cx * CHUNK_X + x);
            float wz = static_cast<float>(cz * CHUNK_Z + z);

            // Climate parameters
            float temp = tempNoise.fbm2D(wx * 0.0025f, wz * 0.0025f, 3);
            float moisture = moistureNoise.fbm2D(wx * 0.0025f, wz * 0.0025f, 3);

            // Continental macro-elevation
            float continent = continentalNoise.fbm2D(wx * 0.0018f, wz * 0.0018f, 4);

            uint8_t biomeId = BiomeRegistry::sampleBiome(temp, moisture, 32.0f + continent * 24.0f);
            const BiomeDef& biome = BiomeRegistry::get(biomeId);

            // Mountain Ridging (Sharpened alpine knife-edges)
            float mNoise = mountainNoise.fbm2D(wx * 0.0055f, wz * 0.0055f, 4);
            float ridge = 1.0f - std::abs(mNoise);
            ridge = ridge * ridge;

            // Serpentine River & Valley carving
            float rSample = std::abs(riverNoise.fbm2D(wx * 0.004f, wz * 0.004f, 3));
            float riverValley = 0.0f;
            if (rSample < 0.075f) {
                riverValley = (1.0f - (rSample / 0.075f)) * 14.0f;
            }

            // Surface micro-roughness
            float rough = roughnessNoise.fbm2D(wx * 0.016f, wz * 0.016f, 3) * (biome.roughness * 0.75f);

            // Mountain elevation bonus
            float mountainBonus = (biome.roughness >= 10.0f) ? (ridge * biome.roughness * 1.8f) : (ridge * 4.0f);

            // Composite terrain height
            float rawHeight = biome.baseElevation + continent * 14.0f + mountainBonus + rough - riverValley;
            int height = static_cast<int>(std::round(rawHeight));
            height = std::clamp(height, 8, CHUNK_Y - 14);

            heights[x][z] = height;
            biomeMap[x][z] = biomeId;
            isRiverBank[x][z] = (rSample < 0.075f);
        }
    }

    // -------------------------------------------------------------------------
    // 2. STRATA & ORE POPULATION
    // -------------------------------------------------------------------------
    for (int x = 0; x < CHUNK_X; ++x) {
        for (int z = 0; z < CHUNK_Z; ++z) {
            float wx = static_cast<float>(cx * CHUNK_X + x);
            float wz = static_cast<float>(cz * CHUNK_Z + z);

            int height = heights[x][z];
            uint8_t biomeId = biomeMap[x][z];
            const BiomeDef& biome = BiomeRegistry::get(biomeId);

            for (int y = 0; y < CHUNK_Y; ++y) {
                if (y == 0) {
                    chunk->setBlock(x, y, z, 52); // Indestructible Obsidian / Bedrock
                } else if (y <= 3) {
                    // Bedrock protrusions & Deepslate floor
                    float brk = localHash(static_cast<int>(wx), static_cast<int>(wz), y * 13 + 7);
                    chunk->setBlock(x, y, z, (brk > 0.45f) ? 52 : 45);
                } else if (y < 12) {
                    chunk->setBlock(x, y, z, 45); // Deepslate strata
                } else if (y < height - 3) {
                    // 3D Ore Vein Distribution
                    float oreVal = oreNoise.fbm3D(wx * 0.12f, static_cast<float>(y) * 0.12f, wz * 0.12f, 2);

                    if (y < 18) {
                        // Deepslate ores
                        if (oreVal > 0.68f) chunk->setBlock(x, y, z, 139);      // Deepslate Diamond / Mythril
                        else if (oreVal > 0.60f) chunk->setBlock(x, y, z, 138); // Deepslate Gold
                        else if (oreVal > 0.50f) chunk->setBlock(x, y, z, 137); // Deepslate Iron
                        else if (oreVal < -0.55f) chunk->setBlock(x, y, z, 136);// Deepslate Coal
                        else chunk->setBlock(x, y, z, 45);                      // Deepslate
                    } else {
                        // Upper geological stone ores
                        if (y <= 24 && oreVal > 0.66f) chunk->setBlock(x, y, z, 126);       // Gold Ore
                        else if (y <= 50 && (oreVal > 0.54f || (oreVal > 0.45f && y < 32))) chunk->setBlock(x, y, z, 124); // Iron Ore
                        else if (y <= 42 && oreVal > 0.48f && oreVal <= 0.54f) chunk->setBlock(x, y, z, 122); // Copper Ore
                        else if (oreVal < -0.46f) chunk->setBlock(x, y, z, 121);            // Coal Ore
                        else chunk->setBlock(x, y, z, biome.deepStoneBlock);
                    }
                } else if (y < height) {
                    chunk->setBlock(x, y, z, biome.subSurfaceBlock);
                } else if (y == height) {
                    // Surface block handling (Shorelines, Beaches, and Biome covers)
                    if (height <= SEA_LEVEL) {
                        if (biome.temperature < -0.3f) {
                            chunk->setBlock(x, y, z, 18); // Packed Ice floor
                        } else if (height >= SEA_LEVEL - 2) {
                            chunk->setBlock(x, y, z, 11); // Sandy Beach
                        } else {
                            chunk->setBlock(x, y, z, (biomeId == 15 ? 27 : 14)); // Coral sand / Gravel oceanbed
                        }
                    } else if (height <= SEA_LEVEL + 2 && isRiverBank[x][z]) {
                        chunk->setBlock(x, y, z, 11); // Sand along river banks
                    } else {
                        chunk->setBlock(x, y, z, biome.surfaceBlock);
                    }
                } else if (y > height && y <= SEA_LEVEL) {
                    // Submerged Water / Ice basin
                    if (biome.temperature < -0.3f) {
                        chunk->setBlock(x, y, z, (y == SEA_LEVEL) ? 17 : 18); // Ice surface / Packed ice
                    } else {
                        chunk->setBlock(x, y, z, 17); // Transparent water-ice
                    }
                } else {
                    chunk->setBlock(x, y, z, 0); // Air
                }
            }
        }
    }

    // -------------------------------------------------------------------------
    // 3. TRUE 3D UNDERGROUND CAVES (Worm Tunnels & Caverns)
    // -------------------------------------------------------------------------
    for (int x = 0; x < CHUNK_X; ++x) {
        for (int z = 0; z < CHUNK_Z; ++z) {
            float wx = static_cast<float>(cx * CHUNK_X + x);
            float wz = static_cast<float>(cz * CHUNK_Z + z);
            int height = heights[x][z];

            for (int y = 4; y < CHUNK_Y; ++y) {
                if (y <= height - 3) {
                    // Dual 3D noise worm caves
                    float cw1 = caveNoise1.fbm3D(wx * 0.036f, static_cast<float>(y) * 0.055f, wz * 0.036f, 2);
                    float cw2 = caveNoise2.fbm3D(wx * 0.036f + 43.1f, static_cast<float>(y) * 0.055f + 67.8f, wz * 0.036f + 19.4f, 2);
                    bool isWorm = (cw1 * cw1 + cw2 * cw2 < 0.022f);

                    // Large Swiss-cheese caverns
                    float cav = cavernNoise.fbm3D(wx * 0.02f, static_cast<float>(y) * 0.032f, wz * 0.02f, 2);
                    bool isCavern = (cav > 0.62f && y >= 6 && y <= 46);

                    if (isWorm || isCavern) {
                        chunk->setBlock(x, y, z, 0); // Carve true underground air pocket!
                    }
                } else if (height > 50 && y >= height - 4 && y <= height) {
                    // Rare natural cave entrance carved into high mountain crags
                    float cavEnt = cavernNoise.fbm3D(wx * 0.035f, static_cast<float>(y) * 0.04f, wz * 0.035f, 2);
                    if (cavEnt > 0.74f) {
                        chunk->setBlock(x, y, z, 0);
                    }
                }
            }
        }
    }

    // -------------------------------------------------------------------------
    // 4. UNDERGROUND CAVE DECORATIONS (Dripstone, Crystals, Moss, Molten pools)
    // -------------------------------------------------------------------------
    for (int x = 0; x < CHUNK_X; ++x) {
        for (int z = 0; z < CHUNK_Z; ++z) {
            int height = heights[x][z];

            for (int y = 4; y <= height - 3; ++y) {
                if (chunk->getBlock(x, y, z) == 0) { // Air inside cave
                    uint16_t below = chunk->getBlock(x, y - 1, z);
                    uint16_t above = (y + 1 < CHUNK_Y) ? chunk->getBlock(x, y + 1, z) : 0;

                    float decRand = localHash(cx * CHUNK_X + x, cz * CHUNK_Z + z, y * 31 + 17);

                    // Cave Floor decoration
                    if (below != 0) {
                        if (y <= 6) {
                            if (decRand < 0.35f) chunk->setBlock(x, y - 1, z, 32); // Molten magma pool
                        } else {
                            if (decRand < 0.035f) {
                                chunk->setBlock(x, y, z, 49); // Stalagmite Dripstone
                            } else if (decRand < 0.065f) {
                                uint16_t crystalId = 186 + (static_cast<int>(decRand * 100.0f) % 20);
                                placeCrystal(chunk, x, y, z, crystalId);
                            } else if (decRand < 0.11f) {
                                chunk->setBlock(x, y - 1, z, 26); // Moss floor patch
                            }
                        }
                    }

                    // Cave Ceiling decoration
                    if (above != 0 && decRand > 0.965f) {
                        chunk->setBlock(x, y, z, 49); // Hanging Dripstone stalactite
                    }
                }
            }
        }
    }

    // -------------------------------------------------------------------------
    // 5. SURFACE DRESSING (Diverse Trees, Boulders, Fallen Logs, Mushrooms)
    // -------------------------------------------------------------------------
    std::vector<std::pair<int, int>> placedTrees;

    for (int x = 2; x <= CHUNK_X - 3; ++x) {
        for (int z = 2; z <= CHUNK_Z - 3; ++z) {
            float wx = static_cast<float>(cx * CHUNK_X + x);
            float wz = static_cast<float>(cz * CHUNK_Z + z);

            // Authoritative ground height (never scan downward through tree leaves!)
            int surfaceY = heights[x][z];
            while (surfaceY > 5 && chunk->getBlock(x, surfaceY, z) == 0) {
                surfaceY--; // Adjust if surface was carved by cave opening
            }

            if (surfaceY < SEA_LEVEL || surfaceY + 14 >= CHUNK_Y) continue;

            uint16_t surfBlock = chunk->getBlock(x, surfaceY, z);
            if (surfBlock == 0 || surfBlock == 17) continue;

            uint8_t biomeId = biomeMap[x][z];
            const BiomeDef& biome = BiomeRegistry::get(biomeId);

            float decorRand = localHash(static_cast<int>(wx), static_cast<int>(wz), 4242);

            // Trees (Only placed on legitimate soil/sand with spacing and air clearance)
            if (decorRand < biome.treeDensity && biome.logBlock != 0) {
                if (isValidTreeSoil(surfBlock, biomeId)) {
                    // Tree spacing check: ensure healthy distance between tree trunks
                    bool tooClose = false;
                    for (const auto& pt : placedTrees) {
                        int dx = pt.first - x;
                        int dz = pt.second - z;
                        if (dx * dx + dz * dz < 9) { // At least 3 blocks apart
                            tooClose = true;
                            break;
                        }
                    }

                    // Overhead air clearance check: trunk space must be completely empty air
                    bool hasAirClearance = true;
                    for (int cy = 1; cy <= 5; ++cy) {
                        if (surfaceY + cy >= CHUNK_Y || chunk->getBlock(x, surfaceY + cy, z) != 0) {
                            hasAirClearance = false;
                            break;
                        }
                    }

                    if (!tooClose && hasAirClearance) {
                        placedTrees.emplace_back(x, z);
                        placeBiomeTree(chunk, x, surfaceY + 1, z, biomeId, biome.logBlock, biome.leafBlock);
                    }
                }
            }
            // Surface Rock Boulders
            else if (decorRand > 0.968f) {
                if (chunk->getBlock(x, surfaceY + 1, z) == 0) {
                    uint16_t boulderStone = (biome.roughness > 10.0f) ? 39 : 38; // Granite or Mossy Cobble
                    placeBoulder(chunk, x, surfaceY + 1, z, boulderStone);
                }
            }
            // Fallen Tree Logs
            else if (decorRand > 0.948f && decorRand <= 0.968f && biome.logBlock != 0) {
                if (chunk->getBlock(x, surfaceY + 1, z) == 0) {
                    placeFallenLog(chunk, x, surfaceY + 1, z, biome.logBlock);
                }
            }
            // Elemental Crystal Spikes
            else if (decorRand > (1.0f - biome.crystalDensity) && biome.crystalDensity > 0.0f) {
                if (chunk->getBlock(x, surfaceY + 1, z) == 0) {
                    uint16_t crystalId = 186 + (biomeId % 20);
                    placeCrystal(chunk, x, surfaceY + 1, z, crystalId);
                }
            }
            // Wild Mushrooms
            else if (decorRand > 0.930f && decorRand <= 0.945f) {
                uint16_t mushId = (decorRand > 0.938f) ? 119 : 120;
                if (chunk->getBlock(x, surfaceY + 1, z) == 0) {
                    chunk->setBlock(x, surfaceY + 1, z, mushId);
                }
            }
        }
    }

    // Procedural RPG Structures (Shrines, Towers, Cottages, Pyramids, Catacombs, Mines)
    StructureGenerator::generateStructures(chunk, seed);
}

// -----------------------------------------------------------------------------
// DIVERSE PROCEDURAL TREE PLACEMENT DISPATCHER
// -----------------------------------------------------------------------------
void WorldGenerator::placeBiomeTree(Chunk* chunk, int localX, int localY, int localZ, uint8_t biomeId, uint16_t logId, uint16_t leafId) {
    switch (biomeId) {
        case 3:  // Golden Autumn Grove
            placeMapleTree(chunk, localX, localY, localZ, logId, leafId);
            break;
        case 4:  // Silver Birch Woods
            placeBirchTree(chunk, localX, localY, localZ, logId, leafId);
            break;
        case 5:  // Snowy Pine Taiga
        case 6:  // Frostbite Tundra
        case 17: // Granite Highlands
        case 30: // Windblown Steppes
            placeSpruceTree(chunk, localX, localY, localZ, logId, leafId);
            break;
        case 10: // Golden Savannah
            placeAcaciaTree(chunk, localX, localY, localZ, logId, leafId);
            break;
        case 11: // Lush Tropical Jungle
            placeJungleTree(chunk, localX, localY, localZ, logId, leafId);
            break;
        case 12: // Tranquil Bamboo Basin
            placeBambooClump(chunk, localX, localY, localZ, logId, leafId);
            break;
        case 13: // Murky Bayou Swamp
        case 14: // Mangrove Estuary
            placeSwampWillow(chunk, localX, localY, localZ, logId, leafId);
            break;
        case 15: // Coral Coast
        case 27: // Desert Oasis
            placePalmTree(chunk, localX, localY, localZ, logId, leafId);
            break;
        case 19: // Giant Fungal Isle
            placeGiantMushroom(chunk, localX, localY, localZ, 74, (localX % 2 == 0) ? 119 : 120);
            break;
        case 20: // Cherry Blossom Sanctuary
            placeCherryTree(chunk, localX, localY, localZ, logId, leafId);
            break;
        default: // Verdant Plains, Ancient Oak Forest, etc.
            placeOakTree(chunk, localX, localY, localZ, logId, leafId);
            break;
    }
}

// 1. Broad Oak Tree: Natural rounded spherical crown with branch offsets
void WorldGenerator::placeOakTree(Chunk* chunk, int localX, int localY, int localZ, uint16_t logId, uint16_t leafId) {
    int trunkHeight = 5 + ((localX + localZ) % 2);

    for (int y = 0; y < trunkHeight; ++y) {
        chunk->setBlock(localX, localY + y, localZ, logId);
    }

    int crownBase = localY + trunkHeight - 2;
    for (int dy = -2; dy <= 2; ++dy) {
        int r = (dy == -2 || dy == 2) ? 1 : 2;
        int py = crownBase + dy + 2;
        for (int dx = -r; dx <= r; ++dx) {
            for (int dz = -r; dz <= r; ++dz) {
                if (std::abs(dx) == r && std::abs(dz) == r && (dy == -2 || dy == 2)) continue;
                if (dx == 0 && dz == 0 && dy < 0) continue; // Trunk space

                int tx = localX + dx;
                int tz = localZ + dz;
                if (tx >= 0 && tx < CHUNK_X && py >= 0 && py < CHUNK_Y && tz >= 0 && tz < CHUNK_Z) {
                    if (chunk->getBlock(tx, py, tz) == 0) {
                        chunk->setBlock(tx, py, tz, leafId);
                    }
                }
            }
        }
    }
}

// 2. Slender Birch Tree: Tall slender trunk with compact airy canopy
void WorldGenerator::placeBirchTree(Chunk* chunk, int localX, int localY, int localZ, uint16_t logId, uint16_t leafId) {
    int trunkHeight = 7 + (localX % 3);

    for (int y = 0; y < trunkHeight; ++y) {
        chunk->setBlock(localX, localY + y, localZ, logId);
    }

    int crownTop = localY + trunkHeight;
    for (int dy = -3; dy <= 1; ++dy) {
        int r = (dy == 1 || dy == -3) ? 1 : 2;
        int py = crownTop + dy;
        for (int dx = -r; dx <= r; ++dx) {
            for (int dz = -r; dz <= r; ++dz) {
                if (std::abs(dx) == r && std::abs(dz) == r && (dy == 1 || dy == -3)) continue;
                int tx = localX + dx;
                int tz = localZ + dz;
                if (tx >= 0 && tx < CHUNK_X && py >= 0 && py < CHUNK_Y && tz >= 0 && tz < CHUNK_Z) {
                    if (chunk->getBlock(tx, py, tz) == 0) {
                        chunk->setBlock(tx, py, tz, leafId);
                    }
                }
            }
        }
    }
}

// 3. Conical Spruce / Pine Tree: Tiered evergreen rings tapering to a sharp needle crown
void WorldGenerator::placeSpruceTree(Chunk* chunk, int localX, int localY, int localZ, uint16_t logId, uint16_t leafId) {
    int trunkHeight = 8 + ((localX * 3 + localZ) % 3);

    for (int y = 0; y < trunkHeight; ++y) {
        chunk->setBlock(localX, localY + y, localZ, logId);
    }

    int topY = localY + trunkHeight;
    // Needle point at top
    if (topY < CHUNK_Y) chunk->setBlock(localX, topY, localZ, leafId);
    if (topY + 1 < CHUNK_Y) chunk->setBlock(localX, topY + 1, localZ, leafId);

    // Tiered rings: alternating radii
    int ringTiers = trunkHeight - 2;
    for (int dy = 1; dy <= ringTiers; ++dy) {
        int py = topY - dy;
        int r = ((dy % 2 == 1) ? 1 : 2);
        if (dy >= ringTiers - 1) r = 2; // Bottom-most ring is wide

        for (int dx = -r; dx <= r; ++dx) {
            for (int dz = -r; dz <= r; ++dz) {
                if (std::abs(dx) == r && std::abs(dz) == r && dy % 2 == 1) continue;
                int tx = localX + dx;
                int tz = localZ + dz;
                if (tx >= 0 && tx < CHUNK_X && py >= 0 && py < CHUNK_Y && tz >= 0 && tz < CHUNK_Z) {
                    if (chunk->getBlock(tx, py, tz) == 0) {
                        chunk->setBlock(tx, py, tz, leafId);
                    }
                }
            }
        }
    }
}

// 4. Towering Jungle Mega-Tree: Tall massive trunk with wide umbrella canopy
void WorldGenerator::placeJungleTree(Chunk* chunk, int localX, int localY, int localZ, uint16_t logId, uint16_t leafId) {
    int trunkHeight = 10 + (localZ % 4);

    for (int y = 0; y < trunkHeight; ++y) {
        chunk->setBlock(localX, localY + y, localZ, logId);
    }

    int topY = localY + trunkHeight;
    for (int dy = -1; dy <= 2; ++dy) {
        int r = (dy == 2) ? 1 : (dy == 1 ? 3 : 2);
        int py = topY + dy;
        for (int dx = -r; dx <= r; ++dx) {
            for (int dz = -r; dz <= r; ++dz) {
                if (std::abs(dx) == r && std::abs(dz) == r && dy != 1) continue;
                int tx = localX + dx;
                int tz = localZ + dz;
                if (tx >= 0 && tx < CHUNK_X && py >= 0 && py < CHUNK_Y && tz >= 0 && tz < CHUNK_Z) {
                    if (chunk->getBlock(tx, py, tz) == 0) {
                        chunk->setBlock(tx, py, tz, leafId);
                    }
                }
            }
        }
    }
}

// 5. Cherry Blossom Tree: Branching curved trunk with wide cascading pink blossom canopy
void WorldGenerator::placeCherryTree(Chunk* chunk, int localX, int localY, int localZ, uint16_t logId, uint16_t leafId) {
    int trunkHeight = 5;

    for (int y = 0; y < trunkHeight; ++y) {
        chunk->setBlock(localX, localY + y, localZ, logId);
    }
    // Curved branch
    int bx = localX + 1;
    if (bx < CHUNK_X) {
        chunk->setBlock(bx, localY + trunkHeight - 1, localZ, logId);
        chunk->setBlock(bx, localY + trunkHeight, localZ, logId);
    }

    int topY = localY + trunkHeight;
    for (int dy = -1; dy <= 2; ++dy) {
        int r = (dy == 2) ? 1 : 2;
        int py = topY + dy;
        for (int dx = -r; dx <= r; ++dx) {
            for (int dz = -r; dz <= r; ++dz) {
                if (std::abs(dx) == r && std::abs(dz) == r && dy >= 1) continue;
                int tx = localX + dx;
                int tz = localZ + dz;
                if (tx >= 0 && tx < CHUNK_X && py >= 0 && py < CHUNK_Y && tz >= 0 && tz < CHUNK_Z) {
                    if (chunk->getBlock(tx, py, tz) == 0) {
                        chunk->setBlock(tx, py, tz, leafId);
                    }
                }
            }
        }
    }
}

// 6. Autumn Maple Tree: Rounded golden-orange canopy
void WorldGenerator::placeMapleTree(Chunk* chunk, int localX, int localY, int localZ, uint16_t logId, uint16_t leafId) {
    placeOakTree(chunk, localX, localY, localZ, logId, leafId);
}

// 7. Savannah Acacia Tree: Angled diagonal branch with flat umbrella foliage
void WorldGenerator::placeAcaciaTree(Chunk* chunk, int localX, int localY, int localZ, uint16_t logId, uint16_t leafId) {
    int trunkHeight = 5;
    for (int y = 0; y < 3; ++y) {
        chunk->setBlock(localX, localY + y, localZ, logId);
    }
    // Diagonal branches
    int b1x = std::min(localX + 1, CHUNK_X - 1);
    int b2x = std::max(localX - 1, 0);
    chunk->setBlock(b1x, localY + 3, localZ, logId);
    chunk->setBlock(b1x, localY + 4, localZ, logId);
    chunk->setBlock(b2x, localY + 3, localZ, logId);

    // Flat umbrella disc
    int topY = localY + 5;
    for (int dx = -2; dx <= 2; ++dx) {
        for (int dz = -2; dz <= 2; ++dz) {
            if (std::abs(dx) == 2 && std::abs(dz) == 2) continue;
            int tx = localX + dx;
            int tz = localZ + dz;
            if (tx >= 0 && tx < CHUNK_X && topY < CHUNK_Y && tz >= 0 && tz < CHUNK_Z) {
                if (chunk->getBlock(tx, topY, tz) == 0) {
                    chunk->setBlock(tx, topY, tz, leafId);
                }
            }
        }
    }
}

// 8. Swamp Willow: Weeping cascading fronds
void WorldGenerator::placeSwampWillow(Chunk* chunk, int localX, int localY, int localZ, uint16_t logId, uint16_t leafId) {
    int trunkHeight = 6;
    for (int y = 0; y < trunkHeight; ++y) {
        chunk->setBlock(localX, localY + y, localZ, logId);
    }

    int topY = localY + trunkHeight;
    for (int dy = -3; dy <= 1; ++dy) {
        int r = (dy == 1) ? 1 : 2;
        int py = topY + dy;
        for (int dx = -r; dx <= r; ++dx) {
            for (int dz = -r; dz <= r; ++dz) {
                if (std::abs(dx) == r && std::abs(dz) == r && dy == 1) continue;
                int tx = localX + dx;
                int tz = localZ + dz;
                if (tx >= 0 && tx < CHUNK_X && py >= 0 && py < CHUNK_Y && tz >= 0 && tz < CHUNK_Z) {
                    if (chunk->getBlock(tx, py, tz) == 0) {
                        chunk->setBlock(tx, py, tz, leafId);
                    }
                }
            }
        }
    }
}

// 9. Giant Mushroom: Thick stalk with wide dome cap
void WorldGenerator::placeGiantMushroom(Chunk* chunk, int localX, int localY, int localZ, uint16_t stemId, uint16_t capId) {
    int stemHeight = 5 + (localX % 2);
    for (int y = 0; y < stemHeight; ++y) {
        chunk->setBlock(localX, localY + y, localZ, stemId);
    }

    int capY = localY + stemHeight;
    for (int dx = -2; dx <= 2; ++dx) {
        for (int dz = -2; dz <= 2; ++dz) {
            int tx = localX + dx;
            int tz = localZ + dz;
            if (tx >= 0 && tx < CHUNK_X && capY < CHUNK_Y && tz >= 0 && tz < CHUNK_Z) {
                chunk->setBlock(tx, capY, tz, capId);
                // Beveled cap rim
                if ((std::abs(dx) == 2 || std::abs(dz) == 2) && capY - 1 >= 0) {
                    chunk->setBlock(tx, capY - 1, tz, capId);
                }
            }
        }
    }
}

// 10. Bamboo Clump: Slender segmented culms
void WorldGenerator::placeBambooClump(Chunk* chunk, int localX, int localY, int localZ, uint16_t stalkId, uint16_t leafId) {
    int stalkHeight = 6 + (localZ % 5);
    for (int y = 0; y < stalkHeight; ++y) {
        chunk->setBlock(localX, localY + y, localZ, stalkId);
    }
    if (localY + stalkHeight < CHUNK_Y) {
        chunk->setBlock(localX, localY + stalkHeight, localZ, leafId);
    }
}

// 11. Palm Tree: Slanted desert trunk with spreading fronds
void WorldGenerator::placePalmTree(Chunk* chunk, int localX, int localY, int localZ, uint16_t logId, uint16_t leafId) {
    int trunkHeight = 6;
    for (int y = 0; y < trunkHeight; ++y) {
        int ox = (y >= 3) ? 1 : 0;
        int tx = std::min(localX + ox, CHUNK_X - 1);
        chunk->setBlock(tx, localY + y, localZ, logId);
    }

    int topY = localY + trunkHeight;
    int cx = std::min(localX + 1, CHUNK_X - 1);
    static const int palmOffsets[8][2] = {
        {1,0}, {-1,0}, {0,1}, {0,-1}, {2,0}, {-2,0}, {0,2}, {0,-2}
    };
    for (int i = 0; i < 8; ++i) {
        int px = cx + palmOffsets[i][0];
        int pz = localZ + palmOffsets[i][1];
        int py = topY + (i >= 4 ? -1 : 0);
        if (px >= 0 && px < CHUNK_X && py >= 0 && py < CHUNK_Y && pz >= 0 && pz < CHUNK_Z) {
            chunk->setBlock(px, py, pz, leafId);
        }
    }
}

// -----------------------------------------------------------------------------
// SURFACE FEATURE PLACERS (Logs, Boulders, Crystals)
// -----------------------------------------------------------------------------
void WorldGenerator::placeFallenLog(Chunk* chunk, int localX, int localY, int localZ, uint16_t logId) {
    for (int dx = 0; dx < 3; ++dx) {
        int tx = localX + dx;
        if (tx < CHUNK_X) {
            if (chunk->getBlock(tx, localY, localZ) == 0) {
                chunk->setBlock(tx, localY, localZ, logId);
            }
        }
    }
}

void WorldGenerator::placeBoulder(Chunk* chunk, int localX, int localY, int localZ, uint16_t stoneId) {
    chunk->setBlock(localX, localY, localZ, stoneId);
    if (localX + 1 < CHUNK_X) chunk->setBlock(localX + 1, localY, localZ, stoneId);
    if (localZ + 1 < CHUNK_Z) chunk->setBlock(localX, localY, localZ + 1, stoneId);
    if (localY + 1 < CHUNK_Y) chunk->setBlock(localX, localY + 1, localZ, stoneId);
}

void WorldGenerator::placeCrystal(Chunk* chunk, int localX, int localY, int localZ, uint16_t crystalId) {
    chunk->setBlock(localX, localY, localZ, crystalId);
    if (localY + 1 < CHUNK_Y) {
        chunk->setBlock(localX, localY + 1, localZ, crystalId);
    }
}

} // namespace Aetheria
