#include "StructureGenerator.hpp"
#include "Chunk.hpp"
#include "BiomeRegistry.hpp"
#include <cmath>
#include <cstdlib>

namespace Aetheria {

void StructureGenerator::setBlockSafe(Chunk* chunk, int x, int y, int z, uint16_t blockId) {
    if (x >= 0 && x < CHUNK_X && y >= 0 && y < CHUNK_Y && z >= 0 && z < CHUNK_Z) {
        chunk->setBlock(x, y, z, blockId);
    }
}

void StructureGenerator::generateStructures(Chunk* chunk, uint32_t seed) {
    int cx = chunk->getChunkX();
    int cz = chunk->getChunkZ();

    // Deterministic pseudo-random hash per chunk
    uint32_t h = (static_cast<uint32_t>(cx * 374761393) ^ static_cast<uint32_t>(cz * 668265263) ^ seed);
    h = (h ^ (h >> 13)) * 1274126177;

    // Check if this chunk is eligible for a surface structure (~1 in 6 chunks)
    bool hasSurfaceStructure = ((h % 6) == 0);
    // Check if this chunk has an underground crypt/mineshaft (~1 in 5 chunks)
    bool hasUndergroundStructure = (((h >> 8) % 5) == 0);

    // Find surface height in chunk center
    int midX = CHUNK_X / 2;
    int midZ = CHUNK_Z / 2;
    int surfaceY = 32;
    for (int y = CHUNK_Y - 5; y >= 5; --y) {
        if (chunk->getBlock(midX, y, midZ) != 0) {
            surfaceY = y;
            break;
        }
    }

    if (surfaceY > 15 && surfaceY < CHUNK_Y - 20) {
        float wx = static_cast<float>(cx * CHUNK_X + midX);
        float wz = static_cast<float>(cz * CHUNK_Z + midZ);

        // Approximate biome classification based on coordinates
        int biomeHint = static_cast<int>(std::abs(std::sin(wx * 0.004f) * 10.0f + std::cos(wz * 0.004f) * 10.0f)) % 35;

        if (hasSurfaceStructure) {
            uint32_t typePick = (h >> 4) % 4;

            if (biomeHint == 10 || biomeHint == 11 || biomeHint == 12) { // Desert / Dunes
                buildDesertPyramid(chunk, 2, surfaceY, 2);
            } else if (typePick == 0) {
                buildAncientShrine(chunk, 4, surfaceY, 4);
            } else if (typePick == 1) {
                buildWoodlandCottage(chunk, 4, surfaceY, 4);
            } else if (typePick == 2) {
                buildWizardTower(chunk, 5, surfaceY, 5);
            } else {
                buildAncientShrine(chunk, 4, surfaceY, 4);
            }
        }
    }

    // Underground structures (Crypts and Mineshafts)
    if (hasUndergroundStructure) {
        uint32_t uPick = (h >> 16) % 2;
        if (uPick == 0) {
            buildCatacombCrypt(chunk, 4, 18, 4);
        } else {
            buildMineshaftOutpost(chunk, 4, 28, 4);
        }
    }
}

void StructureGenerator::buildAncientShrine(Chunk* chunk, int startX, int surfaceY, int startZ) {
    // 7x7 Stone Brick Platform
    for (int dx = 0; dx < 7; ++dx) {
        for (int dz = 0; dz < 7; ++dz) {
            setBlockSafe(chunk, startX + dx, surfaceY, startZ + dz, 39); // Stone Bricks
            // Clear air above
            for (int dy = 1; dy <= 6; ++dy) {
                setBlockSafe(chunk, startX + dx, surfaceY + dy, startZ + dz, 0);
            }
        }
    }

    // 4 Corner Pillars (Height 4)
    int pillars[4][2] = { {1, 1}, {1, 5}, {5, 1}, {5, 5} };
    for (auto& p : pillars) {
        int px = startX + p[0];
        int pz = startZ + p[1];
        for (int dy = 1; dy <= 4; ++dy) {
            setBlockSafe(chunk, px, surfaceY + dy, pz, 241); // Chiseled Brick Pillar
        }
        setBlockSafe(chunk, px, surfaceY + 5, pz, 551); // Torch on top
    }

    // Crossbeams linking pillars at top
    for (int i = 1; i <= 5; ++i) {
        setBlockSafe(chunk, startX + 1, surfaceY + 4, startZ + i, 39);
        setBlockSafe(chunk, startX + 5, surfaceY + 4, startZ + i, 39);
        setBlockSafe(chunk, startX + i, surfaceY + 4, startZ + 1, 39);
        setBlockSafe(chunk, startX + i, surfaceY + 4, startZ + 5, 39);
    }

    // Center Altar with Elemental Crystal
    setBlockSafe(chunk, startX + 3, surfaceY + 1, startZ + 3, 206); // Elder Rune Stone
    setBlockSafe(chunk, startX + 3, surfaceY + 2, startZ + 3, 186); // Glowing Radiant Crystal
    setBlockSafe(chunk, startX + 3, surfaceY + 1, startZ + 2, 333); // Relic Chest
}

void StructureGenerator::buildWoodlandCottage(Chunk* chunk, int startX, int surfaceY, int startZ) {
    int w = 8, d = 7, h = 4;

    // Floor (Oak Planks)
    for (int dx = 0; dx < w; ++dx) {
        for (int dz = 0; dz < d; ++dz) {
            setBlockSafe(chunk, startX + dx, surfaceY, startZ + dz, 72); // Oak Planks
            for (int dy = 1; dy <= 6; ++dy) {
                setBlockSafe(chunk, startX + dx, surfaceY + dy, startZ + dz, 0); // Clear interior air
            }
        }
    }

    // Corner Log Pillars
    for (int dy = 1; dy <= h; ++dy) {
        setBlockSafe(chunk, startX, surfaceY + dy, startZ, 71); // Oak Log
        setBlockSafe(chunk, startX + w - 1, surfaceY + dy, startZ, 71);
        setBlockSafe(chunk, startX, surfaceY + dy, startZ + d - 1, 71);
        setBlockSafe(chunk, startX + w - 1, surfaceY + dy, startZ + d - 1, 71);
    }

    // Walls (Oak Planks with Glass Windows)
    for (int dx = 1; dx < w - 1; ++dx) {
        for (int dy = 1; dy <= h; ++dy) {
            // Front wall has door opening at dx = 3, 4
            if ((dx == 3 || dx == 4) && (dy <= 2)) {
                setBlockSafe(chunk, startX + dx, surfaceY + dy, startZ, 0); // Doorway
            } else if (dy == 2 && (dx == 1 || dx == 6)) {
                setBlockSafe(chunk, startX + dx, surfaceY + dy, startZ, 281); // Glass window
            } else {
                setBlockSafe(chunk, startX + dx, surfaceY + dy, startZ, 72); // Plank wall
            }

            // Back wall
            if (dy == 2 && (dx == 2 || dx == 5)) {
                setBlockSafe(chunk, startX + dx, surfaceY + dy, startZ + d - 1, 281);
            } else {
                setBlockSafe(chunk, startX + dx, surfaceY + dy, startZ + d - 1, 72);
            }
        }
    }

    // Side Walls
    for (int dz = 1; dz < d - 1; ++dz) {
        for (int dy = 1; dy <= h; ++dy) {
            setBlockSafe(chunk, startX, surfaceY + dy, startZ + dz, 72);
            setBlockSafe(chunk, startX + w - 1, surfaceY + dy, startZ + dz, 72);
        }
    }

    // Pitch Roof (Oak Planks)
    for (int dx = 0; dx < w; ++dx) {
        setBlockSafe(chunk, startX + dx, surfaceY + h + 1, startZ + 1, 72);
        setBlockSafe(chunk, startX + dx, surfaceY + h + 1, startZ + d - 2, 72);
        setBlockSafe(chunk, startX + dx, surfaceY + h + 2, startZ + 2, 72);
        setBlockSafe(chunk, startX + dx, surfaceY + h + 2, startZ + d - 3, 72);
        setBlockSafe(chunk, startX + dx, surfaceY + h + 3, startZ + 3, 72);
    }

    // Hearth & Cobblestone Chimney (with Furnace & Crafting Table!)
    for (int dy = 1; dy <= h + 4; ++dy) {
        setBlockSafe(chunk, startX + 1, surfaceY + dy, startZ + d - 2, 37); // Cobblestone chimney
    }
    setBlockSafe(chunk, startX + 2, surfaceY + 1, startZ + d - 2, 332); // Furnace (Ocak)!
    setBlockSafe(chunk, startX + 2, surfaceY + 1, startZ + 1, 331);     // Crafting Table!
    setBlockSafe(chunk, startX + w - 2, surfaceY + 1, startZ + d - 2, 333); // Chest!
    setBlockSafe(chunk, startX + 3, surfaceY + 3, startZ + 1, 551);     // Torch on wall
}

void StructureGenerator::buildDesertPyramid(Chunk* chunk, int startX, int surfaceY, int startZ) {
    int baseSize = 11;
    for (int step = 0; step < 5; ++step) {
        int curSize = baseSize - step * 2;
        int off = step;
        int y = surfaceY + step;

        for (int dx = 0; dx < curSize; ++dx) {
            for (int dz = 0; dz < curSize; ++dz) {
                int bx = startX + off + dx;
                int bz = startZ + off + dz;
                // Hollow inner chamber on bottom 2 steps
                if (step <= 2 && dx >= 2 && dx < curSize - 2 && dz >= 2 && dz < curSize - 2) {
                    setBlockSafe(chunk, bx, y, bz, 0); // Air inside chamber
                } else {
                    setBlockSafe(chunk, bx, y, bz, 24); // Sandstone
                }
            }
        }
    }

    // Pyramid Burial Sanctum: Gold Blocks & Relic Chest
    setBlockSafe(chunk, startX + 5, surfaceY + 1, startZ + 5, 126); // Gold Block!
    setBlockSafe(chunk, startX + 5, surfaceY + 2, startZ + 5, 333); // Chest!
    setBlockSafe(chunk, startX + 4, surfaceY + 1, startZ + 5, 207); // Carved Sandstone Rune
    setBlockSafe(chunk, startX + 6, surfaceY + 1, startZ + 5, 207);
    setBlockSafe(chunk, startX + 5, surfaceY + 1, startZ + 4, 207);
    setBlockSafe(chunk, startX + 5, surfaceY + 1, startZ + 6, 207);
}

void StructureGenerator::buildWizardTower(Chunk* chunk, int startX, int surfaceY, int startZ) {
    int towerH = 12;

    // 6x6 Round-corner Tower
    for (int dy = 0; dy <= towerH; ++dy) {
        int y = surfaceY + dy;
        for (int dx = 0; dx < 6; ++dx) {
            for (int dz = 0; dz < 6; ++dz) {
                // Cut corners for circular silhouette
                if ((dx == 0 || dx == 5) && (dz == 0 || dz == 5)) continue;

                int bx = startX + dx;
                int bz = startZ + dz;

                bool isWall = (dx == 0 || dx == 5 || dz == 0 || dz == 5);

                if (dy == 0) {
                    setBlockSafe(chunk, bx, y, bz, 39); // Stone Brick floor
                } else if (dy == 5 || dy == 10) {
                    setBlockSafe(chunk, bx, y, bz, 72); // Intermediate wooden floors
                } else if (isWall) {
                    if (dy == 2 && dx == 2 && dz == 0) {
                        setBlockSafe(chunk, bx, y, bz, 0); // Entrance
                    } else if ((dy == 3 || dy == 8) && (dx == 2 || dx == 3) && (dz == 0 || dz == 5)) {
                        setBlockSafe(chunk, bx, y, bz, 281); // Stained glass window
                    } else {
                        setBlockSafe(chunk, bx, y, bz, 39); // Stone Bricks
                    }
                } else {
                    setBlockSafe(chunk, bx, y, bz, 0); // Interior air
                }
            }
        }
    }

    // Top Battlements & Arcane Crystal Spire
    int topY = surfaceY + towerH + 1;
    for (int dx = 0; dx < 6; ++dx) {
        for (int dz = 0; dz < 6; ++dz) {
            if ((dx == 0 || dx == 5) && (dz == 0 || dz == 5)) continue;
            int bx = startX + dx;
            int bz = startZ + dz;
            if (dx == 0 || dx == 5 || dz == 0 || dz == 5) {
                if ((dx + dz) % 2 == 0) {
                    setBlockSafe(chunk, bx, topY, bz, 39); // Battlement parapet
                }
            }
        }
    }

    // Arcane Spire in center of roof
    setBlockSafe(chunk, startX + 2, topY, startZ + 2, 206);
    setBlockSafe(chunk, startX + 3, topY, startZ + 2, 206);
    setBlockSafe(chunk, startX + 2, topY, startZ + 3, 206);
    setBlockSafe(chunk, startX + 3, topY, startZ + 3, 206);
    setBlockSafe(chunk, startX + 2, topY + 1, startZ + 2, 191); // Arcane Crystal Beacon
    setBlockSafe(chunk, startX + 2, topY + 2, startZ + 2, 191);
}

void StructureGenerator::buildCatacombCrypt(Chunk* chunk, int startX, int depthY, int startZ) {
    int w = 7, d = 6, h = 4;

    // Hollow out room
    for (int dx = 0; dx < w; ++dx) {
        for (int dz = 0; dz < d; ++dz) {
            for (int dy = 0; dy <= h; ++dy) {
                int bx = startX + dx;
                int by = depthY + dy;
                int bz = startZ + dz;

                if (dy == 0) {
                    setBlockSafe(chunk, bx, by, bz, 45); // Deepslate Floor
                } else if (dy == h) {
                    setBlockSafe(chunk, bx, by, bz, 45); // Deepslate Ceiling
                } else if (dx == 0 || dx == w - 1 || dz == 0 || dz == d - 1) {
                    setBlockSafe(chunk, bx, by, bz, 242); // Catacomb Bricks
                } else {
                    setBlockSafe(chunk, bx, by, bz, 0); // Air
                }
            }
        }
    }

    // Iron Bar alcoves and Bone Slabs
    setBlockSafe(chunk, startX + 1, depthY + 1, startZ + 2, 245); // Bone Block Sarcophagus
    setBlockSafe(chunk, startX + 1, depthY + 1, startZ + 3, 245);
    setBlockSafe(chunk, startX + 1, depthY + 2, startZ + 2, 333); // Relic Chest
    setBlockSafe(chunk, startX + w - 2, depthY + 1, startZ + 2, 243); // Iron Bars
    setBlockSafe(chunk, startX + w - 2, depthY + 1, startZ + 3, 243);
    setBlockSafe(chunk, startX + 3, depthY + 2, startZ + d - 2, 551); // Crypt Torch
}

void StructureGenerator::buildMineshaftOutpost(Chunk* chunk, int startX, int depthY, int startZ) {
    // Hollow shaft corridor
    for (int dz = 0; dz < 10; ++dz) {
        for (int dx = 0; dx < 4; ++dx) {
            for (int dy = 0; dy < 4; ++dy) {
                int bx = startX + dx;
                int by = depthY + dy;
                int bz = startZ + dz;
                if (dy == 0) {
                    setBlockSafe(chunk, bx, by, bz, 72); // Oak Plank flooring
                } else {
                    setBlockSafe(chunk, bx, by, bz, 0);  // Tunnel Air
                }
            }
        }
        // Support Arch every 3 blocks
        if (dz % 3 == 0) {
            setBlockSafe(chunk, startX, depthY + 1, startZ + dz, 71); // Oak Log Posts
            setBlockSafe(chunk, startX, depthY + 2, startZ + dz, 71);
            setBlockSafe(chunk, startX + 3, depthY + 1, startZ + dz, 71);
            setBlockSafe(chunk, startX + 3, depthY + 2, startZ + dz, 71);
            setBlockSafe(chunk, startX + 1, depthY + 3, startZ + dz, 72); // Oak Plank Crossbeam
            setBlockSafe(chunk, startX + 2, depthY + 3, startZ + dz, 72);
            setBlockSafe(chunk, startX + 1, depthY + 2, startZ + dz, 551); // Torch
        }
    }

    // Exposed Ore Vein at end of tunnel
    setBlockSafe(chunk, startX + 1, depthY + 1, startZ + 9, 124); // Iron Ore
    setBlockSafe(chunk, startX + 2, depthY + 1, startZ + 9, 126); // Gold Ore
    setBlockSafe(chunk, startX + 1, depthY + 2, startZ + 9, 121); // Coal Ore
    setBlockSafe(chunk, startX + 2, depthY + 1, startZ + 8, 333); // Abandoned Mine Chest
}

} // namespace Aetheria
