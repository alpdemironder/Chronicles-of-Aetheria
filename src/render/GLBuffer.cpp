#include "GLBuffer.hpp"
#include <iostream>

namespace Aetheria {

GLBuffer::GLBuffer() {}

GLBuffer::~GLBuffer() {
    if (vbo) {
        glDeleteBuffers(1, &vbo);
        vbo = 0;
    }
    if (vao) {
        glDeleteVertexArrays(1, &vao);
        vao = 0;
    }
}

void GLBuffer::uploadVoxelData(const void* data, size_t sizeBytes, uint32_t count, bool dynamic) {
    vertexCount = count;
    if (count == 0 || !data) return;

    if (!vao) glGenVertexArrays(1, &vao);
    if (!vbo) glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeBytes, data, dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);

    // VoxelVertex: (x,y,z: 12B), (u,v: 8B), (nx,ny,nz: 12B), (r,g,b,a: 16B), (texIndex: 4B) = 52B total stride
    // 0: inPos
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 52, (void*)0);

    // 1: inUV
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 52, (void*)12);

    // 2: inNormal
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 52, (void*)20);

    // 3: inColor
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 52, (void*)32);

    // 4: inTexIndex
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, 52, (void*)48);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void GLBuffer::uploadUIData(const void* data, size_t sizeBytes, uint32_t count) {
    vertexCount = count;
    if (count == 0 || !data) return;

    if (!vao) glGenVertexArrays(1, &vao);
    if (!vbo) glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeBytes, data, GL_DYNAMIC_DRAW);

    // UIVertex: (x,y: 8B), (u,v: 8B), (r,g,b,a: 16B) = 32B total stride
    // 0: inPos
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 32, (void*)0);

    // 1: inUV
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 32, (void*)8);

    // 2: inColor
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 32, (void*)16);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void GLBuffer::draw() {
    if (vertexCount > 0 && vao != 0) {
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, vertexCount);
        glBindVertexArray(0);
    }
}

} // namespace Aetheria
