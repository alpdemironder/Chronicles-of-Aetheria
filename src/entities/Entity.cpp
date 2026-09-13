#include "Entity.hpp"
#include "../world/World.hpp"
#include "../world/BlockRegistry.hpp"
#include <cmath>
#include <algorithm>

namespace Aetheria {

static inline bool isSolidBlock(World* world, int x, int y, int z) {
    uint16_t bId = world->getBlock(x, y, z);
    if (bId == 0) return false;
    return BlockRegistry::get(bId).isSolid;
}

Entity::Entity(const Vec3& pos, const Vec3& size)
    : position(pos), size(size) {
}

AABB Entity::getAABB() const {
    Vec3 halfSize = { size.x * 0.5f, 0.0f, size.z * 0.5f };
    return AABB(
        Vec3(position.x - halfSize.x, position.y, position.z - halfSize.z),
        Vec3(position.x + halfSize.x, position.y + size.y, position.z + halfSize.z)
    );
}

void Entity::update(World* world, float dt) {
    // Apply gravity
    if (hasGravity) {
        velocity.y -= 26.0f * dt;
        if (velocity.y < -40.0f) velocity.y = -40.0f;
    }

    Vec3 delta = velocity * dt;
    resolveCollisions(world, delta);
}

#include <vector>

void Entity::resolveCollisions(World* world, const Vec3& delta) {
    onGround = false;
    if (!world) {
        position += delta;
        return;
    }

    // 0. Anti-Clipping Recovery Guard:
    // If the entity is currently intersecting solid blocks (e.g. spawned inside or trapped),
    // smoothly pop them upwards out of the solid block if headroom exists.
    AABB curBox = getAABB();
    int curMinX = static_cast<int>(std::floor(curBox.min.x + 0.001f));
    int curMaxX = static_cast<int>(std::floor(curBox.max.x - 0.001f));
    int curMinY = static_cast<int>(std::floor(curBox.min.y + 0.001f));
    int curMaxY = static_cast<int>(std::floor(curBox.max.y - 0.001f));
    int curMinZ = static_cast<int>(std::floor(curBox.min.z + 0.001f));
    int curMaxZ = static_cast<int>(std::floor(curBox.max.z - 0.001f));

    for (int y = curMinY; y <= curMaxY; ++y) {
        for (int z = curMinZ; z <= curMaxZ; ++z) {
            for (int x = curMinX; x <= curMaxX; ++x) {
                if (isSolidBlock(world, x, y, z)) {
                    AABB blk(Vec3(x, y, z), Vec3(x + 1, y + 1, z + 1));
                    if (curBox.intersects(blk)) {
                        float popY = static_cast<float>(y + 1);
                        if (popY >= position.y && popY - position.y <= 1.25f) {
                            position.y = popY;
                            curBox = getAABB();
                        }
                    }
                }
            }
        }
    }

    AABB originalBox = getAABB();
    AABB box = originalBox;
    Vec3 moveDelta = delta;

    // Ledge / Edge fall protection when sneaking/crouching on ground (Minecraft sneak)
    if (isCrouching && onGround) {
        Vec3 halfSize = { size.x * 0.5f, 0.0f, size.z * 0.5f };
        int yUnder = static_cast<int>(std::floor(position.y - 0.2f));

        if (std::abs(moveDelta.x) > 0.0001f) {
            bool groundUnderX = false;
            int testMinX = static_cast<int>(std::floor(position.x + moveDelta.x - halfSize.x));
            int testMaxX = static_cast<int>(std::floor(position.x + moveDelta.x + halfSize.x));
            int testMinZ = static_cast<int>(std::floor(position.z - halfSize.z));
            int testMaxZ = static_cast<int>(std::floor(position.z + halfSize.z));

            for (int z = testMinZ; z <= testMaxZ; ++z) {
                for (int x = testMinX; x <= testMaxX; ++x) {
                    if (isSolidBlock(world, x, yUnder, z)) {
                        groundUnderX = true;
                        break;
                    }
                }
                if (groundUnderX) break;
            }

            if (!groundUnderX) {
                moveDelta.x = 0.0f;
                velocity.x = 0.0f;
            }
        }

        if (std::abs(moveDelta.z) > 0.0001f) {
            bool groundUnderZ = false;
            int testMinX = static_cast<int>(std::floor(position.x - halfSize.x));
            int testMaxX = static_cast<int>(std::floor(position.x + halfSize.x));
            int testMinZ = static_cast<int>(std::floor(position.z + moveDelta.z - halfSize.z));
            int testMaxZ = static_cast<int>(std::floor(position.z + moveDelta.z + halfSize.z));

            for (int z = testMinZ; z <= testMaxZ; ++z) {
                for (int x = testMinX; x <= testMaxX; ++x) {
                    if (isSolidBlock(world, x, yUnder, z)) {
                        groundUnderZ = true;
                        break;
                    }
                }
                if (groundUnderZ) break;
            }

            if (!groundUnderZ) {
                moveDelta.z = 0.0f;
                velocity.z = 0.0f;
            }
        }
    }

    // Broadphase: Gather all solid blocks within the expanded movement bounding box
    AABB broadphase = box.expand(moveDelta.x, moveDelta.y, moveDelta.z);
    broadphase.max.y += stepHeight + 0.1f;
    broadphase.min.y -= 0.5f;

    int scanMinX = static_cast<int>(std::floor(broadphase.min.x)) - 1;
    int scanMaxX = static_cast<int>(std::floor(broadphase.max.x)) + 1;
    int scanMinY = std::max(0, static_cast<int>(std::floor(broadphase.min.y)) - 1);
    int scanMaxY = std::min(CHUNK_Y - 1, static_cast<int>(std::floor(broadphase.max.y)) + 1);
    int scanMinZ = static_cast<int>(std::floor(broadphase.min.z)) - 1;
    int scanMaxZ = static_cast<int>(std::floor(broadphase.max.z)) + 1;

    std::vector<AABB> solidBoxes;
    solidBoxes.reserve(64);
    for (int y = scanMinY; y <= scanMaxY; ++y) {
        for (int z = scanMinZ; z <= scanMaxZ; ++z) {
            for (int x = scanMinX; x <= scanMaxX; ++x) {
                if (isSolidBlock(world, x, y, z)) {
                    solidBoxes.emplace_back(Vec3(x, y, z), Vec3(x + 1, y + 1, z + 1));
                }
            }
        }
    }

    // 1. Move Y first (Swept Y offset)
    float dy = moveDelta.y;
    for (const auto& blk : solidBoxes) {
        dy = box.calculateYOffset(blk, dy);
    }
    box = box.offset(Vec3(0, dy, 0));
    if (moveDelta.y != dy) {
        velocity.y = 0.0f;
        if (moveDelta.y < 0.0f) {
            onGround = true;
        }
    } else {
        onGround = false;
    }

    // 2. Move X (Swept X offset)
    float dx = moveDelta.x;
    for (const auto& blk : solidBoxes) {
        dx = box.calculateXOffset(blk, dx);
    }
    box = box.offset(Vec3(dx, 0, 0));
    if (moveDelta.x != dx) {
        velocity.x = 0.0f;
    }

    // 3. Move Z (Swept Z offset)
    float dz = moveDelta.z;
    for (const auto& blk : solidBoxes) {
        dz = box.calculateZOffset(blk, dz);
    }
    box = box.offset(Vec3(0, 0, dz));
    if (moveDelta.z != dz) {
        velocity.z = 0.0f;
    }

    // 4. Minecraft Step-Up Logic (Smoothly step over 0.5 - 1.0 block slabs and stairs)
    bool collidedHorizontally = (moveDelta.x != dx || moveDelta.z != dz);
    if (onGround && collidedHorizontally && stepHeight > 0.0f) {
        AABB stepBox = originalBox;
        float stepUpY = stepHeight;
        for (const auto& blk : solidBoxes) {
            stepUpY = stepBox.calculateYOffset(blk, stepUpY);
        }
        stepBox = stepBox.offset(Vec3(0, stepUpY, 0));

        float stepDx = moveDelta.x;
        for (const auto& blk : solidBoxes) {
            stepDx = stepBox.calculateXOffset(blk, stepDx);
        }
        stepBox = stepBox.offset(Vec3(stepDx, 0, 0));

        float stepDz = moveDelta.z;
        for (const auto& blk : solidBoxes) {
            stepDz = stepBox.calculateZOffset(blk, stepDz);
        }
        stepBox = stepBox.offset(Vec3(0, 0, stepDz));

        float stepDownY = -stepUpY;
        for (const auto& blk : solidBoxes) {
            stepDownY = stepBox.calculateYOffset(blk, stepDownY);
        }
        stepBox = stepBox.offset(Vec3(0, stepDownY, 0));

        float flatDistSq = dx * dx + dz * dz;
        float stepDistSq = stepDx * stepDx + stepDz * stepDz;

        if (stepDistSq > flatDistSq) {
            box = stepBox;
        }
    }

    // Update entity position from solved bounding box
    position.x = (box.min.x + box.max.x) * 0.5f;
    position.y = box.min.y;
    position.z = (box.min.z + box.max.z) * 0.5f;
}

} // namespace Aetheria
