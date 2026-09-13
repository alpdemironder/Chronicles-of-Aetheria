#include "Chunk.hpp"
#include "BlockRegistry.hpp"
#include "../render/TextureAtlas.hpp"
#include <algorithm>

namespace Aetheria {

Chunk::Chunk(int chunkX, int chunkZ) : chunkX(chunkX), chunkZ(chunkZ) {
    std::fill_n(blocks, CHUNK_X * CHUNK_Z * CHUNK_Y, (uint16_t)0);
}

Chunk::~Chunk() {}

uint16_t Chunk::getBlock(int x, int y, int z) const {
    if (x < 0 || x >= CHUNK_X || y < 0 || y >= CHUNK_Y || z < 0 || z >= CHUNK_Z) {
        return 0;
    }
    return blocks[(y * CHUNK_Z + z) * CHUNK_X + x];
}

void Chunk::setBlock(int x, int y, int z, uint16_t id) {
    if (x < 0 || x >= CHUNK_X || y < 0 || y >= CHUNK_Y || z < 0 || z >= CHUNK_Z) {
        return;
    }
    blocks[(y * CHUNK_Z + z) * CHUNK_X + x] = id;
    isDirty = true;
}

void Chunk::updateMesh(const Chunk* left, const Chunk* right,
                       const Chunk* back, const Chunk* front) {
    if (!isDirty && vertexCount > 0) return;

    std::vector<VoxelVertex> vertices;
    vertices.reserve(4096);

    auto getNeighborBlock = [&](int x, int y, int z) -> uint16_t {
        if (y < 0 || y >= CHUNK_Y) return 0;
        if (x < 0) return left ? left->getBlock(x + CHUNK_X, y, z) : 0xFFFF;
        if (x >= CHUNK_X) return right ? right->getBlock(x - CHUNK_X, y, z) : 0xFFFF;
        if (z < 0) return back ? back->getBlock(x, y, z + CHUNK_Z) : 0xFFFF;
        if (z >= CHUNK_Z) return front ? front->getBlock(x, y, z - CHUNK_Z) : 0xFFFF;
        return getBlock(x, y, z);
    };

    auto isSolid = [&](int nx, int ny, int nz) -> int {
        uint16_t nb = getNeighborBlock(nx, ny, nz);
        if (nb == 0 || nb == 0xFFFF) return 0;
        return BlockRegistry::get(nb).isSolid ? 1 : 0;
    };

    auto isFoliage = [](uint16_t id) -> bool {
        return (id >= 73 && id <= 118 && ((id - 71) % 3 == 2)) || (id >= 366 && id <= 369);
    };

    auto shouldRenderFace = [&](uint16_t selfId, uint16_t neighborId) -> bool {
        if (neighborId == 0xFFFF) return false; // Unloaded neighbor chunk: do not create phantom cross-section walls
        if (neighborId == 0) return true;       // Exposed to air

        const auto& selfDef = BlockRegistry::get(selfId);
        const auto& nbDef = BlockRegistry::get(neighborId);

        // Cull internal faces between touching leaves/foliage blocks
        if (isFoliage(selfId) && isFoliage(neighborId)) {
            return false;
        }

        // Fluid / Liquid Meshing & Culling
        if (selfDef.isLiquid) {
            // Liquid touching another liquid: NEVER render internal faces between fluids!
            if (nbDef.isLiquid) return false;
            // Liquid touching solid block: do not render liquid quad on solid surfaces
            if (nbDef.isSolid) return false;
            // Liquid surface exposed to air or non-solid decorations
            return true;
        }

        // Solid block touching liquid: render face so riverbed/lakebed is visible through water!
        if (nbDef.isLiquid) {
            return true;
        }

        // If neighbor is transparent (glass, leaves), solid surfaces should render face
        if (nbDef.isTransparent) {
            // Same transparent types do not render internal faces against each other (ice-to-ice, glass-to-glass)
            if (selfDef.isTransparent && selfId == neighborId) return false;
            return true;
        }

        return false;
    };

    auto vertexAO = [&](int s1, int s2, int c) -> float {
        if (s1 && s2) return 0.52f;
        int sum = s1 + s2 + c;
        if (sum == 0) return 1.0f;
        if (sum == 1) return 0.85f;
        return 0.68f;
    };

    for (int y = 0; y < CHUNK_Y; ++y) {
        for (int z = 0; z < CHUNK_Z; ++z) {
            for (int x = 0; x < CHUNK_X; ++x) {
                uint16_t bId = getBlock(x, y, z);
                if (bId == 0) continue;

                // Special rendering for Saplings & Floral Sprites (authentic Minecraft crossed diagonal quads)
                if (bId >= 366 && bId <= 369) {
                    float fx = static_cast<float>(chunkX * CHUNK_X + x);
                    float fy = static_cast<float>(y);
                    float fz = static_cast<float>(chunkZ * CHUNK_Z + z);
                    float tIdx = static_cast<float>(bId);
                    Vec4 c(1.0f, 1.0f, 1.0f, 1.0f);

                    // Diagonal 1: (fx, fy, fz) to (fx+1, fy+1, fz+1)
                    Vec3 a0{fx, fy, fz}, a1{fx + 1.0f, fy, fz + 1.0f};
                    Vec3 a2{fx + 1.0f, fy + 1.0f, fz + 1.0f}, a3{fx, fy + 1.0f, fz};
                    Vec3 nA = Vec3(1.0f, 0.0f, -1.0f).normalized();

                    vertices.push_back({a0.x, a0.y, a0.z, 0.0f, 0.0f, nA.x, nA.y, nA.z, c.x, c.y, c.z, c.w, tIdx});
                    vertices.push_back({a1.x, a1.y, a1.z, 1.0f, 0.0f, nA.x, nA.y, nA.z, c.x, c.y, c.z, c.w, tIdx});
                    vertices.push_back({a2.x, a2.y, a2.z, 1.0f, 1.0f, nA.x, nA.y, nA.z, c.x, c.y, c.z, c.w, tIdx});
                    vertices.push_back({a0.x, a0.y, a0.z, 0.0f, 0.0f, nA.x, nA.y, nA.z, c.x, c.y, c.z, c.w, tIdx});
                    vertices.push_back({a2.x, a2.y, a2.z, 1.0f, 1.0f, nA.x, nA.y, nA.z, c.x, c.y, c.z, c.w, tIdx});
                    vertices.push_back({a3.x, a3.y, a3.z, 0.0f, 1.0f, nA.x, nA.y, nA.z, c.x, c.y, c.z, c.w, tIdx});

                    // Diagonal 2: (fx+1, fy, fz) to (fx, fy+1, fz+1)
                    Vec3 b0{fx + 1.0f, fy, fz}, b1{fx, fy, fz + 1.0f};
                    Vec3 b2{fx, fy + 1.0f, fz + 1.0f}, b3{fx + 1.0f, fy + 1.0f, fz};
                    Vec3 nB = Vec3(1.0f, 0.0f, 1.0f).normalized();

                    vertices.push_back({b0.x, b0.y, b0.z, 0.0f, 0.0f, nB.x, nB.y, nB.z, c.x, c.y, c.z, c.w, tIdx});
                    vertices.push_back({b1.x, b1.y, b1.z, 1.0f, 0.0f, nB.x, nB.y, nB.z, c.x, c.y, c.z, c.w, tIdx});
                    vertices.push_back({b2.x, b2.y, b2.z, 1.0f, 1.0f, nB.x, nB.y, nB.z, c.x, c.y, c.z, c.w, tIdx});
                    vertices.push_back({b0.x, b0.y, b0.z, 0.0f, 0.0f, nB.x, nB.y, nB.z, c.x, c.y, c.z, c.w, tIdx});
                    vertices.push_back({b2.x, b2.y, b2.z, 1.0f, 1.0f, nB.x, nB.y, nB.z, c.x, c.y, c.z, c.w, tIdx});
                    vertices.push_back({b3.x, b3.y, b3.z, 0.0f, 1.0f, nB.x, nB.y, nB.z, c.x, c.y, c.z, c.w, tIdx});

                    continue;
                }

                // 6 Faces: +Y, -Y, +Z, -Z, +X, -X
                // Top (+Y)
                if (shouldRenderFace(bId, getNeighborBlock(x, y + 1, z))) {
                    float ao[4];
                    ao[0] = vertexAO(isSolid(x - 1, y + 1, z), isSolid(x, y + 1, z + 1), isSolid(x - 1, y + 1, z + 1));
                    ao[1] = vertexAO(isSolid(x + 1, y + 1, z), isSolid(x, y + 1, z + 1), isSolid(x + 1, y + 1, z + 1));
                    ao[2] = vertexAO(isSolid(x + 1, y + 1, z), isSolid(x, y + 1, z - 1), isSolid(x + 1, y + 1, z - 1));
                    ao[3] = vertexAO(isSolid(x - 1, y + 1, z), isSolid(x, y + 1, z - 1), isSolid(x - 1, y + 1, z - 1));
                    addFace(vertices, x, y, z, 0, bId, ao);
                }
                // Bottom (-Y)
                if (y > 0 && shouldRenderFace(bId, getNeighborBlock(x, y - 1, z))) {
                    float ao[4] = { 0.85f, 0.85f, 0.85f, 0.85f };
                    addFace(vertices, x, y, z, 1, bId, ao);
                }
                // North (+Z)
                if (shouldRenderFace(bId, getNeighborBlock(x, y, z + 1))) {
                    float ao[4];
                    ao[0] = vertexAO(isSolid(x - 1, y, z + 1), isSolid(x, y - 1, z + 1), isSolid(x - 1, y - 1, z + 1));
                    ao[1] = vertexAO(isSolid(x + 1, y, z + 1), isSolid(x, y - 1, z + 1), isSolid(x + 1, y - 1, z + 1));
                    ao[2] = vertexAO(isSolid(x + 1, y, z + 1), isSolid(x, y + 1, z + 1), isSolid(x + 1, y + 1, z + 1));
                    ao[3] = vertexAO(isSolid(x - 1, y, z + 1), isSolid(x, y + 1, z + 1), isSolid(x - 1, y + 1, z + 1));
                    addFace(vertices, x, y, z, 2, bId, ao);
                }
                // South (-Z)
                if (shouldRenderFace(bId, getNeighborBlock(x, y, z - 1))) {
                    float ao[4];
                    ao[0] = vertexAO(isSolid(x + 1, y, z - 1), isSolid(x, y - 1, z - 1), isSolid(x + 1, y - 1, z - 1));
                    ao[1] = vertexAO(isSolid(x - 1, y, z - 1), isSolid(x, y - 1, z - 1), isSolid(x - 1, y - 1, z - 1));
                    ao[2] = vertexAO(isSolid(x - 1, y, z - 1), isSolid(x, y + 1, z - 1), isSolid(x - 1, y + 1, z - 1));
                    ao[3] = vertexAO(isSolid(x + 1, y, z - 1), isSolid(x, y + 1, z - 1), isSolid(x + 1, y + 1, z - 1));
                    addFace(vertices, x, y, z, 3, bId, ao);
                }
                // East (+X)
                if (shouldRenderFace(bId, getNeighborBlock(x + 1, y, z))) {
                    float ao[4];
                    ao[0] = vertexAO(isSolid(x + 1, y, z + 1), isSolid(x + 1, y - 1, z), isSolid(x + 1, y - 1, z + 1));
                    ao[1] = vertexAO(isSolid(x + 1, y, z - 1), isSolid(x + 1, y - 1, z), isSolid(x + 1, y - 1, z - 1));
                    ao[2] = vertexAO(isSolid(x + 1, y, z - 1), isSolid(x + 1, y + 1, z), isSolid(x + 1, y + 1, z - 1));
                    ao[3] = vertexAO(isSolid(x + 1, y, z + 1), isSolid(x + 1, y + 1, z), isSolid(x + 1, y + 1, z + 1));
                    addFace(vertices, x, y, z, 4, bId, ao);
                }
                // West (-X)
                if (shouldRenderFace(bId, getNeighborBlock(x - 1, y, z))) {
                    float ao[4];
                    ao[0] = vertexAO(isSolid(x - 1, y, z - 1), isSolid(x - 1, y - 1, z), isSolid(x - 1, y - 1, z - 1));
                    ao[1] = vertexAO(isSolid(x - 1, y, z + 1), isSolid(x - 1, y - 1, z), isSolid(x - 1, y - 1, z + 1));
                    ao[2] = vertexAO(isSolid(x - 1, y, z + 1), isSolid(x - 1, y + 1, z), isSolid(x - 1, y + 1, z + 1));
                    ao[3] = vertexAO(isSolid(x - 1, y, z - 1), isSolid(x - 1, y + 1, z), isSolid(x - 1, y + 1, z - 1));
                    addFace(vertices, x, y, z, 5, bId, ao);
                }
            }
        }
    }

    vertexCount = static_cast<uint32_t>(vertices.size());
    if (vertexCount > 0) {
        meshBuffer.uploadVoxelData(vertices.data(), vertices.size() * sizeof(VoxelVertex), vertexCount);
    }

    isDirty = false;
}

void Chunk::addFace(std::vector<VoxelVertex>& vertices, int x, int y, int z, int faceIndex, uint16_t blockId, float ao[4]) {
    float fx = static_cast<float>(chunkX * CHUNK_X + x);
    float fy = static_cast<float>(y);
    float fz = static_cast<float>(chunkZ * CHUNK_Z + z);

    const BlockDef& def = BlockRegistry::get(blockId);
    Vec4 col = def.color;

    float faceLight = 1.0f;
    float nx = 0, ny = 0, nz = 0;

    Vec3 p0, p1, p2, p3;

    switch (faceIndex) {
    case 0: // +Y (Top)
        faceLight = 1.0f; ny = 1.0f;
        p0 = {fx, fy + 1.0f, fz + 1.0f};
        p1 = {fx + 1.0f, fy + 1.0f, fz + 1.0f};
        p2 = {fx + 1.0f, fy + 1.0f, fz};
        p3 = {fx, fy + 1.0f, fz};
        break;
    case 1: // -Y (Bottom)
        faceLight = 0.55f; ny = -1.0f;
        p0 = {fx, fy, fz};
        p1 = {fx + 1.0f, fy, fz};
        p2 = {fx + 1.0f, fy, fz + 1.0f};
        p3 = {fx, fy, fz + 1.0f};
        break;
    case 2: // +Z (North)
        faceLight = 0.85f; nz = 1.0f;
        p0 = {fx, fy, fz + 1.0f};
        p1 = {fx + 1.0f, fy, fz + 1.0f};
        p2 = {fx + 1.0f, fy + 1.0f, fz + 1.0f};
        p3 = {fx, fy + 1.0f, fz + 1.0f};
        break;
    case 3: // -Z (South)
        faceLight = 0.85f; nz = -1.0f;
        p0 = {fx + 1.0f, fy, fz};
        p1 = {fx, fy, fz};
        p2 = {fx, fy + 1.0f, fz};
        p3 = {fx + 1.0f, fy + 1.0f, fz};
        break;
    case 4: // +X (East)
        faceLight = 0.72f; nx = 1.0f;
        p0 = {fx + 1.0f, fy, fz + 1.0f};
        p1 = {fx + 1.0f, fy, fz};
        p2 = {fx + 1.0f, fy + 1.0f, fz};
        p3 = {fx + 1.0f, fy + 1.0f, fz + 1.0f};
        break;
    case 5: // -X (West)
        faceLight = 0.72f; nx = -1.0f;
        p0 = {fx, fy, fz};
        p1 = {fx, fy, fz + 1.0f};
        p2 = {fx, fy + 1.0f, fz + 1.0f};
        p3 = {fx, fy + 1.0f, fz};
        break;
    }

    Vec4 c0(faceLight, faceLight, faceLight, ao[0]);
    Vec4 c1(faceLight, faceLight, faceLight, ao[1]);
    Vec4 c2(faceLight, faceLight, faceLight, ao[2]);
    Vec4 c3(faceLight, faceLight, faceLight, ao[3]);

    float tIdx = static_cast<float>(getBlockFaceTexture(blockId, faceIndex));

    if (ao[0] + ao[2] < ao[1] + ao[3]) {
        // Flip diagonal to p1-p3 to eliminate AO interpolation creases
        vertices.push_back({p1.x, p1.y, p1.z, 1.0f, 0.0f, nx, ny, nz, c1.x, c1.y, c1.z, c1.w, tIdx});
        vertices.push_back({p2.x, p2.y, p2.z, 1.0f, 1.0f, nx, ny, nz, c2.x, c2.y, c2.z, c2.w, tIdx});
        vertices.push_back({p3.x, p3.y, p3.z, 0.0f, 1.0f, nx, ny, nz, c3.x, c3.y, c3.z, c3.w, tIdx});

        vertices.push_back({p1.x, p1.y, p1.z, 1.0f, 0.0f, nx, ny, nz, c1.x, c1.y, c1.z, c1.w, tIdx});
        vertices.push_back({p3.x, p3.y, p3.z, 0.0f, 1.0f, nx, ny, nz, c3.x, c3.y, c3.z, c3.w, tIdx});
        vertices.push_back({p0.x, p0.y, p0.z, 0.0f, 0.0f, nx, ny, nz, c0.x, c0.y, c0.z, c0.w, tIdx});
    } else {
        vertices.push_back({p0.x, p0.y, p0.z, 0.0f, 0.0f, nx, ny, nz, c0.x, c0.y, c0.z, c0.w, tIdx});
        vertices.push_back({p1.x, p1.y, p1.z, 1.0f, 0.0f, nx, ny, nz, c1.x, c1.y, c1.z, c1.w, tIdx});
        vertices.push_back({p2.x, p2.y, p2.z, 1.0f, 1.0f, nx, ny, nz, c2.x, c2.y, c2.z, c2.w, tIdx});

        vertices.push_back({p0.x, p0.y, p0.z, 0.0f, 0.0f, nx, ny, nz, c0.x, c0.y, c0.z, c0.w, tIdx});
        vertices.push_back({p2.x, p2.y, p2.z, 1.0f, 1.0f, nx, ny, nz, c2.x, c2.y, c2.z, c2.w, tIdx});
        vertices.push_back({p3.x, p3.y, p3.z, 0.0f, 1.0f, nx, ny, nz, c3.x, c3.y, c3.z, c3.w, tIdx});
    }
}

void Chunk::render() {
    meshBuffer.draw();
}

} // namespace Aetheria
