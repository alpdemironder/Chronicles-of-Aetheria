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
    return false;
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

    // Minecraft Enderman teleportation when struck
    if (type == CreatureType::Enderman && currentHP > 0.0f) {
        float tAng = static_cast<float>(std::rand() % 360) * DEG2RAD;
        float tDist = 8.0f + static_cast<float>(std::rand() % 7);
        position.x += std::cos(tAng) * tDist;
        position.z += std::sin(tAng) * tDist;
        velocity = {0, 0, 0};
        if (audio) audio->playSound(SoundID::LevelUp, 1.8f, 0.6f);
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

    // Minecraft Creeper ticking fuse & explosive detonation
    if (type == CreatureType::Creeper && !isTamed) {
        if (distToPlayer <= 3.8f) {
            targetSpeed = 0.0f; // Freeze in place while hissing
            creeperFuse += dt;
            if (audio && creeperFuse <= dt * 1.5f) {
                audio->playSound(SoundID::BlockBreak, 0.45f, 1.9f); // Iconic Creeper hiss!
            }
            if (creeperFuse >= 1.35f) {
                // TNT Detonation!
                if (audio) audio->playSound(SoundID::BlockBreak, 2.0f, 0.45f);
                currentHP = 0.0f;
                isDead = true;
                deathTimer = 0.0f;
            }
        } else {
            creeperFuse = std::max(0.0f, creeperFuse - dt * 1.2f);
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
    float tIdx = 390.0f; // Fur/Wool default
    switch (type) {
    case CreatureType::Cow:
    case CreatureType::Sheep:
    case CreatureType::Pig:
    case CreatureType::Horse:
        tIdx = 390.0f; // Wool / Animal Hide
        break;
    case CreatureType::Chicken:
        tIdx = 390.0f; // Feathers
        break;
    case CreatureType::Zombie:
        tIdx = 398.0f; // Zombie Clothes
        break;
    case CreatureType::Skeleton:
        tIdx = 394.0f; // Skeleton Bone
        break;
    case CreatureType::Spider:
        tIdx = 391.0f; // Spider Dark Chitin
        break;
    case CreatureType::Creeper:
        tIdx = 393.0f; // Creeper Mottled Green Camo
        break;
    case CreatureType::Enderman:
        tIdx = 392.0f; // Enderman Obsidian Void Skin
        break;
    default:
        tIdx = 390.0f;
        break;
    }

    float walkSwing = std::sin(animTime * 10.0f) * 26.0f;
    float attackLunge = (state == AIState::Attack) ? 15.0f : 0.0f;

    // =========================================================================
    // 1. MINECRAFT CREEPER (Iconic 4-Legged Green Explosive Terror)
    // =========================================================================
    if (type == CreatureType::Creeper) {
        float swell = 1.0f;
        Vec4 creeperCol = col;
        if (creeperFuse > 0.0f) {
            swell += (creeperFuse / 1.35f) * 0.32f; // Swells up by 32% while hissing!
            if (static_cast<int>(creeperFuse * 18.0f) % 2 == 1) {
                creeperCol = {1.95f, 1.95f, 1.95f, col.w}; // Rapid white flashing
            }
        }

        float legH = 0.35f * swell;
        float legSz = 0.20f * swell;
        float torsoH = 0.72f * swell;
        float torsoW = 0.40f * swell;
        float torsoD = 0.25f * swell;
        float headSz = 0.48f * swell;

        // 4 Stubby Feet
        float offX = 0.11f * swell;
        float offZ = 0.13f * swell;
        appendOrientedBox(verts, position, mobYaw, {-offX, legH * 0.5f,  offZ}, {legSz, legH, legSz}, creeperCol, 393.0f,  walkSwing, deathRoll);
        appendOrientedBox(verts, position, mobYaw, { offX, legH * 0.5f,  offZ}, {legSz, legH, legSz}, creeperCol, 393.0f, -walkSwing, deathRoll);
        appendOrientedBox(verts, position, mobYaw, {-offX, legH * 0.5f, -offZ}, {legSz, legH, legSz}, creeperCol, 393.0f, -walkSwing, deathRoll);
        appendOrientedBox(verts, position, mobYaw, { offX, legH * 0.5f, -offZ}, {legSz, legH, legSz}, creeperCol, 393.0f,  walkSwing, deathRoll);

        // Rectangular Torso
        Vec3 torsoPos = {0, legH + torsoH * 0.5f, 0};
        appendOrientedBox(verts, position, mobYaw, torsoPos, {torsoW, torsoH, torsoD}, creeperCol, 393.0f, attackLunge, deathRoll);

        // Head with THE ICONIC MINECRAFT CREEPER FACE
        Vec3 headPos = {0, legH + torsoH + headSz * 0.5f, 0};
        appendOrientedBox(verts, position, mobYaw, headPos, {headSz, headSz, headSz}, creeperCol, 396.0f, attackLunge * 0.5f, deathRoll);
        return;
    }

    // =========================================================================
    // 2. MINECRAFT ENDERMAN (2.9m Slender Void Stalker with Glowing Purple Eyes)
    // =========================================================================
    if (type == CreatureType::Enderman) {
        Vec4 enderCol = {0.08f, 0.08f, 0.10f, col.w};
        if (hurtFlashTimer > 0.0f) enderCol = col;

        float legH = 1.60f;
        float legW = 0.10f;
        float torsoH = 0.85f;
        float torsoW = 0.32f;
        float torsoD = 0.18f;
        float armH = 1.55f;
        float headSz = 0.40f;

        // Long Slender Legs
        appendOrientedBox(verts, position, mobYaw, {-0.10f, legH * 0.5f, 0}, {legW, legH, legW}, enderCol, 392.0f,  walkSwing * 0.65f, deathRoll);
        appendOrientedBox(verts, position, mobYaw, { 0.10f, legH * 0.5f, 0}, {legW, legH, legW}, enderCol, 392.0f, -walkSwing * 0.65f, deathRoll);

        // Slender Torso
        Vec3 torsoPos = {0, legH + torsoH * 0.5f, 0};
        appendOrientedBox(verts, position, mobYaw, torsoPos, {torsoW, torsoH, torsoD}, enderCol, 392.0f, attackLunge, deathRoll);

        // Long Arms reaching down past knees
        float leftArmPitch = -walkSwing * 0.5f;
        float rightArmPitch = walkSwing * 0.5f;
        if (state == AIState::Attack) {
            leftArmPitch = -45.0f;
            rightArmPitch = -45.0f;
        }
        appendOrientedBox(verts, position, mobYaw, {-torsoW * 0.56f, legH + torsoH * 0.85f - armH * 0.5f, 0}, {legW, armH, legW}, enderCol, 392.0f, leftArmPitch, deathRoll);
        appendOrientedBox(verts, position, mobYaw, { torsoW * 0.56f, legH + torsoH * 0.85f - armH * 0.5f, 0}, {legW, armH, legW}, enderCol, 392.0f, rightArmPitch, deathRoll);

        // Head
        Vec3 headPos = {0, legH + torsoH + headSz * 0.5f, 0};
        appendOrientedBox(verts, position, mobYaw, headPos, {headSz, headSz, headSz}, enderCol, 392.0f, attackLunge * 0.5f, deathRoll);

        // Glowing Purple Eye Slits
        Vec4 purpleEye = {2.2f, 0.4f, 2.6f, col.w};
        appendOrientedBox(verts, position, mobYaw, headPos + Vec3(-0.09f, 0.02f, headSz * 0.50f + 0.01f), {0.08f, 0.03f, 0.02f}, purpleEye, 396.0f, attackLunge * 0.5f, deathRoll);
        appendOrientedBox(verts, position, mobYaw, headPos + Vec3( 0.09f, 0.02f, headSz * 0.50f + 0.01f), {0.08f, 0.03f, 0.02f}, purpleEye, 396.0f, attackLunge * 0.5f, deathRoll);
        return;
    }

    // =========================================================================
    // 3. MINECRAFT SPIDER (8-Legged Chitin Arachnid with Red Glowing Eyes)
    // =========================================================================
    if (type == CreatureType::Spider) {
        float bodyH = 0.35f;
        float bodyW = 0.50f;
        float bodyD = 0.40f;

        // Cephalothorax (Front head & thorax)
        Vec3 cephPos = {0, bodyH * 0.85f, 0.18f};
        appendOrientedBox(verts, position, mobYaw, cephPos, {bodyW, bodyH, bodyD}, col, 391.0f, attackLunge, deathRoll);

        // Large Bulbous Abdomen
        Vec3 abdoPos = {0, bodyH * 1.15f, -0.42f};
        appendOrientedBox(verts, position, mobYaw, abdoPos, {0.70f, 0.52f, 0.80f}, col * 0.85f, 391.0f, attackLunge * 0.5f, deathRoll);

        // Glowing Ruby Red Eyes (2 central large, 6 smaller surrounding)
        Vec4 eyeRed = {2.5f, 0.2f, 0.2f, col.w};
        appendOrientedBox(verts, position, mobYaw, cephPos + Vec3(-0.12f, 0.04f, bodyD * 0.50f + 0.01f), {0.08f, 0.08f, 0.02f}, eyeRed, 396.0f, attackLunge, deathRoll);
        appendOrientedBox(verts, position, mobYaw, cephPos + Vec3( 0.12f, 0.04f, bodyD * 0.50f + 0.01f), {0.08f, 0.08f, 0.02f}, eyeRed, 396.0f, attackLunge, deathRoll);
        appendOrientedBox(verts, position, mobYaw, cephPos + Vec3(-0.20f, 0.01f, bodyD * 0.46f + 0.01f), {0.04f, 0.04f, 0.02f}, eyeRed, 396.0f, attackLunge, deathRoll);
        appendOrientedBox(verts, position, mobYaw, cephPos + Vec3( 0.20f, 0.01f, bodyD * 0.46f + 0.01f), {0.04f, 0.04f, 0.02f}, eyeRed, 396.0f, attackLunge, deathRoll);

        // Fangs / Pedipalps
        appendOrientedBox(verts, position, mobYaw, cephPos + Vec3(-0.08f, -0.10f, bodyD * 0.48f), {0.06f, 0.10f, 0.06f}, {0.12f, 0.10f, 0.10f, col.w}, 391.0f, -15.0f, deathRoll);
        appendOrientedBox(verts, position, mobYaw, cephPos + Vec3( 0.08f, -0.10f, bodyD * 0.48f), {0.06f, 0.10f, 0.06f}, {0.12f, 0.10f, 0.10f, col.w}, 391.0f, -15.0f, deathRoll);

        // 8 Splayed Crawling Legs (4 pairs)
        float legThick = 0.06f;
        float legLen = 0.65f;
        float legAngles[4] = {35.0f, 65.0f, 115.0f, 145.0f};

        for (int i = 0; i < 4; ++i) {
            float phase = animTime * 12.0f + static_cast<float>(i) * 1.57f;
            float stepLift = std::sin(phase) * 18.0f;

            float angL = legAngles[i];
            float radL = angL * DEG2RAD;
            Vec3 legBaseL = cephPos + Vec3(-bodyW * 0.45f, 0, (1.5f - static_cast<float>(i)) * 0.16f);
            appendOrientedBox(verts, position, mobYaw, legBaseL + Vec3(-std::cos(radL) * legLen * 0.5f, 0.10f, std::sin(radL) * legLen * 0.5f),
                              {legThick, legThick, legLen}, col * 0.75f, 391.0f, stepLift, -angL + deathRoll);

            float angR = -legAngles[i];
            float radR = angR * DEG2RAD;
            Vec3 legBaseR = cephPos + Vec3(bodyW * 0.45f, 0, (1.5f - static_cast<float>(i)) * 0.16f);
            appendOrientedBox(verts, position, mobYaw, legBaseR + Vec3(-std::cos(radR) * legLen * 0.5f, 0.10f, std::sin(radR) * legLen * 0.5f),
                              {legThick, legThick, legLen}, col * 0.75f, 391.0f, -stepLift, -angR + deathRoll);
        }
        return;
    }

    // =========================================================================
    // 4. MINECRAFT CHICKEN (Blocky White Poultry with Flapping Wings)
    // =========================================================================
    if (type == CreatureType::Chicken) {
        float legH = 0.28f;
        float bodyH = 0.40f;
        float bodyW = 0.40f;
        float bodyD = 0.48f;

        // Yellow Legs & Feet
        Vec4 yellowLeg = {1.0f, 0.82f, 0.15f, col.w};
        appendOrientedBox(verts, position, mobYaw, {-0.10f, legH * 0.5f, 0}, {0.06f, legH, 0.06f}, yellowLeg, 398.0f,  walkSwing, deathRoll);
        appendOrientedBox(verts, position, mobYaw, { 0.10f, legH * 0.5f, 0}, {0.06f, legH, 0.06f}, yellowLeg, 398.0f, -walkSwing, deathRoll);
        appendOrientedBox(verts, position, mobYaw, {-0.10f, 0.02f, 0.04f}, {0.12f, 0.02f, 0.12f}, yellowLeg, 398.0f,  walkSwing, deathRoll);
        appendOrientedBox(verts, position, mobYaw, { 0.10f, 0.02f, 0.04f}, {0.12f, 0.02f, 0.12f}, yellowLeg, 398.0f, -walkSwing, deathRoll);

        // White Body
        Vec3 bodyPos = {0, legH + bodyH * 0.5f, 0};
        appendOrientedBox(verts, position, mobYaw, bodyPos, {bodyW, bodyH, bodyD}, col, 390.0f, attackLunge, deathRoll);

        // Flapping Wings
        float wingFlap = std::sin(animTime * 16.0f) * 28.0f;
        appendOrientedBox(verts, position, mobYaw, bodyPos + Vec3(-bodyW * 0.52f, 0, 0), {0.04f, 0.28f, 0.36f}, col * 0.95f, 390.0f, 0, wingFlap + deathRoll);
        appendOrientedBox(verts, position, mobYaw, bodyPos + Vec3( bodyW * 0.52f, 0, 0), {0.04f, 0.28f, 0.36f}, col * 0.95f, 390.0f, 0, -wingFlap + deathRoll);

        // Head
        Vec3 headPos = {0, legH + bodyH * 0.85f, 0.22f};
        appendOrientedBox(verts, position, mobYaw, headPos, {0.24f, 0.30f, 0.25f}, col, 390.0f, attackLunge, deathRoll);

        // Yellow Beak
        appendOrientedBox(verts, position, mobYaw, headPos + Vec3(0, -0.04f, 0.18f), {0.12f, 0.08f, 0.12f}, yellowLeg, 398.0f, 0, deathRoll);

        // Red Wattle under beak
        Vec4 redWattle = {0.95f, 0.15f, 0.18f, col.w};
        appendOrientedBox(verts, position, mobYaw, headPos + Vec3(0, -0.14f, 0.14f), {0.08f, 0.10f, 0.08f}, redWattle, 398.0f, 0, deathRoll);
        return;
    }

    // =========================================================================
    // 5. MINECRAFT ZOMBIE (Classic Steve Silhouette, Cyan Shirt, Outstretched Arms)
    // =========================================================================
    if (type == CreatureType::Zombie) {
        float legH = 0.75f;
        float legW = 0.24f;
        float torsoH = 0.72f;
        float torsoW = 0.50f;
        float torsoD = 0.25f;
        float armW = 0.22f;
        float headSz = 0.50f;

        // Blue Pants Legs
        Vec4 pantsCol = {0.16f, 0.16f, 0.52f, col.w};
        appendOrientedBox(verts, position, mobYaw, {-0.13f, legH * 0.5f, 0}, {legW, legH, legW}, pantsCol, 398.0f,  walkSwing, deathRoll);
        appendOrientedBox(verts, position, mobYaw, { 0.13f, legH * 0.5f, 0}, {legW, legH, legW}, pantsCol, 398.0f, -walkSwing, deathRoll);

        // Cyan Tunic Torso
        Vec4 shirtCol = {0.0f, 0.65f, 0.65f, col.w};
        Vec3 torsoPos = {0, legH + torsoH * 0.5f, 0};
        appendOrientedBox(verts, position, mobYaw, torsoPos, {torsoW, torsoH, torsoD}, shirtCol, 398.0f, attackLunge, deathRoll);

        // Outstretched Arms (Classic Zombie -90 deg Pitch)
        float armPitch = -90.0f;
        Vec3 leftArmPos = {-torsoW * 0.58f, legH + torsoH * 0.75f, 0};
        Vec3 rightArmPos = { torsoW * 0.58f, legH + torsoH * 0.75f, 0};
        appendOrientedBox(verts, position, mobYaw, leftArmPos, {armW, torsoH, armW}, col, 398.0f, armPitch, deathRoll);
        appendOrientedBox(verts, position, mobYaw, rightArmPos, {armW, torsoH, armW}, col, 398.0f, armPitch, deathRoll);

        // Head with Zombie Face (Layer 395)
        Vec3 headPos = {0, legH + torsoH + headSz * 0.5f, 0};
        appendOrientedBox(verts, position, mobYaw, headPos, {headSz, headSz, headSz}, col, 395.0f, attackLunge * 0.5f, deathRoll);
        return;
    }

    // =========================================================================
    // 6. MINECRAFT SKELETON (Ivory Bones, Skull, Wooden Bow)
    // =========================================================================
    if (type == CreatureType::Skeleton) {
        float legH = 0.75f;
        float legW = 0.12f;
        float torsoH = 0.70f;
        float torsoW = 0.42f;
        float torsoD = 0.20f;
        float armW = 0.12f;
        float headSz = 0.50f;

        Vec4 boneCol = {0.88f, 0.88f, 0.85f, col.w};

        // Thin Bone Legs
        appendOrientedBox(verts, position, mobYaw, {-0.12f, legH * 0.5f, 0}, {legW, legH, legW}, boneCol, 394.0f,  walkSwing, deathRoll);
        appendOrientedBox(verts, position, mobYaw, { 0.12f, legH * 0.5f, 0}, {legW, legH, legW}, boneCol, 394.0f, -walkSwing, deathRoll);

        // Bone Ribcage Torso
        Vec3 torsoPos = {0, legH + torsoH * 0.5f, 0};
        appendOrientedBox(verts, position, mobYaw, torsoPos, {torsoW, torsoH, torsoD}, boneCol, 394.0f, attackLunge, deathRoll);

        // Arms: Left Aiming Forward holding Bow, Right drawn back
        Vec3 leftArmPos = {-torsoW * 0.56f, legH + torsoH * 0.75f, 0};
        Vec3 rightArmPos = { torsoW * 0.56f, legH + torsoH * 0.75f, 0};
        appendOrientedBox(verts, position, mobYaw, leftArmPos, {armW, torsoH, armW}, boneCol, 394.0f, -85.0f, deathRoll);
        appendOrientedBox(verts, position, mobYaw, rightArmPos, {armW, torsoH, armW}, boneCol, 394.0f, -40.0f, deathRoll);

        // Blocky Wooden Bow in Left Hand
        Vec3 bowPos = leftArmPos + Vec3(-0.04f, 0, torsoH * 0.65f);
        appendOrientedBox(verts, position, mobYaw, bowPos, {0.06f, 0.75f, 0.08f}, {0.55f, 0.35f, 0.18f, col.w}, 397.0f, -85.0f, deathRoll);

        // Skull (Layer 394)
        Vec3 headPos = {0, legH + torsoH + headSz * 0.5f, 0};
        appendOrientedBox(verts, position, mobYaw, headPos, {headSz, headSz, headSz}, boneCol, 394.0f, attackLunge * 0.5f, deathRoll);
        return;
    }

    // =========================================================================
    // 7. MINECRAFT QUADRUPEDS (Pig, Cow, Sheep, Horse)
    // =========================================================================
    float legH = def.size.y * 0.45f;
    float legW = def.size.x * 0.22f;
    float bodyH = def.size.y * 0.48f;
    float bodyW = def.size.x * 0.72f;
    float bodyD = def.size.z * 1.05f;

    float offX = bodyW * 0.35f;
    float offZ = bodyD * 0.34f;

    // 4 Blocky Legs
    appendOrientedBox(verts, position, mobYaw, {-offX, legH * 0.5f,  offZ}, {legW, legH, legW}, col * 0.85f, tIdx,  walkSwing, deathRoll);
    appendOrientedBox(verts, position, mobYaw, { offX, legH * 0.5f,  offZ}, {legW, legH, legW}, col * 0.85f, tIdx, -walkSwing, deathRoll);
    appendOrientedBox(verts, position, mobYaw, {-offX, legH * 0.5f, -offZ}, {legW, legH, legW}, col * 0.85f, tIdx, -walkSwing, deathRoll);
    appendOrientedBox(verts, position, mobYaw, { offX, legH * 0.5f, -offZ}, {legW, legH, legW}, col * 0.85f, tIdx,  walkSwing, deathRoll);

    // Torso
    Vec3 bodyPos = {0, legH + bodyH * 0.5f, 0};
    appendOrientedBox(verts, position, mobYaw, bodyPos, {bodyW, bodyH, bodyD}, col, tIdx, attackLunge, deathRoll);

    // Head
    float headSz = def.size.x * 0.50f;
    Vec3 headPos = {0, legH + bodyH * 0.82f, offZ + headSz * 0.42f};

    // =========================
    // MINECRAFT PIG
    // =========================
    if (type == CreatureType::Pig) {
        appendOrientedBox(verts, position, mobYaw, headPos, {headSz, headSz * 0.88f, headSz}, col, tIdx, attackLunge, deathRoll);
        // Protruding Snout Box
        Vec4 darkerPink = {0.90f, 0.55f, 0.60f, col.w};
        appendOrientedBox(verts, position, mobYaw, headPos + Vec3(0, -headSz * 0.16f, headSz * 0.44f), {headSz * 0.50f, headSz * 0.32f, 0.10f}, darkerPink, 398.0f, attackLunge, deathRoll);
    }
    // =========================
    // MINECRAFT COW
    // =========================
    else if (type == CreatureType::Cow) {
        // Mottled White Patches
        Vec4 whitePatch = {0.95f, 0.95f, 0.95f, col.w};
        appendOrientedBox(verts, position, mobYaw, bodyPos + Vec3(-bodyW * 0.22f, bodyH * 0.15f, 0.10f), {bodyW * 0.52f, bodyH * 0.65f, bodyD * 0.40f}, whitePatch, tIdx, attackLunge, deathRoll);
        appendOrientedBox(verts, position, mobYaw, bodyPos + Vec3( bodyW * 0.26f, -bodyH * 0.10f, -0.20f), {bodyW * 0.42f, bodyH * 0.55f, bodyD * 0.38f}, whitePatch, tIdx, attackLunge, deathRoll);

        // Head
        appendOrientedBox(verts, position, mobYaw, headPos, {headSz, headSz * 0.88f, headSz}, col, tIdx, attackLunge, deathRoll);
        // Pink Snout
        Vec4 pinkMuzzle = {0.95f, 0.72f, 0.75f, col.w};
        appendOrientedBox(verts, position, mobYaw, headPos + Vec3(0, -headSz * 0.20f, headSz * 0.45f), {headSz * 0.68f, headSz * 0.42f, 0.12f}, pinkMuzzle, 398.0f, attackLunge, deathRoll);

        // Blocky Horns
        Vec4 hornCol = {0.80f, 0.80f, 0.78f, col.w};
        appendOrientedBox(verts, position, mobYaw, headPos + Vec3(-headSz * 0.40f, headSz * 0.45f, 0), {0.08f, 0.16f, 0.08f}, hornCol, 394.0f, 0, deathRoll);
        appendOrientedBox(verts, position, mobYaw, headPos + Vec3( headSz * 0.40f, headSz * 0.45f, 0), {0.08f, 0.16f, 0.08f}, hornCol, 394.0f, 0, deathRoll);

        // Pink Udder beneath belly
        appendOrientedBox(verts, position, mobYaw, bodyPos + Vec3(0, -bodyH * 0.42f, -bodyD * 0.20f), {bodyW * 0.35f, 0.12f, bodyD * 0.25f}, pinkMuzzle, 398.0f, 0, deathRoll);
    }
    // =========================
    // MINECRAFT SHEEP
    // =========================
    else if (type == CreatureType::Sheep) {
        // Big Fluffy Outer Wool Coat
        appendOrientedBox(verts, position, mobYaw, bodyPos, {bodyW * 1.25f, bodyH * 1.22f, bodyD * 1.15f}, {1.05f, 1.05f, 1.05f, col.w}, 390.0f, attackLunge, deathRoll);
        // Exposed Head & Ears
        appendOrientedBox(verts, position, mobYaw, headPos, {headSz * 0.82f, headSz * 0.82f, headSz * 0.82f}, {0.85f, 0.80f, 0.78f, col.w}, 390.0f, attackLunge, deathRoll);
    }
    // =========================
    // MINECRAFT HORSE
    // =========================
    else if (type == CreatureType::Horse) {
        // Arched Neck
        Vec3 neckPos = bodyPos + Vec3(0, bodyH * 0.55f, offZ * 0.70f);
        appendOrientedBox(verts, position, mobYaw, neckPos, {bodyW * 0.38f, bodyH * 0.85f, bodyD * 0.45f}, col, tIdx, -32.0f + attackLunge, deathRoll);

        // Head atop neck
        Vec3 horseHeadPos = neckPos + Vec3(0, bodyH * 0.58f, 0.28f);
        appendOrientedBox(verts, position, mobYaw, horseHeadPos, {headSz * 0.75f, headSz * 0.75f, headSz * 1.15f}, col, tIdx, attackLunge, deathRoll);

        // Dark Mane
        Vec4 maneCol = {0.18f, 0.14f, 0.12f, col.w};
        appendOrientedBox(verts, position, mobYaw, neckPos + Vec3(0, bodyH * 0.25f, -bodyD * 0.18f), {0.10f, bodyH * 0.82f, 0.14f}, maneCol, 390.0f, -32.0f + attackLunge, deathRoll);

        // Ears
        appendOrientedBox(verts, position, mobYaw, horseHeadPos + Vec3(-headSz * 0.22f, headSz * 0.42f, -0.15f), {0.06f, 0.16f, 0.06f}, col, tIdx, 0, deathRoll);
        appendOrientedBox(verts, position, mobYaw, horseHeadPos + Vec3( headSz * 0.22f, headSz * 0.42f, -0.15f), {0.06f, 0.16f, 0.06f}, col, tIdx, 0, deathRoll);

        // Tail
        float tailSway = std::sin(animTime * 6.0f) * 18.0f;
        appendOrientedBox(verts, position, mobYaw, bodyPos + Vec3(0, bodyH * 0.20f, -bodyD * 0.52f), {0.12f, bodyH * 0.95f, 0.14f}, maneCol, 390.0f, 25.0f, tailSway + deathRoll);
    }
}

} // namespace Aetheria
