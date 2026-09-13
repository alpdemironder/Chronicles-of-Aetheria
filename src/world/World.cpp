#include "World.hpp"
#include "../core/Settings.hpp"
#include <cmath>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <cstdio>

namespace Aetheria {

World::World(uint32_t seed)
    : worldSeed(seed),
      generator(seed),
      tempNoise(seed + 101),
      moistureNoise(seed + 202),
      elevNoise(seed) {
    int initRadius = std::min(Settings::instance().graphics.renderDistance, 8);
    loadChunksAround(0, 0, initRadius);
}

World::~World() {
    chunks.clear();
}

void World::loadChunksAround(int centerChunkX, int centerChunkZ, int radius) {
    radius = std::clamp(radius, 2, 128);
    // Expand chunks outwards from center in concentric rings
    for (int r = 0; r <= radius; ++r) {
        for (int dx = -r; dx <= r; ++dx) {
            for (int dz = -r; dz <= r; ++dz) {
                if (std::max(std::abs(dx), std::abs(dz)) != r) continue;
                int cx = centerChunkX + dx;
                int cz = centerChunkZ + dz;
                auto key = std::make_pair(cx, cz);
                if (chunks.find(key) == chunks.end()) {
                    auto chunk = std::make_unique<Chunk>(cx, cz);
                    generator.generateChunk(chunk.get());
                    chunks[key] = std::move(chunk);

                    // Mark existing adjacent neighbors dirty so boundary faces update seamlessly
                    Chunk* l = getChunk(cx - 1, cz); if (l) l->setModified(true);
                    Chunk* r = getChunk(cx + 1, cz); if (r) r->setModified(true);
                    Chunk* b = getChunk(cx, cz - 1); if (b) b->setModified(true);
                    Chunk* f = getChunk(cx, cz + 1); if (f) f->setModified(true);
                }
            }
        }
    }
}

void World::update(const Vec3& playerPos, float dt) {
    // Advance day/night cycle
    timeOfDay += dt / dayCycleDuration;
    if (timeOfDay >= 1.0f) timeOfDay -= 1.0f;
    else if (timeOfDay < 0.0f) timeOfDay += 1.0f;

    int curChunkX = static_cast<int>(std::floor(playerPos.x / static_cast<float>(CHUNK_X)));
    int curChunkZ = static_cast<int>(std::floor(playerPos.z / static_cast<float>(CHUNK_Z)));

    int curRD = std::clamp(Settings::instance().graphics.renderDistance, 2, 128);

    if (curChunkX != lastPlayerChunkX || curChunkZ != lastPlayerChunkZ) {
        lastPlayerChunkX = curChunkX;
        lastPlayerChunkZ = curChunkZ;
        loadChunksAround(curChunkX, curChunkZ, curRD);

        // Cull distant chunks beyond render distance to preserve system memory
        if (chunks.size() > 256) {
            int cullDist = curRD + 4;
            for (auto it = chunks.begin(); it != chunks.end();) {
                int dx = std::abs(it->first.first - curChunkX);
                int dz = std::abs(it->first.second - curChunkZ);
                if (dx > cullDist || dz > cullDist) {
                    it = chunks.erase(it);
                } else {
                    ++it;
                }
            }
        }
    }

    // Update dirty chunk meshes (scaled dynamically with render distance)
    int maxMeshes = std::clamp(curRD * 2, 4, 24);
    int meshesUpdated = 0;
    for (auto& pair : chunks) {
        if (pair.second->getIsDirty()) {
            int cx = pair.first.first;
            int cz = pair.first.second;

            Chunk* left = getChunk(cx - 1, cz);
            Chunk* right = getChunk(cx + 1, cz);
            Chunk* back = getChunk(cx, cz - 1);
            Chunk* front = getChunk(cx, cz + 1);

            pair.second->updateMesh(left, right, back, front);
            meshesUpdated++;
            if (meshesUpdated >= maxMeshes) break;
        }
    }

    // Update planted saplings (maturation into procedural trees)
    for (size_t i = 0; i < saplings.size(); ) {
        auto& s = saplings[i];
        uint16_t currentBlock = getBlock(s.x, s.y, s.z);
        if (currentBlock != s.saplingId) {
            // Sapling broken or replaced
            saplings.erase(saplings.begin() + i);
            continue;
        }

        s.growthTimer -= dt;
        if (s.growthTimer <= 0.0f) {
            int sx = s.x;
            int sy = s.y;
            int sz = s.z;
            uint16_t sid = s.saplingId;
            saplings.erase(saplings.begin() + i);
            growTree(sx, sy, sz, sid);
        } else {
            ++i;
        }
    }
}

void World::render() {
    for (auto& pair : chunks) {
        pair.second->render();
    }
}

Chunk* World::getChunk(int chunkX, int chunkZ) const {
    auto it = chunks.find(std::make_pair(chunkX, chunkZ));
    if (it != chunks.end()) {
        return it->second.get();
    }
    return nullptr;
}

uint16_t World::getBlock(int x, int y, int z) const {
    if (y < 0 || y >= CHUNK_Y) return 0;

    int cx = static_cast<int>(std::floor(static_cast<float>(x) / static_cast<float>(CHUNK_X)));
    int cz = static_cast<int>(std::floor(static_cast<float>(z) / static_cast<float>(CHUNK_Z)));

    Chunk* chunk = getChunk(cx, cz);
    if (!chunk) return 0;

    int lx = x - cx * CHUNK_X;
    int lz = z - cz * CHUNK_Z;
    return chunk->getBlock(lx, y, lz);
}

void World::setBlock(int x, int y, int z, uint16_t id) {
    if (y < 0 || y >= CHUNK_Y) return;

    int cx = static_cast<int>(std::floor(static_cast<float>(x) / static_cast<float>(CHUNK_X)));
    int cz = static_cast<int>(std::floor(static_cast<float>(z) / static_cast<float>(CHUNK_Z)));

    Chunk* chunk = getChunk(cx, cz);
    if (!chunk) return;

    int lx = x - cx * CHUNK_X;
    int lz = z - cz * CHUNK_Z;
    chunk->setBlock(lx, y, lz, id);

    // If on chunk border, mark neighbor dirty
    if (lx == 0) { Chunk* c = getChunk(cx - 1, cz); if (c) c->setModified(true); }
    if (lx == CHUNK_X - 1) { Chunk* c = getChunk(cx + 1, cz); if (c) c->setModified(true); }
    if (lz == 0) { Chunk* c = getChunk(cx, cz - 1); if (c) c->setModified(true); }
    if (lz == CHUNK_Z - 1) { Chunk* c = getChunk(cx, cz + 1); if (c) c->setModified(true); }

    // Planted Sapling state tracking
    if (isSapling(id)) {
        addSapling(x, y, z, id);
    } else {
        removeSapling(x, y, z);
    }
}

int World::getHighestBlock(int x, int z) const {
    for (int y = CHUNK_Y - 1; y >= 0; --y) {
        if (getBlock(x, y, z) != 0) {
            return y;
        }
    }
    return 32; // Default baseline terrain height
}

RaycastResult World::raycast(const Ray& ray, float maxDist) const {
    RaycastResult res;
    Vec3 pos = ray.origin;
    Vec3 dir = ray.direction.normalized();

    // 3D DDA Voxel Traversal
    int mapX = static_cast<int>(std::floor(pos.x));
    int mapY = static_cast<int>(std::floor(pos.y));
    int mapZ = static_cast<int>(std::floor(pos.z));

    float deltaDistX = (dir.x == 0.0f) ? 1e30f : std::abs(1.0f / dir.x);
    float deltaDistY = (dir.y == 0.0f) ? 1e30f : std::abs(1.0f / dir.y);
    float deltaDistZ = (dir.z == 0.0f) ? 1e30f : std::abs(1.0f / dir.z);

    int stepX = (dir.x < 0.0f) ? -1 : 1;
    int stepY = (dir.y < 0.0f) ? -1 : 1;
    int stepZ = (dir.z < 0.0f) ? -1 : 1;

    float sideDistX = (dir.x < 0.0f) ? (pos.x - static_cast<float>(mapX)) * deltaDistX : (static_cast<float>(mapX + 1) - pos.x) * deltaDistX;
    float sideDistY = (dir.y < 0.0f) ? (pos.y - static_cast<float>(mapY)) * deltaDistY : (static_cast<float>(mapY + 1) - pos.y) * deltaDistY;
    float sideDistZ = (dir.z < 0.0f) ? (pos.z - static_cast<float>(mapZ)) * deltaDistZ : (static_cast<float>(mapZ + 1) - pos.z) * deltaDistZ;

    float dist = 0.0f;
    IVec3 lastAdjacent = {mapX, mapY, mapZ};

    while (dist < maxDist) {
        uint16_t bId = getBlock(mapX, mapY, mapZ);
        if (bId != 0) {
            res.hit = true;
            res.hitBlockPos = {mapX, mapY, mapZ};
            res.adjacentPos = lastAdjacent;
            res.blockId = bId;
            res.distance = dist;
            return res;
        }

        lastAdjacent = {mapX, mapY, mapZ};

        if (sideDistX < sideDistY) {
            if (sideDistX < sideDistZ) {
                dist = sideDistX;
                sideDistX += deltaDistX;
                mapX += stepX;
            } else {
                dist = sideDistZ;
                sideDistZ += deltaDistZ;
                mapZ += stepZ;
            }
        } else {
            if (sideDistY < sideDistZ) {
                dist = sideDistY;
                sideDistY += deltaDistY;
                mapY += stepY;
            } else {
                dist = sideDistZ;
                sideDistZ += deltaDistZ;
                mapZ += stepZ;
            }
        }
    }

    return res;
}

uint8_t World::getBiomeAt(float worldX, float worldZ) const {
    float temp = tempNoise.fbm2D(worldX * 0.003f, worldZ * 0.003f, 3, 0.5f);
    float moist = moistureNoise.fbm2D(worldX * 0.003f, worldZ * 0.003f, 3, 0.5f);
    float elev = elevNoise.fbm2D(worldX * 0.005f, worldZ * 0.005f, 4, 0.5f);
    return BiomeRegistry::sampleBiome(temp, moist, elev);
}

void World::addSapling(int x, int y, int z, uint16_t saplingId, float timer) {
    removeSapling(x, y, z);
    float gTimer = (timer > 0.0f) ? timer : (15.0f + static_cast<float>(std::rand() % 13));
    saplings.push_back({x, y, z, saplingId, gTimer});
}

void World::removeSapling(int x, int y, int z) {
    for (auto it = saplings.begin(); it != saplings.end(); ++it) {
        if (it->x == x && it->y == y && it->z == z) {
            saplings.erase(it);
            return;
        }
    }
}

bool World::hasSapling(int x, int y, int z) const {
    for (const auto& s : saplings) {
        if (s.x == x && s.y == y && s.z == z) return true;
    }
    return false;
}

void World::growTree(int x, int y, int z, uint16_t saplingId) {
    uint16_t logId = 71;   // Oak Log
    uint16_t leafId = 73;  // Oak Leaves
    int trunkHeight = 5 + (std::rand() % 3);

    if (saplingId == 367) { // Birch
        logId = 74; leafId = 76;
        trunkHeight = 6 + (std::rand() % 3);
    } else if (saplingId == 368) { // Spruce
        logId = 77; leafId = 79;
        trunkHeight = 7 + (std::rand() % 4);
    } else if (saplingId == 369) { // Cherry
        logId = 95; leafId = 97;
        trunkHeight = 5 + (std::rand() % 2);
    }

    // Replace sapling and erect vertical trunk logs
    for (int dy = 0; dy < trunkHeight; ++dy) {
        setBlock(x, y + dy, z, logId);
    }

    int topY = y + trunkHeight;

    if (saplingId == 368) {
        // Conical Spruce / Pine tree canopy
        int startY = y + 2;
        for (int cy = startY; cy <= topY + 1; ++cy) {
            int radius = (topY + 1 - cy) % 2 == 0 ? 2 : 1;
            if (cy == topY + 1) radius = 0;
            for (int dx = -radius; dx <= radius; ++dx) {
                for (int dz = -radius; dz <= radius; ++dz) {
                    if (std::abs(dx) == radius && std::abs(dz) == radius && radius > 1) continue;
                    int tx = x + dx;
                    int ty = cy;
                    int tz = z + dz;
                    if (getBlock(tx, ty, tz) == 0) {
                        setBlock(tx, ty, tz, leafId);
                    }
                }
            }
        }
    } else if (saplingId == 367) {
        // Slender Birch Tree Canopy
        for (int dy = -2; dy <= 1; ++dy) {
            int radius = (dy == 1) ? 1 : 2;
            for (int dx = -radius; dx <= radius; ++dx) {
                for (int dz = -radius; dz <= radius; ++dz) {
                    if (std::abs(dx) == radius && std::abs(dz) == radius && (dy == 1 || dy == -2)) continue;
                    int tx = x + dx;
                    int ty = topY + dy;
                    int tz = z + dz;
                    if (getBlock(tx, ty, tz) == 0) {
                        setBlock(tx, ty, tz, leafId);
                    }
                }
            }
        }
    } else if (saplingId == 369) {
        // Sprawling Cherry Blossom Canopy
        for (int dy = -2; dy <= 2; ++dy) {
            int radius = (dy == 2) ? 1 : ((std::abs(dy) == 1) ? 3 : 2);
            for (int dx = -radius; dx <= radius; ++dx) {
                for (int dz = -radius; dz <= radius; ++dz) {
                    if (std::abs(dx) == radius && std::abs(dz) == radius && (dy == 2 || dy == -2)) continue;
                    int tx = x + dx;
                    int ty = topY + dy;
                    int tz = z + dz;
                    if (getBlock(tx, ty, tz) == 0) {
                        setBlock(tx, ty, tz, leafId);
                    }
                }
            }
        }
    } else {
        // Full Rounded Oak Canopy
        for (int dy = -2; dy <= 2; ++dy) {
            int radius = (dy == 2) ? 1 : ((std::abs(dy) == 1) ? 2 : 3);
            for (int dx = -radius; dx <= radius; ++dx) {
                for (int dz = -radius; dz <= radius; ++dz) {
                    if (std::abs(dx) == radius && std::abs(dz) == radius && (dy == 2 || dy == -2)) continue;
                    int tx = x + dx;
                    int ty = topY + dy;
                    int tz = z + dz;
                    if (getBlock(tx, ty, tz) == 0) {
                        setBlock(tx, ty, tz, leafId);
                    }
                }
            }
        }
    }
}

Vec3 World::getSunDirection() const {
    // 0.25 = sunrise (east, x=-1, y=0)
    // 0.50 = high noon (overhead, x=0, y=1)
    // 0.75 = sunset (west, x=1, y=0)
    // 0.00 = midnight (nadir, x=0, y=-1)
    float theta = timeOfDay * 6.2831853f - 1.5707963f;
    Vec3 dir(-std::cos(theta), std::sin(theta), 0.32f * std::sin(theta));
    return dir.normalized();
}

Vec3 World::getMoonDirection() const {
    float theta = timeOfDay * 6.2831853f - 1.5707963f;
    Vec3 dir(std::cos(theta), -std::sin(theta), -0.32f * std::sin(theta));
    return dir.normalized();
}

Vec3 World::getActiveLightDirection() const {
    float theta = timeOfDay * 6.2831853f - 1.5707963f;
    float s = std::sin(theta);
    if (s > -0.05f) {
        return getSunDirection();
    }
    return getMoonDirection();
}

float World::getDaylightFactor() const {
    float theta = timeOfDay * 6.2831853f - 1.5707963f;
    float s = std::sin(theta);
    if (s <= -0.12f) return 0.0f;
    if (s >= 0.22f) return 1.0f;
    float t = (s - (-0.12f)) / (0.22f - (-0.12f));
    return t * t * (3.0f - 2.0f * t); // smoothstep
}

bool World::isNight() const {
    return getDaylightFactor() < 0.22f;
}

static inline Vec3 lerpVec3(const Vec3& a, const Vec3& b, float t) {
    return Vec3(
        a.x + (b.x - a.x) * t,
        a.y + (b.y - a.y) * t,
        a.z + (b.z - a.z) * t
    );
}

Vec3 World::getSkyColor(const Vec3& biomeSkyColor) const {
    float dayFactor = getDaylightFactor();

    // Night deep indigo sky
    Vec3 nightSky(0.025f, 0.045f, 0.095f);

    // Sunset / Dawn fiery apricot / gold
    Vec3 sunsetSky(0.92f, 0.48f, 0.22f);

    float theta = timeOfDay * 6.2831853f - 1.5707963f;
    float s = std::sin(theta);
    float sunsetFactor = std::clamp(1.0f - std::abs(s) / 0.24f, 0.0f, 1.0f);
    sunsetFactor = sunsetFactor * sunsetFactor * (3.0f - 2.0f * sunsetFactor);

    Vec3 base = lerpVec3(nightSky, biomeSkyColor, dayFactor);
    return lerpVec3(base, sunsetSky, sunsetFactor * 0.70f);
}

Vec3 World::getFogColor(const Vec3& biomeFogColor) const {
    float dayFactor = getDaylightFactor();

    // Night soft indigo-mist fog
    Vec3 nightFog(0.040f, 0.065f, 0.120f);

    // Sunset warm amber-tinted mist
    Vec3 sunsetFog(0.85f, 0.52f, 0.28f);

    float theta = timeOfDay * 6.2831853f - 1.5707963f;
    float s = std::sin(theta);
    float sunsetFactor = std::clamp(1.0f - std::abs(s) / 0.24f, 0.0f, 1.0f);
    sunsetFactor = sunsetFactor * sunsetFactor * (3.0f - 2.0f * sunsetFactor);

    Vec3 base = lerpVec3(nightFog, biomeFogColor, dayFactor);
    return lerpVec3(base, sunsetFog, sunsetFactor * 0.65f);
}

std::string World::getFormattedTime() const {
    float totalHours = timeOfDay * 24.0f;
    int hours = static_cast<int>(totalHours);
    int minutes = static_cast<int>((totalHours - hours) * 60.0f);
    int displayHour = hours % 12;
    if (displayHour == 0) displayHour = 12;
    const char* ampm = (hours < 12) ? "AM" : "PM";

    char buf[32];
    std::snprintf(buf, sizeof(buf), "%02d:%02d %s", displayHour, minutes, ampm);
    return std::string(buf);
}

std::string World::getTimePeriodName() const {
    if (timeOfDay >= 0.20f && timeOfDay < 0.28f) return "DAWN";
    if (timeOfDay >= 0.28f && timeOfDay < 0.45f) return "MORNING";
    if (timeOfDay >= 0.45f && timeOfDay < 0.55f) return "NOON";
    if (timeOfDay >= 0.55f && timeOfDay < 0.70f) return "AFTERNOON";
    if (timeOfDay >= 0.70f && timeOfDay < 0.78f) return "DUSK";
    if (timeOfDay >= 0.78f && timeOfDay < 0.95f) return "NIGHT";
    return "MIDNIGHT";
}

} // namespace Aetheria
