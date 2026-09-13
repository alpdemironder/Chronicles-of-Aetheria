#include "Creature.hpp"
#include "../world/Chunk.hpp"
#include "../world/World.hpp"
#include "../core/Audio.hpp"
#include "../building/BuildingManager.hpp"
#include <cmath>
#include <cstdlib>
#include <algorithm>

namespace Aetheria {

Creature::Creature(CreatureType type, const Vec3& pos)
    : Entity(pos, CreatureRegistry::get(type).size), type(type) {
    const auto& def = CreatureRegistry::get(type);
    maxHP = def.maxHP;
    currentHP = maxHP;
    wanderAngle = static_cast<float>(rand() % 360) * DEG2RAD;
    if (isFlyingType()) {
        setHasGravity(false);
    }
}

bool Creature::isFlyingType() const {
    return (type == CreatureType::Pixie ||
            type == CreatureType::VoidPhantom ||
            type == CreatureType::Harpy);
}

void Creature::setTamed(bool tamed) {
    isTamed = tamed;
    if (tamed) {
        currentHP = maxHP;
        companionTarget = nullptr;
        stance = CompanionStance::Follow;
        int lvl = std::max(1, static_cast<int>(maxHP / 20.0f));
        if (companionLevel < lvl) companionLevel = lvl;
    }
}

void Creature::cycleStance() {
    if (stance == CompanionStance::Follow) {
        stance = CompanionStance::Stay;
    } else if (stance == CompanionStance::Stay) {
        stance = CompanionStance::WorkAtBase;
    } else {
        stance = CompanionStance::Follow;
    }
}

std::string Creature::getStanceName() const {
    switch (stance) {
        case CompanionStance::Follow: return "FOLLOW & DEFEND";
        case CompanionStance::Stay: return "STAY / GUARD";
        case CompanionStance::WorkAtBase: return "WORK AT BASE";
    }
    return "FOLLOW";
}

void Creature::gainCompanionXP(float amount, AudioEngine* audio) {
    if (!isTamed) return;
    companionXP += amount;
    while (companionXP >= companionXPToNext) {
        companionXP -= companionXPToNext;
        companionLevel++;
        companionXPToNext = 100.0f + static_cast<float>(companionLevel - 1) * 75.0f;
        maxHP += 15.0f;
        currentHP = maxHP;
        if (audio) {
            audio->playSound(SoundID::LevelUp, 1.1f, 1.1f);
        }
    }
}

void Creature::notifyMasterDamaged(Creature* attacker) {
    if (isTamed && attacker && !attacker->getIsDead() && stance == CompanionStance::Follow) {
        companionTarget = attacker;
        outOfCombatTimer = 0.0f;
    }
}

void Creature::notifyMasterAttacked(Creature* target) {
    if (isTamed && target && !target->getIsDead() && !target->getIsTamed() && stance == CompanionStance::Follow) {
        companionTarget = target;
        outOfCombatTimer = 0.0f;
    }
}

void Creature::takeDamage(float amount, const Vec3& knockbackDir, AudioEngine* audio) {
    if (isDead) return;

    currentHP -= amount;
    hurtFlashTimer = 0.25f;
    outOfCombatTimer = 0.0f;

    // Apply knockback
    velocity.x += knockbackDir.x * 6.0f;
    velocity.y += 4.0f;
    velocity.z += knockbackDir.z * 6.0f;

    if (audio) {
        audio->playSound(SoundID::CreatureHit, 1.0f + (rand() % 20) / 100.0f, 1.0f);
    }

    if (currentHP <= 0.0f) {
        currentHP = 0.0f;
        isDead = true;
    }
}

void Creature::updateAI(World* world, const Vec3& playerPos, BuildingManager* buildingMgr, AudioEngine* audio, float dt) {
    if (isDead) {
        deathTimer += dt;
        return;
    }
    if (inCapture) {
        return;
    }

    animTime += dt;
    if (attackCooldown > 0.0f) attackCooldown -= dt;
    if (hurtFlashTimer > 0.0f) hurtFlashTimer -= dt;
    stateTimer -= dt;

    const auto& def = CreatureRegistry::get(type);
    Vec3 toPlayer = playerPos - position;
    float distToPlayer = toPlayer.length();

    float targetSpeed = 0.0f;
    Vec3 moveDir{0, 0, 0};

    // =========================================================================
    // 1. TAMED COMPANION AI BRANCH
    // =========================================================================
    if (isTamed) {
        outOfCombatTimer += dt;
        if (outOfCombatTimer >= 3.5f && currentHP < maxHP) {
            currentHP = std::min(maxHP, currentHP + 4.5f * dt);
        }

        // Clean up dead or out-of-range combat target
        if (companionTarget) {
            if (companionTarget->getIsDead() || (companionTarget->getPosition() - position).length() > 32.0f) {
                companionTarget = nullptr;
            }
        }

        if (stance == CompanionStance::Stay) {
            targetSpeed = 0.0f;
            state = AIState::Idle;
            if (companionTarget) {
                Vec3 toTgt = companionTarget->getPosition() - position;
                yaw = -std::atan2(toTgt.z, toTgt.x) * RAD2DEG + 90.0f;
                if (toTgt.length() <= 2.4f + def.size.x * 0.5f) {
                    state = AIState::Attack;
                    if (attackCooldown <= 0.0f) {
                        attackCooldown = 1.1f;
                        if (audio) audio->playSound(SoundID::SwordSwing, 0.9f, 1.1f);
                    }
                }
            }
        } else if (stance == CompanionStance::WorkAtBase) {
            if (buildingMgr && buildingMgr->hasBaseCamp()) {
                Vec3 baseCenter = buildingMgr->getBaseCampCenter();
                Vec3 toCenter = baseCenter - position;
                if (toCenter.length() > buildingMgr->getBaseCampRadius() * 0.9f) {
                    moveDir = Vec3(toCenter.x, 0, toCenter.z).normalized();
                    targetSpeed = def.moveSpeed * 0.85f;
                } else {
                    targetSpeed = def.moveSpeed * 0.45f;
                    if (stateTimer <= 0.0f) {
                        wanderAngle = static_cast<float>(rand() % 360) * DEG2RAD;
                        stateTimer = 2.0f + (rand() % 30) / 10.0f;
                    }
                    moveDir = Vec3(std::cos(wanderAngle), 0, std::sin(wanderAngle));
                }
                if (moveDir.lengthSq() > 0.001f) {
                    yaw = -std::atan2(moveDir.z, moveDir.x) * RAD2DEG + 90.0f;
                }
            } else {
                targetSpeed = 0.0f;
                state = AIState::Idle;
            }
        } else { // CompanionStance::Follow
            if (companionTarget) {
                Vec3 toTgt = companionTarget->getPosition() - position;
                float distTgt = toTgt.length();
                if (distTgt <= 2.2f + def.size.x * 0.5f) {
                    state = AIState::Attack;
                    yaw = -std::atan2(toTgt.z, toTgt.x) * RAD2DEG + 90.0f;
                    targetSpeed = 0.0f;
                    if (attackCooldown <= 0.0f) {
                        attackCooldown = 1.0f;
                        if (audio) audio->playSound(SoundID::SwordSwing, 0.95f, 1.15f);
                    }
                } else {
                    state = AIState::Chase;
                    targetSpeed = def.moveSpeed * 1.25f;
                    moveDir = Vec3(toTgt.x, 0, toTgt.z).normalized();
                    yaw = -std::atan2(moveDir.z, moveDir.x) * RAD2DEG + 90.0f;
                }
            } else {
                // Follow master
                if (distToPlayer > 38.0f) {
                    // Teleport near master if fell too far behind or stuck
                    position = playerPos + Vec3(std::cos(wanderAngle) * 2.8f, 0.5f, std::sin(wanderAngle) * 2.8f);
                    velocity = {0, 0, 0};
                } else if (distToPlayer > 4.5f) {
                    state = AIState::Chase;
                    targetSpeed = def.moveSpeed * (distToPlayer > 12.0f ? 1.4f : 1.05f);
                    moveDir = Vec3(toPlayer.x, 0, toPlayer.z).normalized();
                    yaw = -std::atan2(moveDir.z, moveDir.x) * RAD2DEG + 90.0f;
                } else {
                    state = AIState::Idle;
                    targetSpeed = 0.0f;
                    if (distToPlayer > 0.001f) {
                        yaw = -std::atan2(toPlayer.z, toPlayer.x) * RAD2DEG + 90.0f;
                    }
                }
            }
        }
    } else {
        // =====================================================================
        // 2. WILD / HOSTILE CREATURE AI BRANCH
        // =====================================================================
        // Palworld Base Camp Automation: if base camp exists and mob wanders in
        if (buildingMgr && buildingMgr->hasBaseCamp()) {
            float distToBase = (position - buildingMgr->getBaseCampCenter()).length();
            if (distToBase <= buildingMgr->getBaseCampRadius()) {
                state = AIState::WorkAtBase;
            }
        }

        // AI State Transitions
        if (state != AIState::WorkAtBase) {
            if (def.isHostile) {
                if (distToPlayer <= def.detectionRange) {
                    if (currentHP < maxHP * 0.25f) {
                        state = AIState::Flee;
                    } else if (distToPlayer <= 2.2f + def.size.x * 0.5f) {
                        state = AIState::Attack;
                    } else {
                        state = AIState::Chase;
                    }
                } else {
                    state = AIState::Wander;
                }
            }
        }

        switch (state) {
        case AIState::Idle:
            if (stateTimer <= 0.0f) {
                state = AIState::Wander;
                stateTimer = 2.5f + (rand() % 30) / 10.0f;
                wanderAngle = static_cast<float>(rand() % 360) * DEG2RAD;
            }
            break;

        case AIState::Wander:
            targetSpeed = def.moveSpeed * 0.5f;
            moveDir = Vec3(std::cos(wanderAngle), 0, std::sin(wanderAngle));
            yaw = -wanderAngle * RAD2DEG + 90.0f;

            if (stateTimer <= 0.0f) {
                state = AIState::Idle;
                stateTimer = 1.5f + (rand() % 20) / 10.0f;
            }
            break;

        case AIState::Chase:
            targetSpeed = def.moveSpeed;
            if (distToPlayer > 0.001f) {
                moveDir = Vec3(toPlayer.x, 0, toPlayer.z).normalized();
                yaw = -std::atan2(moveDir.z, moveDir.x) * RAD2DEG + 90.0f;
            }
            break;

        case AIState::Attack:
            yaw = -std::atan2(toPlayer.z, toPlayer.x) * RAD2DEG + 90.0f;
            if (attackCooldown <= 0.0f) {
                attackCooldown = 1.2f;
                if (audio) audio->playSound(SoundID::SwordSwing, 0.8f, 0.9f);
            }
            break;

        case AIState::Flee:
            targetSpeed = def.moveSpeed * 1.2f;
            if (distToPlayer > 0.001f) {
                moveDir = Vec3(-toPlayer.x, 0, -toPlayer.z).normalized();
                yaw = -std::atan2(moveDir.z, moveDir.x) * RAD2DEG + 90.0f;
            }
            break;

        case AIState::WorkAtBase:
            targetSpeed = def.moveSpeed * 0.6f;
            if (buildingMgr) {
                Vec3 baseCenter = buildingMgr->getBaseCampCenter();
                Vec3 toCenter = baseCenter - position;
                if (toCenter.length() > 15.0f) {
                    moveDir = Vec3(toCenter.x, 0, toCenter.z).normalized();
                } else {
                    moveDir = Vec3(std::cos(wanderAngle), 0, std::sin(wanderAngle));
                }
                yaw = -std::atan2(moveDir.z, moveDir.x) * RAD2DEG + 90.0f;
            }
            break;
        }
    }

    // Smooth horizontal velocity towards target direction
    velocity.x += (moveDir.x * targetSpeed - velocity.x) * 10.0f * dt;
    velocity.z += (moveDir.z * targetSpeed - velocity.z) * 10.0f * dt;

    if (isFlyingType()) {
        // Floating / hovering flight physics
        int groundY = world->getHighestBlock(static_cast<int>(std::floor(position.x)), static_cast<int>(std::floor(position.z)));
        float targetY = static_cast<float>(groundY) + 2.4f + std::sin(animTime * 3.5f) * 0.35f;
        velocity.y += (targetY - position.y) * 4.0f * dt;
        velocity.y *= 0.94f; // Air drag
    } else {
        // Obstacle jumping & step climbing
        if (onGround && (moveDir.x != 0.0f || moveDir.z != 0.0f)) {
            int checkX = static_cast<int>(std::floor(position.x + moveDir.x * 0.7f));
            int checkY = static_cast<int>(std::floor(position.y));
            int checkZ = static_cast<int>(std::floor(position.z + moveDir.z * 0.7f));

            if (world->getBlock(checkX, checkY, checkZ) != 0 && world->getBlock(checkX, checkY + 1, checkZ) == 0) {
                velocity.y = 8.5f; // Jump over voxel obstacle
            }
        }
    }

    Entity::update(world, dt);
}

static void appendOrientedBox(std::vector<VoxelVertex>& verts,
                              const Vec3& origin,
                              float yawDeg,
                              const Vec3& localOffset,
                              const Vec3& size,
                              const Vec4& col,
                              float tIdx,
                              float pitchDeg = 0.0f,
                              float rollDeg = 0.0f) {
    Vec3 h = size * 0.5f;

    // 8 local corners centered around localOffset
    Vec3 corners[8] = {
        {-h.x, -h.y, -h.z}, {+h.x, -h.y, -h.z}, {+h.x, +h.y, -h.z}, {-h.x, +h.y, -h.z}, // Back 4 (Z-)
        {-h.x, -h.y, +h.z}, {+h.x, -h.y, +h.z}, {+h.x, +h.y, +h.z}, {-h.x, +h.y, +h.z}  // Front 4 (Z+)
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

        // 4. Rotate around mob Yaw and add mob origin
        return Vec3(
            origin.x + (lx * cy + lz * sy),
            origin.y + ly,
            origin.z + (-lx * sy + lz * cy)
        );
    };

    Vec3 wc[8];
    for (int i = 0; i < 8; ++i) {
        wc[i] = transformPt(corners[i]);
    }

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

    // 6 Faces with counter-clockwise winding:
    addFaceQuad(7, 6, 2, 3, {0, 1, 0}, 1.0f);  // Top (+Y)
    addFaceQuad(0, 1, 5, 4, {0, -1, 0}, 0.55f);// Bottom (-Y)
    addFaceQuad(4, 5, 6, 7, {0, 0, 1}, 0.85f); // Front (+Z)
    addFaceQuad(1, 0, 3, 2, {0, 0, -1}, 0.80f);// Back (-Z)
    addFaceQuad(5, 1, 2, 6, {1, 0, 0}, 0.70f);  // Right (+X)
    addFaceQuad(0, 4, 7, 3, {-1, 0, 0}, 0.75f); // Left (-X)
}

// Renders an aggressive 4-sided pyramid / spike tapering to a sharp apex point
static void appendOrientedSpike(std::vector<VoxelVertex>& verts,
                                const Vec3& origin,
                                float yawDeg,
                                const Vec3& localOffset,
                                const Vec3& baseSize,
                                float height,
                                const Vec4& col,
                                float tIdx,
                                float pitchDeg = 0.0f,
                                float rollDeg = 0.0f) {
    Vec3 h = {baseSize.x * 0.5f, 0.0f, baseSize.z * 0.5f};
    Vec3 corners[5] = {
        {-h.x, 0.0f, -h.z}, {+h.x, 0.0f, -h.z}, {+h.x, 0.0f, +h.z}, {-h.x, 0.0f, +h.z}, // Base 4 (Y=0)
        {0.0f, height, 0.0f} // Apex tip
    };

    float rP = pitchDeg * DEG2RAD;
    float cp = std::cos(rP), sp = std::sin(rP);
    float rR = rollDeg * DEG2RAD;
    float cr = std::cos(rR), sr = std::sin(rR);
    float rY = yawDeg * DEG2RAD;
    float cy = std::cos(rY), sy = std::sin(rY);

    auto transformPt = [&](const Vec3& c) -> Vec3 {
        float y1 = c.y * cp - c.z * sp;
        float z1 = c.y * sp + c.z * cp;
        float x1 = c.x;
        float x2 = x1 * cr - y1 * sr;
        float y2 = x1 * sr + y1 * cr;
        float z2 = z1;
        float lx = x2 + localOffset.x;
        float ly = y2 + localOffset.y;
        float lz = z2 + localOffset.z;
        return Vec3(
            origin.x + (lx * cy + lz * sy),
            origin.y + ly,
            origin.z + (-lx * sy + lz * cy)
        );
    };

    Vec3 wc[5];
    for (int i = 0; i < 5; ++i) wc[i] = transformPt(corners[i]);
    Vec3 spikeCenter = (wc[0] + wc[1] + wc[2] + wc[3] + wc[4]) * 0.2f;

    auto addTri = [&](int i0, int i1, int i2, float shade) {
        Vec3 e1 = wc[i1] - wc[i0];
        Vec3 e2 = wc[i2] - wc[i0];
        Vec3 wn = (e1.cross(e2)).normalized();
        Vec3 faceCenter = (wc[i0] + wc[i1] + wc[i2]) * (1.0f / 3.0f);
        if (wn.dot(faceCenter - spikeCenter) < 0.0f) wn = -wn;
        Vec4 c = {col.x * shade, col.y * shade, col.z * shade, col.w};
        verts.push_back({wc[i0].x, wc[i0].y, wc[i0].z, 0, 0, wn.x, wn.y, wn.z, c.x, c.y, c.z, c.w, tIdx});
        verts.push_back({wc[i1].x, wc[i1].y, wc[i1].z, 1, 0, wn.x, wn.y, wn.z, c.x, c.y, c.z, c.w, tIdx});
        verts.push_back({wc[i2].x, wc[i2].y, wc[i2].z, 0.5f, 1, wn.x, wn.y, wn.z, c.x, c.y, c.z, c.w, tIdx});
    };

    addTri(1, 0, 4, 0.85f); // Back face
    addTri(3, 2, 4, 0.95f); // Front face
    addTri(2, 1, 4, 0.75f); // Right face
    addTri(0, 3, 4, 0.65f); // Left face
    // Base quad
    addTri(0, 1, 2, 0.55f);
    addTri(0, 2, 3, 0.55f);
}

// Renders an aggressive 5-sided wedge/prism slanting along +Z
static void appendOrientedWedge(std::vector<VoxelVertex>& verts,
                                const Vec3& origin,
                                float yawDeg,
                                const Vec3& localOffset,
                                const Vec3& size,
                                const Vec4& col,
                                float tIdx,
                                float pitchDeg = 0.0f,
                                float rollDeg = 0.0f) {
    Vec3 h = size * 0.5f;
    // Back 4 (Z-): full rectangle; Front (Z+): collapsed to bottom edge
    Vec3 corners[6] = {
        {-h.x, -h.y, -h.z}, {+h.x, -h.y, -h.z}, {+h.x, +h.y, -h.z}, {-h.x, +h.y, -h.z}, // Back 4 (Z-)
        {-h.x, -h.y, +h.z}, {+h.x, -h.y, +h.z}                                            // Front 2 (Z+ bottom ridge)
    };

    float rP = pitchDeg * DEG2RAD;
    float cp = std::cos(rP), sp = std::sin(rP);
    float rR = rollDeg * DEG2RAD;
    float cr = std::cos(rR), sr = std::sin(rR);
    float rY = yawDeg * DEG2RAD;
    float cy = std::cos(rY), sy = std::sin(rY);

    auto transformPt = [&](const Vec3& c) -> Vec3 {
        float y1 = c.y * cp - c.z * sp;
        float z1 = c.y * sp + c.z * cp;
        float x1 = c.x;
        float x2 = x1 * cr - y1 * sr;
        float y2 = x1 * sr + y1 * cr;
        float z2 = z1;
        float lx = x2 + localOffset.x;
        float ly = y2 + localOffset.y;
        float lz = z2 + localOffset.z;
        return Vec3(
            origin.x + (lx * cy + lz * sy),
            origin.y + ly,
            origin.z + (-lx * sy + lz * cy)
        );
    };

    Vec3 wc[6];
    for (int i = 0; i < 6; ++i) wc[i] = transformPt(corners[i]);
    Vec3 wedgeCenter = (wc[0] + wc[1] + wc[2] + wc[3] + wc[4] + wc[5]) * (1.0f / 6.0f);

    auto addTri = [&](int i0, int i1, int i2, float shade) {
        Vec3 e1 = wc[i1] - wc[i0];
        Vec3 e2 = wc[i2] - wc[i0];
        Vec3 wn = (e1.cross(e2)).normalized();
        Vec3 faceCenter = (wc[i0] + wc[i1] + wc[i2]) * (1.0f / 3.0f);
        if (wn.dot(faceCenter - wedgeCenter) < 0.0f) wn = -wn;
        Vec4 c = {col.x * shade, col.y * shade, col.z * shade, col.w};
        verts.push_back({wc[i0].x, wc[i0].y, wc[i0].z, 0, 0, wn.x, wn.y, wn.z, c.x, c.y, c.z, c.w, tIdx});
        verts.push_back({wc[i1].x, wc[i1].y, wc[i1].z, 1, 0, wn.x, wn.y, wn.z, c.x, c.y, c.z, c.w, tIdx});
        verts.push_back({wc[i2].x, wc[i2].y, wc[i2].z, 0.5f, 1, wn.x, wn.y, wn.z, c.x, c.y, c.z, c.w, tIdx});
    };

    // Slanted top/front face
    addTri(3, 2, 5, 1.0f);
    addTri(3, 5, 4, 1.0f);
    // Flat bottom face
    addTri(0, 1, 5, 0.55f);
    addTri(0, 5, 4, 0.55f);
    // Back face
    addTri(1, 0, 3, 0.75f);
    addTri(1, 3, 2, 0.75f);
    // Left side triangle
    addTri(0, 4, 3, 0.70f);
    // Right side triangle
    addTri(5, 1, 2, 0.80f);
}

void Creature::appendModelVertices(std::vector<VoxelVertex>& verts, float totalTime) const {
    if (inCapture || (isDead && deathTimer >= 0.8f)) return;

    const auto& def = CreatureRegistry::get(type);
    Vec4 col = def.primaryColor;

    // Damage flash red
    if (hurtFlashTimer > 0.0f) {
        col = {2.2f, 0.3f, 0.3f, 1.0f};
    }

    // Death animation: roll/fall onto side and fade alpha
    float mobYaw = yaw;
    float deathRoll = 0.0f;
    if (isDead) {
        deathRoll = std::min(deathTimer / 0.4f, 1.0f) * 90.0f;
        col.w = std::max(0.0f, 1.0f - deathTimer / 0.8f);
    }

    // Select texture layer
    float tIdx = 390.0f; // Fur default
    switch (type) {
    case CreatureType::Sheep:
    case CreatureType::Boar:
    case CreatureType::Stag:
    case CreatureType::Wolf:
    case CreatureType::Camel:
    case CreatureType::AlpineGoat:
    case CreatureType::Harpy:
        tIdx = 390.0f; // Fur
        break;
    case CreatureType::Goblin:
    case CreatureType::Zombie:
    case CreatureType::Spider:
    case CreatureType::Scorpion:
    case CreatureType::Tortoise:
    case CreatureType::SwampHag:
        tIdx = 391.0f; // Scales / hide
        break;
    case CreatureType::TitanGolem:
    case CreatureType::IceGolem:
    case CreatureType::MagmaDrake:
    case CreatureType::VoidHarbinger:
        tIdx = 392.0f; // Boss rune obsidian
        break;
    case CreatureType::Pixie:
    case CreatureType::VoidPhantom:
        tIdx = 393.0f; // Fae luminescent crystal
        break;
    case CreatureType::Skeleton:
        tIdx = 394.0f; // Bone
        break;
    default:
        tIdx = 390.0f;
        break;
    }

    float walkSwing = std::sin(animTime * 10.0f) * 26.0f;
    float attackLunge = (state == AIState::Attack) ? 15.0f : 0.0f;

    // =========================================================================
    // 1. FLYERS (Pixie, VoidPhantom) - HYTALE STYLIZED FAE
    // =========================================================================
    if (type == CreatureType::Pixie) {
        float hoverY = std::sin(animTime * 4.0f) * 0.15f + 0.5f;
        Vec3 bodyPos = {0, hoverY, 0};

        // Chibi fairy torso
        appendOrientedBox(verts, position, mobYaw, bodyPos, {0.30f, 0.38f, 0.28f}, col * 1.1f, 398.0f, attackLunge, deathRoll);

        // Chibi head with expressive eyes
        Vec3 headPos = bodyPos + Vec3(0, 0.32f, 0.06f);
        appendOrientedBox(verts, position, mobYaw, headPos, {0.32f, 0.32f, 0.32f}, {1.15f, 1.15f, 1.05f, col.w}, tIdx, attackLunge, deathRoll);
        appendOrientedBox(verts, position, mobYaw, headPos + Vec3(0, 0, 0.16f + 0.01f), {0.28f, 0.28f, 0.02f}, {1, 1, 1, col.w}, 395.0f, attackLunge, deathRoll);

        // Flower/Leaf Sprout Hair on head
        appendOrientedBox(verts, position, mobYaw, headPos + Vec3(0, 0.18f, 0), {0.16f, 0.14f, 0.16f}, {0.3f, 0.9f, 0.4f, col.w}, 390.0f, 15.0f, deathRoll);

        // Fluttering Translucent Fae Wings
        float wingFlap = std::sin(animTime * 35.0f) * 45.0f;
        Vec4 wingCol = {0.6f, 0.9f, 1.0f, col.w * 0.85f};
        appendOrientedBox(verts, position, mobYaw, bodyPos + Vec3(-0.25f, 0.15f, -0.1f), {0.45f, 0.04f, 0.32f}, wingCol, 393.0f, 0, wingFlap + deathRoll);
        appendOrientedBox(verts, position, mobYaw, bodyPos + Vec3( 0.25f, 0.15f, -0.1f), {0.45f, 0.04f, 0.32f}, wingCol, 393.0f, 0, -wingFlap + deathRoll);
        return;
    }
    if (type == CreatureType::VoidPhantom) {
        float hoverY = std::sin(animTime * 3.0f) * 0.2f + 0.6f;
        Vec3 bodyPos = {0, hoverY, 0};

        // Shrouded robe torso
        appendOrientedBox(verts, position, mobYaw, bodyPos, {0.55f, 0.85f, 0.45f}, col, 398.0f, attackLunge, deathRoll);

        // Menacing hooded skull
        Vec3 headPos = bodyPos + Vec3(0, 0.58f, 0.12f);
        appendOrientedBox(verts, position, mobYaw, headPos, {0.44f, 0.44f, 0.44f}, {0.3f, 0.15f, 0.4f, col.w}, tIdx, attackLunge, deathRoll);
        // Glowing void eye slit
        appendOrientedBox(verts, position, mobYaw, headPos + Vec3(0, 0, 0.22f + 0.01f), {0.38f, 0.18f, 0.02f}, {2.0f, 0.4f, 2.5f, col.w}, 396.0f, attackLunge, deathRoll);

        // Ethereal trailing shroud ribbons
        float ribbonSway = std::sin(animTime * 6.0f) * 20.0f;
        appendOrientedBox(verts, position, mobYaw, bodyPos + Vec3(-0.2f, -0.45f, 0), {0.14f, 0.55f, 0.14f}, col * 0.7f, tIdx, ribbonSway, deathRoll);
        appendOrientedBox(verts, position, mobYaw, bodyPos + Vec3( 0.2f, -0.45f, 0), {0.14f, 0.55f, 0.14f}, col * 0.7f, tIdx, -ribbonSway, deathRoll);
        return;
    }

    // =========================================================================
    // 2. BIPEDS (Goblin, Skeleton, Zombie, SwampHag, Harpy, IceGolem) - HYTALE RIG
    // =========================================================================
    bool isBiped = (type == CreatureType::Goblin || type == CreatureType::Skeleton ||
                    type == CreatureType::Zombie || type == CreatureType::SwampHag ||
                    type == CreatureType::Harpy || type == CreatureType::IceGolem);

    if (isBiped) {
        float legH = def.size.y * 0.46f;
        float legW = def.size.x * 0.22f;
        float torsoH = def.size.y * 0.44f;
        float torsoW = def.size.x * 0.62f;
        float torsoD = def.size.z * 0.44f;

        // Tapered Booted Legs
        appendOrientedBox(verts, position, mobYaw, {-torsoW * 0.28f, legH * 0.5f, 0}, {legW, legH, legW}, col * 0.85f, (type == CreatureType::Skeleton ? 394.0f : 398.0f), walkSwing, deathRoll);
        appendOrientedBox(verts, position, mobYaw, { torsoW * 0.28f, legH * 0.5f, 0}, {legW, legH, legW}, col * 0.85f, (type == CreatureType::Skeleton ? 394.0f : 398.0f), -walkSwing, deathRoll);

        // Torso with Leather Belt & Tunic
        Vec3 torsoPos = {0, legH + torsoH * 0.5f, 0};
        appendOrientedBox(verts, position, mobYaw, torsoPos, {torsoW, torsoH, torsoD}, col, (type == CreatureType::Skeleton ? 394.0f : 398.0f), attackLunge, deathRoll);

        // Armor Pauldrons on Shoulders with Aggressive Spikes
        float pauldW = torsoW * 0.36f;
        appendOrientedBox(verts, position, mobYaw, torsoPos + Vec3(-torsoW * 0.55f, torsoH * 0.35f, 0), {pauldW, pauldW * 0.65f, pauldW * 1.1f}, {1.1f, 1.1f, 1.15f, col.w}, 397.0f, attackLunge, 15.0f + deathRoll);
        appendOrientedBox(verts, position, mobYaw, torsoPos + Vec3( torsoW * 0.55f, torsoH * 0.35f, 0), {pauldW, pauldW * 0.65f, pauldW * 1.1f}, {1.1f, 1.1f, 1.15f, col.w}, 397.0f, attackLunge, -15.0f + deathRoll);
        // Razor pauldron edge spikes
        appendOrientedSpike(verts, position, mobYaw, torsoPos + Vec3(-torsoW * 0.65f, torsoH * 0.55f, 0), {0.08f, 0.08f, 0.08f}, 0.20f, {1.3f, 1.3f, 1.4f, col.w}, 397.0f, 0, 35.0f + deathRoll);
        appendOrientedSpike(verts, position, mobYaw, torsoPos + Vec3( torsoW * 0.65f, torsoH * 0.55f, 0), {0.08f, 0.08f, 0.08f}, 0.20f, {1.3f, 1.3f, 1.4f, col.w}, 397.0f, 0, -35.0f + deathRoll);

        // Arms
        float leftArmPitch = -walkSwing * 0.8f;
        float rightArmPitch = walkSwing * 0.8f;
        if (state == AIState::Attack) {
            rightArmPitch = -80.0f;
        } else if (state == AIState::Chase || type == CreatureType::Zombie) {
            leftArmPitch = -65.0f;
            rightArmPitch = -65.0f;
        }

        float armW = legW * 0.9f;
        float armH = legH * 0.95f;
        Vec3 leftArmPos = {-torsoW * 0.58f, legH + torsoH * 0.68f, 0};
        Vec3 rightArmPos = { torsoW * 0.58f, legH + torsoH * 0.68f, 0};
        appendOrientedBox(verts, position, mobYaw, leftArmPos, {armW, armH, armW}, col * 0.9f, (type == CreatureType::Skeleton ? 394.0f : 398.0f), leftArmPitch + attackLunge, deathRoll);
        appendOrientedBox(verts, position, mobYaw, rightArmPos, {armW, armH, armW}, col * 0.9f, (type == CreatureType::Skeleton ? 394.0f : 398.0f), rightArmPitch + attackLunge, deathRoll);

        // WEAPONS & SHIELDS!
        if (type == CreatureType::Skeleton) {
            // Honed Steel Broadsword with Razor Wedge Tip in Right Hand!
            Vec3 swordPos = rightArmPos + Vec3(0, -armH * 0.4f, armW * 0.8f);
            appendOrientedBox(verts, position, mobYaw, swordPos, {0.08f, 0.65f, 0.16f}, {1.2f, 1.2f, 1.3f, col.w}, 397.0f, rightArmPitch + attackLunge + 25.0f, deathRoll);
            appendOrientedSpike(verts, position, mobYaw, swordPos + Vec3(0, -0.42f, 0), {0.08f, 0.16f, 0.08f}, -0.22f, {1.4f, 1.4f, 1.5f, col.w}, 397.0f, rightArmPitch + attackLunge + 25.0f, deathRoll);
            // Wooden Buckler Shield on Left Arm!
            Vec3 shieldPos = leftArmPos + Vec3(-armW * 0.4f, -0.05f, 0.05f);
            appendOrientedBox(verts, position, mobYaw, shieldPos, {0.06f, 0.45f, 0.45f}, {0.8f, 0.55f, 0.3f, col.w}, 370.0f, leftArmPitch + attackLunge, deathRoll);
        } else if (type == CreatureType::Goblin) {
            // Jagged Dagger with Razor Tip in Right Hand!
            Vec3 daggerPos = rightArmPos + Vec3(0, -armH * 0.35f, armW * 0.6f);
            appendOrientedWedge(verts, position, mobYaw, daggerPos, {0.06f, 0.35f, 0.14f}, {1.2f, 1.2f, 1.2f, col.w}, 397.0f, rightArmPitch + attackLunge + 30.0f, deathRoll);
        }

        // Chibi Sculpted Head
        float headSz = def.size.x * 0.52f;
        Vec3 headPos = {0, legH + torsoH + headSz * 0.46f, 0.04f};
        appendOrientedBox(verts, position, mobYaw, headPos, {headSz, headSz * 0.92f, headSz}, col * 1.1f, (type == CreatureType::Skeleton ? 394.0f : tIdx), attackLunge * 0.5f, deathRoll);

        // Expressive Face Decal Plate
        appendOrientedBox(verts, position, mobYaw, headPos + Vec3(0, 0, headSz * 0.50f + 0.01f), {headSz * 0.88f, headSz * 0.82f, 0.02f}, {1.0f, 1.0f, 1.0f, col.w}, 395.0f, attackLunge * 0.5f, deathRoll);

        // Goblin Trork Ear Wings & Tusks
        if (type == CreatureType::Goblin) {
            appendOrientedSpike(verts, position, mobYaw, headPos + Vec3(-headSz * 0.52f, 0.06f, -0.05f), {0.12f, 0.08f, 0.12f}, -0.32f, col * 1.2f, tIdx, 0, 90.0f + deathRoll);
            appendOrientedSpike(verts, position, mobYaw, headPos + Vec3( headSz * 0.52f, 0.06f, -0.05f), {0.12f, 0.08f, 0.12f}, 0.32f, col * 1.2f, tIdx, 0, -90.0f + deathRoll);
            // Protruding Underbite Tusks
            appendOrientedSpike(verts, position, mobYaw, headPos + Vec3(-headSz * 0.22f, -headSz * 0.35f, headSz * 0.48f), {0.05f, 0.05f, 0.05f}, 0.18f, {1.3f, 1.3f, 1.2f, col.w}, 394.0f, -25.0f, deathRoll);
            appendOrientedSpike(verts, position, mobYaw, headPos + Vec3( headSz * 0.22f, -headSz * 0.35f, headSz * 0.48f), {0.05f, 0.05f, 0.05f}, 0.18f, {1.3f, 1.3f, 1.2f, col.w}, 394.0f, -25.0f, deathRoll);
        }
        return;
    }

    // =========================================================================
    // 3. WORLD BOSSES (TitanGolem, MagmaDrake, VoidHarbinger) - EDGY TITAN RIG
    // =========================================================================
    if (def.isBoss) {
        float scale = 1.8f;
        float legH = def.size.y * 0.45f * scale;
        float legW = def.size.x * 0.26f * scale;
        float torsoH = def.size.y * 0.45f * scale;
        float torsoW = def.size.x * 0.82f * scale;

        // Chiseled Monolithic Stone Legs
        appendOrientedBox(verts, position, mobYaw, {-torsoW * 0.3f, legH * 0.5f, 0}, {legW, legH, legW}, col * 0.8f, 392.0f, walkSwing * 0.6f, deathRoll);
        appendOrientedBox(verts, position, mobYaw, { torsoW * 0.3f, legH * 0.5f, 0}, {legW, legH, legW}, col * 0.8f, 392.0f, -walkSwing * 0.6f, deathRoll);

        // Massive Monolith Torso
        Vec3 torsoPos = {0, legH + torsoH * 0.5f, 0};
        appendOrientedBox(verts, position, mobYaw, torsoPos, {torsoW, torsoH, torsoW * 0.65f}, col, 392.0f, attackLunge, deathRoll);

        // Glowing Core Crest in Center of Chest
        appendOrientedBox(verts, position, mobYaw, torsoPos + Vec3(0, 0, torsoW * 0.33f + 0.02f), {torsoW * 0.42f, torsoW * 0.42f, 0.04f}, {2.0f, 1.2f, 0.4f, col.w}, 396.0f, attackLunge, deathRoll);

        // Floating Runic Shoulder Monoliths with Obsidian Spikes
        float pSz = torsoW * 0.40f;
        float floatHover = std::sin(animTime * 3.0f) * 0.06f;
        appendOrientedBox(verts, position, mobYaw, {-torsoW * 0.72f, legH + torsoH * 0.85f + floatHover, 0}, {pSz, pSz, pSz}, col * 1.2f, 396.0f, attackLunge, deathRoll);
        appendOrientedBox(verts, position, mobYaw, { torsoW * 0.72f, legH + torsoH * 0.85f + floatHover, 0}, {pSz, pSz, pSz}, col * 1.2f, 396.0f, attackLunge, deathRoll);
        appendOrientedSpike(verts, position, mobYaw, {-torsoW * 0.72f - pSz * 0.5f, legH + torsoH * 0.85f + floatHover, 0}, {0.18f, 0.18f, 0.18f}, -0.35f, col * 0.7f, 392.0f, 0, 90.0f + deathRoll);
        appendOrientedSpike(verts, position, mobYaw, { torsoW * 0.72f + pSz * 0.5f, legH + torsoH * 0.85f + floatHover, 0}, {0.18f, 0.18f, 0.18f}, 0.35f, col * 0.7f, 392.0f, 0, -90.0f + deathRoll);

        // Heavy Hammer Fists with Knuckle Spikes
        float armSwing = (state == AIState::Attack) ? -85.0f : (walkSwing * 0.5f);
        float armH = legH * 0.92f;
        appendOrientedBox(verts, position, mobYaw, {-torsoW * 0.65f, legH * 0.5f, 0}, {legW * 1.35f, armH, legW * 1.35f}, col * 0.9f, 392.0f, -armSwing, deathRoll);
        appendOrientedBox(verts, position, mobYaw, { torsoW * 0.65f, legH * 0.5f, 0}, {legW * 1.35f, armH, legW * 1.35f}, col * 0.9f, 392.0f, armSwing, deathRoll);
        appendOrientedSpike(verts, position, mobYaw, {-torsoW * 0.65f, legH * 0.30f, legW * 0.68f}, {0.10f, 0.10f, 0.10f}, 0.22f, {1.8f, 0.9f, 0.2f, col.w}, 396.0f, -armSwing + 80.0f, deathRoll);
        appendOrientedSpike(verts, position, mobYaw, { torsoW * 0.65f, legH * 0.30f, legW * 0.68f}, {0.10f, 0.10f, 0.10f}, 0.22f, {1.8f, 0.9f, 0.2f, col.w}, 396.0f,  armSwing + 80.0f, deathRoll);

        // Head with Glowing Runic Visor
        float headSz = torsoW * 0.46f;
        Vec3 headPos = {0, legH + torsoH + headSz * 0.38f, 0.1f};
        appendOrientedBox(verts, position, mobYaw, headPos, {headSz, headSz * 0.85f, headSz * 0.85f}, col * 1.3f, 392.0f, attackLunge, deathRoll);
        appendOrientedBox(verts, position, mobYaw, headPos + Vec3(0, 0, headSz * 0.43f + 0.01f), {headSz * 0.84f, headSz * 0.35f, 0.04f}, {2.8f, 0.8f, 0.2f, col.w}, 396.0f, attackLunge, deathRoll);
        return;
    }

    // =========================================================================
    // 4. QUADRUPEDS (Wolf, Boar, Stag, Sheep, Camel, AlpineGoat) - EDGY BEASTS
    // =========================================================================
    float legH = def.size.y * 0.45f;
    float legW = def.size.x * 0.22f;
    float bodyH = def.size.y * 0.48f;
    float bodyW = def.size.x * 0.72f;
    float bodyD = def.size.z * 1.05f;

    // 4 Articulated Legs
    float offX = bodyW * 0.35f;
    float offZ = bodyD * 0.34f;
    appendOrientedBox(verts, position, mobYaw, {-offX, legH * 0.5f,  offZ}, {legW, legH, legW}, col * 0.85f, tIdx,  walkSwing, deathRoll);
    appendOrientedBox(verts, position, mobYaw, { offX, legH * 0.5f,  offZ}, {legW, legH, legW}, col * 0.85f, tIdx, -walkSwing, deathRoll);
    appendOrientedBox(verts, position, mobYaw, {-offX, legH * 0.5f, -offZ}, {legW, legH, legW}, col * 0.85f, tIdx, -walkSwing, deathRoll);
    appendOrientedBox(verts, position, mobYaw, { offX, legH * 0.5f, -offZ}, {legW, legH, legW}, col * 0.85f, tIdx,  walkSwing, deathRoll);

    // Torso with Fur Shading
    Vec3 bodyPos = {0, legH + bodyH * 0.5f, 0};
    appendOrientedBox(verts, position, mobYaw, bodyPos, {bodyW, bodyH, bodyD}, col, tIdx, attackLunge, deathRoll);

    // Sculpted Head with 3D Snout & Expressive Eyes
    float headSz = def.size.x * 0.50f;
    Vec3 headPos = {0, legH + bodyH * 0.82f, offZ + headSz * 0.42f};
    appendOrientedBox(verts, position, mobYaw, headPos, {headSz, headSz * 0.90f, headSz}, col * 1.12f, tIdx, attackLunge, deathRoll);

    // Expressive Eye Plate
    appendOrientedBox(verts, position, mobYaw, headPos + Vec3(0, 0.05f, headSz * 0.50f + 0.01f), {headSz * 0.84f, headSz * 0.65f, 0.02f}, {1.0f, 1.0f, 1.0f, col.w}, 395.0f, attackLunge, deathRoll);

    if (type == CreatureType::Wolf) {
        // Shadowfang Wolf: Sleek, Aggressive, Edgy Predator Design
        // 1. Tapered Predator Wedge Snout
        appendOrientedWedge(verts, position, mobYaw, headPos + Vec3(0, -headSz * 0.15f, headSz * 0.40f), {headSz * 0.46f, headSz * 0.32f, headSz * 0.52f}, col * 1.05f, 390.0f, attackLunge, deathRoll);

        // 2. Razor Upper Fangs (Ivory Spikes protruding down)
        appendOrientedSpike(verts, position, mobYaw, headPos + Vec3(-headSz * 0.16f, -headSz * 0.28f, headSz * 0.58f), {0.04f, 0.04f, 0.04f}, -0.14f, {1.2f, 1.2f, 1.15f, col.w}, 394.0f, attackLunge, deathRoll);
        appendOrientedSpike(verts, position, mobYaw, headPos + Vec3( headSz * 0.16f, -headSz * 0.28f, headSz * 0.58f), {0.04f, 0.04f, 0.04f}, -0.14f, {1.2f, 1.2f, 1.15f, col.w}, 394.0f, attackLunge, deathRoll);

        // 3. Razor Predator Ears (Swept back spikes)
        appendOrientedSpike(verts, position, mobYaw, headPos + Vec3(-headSz * 0.32f, headSz * 0.45f, -0.05f), {0.12f, 0.10f, 0.10f}, 0.28f, col * 1.25f, 390.0f, 15.0f, 22.0f + deathRoll);
        appendOrientedSpike(verts, position, mobYaw, headPos + Vec3( headSz * 0.32f, headSz * 0.45f, -0.05f), {0.12f, 0.10f, 0.10f}, 0.28f, col * 1.25f, 390.0f, 15.0f, -22.0f + deathRoll);

        // 4. Aggressive Spiked Dorsal Mane (3 Razor fur quills running down the nape)
        appendOrientedSpike(verts, position, mobYaw, bodyPos + Vec3(0, bodyH * 0.48f, offZ * 0.70f), {0.15f, 0.12f, 0.15f}, 0.24f, col * 1.2f, 390.0f, -25.0f, deathRoll);
        appendOrientedSpike(verts, position, mobYaw, bodyPos + Vec3(0, bodyH * 0.48f, 0.0f), {0.15f, 0.12f, 0.15f}, 0.22f, col * 1.2f, 390.0f, -25.0f, deathRoll);
        appendOrientedSpike(verts, position, mobYaw, bodyPos + Vec3(0, bodyH * 0.48f, -offZ * 0.70f), {0.15f, 0.12f, 0.15f}, 0.20f, col * 1.2f, 390.0f, -25.0f, deathRoll);

        // 5. Razor Claws on Front Paws
        appendOrientedSpike(verts, position, mobYaw, {-offX - 0.04f, 0.04f, offZ + legW * 0.5f}, {0.03f, 0.03f, 0.03f}, 0.07f, {0.15f, 0.15f, 0.18f, col.w}, 394.0f, -80.0f, deathRoll);
        appendOrientedSpike(verts, position, mobYaw, {-offX + 0.04f, 0.04f, offZ + legW * 0.5f}, {0.03f, 0.03f, 0.03f}, 0.07f, {0.15f, 0.15f, 0.18f, col.w}, 394.0f, -80.0f, deathRoll);
        appendOrientedSpike(verts, position, mobYaw, { offX - 0.04f, 0.04f, offZ + legW * 0.5f}, {0.03f, 0.03f, 0.03f}, 0.07f, {0.15f, 0.15f, 0.18f, col.w}, 394.0f, -80.0f, deathRoll);
        appendOrientedSpike(verts, position, mobYaw, { offX + 0.04f, 0.04f, offZ + legW * 0.5f}, {0.03f, 0.03f, 0.03f}, 0.07f, {0.15f, 0.15f, 0.18f, col.w}, 394.0f, -80.0f, deathRoll);

        // Bushy Tail
        float tailSway = std::sin(animTime * 7.0f) * 22.0f;
        appendOrientedBox(verts, position, mobYaw, {0, legH + bodyH * 0.65f, -offZ - 0.25f}, {0.16f, 0.16f, 0.55f}, col * 0.95f, 390.0f, 30.0f, tailSway + deathRoll);
    }
    else if (type == CreatureType::Boar) {
        // 3D Wedge Snout with Razor Ivory Tusks
        appendOrientedWedge(verts, position, mobYaw, headPos + Vec3(0, -headSz * 0.14f, headSz * 0.36f), {headSz * 0.58f, headSz * 0.40f, headSz * 0.48f}, col * 1.1f, 390.0f, attackLunge, deathRoll);
        // Menacing upward curving tusks
        appendOrientedSpike(verts, position, mobYaw, headPos + Vec3(-headSz * 0.42f, -0.08f, headSz * 0.44f), {0.07f, 0.07f, 0.07f}, 0.32f, {1.3f, 1.3f, 1.2f, col.w}, 394.0f, -40.0f, 20.0f + deathRoll);
        appendOrientedSpike(verts, position, mobYaw, headPos + Vec3( headSz * 0.42f, -0.08f, headSz * 0.44f), {0.07f, 0.07f, 0.07f}, 0.32f, {1.3f, 1.3f, 1.2f, col.w}, 394.0f, -40.0f, -20.0f + deathRoll);
    }
    else if (type == CreatureType::Stag) {
        // Multi-tier Branching Antlers
        Vec3 antlerBase = headPos + Vec3(0, headSz * 0.52f + 0.05f, -0.05f);
        appendOrientedSpike(verts, position, mobYaw, antlerBase + Vec3(-0.25f, 0.10f, 0), {0.08f, 0.08f, 0.08f}, 0.65f, {1.1f, 1.05f, 0.9f, col.w}, 394.0f, 0, 32.0f + deathRoll);
        appendOrientedSpike(verts, position, mobYaw, antlerBase + Vec3( 0.25f, 0.10f, 0), {0.08f, 0.08f, 0.08f}, 0.65f, {1.1f, 1.05f, 0.9f, col.w}, 394.0f, 0, -32.0f + deathRoll);
        appendOrientedSpike(verts, position, mobYaw, antlerBase + Vec3(-0.38f, 0.35f, 0.1f), {0.06f, 0.06f, 0.06f}, 0.35f, {1.1f, 1.05f, 0.9f, col.w}, 394.0f, 25.0f, 20.0f + deathRoll);
        appendOrientedSpike(verts, position, mobYaw, antlerBase + Vec3( 0.38f, 0.35f, 0.1f), {0.06f, 0.06f, 0.06f}, 0.35f, {1.1f, 1.05f, 0.9f, col.w}, 394.0f, 25.0f, -20.0f + deathRoll);
    }
    else if (type == CreatureType::Sheep) {
        // Fluffy Wool Layers
        appendOrientedBox(verts, position, mobYaw, bodyPos, {bodyW * 1.18f, bodyH * 1.15f, bodyD * 1.12f}, {1.05f, 1.05f, 1.02f, col.w}, 390.0f, attackLunge, deathRoll);
        // Floppy Ears
        appendOrientedBox(verts, position, mobYaw, headPos + Vec3(-headSz * 0.52f, 0.05f, 0), {0.12f, 0.20f, 0.1f}, {0.3f, 0.3f, 0.3f, col.w}, 390.0f, 0, 25.0f + deathRoll);
        appendOrientedBox(verts, position, mobYaw, headPos + Vec3( headSz * 0.52f, 0.05f, 0), {0.12f, 0.20f, 0.1f}, {0.3f, 0.3f, 0.3f, col.w}, 390.0f, 0, -25.0f + deathRoll);
    }
    else if (type == CreatureType::AlpineGoat) {
        // Swept-back Faceted Mountain Horns
        appendOrientedWedge(verts, position, mobYaw, headPos + Vec3(-headSz * 0.30f, headSz * 0.48f, -0.05f), {0.10f, 0.12f, 0.38f}, {1.2f, 1.15f, 0.95f, col.w}, 394.0f, 45.0f, 15.0f + deathRoll);
        appendOrientedWedge(verts, position, mobYaw, headPos + Vec3( headSz * 0.30f, headSz * 0.48f, -0.05f), {0.10f, 0.12f, 0.38f}, {1.2f, 1.15f, 0.95f, col.w}, 394.0f, 45.0f, -15.0f + deathRoll);
    }
}

} // namespace Aetheria
