#pragma once

#include "../core/Math.hpp"
#include "../inventory/ItemStack.hpp"
#include "../world/Chunk.hpp"
#include <vector>

namespace Aetheria {

class World;
class AudioEngine;

class ItemEntity {
public:
    ItemEntity(const ItemStack& stack, const Vec3& pos, const Vec3& initialVel = {0, 3.5f, 0});

    void update(World* world, const Vec3& playerPos, float dt);
    bool canPickup() const { return pickupDelay <= 0.0f && !isPickedUp; }
    void markPickedUp() { isPickedUp = true; }
    bool isDead() const { return isPickedUp || lifetime <= 0.0f; }

    const ItemStack& getItem() const { return item; }
    const Vec3& getPosition() const { return position; }

    void appendVertices(std::vector<VoxelVertex>& verts, float totalTime) const;

private:
    ItemStack item;
    Vec3 position;
    Vec3 velocity;
    float rotation = 0.0f;
    float bobOffset = 0.0f;
    float pickupDelay = 0.4f; // Prevents instantly absorbing upon dropping
    float lifetime = 300.0f;  // 5 minutes despawn timer
    bool onGround = false;
    bool isPickedUp = false;
};

} // namespace Aetheria
