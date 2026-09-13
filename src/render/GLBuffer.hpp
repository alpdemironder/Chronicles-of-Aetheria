#pragma once

#include "GLHeaders.hpp"
#include <cstdint>

namespace Aetheria {

class GLBuffer {
public:
    GLBuffer();
    ~GLBuffer();

    void uploadVoxelData(const void* data, size_t sizeBytes, uint32_t count, bool dynamic = false);
    void uploadUIData(const void* data, size_t sizeBytes, uint32_t count);
    void draw();

    uint32_t getVertexCount() const { return vertexCount; }
    GLuint getVAO() const { return vao; }
    GLuint getVBO() const { return vbo; }

private:
    GLuint vao = 0;
    GLuint vbo = 0;
    uint32_t vertexCount = 0;
};

} // namespace Aetheria
