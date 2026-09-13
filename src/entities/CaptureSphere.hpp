#pragma once

#include "Entity.hpp"
#include "Creature.hpp"
#include "../core/Math.hpp"
#include <vector>
#include <memory>
#include <functional>

namespace Aetheria {

class World;
class AudioEngine;

enum class CaptureSphereState {
    Flying,
    Capturing,
    MissedGround,
    Finished
};

class CaptureSphere {
public:
    CaptureSphere(uint16_t sphereItemId, const Vec3& pos, const Vec3& initialVelocity);

    void update(World* world,
                std::vector<std::unique_ptr<Creature>>& creatures,
                AudioEngine* audio,
                float dt,
                std::function<void(const std::string&, const Vec4&)> addNotification);

    void appendModelVertices(std::vector<VoxelVertex>& verts, float totalTime) const;

    bool isDead() const { return state == CaptureSphereState::Finished; }
    bool canPickup() const { return state == CaptureSphereState::MissedGround && missedTimer > 0.4f; }
    void markPickedUp() { state = CaptureSphereState::Finished; }

    uint16_t getSphereItemId() const { return sphereItemId; }
    const Vec3& getPosition() const { return position; }
    CaptureSphereState getState() const { return state; }
    Creature* getTargetCreature() const { return targetCreature; }

    // Computes calculated probability (0.0 to 1.0) of capturing target creature
    static float calculateCaptureChance(uint16_t sphereId, const Creature& creature);

private:
    void triggerBreakout(AudioEngine* audio, std::function<void(const std::string&, const Vec4&)> addNotification);
    void triggerSuccess(AudioEngine* audio, std::function<void(const std::string&, const Vec4&)> addNotification);

    uint16_t sphereItemId;
    Vec3 position;
    Vec3 velocity;

    CaptureSphereState state = CaptureSphereState::Flying;
    Creature* targetCreature = nullptr;

    float yaw = 0.0f;
    float pitch = 0.0f;
    float roll = 0.0f;

    float captureChance = 0.5f;
    float stagePassChance = 0.79f;
    int wobbleStage = 0;
    float captureTimer = 0.0f;

    float missedTimer = 0.0f;
    float lifetime = 180.0f;
};

} // namespace Aetheria
