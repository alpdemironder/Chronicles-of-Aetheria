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

void Entity::resolveCollisions(World* world, const Vec3& delta) {
    onGround = false;

    // 1. Move Y first
    position.y += delta.y;
    AABB boxY = getAABB();

    int minX = static_cast<int>(std::floor(boxY.min.x));
    int maxX = static_cast<int>(std::floor(boxY.max.x));
    int minY = static_cast<int>(std::floor(boxY.min.y));
    int maxY = static_cast<int>(std::floor(boxY.max.y));
    int minZ = static_cast<int>(std::floor(boxY.min.z));
    int maxZ = static_cast<int>(std::floor(boxY.max.z));

    for (int y = minY; y <= maxY; ++y) {
        for (int z = minZ; z <= maxZ; ++z) {
            for (int x = minX; x <= maxX; ++x) {
                if (isSolidBlock(world, x, y, z)) {
                    AABB blockBox(Vec3(x, y, z), Vec3(x + 1, y + 1, z + 1));
                    if (boxY.intersects(blockBox)) {
                        if (delta.y < 0.0f) {
                            position.y = static_cast<float>(y + 1);
                            velocity.y = 0.0f;
                            onGround = true;
                        } else if (delta.y > 0.0f) {
                            position.y = static_cast<float>(y) - size.y;
                            velocity.y = 0.0f;
                        }
                        boxY = getAABB();
                    }
                }
            }
        }
    }

    Vec3 moveDelta = delta;

    // Ledge / Edge fall protection when sneaking/crouching on ground (Minecraft sneak)
    if (isCrouching && onGround) {
        Vec3 halfSize = { size.x * 0.5f, 0.0f, size.z * 0.5f };
        int yUnder = static_cast<int>(std::floor(position.y - 0.2f));

        // Test X delta
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

        // Test Z delta
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

    // 2. Move X
    position.x += moveDelta.x;
    AABB boxX = getAABB();
    minX = static_cast<int>(std::floor(boxX.min.x));
    maxX = static_cast<int>(std::floor(boxX.max.x));
    minY = static_cast<int>(std::floor(boxX.min.y));
    maxY = static_cast<int>(std::floor(boxX.max.y));
    minZ = static_cast<int>(std::floor(boxX.min.z));
    maxZ = static_cast<int>(std::floor(boxX.max.z));

    for (int y = minY; y <= maxY; ++y) {
        for (int z = minZ; z <= maxZ; ++z) {
            for (int x = minX; x <= maxX; ++x) {
                if (isSolidBlock(world, x, y, z)) {
                    AABB blockBox(Vec3(x, y, z), Vec3(x + 1, y + 1, z + 1));
                    if (boxX.intersects(blockBox)) {
                        // Step-up attempt
                        float climbDist = static_cast<float>(y + 1) - position.y;
                        if (onGround && climbDist > 0.0f && climbDist <= stepHeight && !isSolidBlock(world, x, y + 1, z)) {
                            position.y = static_cast<float>(y + 1);
                        } else {
                            if (moveDelta.x > 0.0f) position.x = static_cast<float>(x) - size.x * 0.5f;
                            else if (moveDelta.x < 0.0f) position.x = static_cast<float>(x + 1) + size.x * 0.5f;
                            velocity.x = 0.0f;
                        }
                        boxX = getAABB();
                    }
                }
            }
        }
    }

    // 3. Move Z
    position.z += moveDelta.z;
    AABB boxZ = getAABB();
    minX = static_cast<int>(std::floor(boxZ.min.x));
    maxX = static_cast<int>(std::floor(boxZ.max.x));
    minY = static_cast<int>(std::floor(boxZ.min.y));
    maxY = static_cast<int>(std::floor(boxZ.max.y));
    minZ = static_cast<int>(std::floor(boxZ.min.z));
    maxZ = static_cast<int>(std::floor(boxZ.max.z));

    for (int y = minY; y <= maxY; ++y) {
        for (int z = minZ; z <= maxZ; ++z) {
            for (int x = minX; x <= maxX; ++x) {
                if (isSolidBlock(world, x, y, z)) {
                    AABB blockBox(Vec3(x, y, z), Vec3(x + 1, y + 1, z + 1));
                    if (boxZ.intersects(blockBox)) {
                        float climbDist = static_cast<float>(y + 1) - position.y;
                        if (onGround && climbDist > 0.0f && climbDist <= stepHeight && !isSolidBlock(world, x, y + 1, z)) {
                            position.y = static_cast<float>(y + 1);
                        } else {
                            if (moveDelta.z > 0.0f) position.z = static_cast<float>(z) - size.z * 0.5f;
                            else if (moveDelta.z < 0.0f) position.z = static_cast<float>(z + 1) + size.z * 0.5f;
                            velocity.z = 0.0f;
                        }
                        boxZ = getAABB();
                    }
                }
            }
        }
    }
}

} // namespace Aetheria
