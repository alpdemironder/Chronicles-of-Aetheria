#include "ItemEntity.hpp"
#include "../world/World.hpp"
#include "../world/BlockRegistry.hpp"
#include <cmath>
#include <algorithm>

namespace Aetheria {

ItemEntity::ItemEntity(const ItemStack& stack, const Vec3& pos, const Vec3& initialVel)
    : item(stack), position(pos), velocity(initialVel) {
}

void ItemEntity::update(World* world, const Vec3& playerPos, float dt) {
    if (isPickedUp) return;

    lifetime -= dt;
    if (pickupDelay > 0.0f) pickupDelay -= dt;

    // Rotation & floating bob
    rotation += dt * 3.0f;
    bobOffset = std::sin(rotation * 2.2f) * 0.06f;

    // Magnetism towards player when close
    if (pickupDelay <= 0.0f) {
        Vec3 toPlayer = (playerPos + Vec3(0, 0.5f, 0)) - position;
        float dist = toPlayer.length();
        if (dist < 3.2f && dist > 0.05f) {
            float pullStrength = (3.2f - dist) * 7.5f;
            velocity += toPlayer.normalized() * (pullStrength * dt);
        }
    }

    // Apply gravity
    velocity.y -= 22.0f * dt;
    if (velocity.y < -25.0f) velocity.y = -25.0f;

    // Horizontal drag / friction
    float friction = onGround ? 0.82f : 0.96f;
    velocity.x *= friction;
    velocity.z *= friction;

    // Integrate position with voxel collision
    Vec3 newPos = position + velocity * dt;

    int blockX = static_cast<int>(std::floor(newPos.x));
    int blockY = static_cast<int>(std::floor(newPos.y));
    int blockZ = static_cast<int>(std::floor(newPos.z));

    uint16_t bUnder = world->getBlock(blockX, blockY, blockZ);
    if (bUnder != 0) {
        // Collided with solid block ground
        newPos.y = static_cast<float>(blockY + 1);
        if (velocity.y < -2.0f) {
            velocity.y = -velocity.y * 0.28f; // Small elastic bounce
        } else {
            velocity.y = 0.0f;
            onGround = true;
        }
    } else {
        onGround = false;
    }

    position = newPos;
}

void ItemEntity::appendVertices(std::vector<VoxelVertex>& verts, float totalTime) const {
    if (isPickedUp) return;

    // Determine texture layer: if block 1..369 use block ID, else map item ID or fallback
    float texIdx = static_cast<float>(item.id);
    if (item.id > 369) {
        // Map common items to thematic texture layers
        if (item.id == 501) texIdx = 124.0f; // Iron
        else if (item.id == 502) texIdx = 125.0f; // Copper
        else if (item.id == 503) texIdx = 126.0f; // Gold
        else if (item.id >= 510 && item.id <= 519) texIdx = 124.0f; // Weapons
        else if (item.id >= 530 && item.id <= 549) texIdx = 135.0f; // Armor
        else if (item.id == 525) texIdx = 48.0f; // Bone Meal (white calcite bone dust)
        else texIdx = 186.0f; // Crystal/misc
    }

    float s = 0.18f; // Mini half-size (0.36 block width)
    Vec3 center = position + Vec3(0, 0.22f + bobOffset, 0);

    float cosR = std::cos(rotation);
    float sinR = std::sin(rotation);

    auto rot = [&](float lx, float ly, float lz) -> Vec3 {
        return Vec3(
            center.x + lx * cosR - lz * sinR,
            center.y + ly,
            center.z + lx * sinR + lz * cosR
        );
    };

    // Color tinting: slightly brightened item glow
    Vec4 col(1.0f, 1.0f, 1.0f, 1.0f);

    // 8 local box corners
    Vec3 p000 = rot(-s, -s, -s);
    Vec3 p100 = rot( s, -s, -s);
    Vec3 p110 = rot( s,  s, -s);
    Vec3 p010 = rot(-s,  s, -s);
    Vec3 p001 = rot(-s, -s,  s);
    Vec3 p101 = rot( s, -s,  s);
    Vec3 p111 = rot( s,  s,  s);
    Vec3 p011 = rot(-s,  s,  s);

    auto addQuad = [&](const Vec3& v0, const Vec3& v1, const Vec3& v2, const Vec3& v3, const Vec3& n, float shade) {
        Vec4 faceCol = col * shade;
        faceCol.w = 1.0f;
        verts.push_back({v0.x, v0.y, v0.z, 0.0f, 1.0f, n.x, n.y, n.z, faceCol.x, faceCol.y, faceCol.z, faceCol.w, texIdx});
        verts.push_back({v1.x, v1.y, v1.z, 1.0f, 1.0f, n.x, n.y, n.z, faceCol.x, faceCol.y, faceCol.z, faceCol.w, texIdx});
        verts.push_back({v2.x, v2.y, v2.z, 1.0f, 0.0f, n.x, n.y, n.z, faceCol.x, faceCol.y, faceCol.z, faceCol.w, texIdx});

        verts.push_back({v0.x, v0.y, v0.z, 0.0f, 1.0f, n.x, n.y, n.z, faceCol.x, faceCol.y, faceCol.z, faceCol.w, texIdx});
        verts.push_back({v2.x, v2.y, v2.z, 1.0f, 0.0f, n.x, n.y, n.z, faceCol.x, faceCol.y, faceCol.z, faceCol.w, texIdx});
        verts.push_back({v3.x, v3.y, v3.z, 0.0f, 0.0f, n.x, n.y, n.z, faceCol.x, faceCol.y, faceCol.z, faceCol.w, texIdx});
    };

    // 6 faces of the rotating 3D item
    addQuad(p011, p111, p110, p010, {0,  1, 0}, 1.05f); // Top
    addQuad(p000, p100, p101, p001, {0, -1, 0}, 0.65f); // Bottom
    addQuad(p001, p101, p111, p011, {0, 0,  1}, 0.90f); // Front
    addQuad(p100, p000, p010, p110, {0, 0, -1}, 0.80f); // Back
    addQuad(p000, p001, p011, p010, {-1, 0, 0}, 0.75f); // Left
    addQuad(p101, p100, p110, p111, { 1, 0, 0}, 0.85f); // Right
}

} // namespace Aetheria
