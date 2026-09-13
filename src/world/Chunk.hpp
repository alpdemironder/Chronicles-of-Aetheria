#pragma once

#include <cstdint>
#include <vector>
#include "../core/Math.hpp"
#include "../render/GLBuffer.hpp"

namespace Aetheria {

constexpr int CHUNK_X = 16;
constexpr int CHUNK_Z = 16;
constexpr int CHUNK_Y = 128;

struct VoxelVertex {
    float x, y, z;
    float u, v;
    float nx, ny, nz;
    float r, g, b, a;
    float texIndex;
};

class Chunk {
public:
    Chunk(int chunkX, int chunkZ);
    ~Chunk();

    int getChunkX() const { return chunkX; }
    int getChunkZ() const { return chunkZ; }
    IVec3 getChunkPos() const { return {chunkX, 0, chunkZ}; }

    uint16_t getBlock(int x, int y, int z) const;
    void setBlock(int x, int y, int z, uint16_t id);

    void setModified(bool mod = true) { isDirty = mod; }
    bool getIsDirty() const { return isDirty; }

    void updateMesh(const Chunk* left, const Chunk* right,
                    const Chunk* back, const Chunk* front);

    void render();
    uint32_t getVertexCount() const { return vertexCount; }

private:
    int chunkX = 0;
    int chunkZ = 0;
    bool isDirty = true;

    uint16_t blocks[CHUNK_X * CHUNK_Z * CHUNK_Y] = {};

    GLBuffer meshBuffer;
    uint32_t vertexCount = 0;

    void addFace(std::vector<VoxelVertex>& vertices, int x, int y, int z, int faceIndex, uint16_t blockId, float ao[4]);
};

} // namespace Aetheria
