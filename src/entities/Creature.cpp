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

static void appendRiggedBox(std::vector<VoxelVertex>& verts,
                            const Vec3& mobOrigin,
                            float mobYawDeg,
                            const Vec3& pivot,
                            const Vec3& offsetFromPivot,
                            const Vec3& size,
                            const Vec4& col,
                            float tIdx,
                            float jointPitch = 0.0f,
                            float jointYaw = 0.0f,
                            float jointRoll = 0.0f,
                            float parentPitch = 0.0f,
                            float parentRoll = 0.0f) {
    Vec3 h = size * 0.5f;

    // 8 local corners centered around offsetFromPivot
    Vec3 corners[8] = {
        offsetFromPivot + Vec3(-h.x, -h.y, -h.z),
        offsetFromPivot + Vec3(+h.x, -h.y, -h.z),
        offsetFromPivot + Vec3(+h.x, +h.y, -h.z),
        offsetFromPivot + Vec3(-h.x, +h.y, -h.z),
        offsetFromPivot + Vec3(-h.x, -h.y, +h.z),
        offsetFromPivot + Vec3(+h.x, -h.y, +h.z),
        offsetFromPivot + Vec3(+h.x, +h.y, +h.z),
        offsetFromPivot + Vec3(-h.x, +h.y, +h.z)
    };

    float rP = jointPitch * DEG2RAD;
    float cp = std::cos(rP), sp = std::sin(rP);
    float rR = jointRoll * DEG2RAD;
    float cr = std::cos(rR), sr = std::sin(rR);
    float rY = jointYaw * DEG2RAD;
    float cy = std::cos(rY), sy = std::sin(rY);

    float prP = parentPitch * DEG2RAD;
    float pcp = std::cos(prP), psp = std::sin(prP);
    float prR = parentRoll * DEG2RAD;
    float pcr = std::cos(prR), psr = std::sin(prR);

    float mY = mobYawDeg * DEG2RAD;
    float myC = std::cos(mY), myS = std::sin(mY);

    auto transformPt = [&](const Vec3& pt) -> Vec3 {
        // 1. Joint rotation around pivot
        float y1 = pt.y * cp - pt.z * sp;
        float z1 = pt.y * sp + pt.z * cp;
        float x1 = pt.x;

        float x2 = x1 * cr - y1 * sr;
        float y2 = x1 * sr + y1 * cr;
        float z2 = z1;

        float x3 = x2 * cy + z2 * sy;
        float y3 = y2;
        float z3 = -x2 * sy + z2 * cy;

        Vec3 inMob = pivot + Vec3(x3, y3, z3);

        // 2. Parent body rotation (death roll, lunge)
        float py1 = inMob.y * pcp - inMob.z * psp;
        float pz1 = inMob.y * psp + inMob.z * pcp;
        float px1 = inMob.x;

        float px2 = px1 * pcr - py1 * psr;
        float py2 = px1 * psr + py1 * pcr;
        float pz2 = pz1;

        // 3. World placement
        return Vec3(
            mobOrigin.x + (px2 * myC + pz2 * myS),
            mobOrigin.y + py2,
            mobOrigin.z + (-px2 * myS + pz2 * myC)
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

        float x3 = x2 * cy + z2 * sy;
        float y3 = y2;
        float z3 = -x2 * sy + z2 * cy;

        float py1 = y3 * pcp - z3 * psp;
        float pz1 = y3 * psp + z3 * pcp;
        float px1 = x3;

        float px2 = px1 * pcr - py1 * psr;
        float py2 = px1 * psr + py1 * pcr;
        float pz2 = pz1;

        return Vec3(
            px2 * myC + pz2 * myS,
            py2,
            -px2 * myS + pz2 * myC
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
    addFaceQuad(5, 1, 2, 6, {1, 0, 0}, 0.70f);   // Right (+X)
    addFaceQuad(0, 4, 7, 3, {-1, 0, 0}, 0.75f);  // Left (-X)
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
    appendRiggedBox(verts, origin, yawDeg, localOffset, {0, 0, 0}, size, col, tIdx, pitchDeg, 0.0f, rollDeg, 0.0f, 0.0f);
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

    // Dynamic movement & walk animation scaling
    float hSpeed = std::sqrt(velocity.x * velocity.x + velocity.z * velocity.z);
    float moveFactor = std::clamp(hSpeed / 1.1f, 0.0f, 1.0f);
    float walkSwing = std::sin(animTime * 8.0f) * 34.0f * (moveFactor > 0.05f ? moveFactor : 0.0f);
    float attackLunge = (state == AIState::Attack) ? 14.0f : 0.0f;
    float idleBob = std::sin(animTime * 2.2f) * 0.015f;

    // =========================================================================
    // 1. MINECRAFT CREEPER (Iconic 4-Legged Green Explosive Terror)
    // =========================================================================
    if (type == CreatureType::Creeper) {
        float swell = 1.0f;
        Vec4 creeperCol = col;
        if (creeperFuse > 0.0f) {
            swell += (creeperFuse / 1.35f) * 0.35f; // Swells up by 35% while hissing!
            if (static_cast<int>(creeperFuse * 18.0f) % 2 == 1) {
                creeperCol = {1.95f, 1.95f, 1.95f, col.w}; // Rapid white flashing
            }
        }

        float legH = 0.36f * swell;
        float legSz = 0.20f * swell;
        float torsoH = 0.72f * swell;
        float torsoW = 0.40f * swell;
        float torsoD = 0.26f * swell;
        float headSz = 0.48f * swell;

        float offX = 0.12f * swell;
        float offZ = 0.14f * swell;

        // 4 Stubby Feet strictly pivoting at hip height legH
        appendRiggedBox(verts, position, mobYaw, {-offX, legH,  offZ}, {0, -legH * 0.5f, 0}, {legSz, legH, legSz}, creeperCol, 393.0f,  walkSwing, 0, 0, 0, deathRoll);
        appendRiggedBox(verts, position, mobYaw, { offX, legH,  offZ}, {0, -legH * 0.5f, 0}, {legSz, legH, legSz}, creeperCol, 393.0f, -walkSwing, 0, 0, 0, deathRoll);
        appendRiggedBox(verts, position, mobYaw, {-offX, legH, -offZ}, {0, -legH * 0.5f, 0}, {legSz, legH, legSz}, creeperCol, 393.0f, -walkSwing, 0, 0, 0, deathRoll);
        appendRiggedBox(verts, position, mobYaw, { offX, legH, -offZ}, {0, -legH * 0.5f, 0}, {legSz, legH, legSz}, creeperCol, 393.0f,  walkSwing, 0, 0, 0, deathRoll);

        // Rectangular Torso
        appendRiggedBox(verts, position, mobYaw, {0, legH + idleBob, 0}, {0, torsoH * 0.5f, 0}, {torsoW, torsoH, torsoD}, creeperCol, 393.0f, 0, 0, 0, attackLunge, deathRoll);

        // Head with THE ICONIC MINECRAFT CREEPER FACE
        float headTilt = std::sin(animTime * 1.5f) * 4.0f;
        appendRiggedBox(verts, position, mobYaw, {0, legH + torsoH + idleBob, 0}, {0, headSz * 0.5f, 0}, {headSz, headSz, headSz}, creeperCol, 396.0f, headTilt, 0, 0, attackLunge, deathRoll);
        return;
    }

    // =========================================================================
    // 2. MINECRAFT ENDERMAN (2.9m Slender Void Stalker with Glowing Purple Eyes)
    // =========================================================================
    if (type == CreatureType::Enderman) {
        Vec4 enderCol = {0.08f, 0.08f, 0.10f, col.w};
        if (hurtFlashTimer > 0.0f) enderCol = col;

        float legH = 1.60f;
        float legW = 0.12f;
        float torsoH = 0.85f;
        float torsoW = 0.34f;
        float torsoD = 0.20f;
        float armH = 1.55f;
        float headSz = 0.42f;

        // Long Slender Legs pivoting at hips Y=legH
        appendRiggedBox(verts, position, mobYaw, {-0.10f, legH, 0}, {0, -legH * 0.5f, 0}, {legW, legH, legW}, enderCol, 392.0f,  walkSwing * 0.70f, 0, 0, 0, deathRoll);
        appendRiggedBox(verts, position, mobYaw, { 0.10f, legH, 0}, {0, -legH * 0.5f, 0}, {legW, legH, legW}, enderCol, 392.0f, -walkSwing * 0.70f, 0, 0, 0, deathRoll);

        // Slender Torso
        appendRiggedBox(verts, position, mobYaw, {0, legH, 0}, {0, torsoH * 0.5f, 0}, {torsoW, torsoH, torsoD}, enderCol, 392.0f, 0, 0, 0, attackLunge, deathRoll);

        // Long Arms reaching past knees
        float leftArmPitch = -walkSwing * 0.55f;
        float rightArmPitch = walkSwing * 0.55f;
        if (state == AIState::Attack || state == AIState::Chase) {
            leftArmPitch = -50.0f + std::sin(animTime * 18.0f) * 28.0f;
            rightArmPitch = -50.0f - std::sin(animTime * 18.0f) * 28.0f;
        }
        float shoulderY = legH + torsoH * 0.90f;
        appendRiggedBox(verts, position, mobYaw, {-torsoW * 0.58f, shoulderY, 0}, {0, -armH * 0.5f, 0}, {legW, armH, legW}, enderCol, 392.0f, leftArmPitch, 0, 0, attackLunge, deathRoll);
        appendRiggedBox(verts, position, mobYaw, { torsoW * 0.58f, shoulderY, 0}, {0, -armH * 0.5f, 0}, {legW, armH, legW}, enderCol, 392.0f, rightArmPitch, 0, 0, attackLunge, deathRoll);

        // Head with neck joint
        float headShake = (state == AIState::Attack) ? (std::sin(animTime * 22.0f) * 8.0f) : 0.0f;
        Vec3 neckPivot = {0, legH + torsoH, 0};
        appendRiggedBox(verts, position, mobYaw, neckPivot, {0, headSz * 0.5f, 0}, {headSz, headSz, headSz}, enderCol, 392.0f, headShake, 0, 0, attackLunge, deathRoll);

        // Glowing Purple Eye Slits
        Vec4 purpleEye = {2.4f, 0.4f, 2.8f, col.w};
        appendRiggedBox(verts, position, mobYaw, neckPivot, {-0.09f, headSz * 0.52f, headSz * 0.50f + 0.01f}, {0.08f, 0.03f, 0.02f}, purpleEye, 396.0f, headShake, 0, 0, attackLunge, deathRoll);
        appendRiggedBox(verts, position, mobYaw, neckPivot, { 0.09f, headSz * 0.52f, headSz * 0.50f + 0.01f}, {0.08f, 0.03f, 0.02f}, purpleEye, 396.0f, headShake, 0, 0, attackLunge, deathRoll);
        return;
    }

    // =========================================================================
    // 3. MINECRAFT SPIDER (8-Legged Chitin Arachnid with Red Glowing Eyes)
    // =========================================================================
    if (type == CreatureType::Spider) {
        float bodyH = 0.35f;
        float bodyW = 0.50f;
        float bodyD = 0.40f;

        // Cephalothorax
        Vec3 cephPivot = {0, bodyH * 0.65f, 0.18f};
        appendRiggedBox(verts, position, mobYaw, cephPivot, {0, 0, 0}, {bodyW, bodyH, bodyD}, col, 391.0f, 0, 0, 0, attackLunge, deathRoll);

        // Large Bulbous Abdomen
        appendRiggedBox(verts, position, mobYaw, {0, bodyH * 0.85f, -0.15f}, {0, 0, -0.40f}, {0.70f, 0.52f, 0.80f}, col * 0.85f, 391.0f, 0, 0, 0, attackLunge * 0.5f, deathRoll);

        // Glowing Ruby Red Eyes
        Vec4 eyeRed = {2.5f, 0.2f, 0.2f, col.w};
        appendRiggedBox(verts, position, mobYaw, cephPivot, {-0.12f, 0.04f, bodyD * 0.50f + 0.01f}, {0.08f, 0.08f, 0.02f}, eyeRed, 396.0f, 0, 0, 0, attackLunge, deathRoll);
        appendRiggedBox(verts, position, mobYaw, cephPivot, { 0.12f, 0.04f, bodyD * 0.50f + 0.01f}, {0.08f, 0.08f, 0.02f}, eyeRed, 396.0f, 0, 0, 0, attackLunge, deathRoll);

        // Pedipalp Fangs
        appendRiggedBox(verts, position, mobYaw, cephPivot, {-0.08f, -0.12f, bodyD * 0.48f}, {0.06f, 0.12f, 0.06f}, {0.12f, 0.10f, 0.10f, col.w}, 391.0f, -15.0f, 0, 0, attackLunge, deathRoll);
        appendRiggedBox(verts, position, mobYaw, cephPivot, { 0.08f, -0.12f, bodyD * 0.48f}, {0.06f, 0.12f, 0.06f}, {0.12f, 0.10f, 0.10f, col.w}, 391.0f, -15.0f, 0, 0, attackLunge, deathRoll);

        // 8 Articulated Sprawling Legs with spider crawl gait
        float legThick = 0.06f;
        float legLen = 0.65f;
        float legZOffsets[4] = {0.20f, 0.06f, -0.08f, -0.22f};
        float legAngles[4] = {32.0f, 65.0f, 115.0f, 148.0f};

        for (int i = 0; i < 4; ++i) {
            float phase = animTime * 10.0f + static_cast<float>(i) * 1.57f;
            float stepPitch = std::sin(phase) * 22.0f * (moveFactor > 0.05f ? moveFactor : 0.0f);

            // Left leg
            Vec3 hipL = {-bodyW * 0.50f, bodyH * 0.55f, legZOffsets[i]};
            appendRiggedBox(verts, position, mobYaw, hipL, {-legLen * 0.45f, -0.12f, 0}, {legLen, legThick, legThick}, col * 0.75f, 391.0f, stepPitch, -legAngles[i] + 90.0f, -28.0f, 0, deathRoll);

            // Right leg
            Vec3 hipR = { bodyW * 0.50f, bodyH * 0.55f, legZOffsets[i]};
            appendRiggedBox(verts, position, mobYaw, hipR, { legLen * 0.45f, -0.12f, 0}, {legLen, legThick, legThick}, col * 0.75f, 391.0f, -stepPitch, legAngles[i] - 90.0f, 28.0f, 0, deathRoll);
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

        Vec4 yellowLeg = {1.0f, 0.82f, 0.15f, col.w};

        // 2 Yellow Legs pivoting at hips
        appendRiggedBox(verts, position, mobYaw, {-0.10f, legH, 0}, {0, -legH * 0.5f, 0}, {0.06f, legH, 0.06f}, yellowLeg, 398.0f,  walkSwing, 0, 0, 0, deathRoll);
        appendRiggedBox(verts, position, mobYaw, { 0.10f, legH, 0}, {0, -legH * 0.5f, 0}, {0.06f, legH, 0.06f}, yellowLeg, 398.0f, -walkSwing, 0, 0, 0, deathRoll);

        // Yellow Feet Plates
        appendRiggedBox(verts, position, mobYaw, {-0.10f, legH, 0}, {0, -legH + 0.015f, 0.04f}, {0.12f, 0.03f, 0.12f}, yellowLeg, 398.0f,  walkSwing, 0, 0, 0, deathRoll);
        appendRiggedBox(verts, position, mobYaw, { 0.10f, legH, 0}, {0, -legH + 0.015f, 0.04f}, {0.12f, 0.03f, 0.12f}, yellowLeg, 398.0f, -walkSwing, 0, 0, 0, deathRoll);

        // Torso
        appendRiggedBox(verts, position, mobYaw, {0, legH, 0}, {0, bodyH * 0.5f, 0}, {bodyW, bodyH, bodyD}, col, 390.0f, 0, 0, 0, attackLunge, deathRoll);

        // Flapping Wings pivoting at shoulders
        float wingFlap = (hSpeed > 0.1f || !onGround) ? (std::sin(animTime * 24.0f) * 42.0f) : (std::sin(animTime * 2.0f) * 4.0f);
        appendRiggedBox(verts, position, mobYaw, {-bodyW * 0.50f, legH + bodyH * 0.75f, 0}, {-0.02f, -0.14f, 0}, {0.04f, 0.28f, 0.36f}, col * 0.95f, 390.0f, 0, 0,  wingFlap, attackLunge, deathRoll);
        appendRiggedBox(verts, position, mobYaw, { bodyW * 0.50f, legH + bodyH * 0.75f, 0}, { 0.02f, -0.14f, 0}, {0.04f, 0.28f, 0.36f}, col * 0.95f, 390.0f, 0, 0, -wingFlap, attackLunge, deathRoll);

        // Head with peck bob
        float peckPitch = std::sin(animTime * 10.0f) * 12.0f * (moveFactor > 0.05f ? moveFactor : 0.0f);
        Vec3 chickenNeck = {0, legH + bodyH * 0.85f, 0.22f};
        appendRiggedBox(verts, position, mobYaw, chickenNeck, {0, 0.14f, 0}, {0.24f, 0.28f, 0.25f}, col, 390.0f, peckPitch, 0, 0, attackLunge, deathRoll);

        // Yellow Beak
        appendRiggedBox(verts, position, mobYaw, chickenNeck, {0, 0.10f, 0.18f}, {0.12f, 0.08f, 0.12f}, yellowLeg, 398.0f, peckPitch, 0, 0, attackLunge, deathRoll);

        // Red Wattle
        Vec4 redWattle = {0.95f, 0.15f, 0.18f, col.w};
        appendRiggedBox(verts, position, mobYaw, chickenNeck, {0, 0.02f, 0.14f}, {0.08f, 0.10f, 0.08f}, redWattle, 398.0f, peckPitch, 0, 0, attackLunge, deathRoll);
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

        // Indigo Pants Legs swinging from hips
        Vec4 pantsCol = {0.16f, 0.16f, 0.52f, col.w};
        appendRiggedBox(verts, position, mobYaw, {-0.13f, legH, 0}, {0, -legH * 0.5f, 0}, {legW, legH, legW}, pantsCol, 398.0f,  walkSwing, 0, 0, 0, deathRoll);
        appendRiggedBox(verts, position, mobYaw, { 0.13f, legH, 0}, {0, -legH * 0.5f, 0}, {legW, legH, legW}, pantsCol, 398.0f, -walkSwing, 0, 0, 0, deathRoll);

        // Cyan Tunic Torso
        Vec4 shirtCol = {0.0f, 0.65f, 0.65f, col.w};
        appendRiggedBox(verts, position, mobYaw, {0, legH, 0}, {0, torsoH * 0.5f, 0}, {torsoW, torsoH, torsoD}, shirtCol, 398.0f, 0, 0, 0, attackLunge, deathRoll);

        // Outstretched Zombie Arms (-90 deg pitch with subtle swaying)
        float armPitch = -90.0f + std::sin(animTime * 3.5f) * 5.0f;
        float shoulderY = legH + torsoH * 0.90f;
        appendRiggedBox(verts, position, mobYaw, {-torsoW * 0.58f, shoulderY, 0}, {0, -torsoH * 0.5f, 0}, {armW, torsoH, armW}, col, 398.0f, armPitch, 0, 0, attackLunge, deathRoll);
        appendRiggedBox(verts, position, mobYaw, { torsoW * 0.58f, shoulderY, 0}, {0, -torsoH * 0.5f, 0}, {armW, torsoH, armW}, col, 398.0f, armPitch, 0, 0, attackLunge, deathRoll);

        // Head with Zombie Face (Layer 395)
        appendRiggedBox(verts, position, mobYaw, {0, legH + torsoH, 0}, {0, headSz * 0.5f, 0}, {headSz, headSz, headSz}, col, 395.0f, 0, 0, 0, attackLunge * 0.5f, deathRoll);
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
        appendRiggedBox(verts, position, mobYaw, {-0.12f, legH, 0}, {0, -legH * 0.5f, 0}, {legW, legH, legW}, boneCol, 394.0f,  walkSwing, 0, 0, 0, deathRoll);
        appendRiggedBox(verts, position, mobYaw, { 0.12f, legH, 0}, {0, -legH * 0.5f, 0}, {legW, legH, legW}, boneCol, 394.0f, -walkSwing, 0, 0, 0, deathRoll);

        // Bone Ribcage Torso
        appendRiggedBox(verts, position, mobYaw, {0, legH, 0}, {0, torsoH * 0.5f, 0}, {torsoW, torsoH, torsoD}, boneCol, 394.0f, 0, 0, 0, attackLunge, deathRoll);

        // Arms: Left Aiming Forward holding Bow, Right drawn back
        float shoulderY = legH + torsoH * 0.88f;
        appendRiggedBox(verts, position, mobYaw, {-torsoW * 0.56f, shoulderY, 0}, {0, -torsoH * 0.5f, 0}, {armW, torsoH, armW}, boneCol, 394.0f, -85.0f, 12.0f, 0, attackLunge, deathRoll);
        appendRiggedBox(verts, position, mobYaw, { torsoW * 0.56f, shoulderY, 0}, {0, -torsoH * 0.5f, 0}, {armW, torsoH, armW}, boneCol, 394.0f, -48.0f, -15.0f, 0, attackLunge, deathRoll);

        // Blocky Wooden Bow in Left Hand
        appendRiggedBox(verts, position, mobYaw, {-torsoW * 0.56f, shoulderY, 0}, {-0.04f, -torsoH * 0.80f, 0.10f}, {0.06f, 0.75f, 0.08f}, {0.55f, 0.35f, 0.18f, col.w}, 397.0f, -85.0f, 12.0f, 0, attackLunge, deathRoll);

        // Skull (Layer 394)
        appendRiggedBox(verts, position, mobYaw, {0, legH + torsoH, 0}, {0, headSz * 0.5f, 0}, {headSz, headSz, headSz}, boneCol, 394.0f, 0, 0, 0, attackLunge * 0.5f, deathRoll);
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

    // 4 Articulated Legs pivoting at hip height legH
    appendRiggedBox(verts, position, mobYaw, {-offX, legH,  offZ}, {0, -legH * 0.5f, 0}, {legW, legH, legW}, col * 0.85f, tIdx,  walkSwing, 0, 0, 0, deathRoll);
    appendRiggedBox(verts, position, mobYaw, { offX, legH,  offZ}, {0, -legH * 0.5f, 0}, {legW, legH, legW}, col * 0.85f, tIdx, -walkSwing, 0, 0, 0, deathRoll);
    appendRiggedBox(verts, position, mobYaw, {-offX, legH, -offZ}, {0, -legH * 0.5f, 0}, {legW, legH, legW}, col * 0.85f, tIdx, -walkSwing, 0, 0, 0, deathRoll);
    appendRiggedBox(verts, position, mobYaw, { offX, legH, -offZ}, {0, -legH * 0.5f, 0}, {legW, legH, legW}, col * 0.85f, tIdx,  walkSwing, 0, 0, 0, deathRoll);

    // Torso
    appendRiggedBox(verts, position, mobYaw, {0, legH + idleBob, 0}, {0, bodyH * 0.5f, 0}, {bodyW, bodyH, bodyD}, col, tIdx, 0, 0, 0, attackLunge, deathRoll);

    // Head Neck Pivot
    float headSz = def.size.x * 0.50f;
    Vec3 neckPivot = {0, legH + bodyH * 0.78f + idleBob, offZ + 0.05f};
    float headNod = std::sin(animTime * 2.0f) * 4.0f;

    // =========================
    // MINECRAFT PIG
    // =========================
    if (type == CreatureType::Pig) {
        appendRiggedBox(verts, position, mobYaw, neckPivot, {0, headSz * 0.35f, headSz * 0.30f}, {headSz, headSz * 0.88f, headSz}, col, tIdx, headNod, 0, 0, attackLunge, deathRoll);
        // Protruding 3D Snout
        Vec4 darkerPink = {0.90f, 0.55f, 0.60f, col.w};
        appendRiggedBox(verts, position, mobYaw, neckPivot, {0, headSz * 0.18f, headSz * 0.78f}, {headSz * 0.50f, headSz * 0.32f, 0.10f}, darkerPink, 398.0f, headNod, 0, 0, attackLunge, deathRoll);

        // Curly Tail
        float tailWag = std::sin(animTime * 7.0f) * 20.0f;
        appendRiggedBox(verts, position, mobYaw, {0, legH + bodyH * 0.80f, -offZ}, {0, 0, -0.06f}, {0.06f, 0.06f, 0.12f}, darkerPink, 398.0f, 0, tailWag, 0, 0, deathRoll);
    }
    // =========================
    // MINECRAFT COW
    // =========================
    else if (type == CreatureType::Cow) {
        // Mottled White Patches on Torso
        Vec4 whitePatch = {0.95f, 0.95f, 0.95f, col.w};
        appendRiggedBox(verts, position, mobYaw, {0, legH + idleBob, 0}, {-bodyW * 0.22f, bodyH * 0.60f, 0.10f}, {bodyW * 0.52f, bodyH * 0.65f, bodyD * 0.40f}, whitePatch, tIdx, 0, 0, 0, attackLunge, deathRoll);
        appendRiggedBox(verts, position, mobYaw, {0, legH + idleBob, 0}, { bodyW * 0.26f, bodyH * 0.35f, -0.20f}, {bodyW * 0.42f, bodyH * 0.55f, bodyD * 0.38f}, whitePatch, tIdx, 0, 0, 0, attackLunge, deathRoll);

        // Head
        appendRiggedBox(verts, position, mobYaw, neckPivot, {0, headSz * 0.38f, headSz * 0.28f}, {headSz, headSz * 0.88f, headSz}, col, tIdx, headNod, 0, 0, attackLunge, deathRoll);
        // Pink Muzzle
        Vec4 pinkMuzzle = {0.95f, 0.72f, 0.75f, col.w};
        appendRiggedBox(verts, position, mobYaw, neckPivot, {0, headSz * 0.16f, headSz * 0.74f}, {headSz * 0.68f, headSz * 0.42f, 0.12f}, pinkMuzzle, 398.0f, headNod, 0, 0, attackLunge, deathRoll);

        // Blocky Horns
        Vec4 hornCol = {0.80f, 0.80f, 0.78f, col.w};
        appendRiggedBox(verts, position, mobYaw, neckPivot, {-headSz * 0.40f, headSz * 0.80f, headSz * 0.20f}, {0.08f, 0.16f, 0.08f}, hornCol, 394.0f, headNod, 0, 0, attackLunge, deathRoll);
        appendRiggedBox(verts, position, mobYaw, neckPivot, { headSz * 0.40f, headSz * 0.80f, headSz * 0.20f}, {0.08f, 0.16f, 0.08f}, hornCol, 394.0f, headNod, 0, 0, attackLunge, deathRoll);

        // Pink Udder beneath belly
        appendRiggedBox(verts, position, mobYaw, {0, legH + idleBob, 0}, {0, 0.08f, -bodyD * 0.22f}, {bodyW * 0.35f, 0.12f, bodyD * 0.25f}, pinkMuzzle, 398.0f, 0, 0, 0, attackLunge, deathRoll);
    }
    // =========================
    // MINECRAFT SHEEP
    // =========================
    else if (type == CreatureType::Sheep) {
        // Big Fluffy Outer Wool Coat
        appendRiggedBox(verts, position, mobYaw, {0, legH + idleBob, 0}, {0, bodyH * 0.5f, 0}, {bodyW * 1.22f, bodyH * 1.20f, bodyD * 1.12f}, {1.05f, 1.05f, 1.05f, col.w}, 390.0f, 0, 0, 0, attackLunge, deathRoll);
        // Exposed Head
        appendRiggedBox(verts, position, mobYaw, neckPivot, {0, headSz * 0.35f, headSz * 0.30f}, {headSz * 0.82f, headSz * 0.82f, headSz * 0.82f}, {0.85f, 0.80f, 0.78f, col.w}, 390.0f, headNod, 0, 0, attackLunge, deathRoll);
    }
    // =========================
    // MINECRAFT HORSE
    // =========================
    else if (type == CreatureType::Horse) {
        // Arched Neck
        Vec3 neckBase = {0, legH + bodyH * 0.65f + idleBob, offZ * 0.70f};
        appendRiggedBox(verts, position, mobYaw, neckBase, {0, bodyH * 0.40f, 0.12f}, {bodyW * 0.38f, bodyH * 0.85f, bodyD * 0.45f}, col, tIdx, -32.0f, 0, 0, attackLunge, deathRoll);

        // Head atop neck
        appendRiggedBox(verts, position, mobYaw, neckBase, {0, bodyH * 0.80f, 0.36f}, {headSz * 0.75f, headSz * 0.75f, headSz * 1.15f}, col, tIdx, 0, 0, 0, attackLunge, deathRoll);

        // Dark Mane
        Vec4 maneCol = {0.18f, 0.14f, 0.12f, col.w};
        appendRiggedBox(verts, position, mobYaw, neckBase, {0, bodyH * 0.45f, -0.06f}, {0.10f, bodyH * 0.82f, 0.14f}, maneCol, 390.0f, -32.0f, 0, 0, attackLunge, deathRoll);

        // Ears
        appendRiggedBox(verts, position, mobYaw, neckBase, {-headSz * 0.22f, bodyH * 1.05f, 0.22f}, {0.06f, 0.16f, 0.06f}, col, tIdx, 0, 0, 0, attackLunge, deathRoll);
        appendRiggedBox(verts, position, mobYaw, neckBase, { headSz * 0.22f, bodyH * 1.05f, 0.22f}, {0.06f, 0.16f, 0.06f}, col, tIdx, 0, 0, 0, attackLunge, deathRoll);

        // Tail swishing
        float tailSway = std::sin(animTime * 6.0f) * 22.0f;
        appendRiggedBox(verts, position, mobYaw, {0, legH + bodyH * 0.75f + idleBob, -offZ * 0.95f}, {0, -bodyH * 0.40f, -0.08f}, {0.12f, bodyH * 0.85f, 0.14f}, maneCol, 390.0f, 22.0f, tailSway, 0, attackLunge, deathRoll);
    }
}

} // namespace Aetheria
