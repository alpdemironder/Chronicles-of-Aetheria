#pragma once

#include "../core/Math.hpp"
#include <string>

namespace Aetheria {

class World;

class Entity {
public:
    Entity(const Vec3& pos, const Vec3& size);
    virtual ~Entity() = default;

    virtual void update(World* world, float dt);

    const Vec3& getPosition() const { return position; }
    void setPosition(const Vec3& p) { position = p; }

    const Vec3& getVelocity() const { return velocity; }
    void setVelocity(const Vec3& v) { velocity = v; }

    AABB getAABB() const;
    bool isGrounded() const { return onGround; }

    float getYaw() const { return yaw; }
    void setYaw(float y) { yaw = y; }

    bool getHasGravity() const { return hasGravity; }
    void setHasGravity(bool g) { hasGravity = g; }

    void setCrouching(bool c) { isCrouching = c; }
    bool getIsCrouching() const { return isCrouching; }

    float getStepHeight() const { return stepHeight; }
    void setStepHeight(float sh) { stepHeight = sh; }

protected:
    void resolveCollisions(World* world, const Vec3& delta);

    Vec3 position{0, 0, 0};
    Vec3 velocity{0, 0, 0};
    Vec3 size{0.6f, 1.8f, 0.6f}; // Width, Height, Depth
    float yaw = 0.0f;
    bool onGround = false;
    bool hasGravity = true;
    bool isCrouching = false;
    float stepHeight = 0.5f;
};

} // namespace Aetheria
