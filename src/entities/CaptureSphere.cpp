#include "CaptureSphere.hpp"
#include "../world/World.hpp"
#include "../world/Chunk.hpp"
#include "../core/Audio.hpp"
#include <cmath>
#include <cstdlib>
#include <algorithm>

namespace Aetheria {

CaptureSphere::CaptureSphere(uint16_t sphereItemId, const Vec3& pos, const Vec3& initialVelocity)
    : sphereItemId(sphereItemId), position(pos), velocity(initialVelocity) {
    yaw = -std::atan2(velocity.z, velocity.x) * RAD2DEG + 90.0f;
    float horizDist = std::sqrt(velocity.x * velocity.x + velocity.z * velocity.z);
    pitch = -std::atan2(velocity.y, horizDist) * RAD2DEG;
}

float CaptureSphere::calculateCaptureChance(uint16_t sphereId, const Creature& creature) {
    float spherePower = 1.0f;
    if (sphereId == 526) spherePower = 2.0f;      // Mega Sphere
    else if (sphereId == 527) spherePower = 3.5f; // Giga Sphere

    float hpFrac = creature.getHealth() / creature.getMaxHealth();
    // Weakened bonus: 100% HP => 0.28 multiplier, 10% HP => 0.928 multiplier
    float hpFactor = (1.0f - hpFrac * 0.72f);

    float baseRate = 0.65f;
    if (creature.getDef().isBoss) {
        baseRate = 0.18f;
    } else if (creature.getMaxHealth() >= 80.0f) {
        baseRate = 0.38f;
    }

    float chance = baseRate * hpFactor * spherePower;
    return std::clamp(chance, 0.05f, 0.98f);
}

void CaptureSphere::triggerBreakout(AudioEngine* audio, std::function<void(const std::string&, const Vec4&)> addNotification) {
    if (targetCreature) {
        targetCreature->setInCapture(false);
    }
    if (audio) {
        audio->playSound(SoundID::CreatureHit, 1.2f, 1.0f);
    }
    if (addNotification) {
        addNotification("The creature broke free! Weaken it more!", {0.95f, 0.35f, 0.35f, 1.0f});
    }
    state = CaptureSphereState::Finished;
}

void CaptureSphere::triggerSuccess(AudioEngine* audio, std::function<void(const std::string&, const Vec4&)> addNotification) {
    if (targetCreature) {
        targetCreature->setInCapture(false);
        targetCreature->setTamed(true);
        int lvl = targetCreature->getCompanionLevel();
        if (audio) {
            audio->playSound(SoundID::LevelUp, 1.0f, 1.0f);
        }
        if (addNotification) {
            addNotification("★ Captured " + targetCreature->getDef().name + " (Lv." + std::to_string(lvl) + ")! ★", {0.25f, 0.88f, 1.0f, 1.0f});
        }
    }
    state = CaptureSphereState::Finished;
}

void CaptureSphere::update(World* world,
                           std::vector<std::unique_ptr<Creature>>& creatures,
                           AudioEngine* audio,
                           float dt,
                           std::function<void(const std::string&, const Vec4&)> addNotification) {
    if (state == CaptureSphereState::Finished) return;

    if (state == CaptureSphereState::Flying) {
        // Apply flight ballistics
        velocity.y -= 22.0f * dt;
        velocity.x *= (1.0f - 0.25f * dt);
        velocity.z *= (1.0f - 0.25f * dt);

        float horizDist = std::sqrt(velocity.x * velocity.x + velocity.z * velocity.z);
        if (horizDist > 0.05f) {
            yaw = -std::atan2(velocity.z, velocity.x) * RAD2DEG + 90.0f;
            pitch = -std::atan2(velocity.y, horizDist) * RAD2DEG;
        }

        Vec3 newPos = position + velocity * dt;

        // Check collision with wild creatures along flight path
        for (const auto& c : creatures) {
            if (!c->getIsDead() && !c->getIsTamed() && !c->isInCapture()) {
                Vec3 toMob = c->getPosition() + Vec3(0, c->getDef().size.y * 0.5f, 0) - position;
                float d = toMob.length();
                if (d < 1.6f + c->getDef().size.x * 0.45f) {
                    // Strike! Initiate capture sequence
                    targetCreature = c.get();
                    targetCreature->setInCapture(true);
                    position = targetCreature->getPosition() + Vec3(0, targetCreature->getDef().size.y * 0.45f, 0);
                    velocity = {0, 0, 0};
                    state = CaptureSphereState::Capturing;

                    captureChance = calculateCaptureChance(sphereItemId, *targetCreature);
                    stagePassChance = std::pow(captureChance, 1.0f / 3.0f);
                    wobbleStage = 0;
                    captureTimer = 0.0f;
                    roll = 0.0f;

                    if (audio) {
                        audio->playSound(SoundID::ItemPickup, 1.3f, 0.85f);
                    }
                    return;
                }
            }
        }

        // Terrain collision check
        int bx = static_cast<int>(std::floor(newPos.x));
        int by = static_cast<int>(std::floor(newPos.y));
        int bz = static_cast<int>(std::floor(newPos.z));

        if (world->getBlock(bx, by, bz) != 0) {
            if (velocity.lengthSq() > 6.0f) {
                // Bounce off surface
                velocity.y = -velocity.y * 0.35f;
                velocity.x *= 0.5f;
                velocity.z *= 0.5f;
                if (audio) {
                    audio->playSound(SoundID::BlockPlace, 0.7f, 1.3f);
                }
            } else {
                // Settle on ground
                state = CaptureSphereState::MissedGround;
                velocity = {0, 0, 0};
                missedTimer = 0.0f;
            }
        }

        position = newPos;
    }
    else if (state == CaptureSphereState::Capturing) {
        captureTimer += dt;

        if (!targetCreature || targetCreature->getIsDead()) {
            triggerBreakout(audio, addNotification);
            return;
        }

        // Wobble roll animation
        if (captureTimer >= 0.5f && captureTimer <= 0.95f) {
            roll = std::sin((captureTimer - 0.5f) * 22.0f) * 25.0f;
        } else if (captureTimer >= 1.3f && captureTimer <= 1.75f) {
            roll = -std::sin((captureTimer - 1.3f) * 22.0f) * 25.0f;
        } else if (captureTimer >= 2.1f && captureTimer <= 2.55f) {
            roll = std::sin((captureTimer - 2.1f) * 24.0f) * 28.0f;
        } else {
            roll = 0.0f;
        }

        // 3-Stage Wobble Probability Checkpoints
        if (captureTimer >= 0.75f && wobbleStage == 0) {
            wobbleStage = 1;
            float r = static_cast<float>(std::rand() % 1000) / 1000.0f;
            if (r > stagePassChance) {
                triggerBreakout(audio, addNotification);
                return;
            } else if (audio) {
                audio->playSound(SoundID::ItemPickup, 1.2f, 0.95f);
            }
        }
        else if (captureTimer >= 1.55f && wobbleStage == 1) {
            wobbleStage = 2;
            float r = static_cast<float>(std::rand() % 1000) / 1000.0f;
            if (r > stagePassChance) {
                triggerBreakout(audio, addNotification);
                return;
            } else if (audio) {
                audio->playSound(SoundID::ItemPickup, 1.2f, 1.15f);
            }
        }
        else if (captureTimer >= 2.35f && wobbleStage == 2) {
            wobbleStage = 3;
            float r = static_cast<float>(std::rand() % 1000) / 1000.0f;
            if (r > stagePassChance) {
                triggerBreakout(audio, addNotification);
                return;
            } else if (audio) {
                audio->playSound(SoundID::ItemPickup, 1.3f, 1.35f);
            }
        }
        else if (captureTimer >= 2.95f && wobbleStage == 3) {
            triggerSuccess(audio, addNotification);
            return;
        }
    }
    else if (state == CaptureSphereState::MissedGround) {
        missedTimer += dt;
        if (missedTimer > lifetime) {
            state = CaptureSphereState::Finished;
        }
    }
}

static void appendSphereBox(std::vector<VoxelVertex>& verts,
                           const Vec3& origin,
                           float yawDeg,
                           float pitchDeg,
                           float rollDeg,
                           const Vec3& localOffset,
                           const Vec3& size,
                           const Vec4& col,
                           float tIdx) {
    Vec3 h = size * 0.5f;
    Vec3 corners[8] = {
        {-h.x, -h.y, -h.z}, {+h.x, -h.y, -h.z}, {+h.x, +h.y, -h.z}, {-h.x, +h.y, -h.z},
        {-h.x, -h.y, +h.z}, {+h.x, -h.y, +h.z}, {+h.x, +h.y, +h.z}, {-h.x, +h.y, +h.z}
    };

    float rP = pitchDeg * DEG2RAD;
    float cp = std::cos(rP), sp = std::sin(rP);
    float rR = rollDeg * DEG2RAD;
    float cr = std::cos(rR), sr = std::sin(rR);
    float rY = yawDeg * DEG2RAD;
    float cy = std::cos(rY), sy = std::sin(rY);

    auto transformPt = [&](const Vec3& c) -> Vec3 {
        // 1. Pitch around X
        float y1 = c.y * cp - c.z * sp;
        float z1 = c.y * sp + c.z * cp;
        float x1 = c.x;

        // 2. Roll around Z
        float x2 = x1 * cr - y1 * sr;
        float y2 = x1 * sr + y1 * cr;
        float z2 = z1;

        // 3. Local translation
        float lx = x2 + localOffset.x;
        float ly = y2 + localOffset.y;
        float lz = z2 + localOffset.z;

        // 4. Rotate around mob Yaw and add origin
        return Vec3(
            origin.x + (lx * cy + lz * sy),
            origin.y + ly,
            origin.z + (-lx * sy + lz * cy)
        );
    };

    Vec3 wc[8];
    for (int i = 0; i < 8; ++i) wc[i] = transformPt(corners[i]);

    auto transformNorm = [&](const Vec3& n) -> Vec3 {
        float y1 = n.y * cp - n.z * sp;
        float z1 = n.y * sp + n.z * cp;
        float x1 = n.x;

        float x2 = x1 * cr - y1 * sr;
        float y2 = x1 * sr + y1 * cr;
        float z2 = z1;

        return Vec3(
            x2 * cy + z2 * sy,
            y2,
            -x2 * sy + z2 * cy
        ).normalized();
    };

    auto addFaceQuad = [&](int i0, int i1, int i2, int i3, const Vec3& localNorm, float shade) {
        Vec3 wn = transformNorm(localNorm);
        Vec4 c = {col.x * shade, col.y * shade, col.z * shade, col.w};
        verts.push_back({wc[i0].x, wc[i0].y, wc[i0].z, 0, 0, wn.x, wn.y, wn.z, c.x, c.y, c.z, c.w, tIdx});
        verts.push_back({wc[i1].x, wc[i1].y, wc[i1].z, 1, 0, wn.x, wn.y, wn.z, c.x, c.y, c.z, c.w, tIdx});
        verts.push_back({wc[i2].x, wc[i2].y, wc[i2].z, 1, 1, wn.x, wn.y, wn.z, c.x, c.y, c.z, c.w, tIdx});

        verts.push_back({wc[i0].x, wc[i0].y, wc[i0].z, 0, 0, wn.x, wn.y, wn.z, c.x, c.y, c.z, c.w, tIdx});
        verts.push_back({wc[i2].x, wc[i2].y, wc[i2].z, 1, 1, wn.x, wn.y, wn.z, c.x, c.y, c.z, c.w, tIdx});
        verts.push_back({wc[i3].x, wc[i3].y, wc[i3].z, 0, 1, wn.x, wn.y, wn.z, c.x, c.y, c.z, c.w, tIdx});
    };

    addFaceQuad(7, 6, 2, 3, {0, 1, 0}, 1.0f);   // Top (+Y)
    addFaceQuad(0, 1, 5, 4, {0, -1, 0}, 0.55f); // Bottom (-Y)
    addFaceQuad(4, 5, 6, 7, {0, 0, 1}, 0.85f);  // Front (+Z)
    addFaceQuad(1, 0, 3, 2, {0, 0, -1}, 0.80f); // Back (-Z)
    addFaceQuad(5, 1, 2, 6, {1, 0, 0}, 0.70f);  // Right (+X)
    addFaceQuad(0, 4, 7, 3, {-1, 0, 0}, 0.75f); // Left (-X)
}

void CaptureSphere::appendModelVertices(std::vector<VoxelVertex>& verts, float totalTime) const {
    if (state == CaptureSphereState::Finished) return;

    // Sphere color based on tier
    Vec4 topColor(0.15f, 0.50f, 0.95f, 1.0f); // Pal Sphere blue
    Vec4 coreColor(0.0f, 0.95f, 1.0f, 1.5f);  // Cyan button
    if (sphereItemId == 526) {
        topColor = Vec4(0.12f, 0.88f, 0.65f, 1.0f); // Mega Sphere emerald
        coreColor = Vec4(0.95f, 0.85f, 0.20f, 1.5f); // Amber button
    } else if (sphereItemId == 527) {
        topColor = Vec4(0.72f, 0.20f, 0.92f, 1.0f); // Giga Sphere purple
        coreColor = Vec4(1.0f, 0.35f, 0.95f, 1.5f); // Magenta/gold button
    }

    Vec4 bottomColor(0.92f, 0.94f, 0.96f, 1.0f); // White lower shell
    Vec4 seamColor(0.12f, 0.14f, 0.18f, 1.0f);   // Dark equator seam

    float tIdx = 186.0f; // High-tech metallic/crystal texture layer

    Vec3 renderOrigin = position;
    if (state == CaptureSphereState::Capturing) {
        // Floating hover during capture
        renderOrigin.y += std::sin(totalTime * 4.0f) * 0.05f;
    }

    // 1. Top Hemisphere
    appendSphereBox(verts, renderOrigin, yaw, pitch, roll, {0, 0.08f, 0}, {0.32f, 0.16f, 0.32f}, topColor, tIdx);

    // 2. Bottom Hemisphere
    appendSphereBox(verts, renderOrigin, yaw, pitch, roll, {0, -0.08f, 0}, {0.32f, 0.16f, 0.32f}, bottomColor, tIdx);

    // 3. Equator Seam Belt
    appendSphereBox(verts, renderOrigin, yaw, pitch, roll, {0, 0.0f, 0}, {0.33f, 0.04f, 0.33f}, seamColor, tIdx);

    // 4. Central Energy Core Button (Front)
    appendSphereBox(verts, renderOrigin, yaw, pitch, roll, {0, 0.0f, 0.165f}, {0.10f, 0.10f, 0.04f}, coreColor, tIdx);
}

} // namespace Aetheria
