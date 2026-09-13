#pragma once

#include "Entity.hpp"
#include "CreatureRegistry.hpp"
#include "../world/Chunk.hpp"
#include <vector>

namespace Aetheria {

class AudioEngine;
class BuildingManager;

enum class AIState {
    Idle,
    Wander,
    Alert,
    Chase,
    Attack,
    Flee,
    WorkAtBase
};

enum class CompanionStance {
    Follow,
    Stay,
    WorkAtBase
};

class Creature : public Entity {
public:
    Creature(CreatureType type, const Vec3& pos);
    virtual ~Creature() = default;

    void updateAI(World* world, const Vec3& playerPos, BuildingManager* buildingMgr, AudioEngine* audio, float dt);
    void takeDamage(float amount, const Vec3& knockbackDir, AudioEngine* audio);
    void heal(float amount) { currentHP = std::min(maxHP, currentHP + amount); }

    bool getIsDead() const { return isDead; }
    bool getIsTamed() const { return isTamed; }
    void setTamed(bool tamed);

    bool isInCapture() const { return inCapture; }
    void setInCapture(bool cap) { inCapture = cap; }

    CompanionStance getStance() const { return stance; }
    void setStance(CompanionStance s) { stance = s; }
    void cycleStance();
    std::string getStanceName() const;

    int getCompanionLevel() const { return companionLevel; }
    float getCompanionXP() const { return companionXP; }
    float getCompanionXPToNext() const { return companionXPToNext; }
    void gainCompanionXP(float amount, AudioEngine* audio);

    Creature* getCompanionTarget() const { return companionTarget; }
    void setCompanionTarget(Creature* target) { companionTarget = target; }
    void notifyMasterDamaged(Creature* attacker);
    void notifyMasterAttacked(Creature* target);

    float getHealth() const { return currentHP; }
    float getMaxHealth() const { return maxHP; }
    CreatureType getType() const { return type; }
    const CreatureDef& getDef() const { return CreatureRegistry::get(type); }

    bool isFlyingType() const;
    bool isAttacking() const { return state == AIState::Attack && attackCooldown > 0.6f; }
    bool isCompanionAttacking() const { return isTamed && state == AIState::Attack && attackCooldown > 0.6f; }
    float getCompanionAttackDamage() const { return getDef().attackDamage + static_cast<float>(companionLevel) * 2.5f; }
    bool isDespawnReady() const { return isDead && deathTimer >= 0.8f; }
    float getDeathTimer() const { return deathTimer; }
    float getAnimTime() const { return animTime; }
    float getCreeperFuse() const { return creeperFuse; }
    bool isCreeperExploding() const { return creeperFuse >= 1.35f; }

    // Renders hierarchical 3D voxel box model with full yaw orientation
    void appendModelVertices(std::vector<VoxelVertex>& verts, float totalTime) const;

private:
    CreatureType type;
    float currentHP = 50.0f;
    float maxHP = 50.0f;
    bool isDead = false;
    bool isTamed = false;
    bool inCapture = false;
    float deathTimer = 0.0f;

    CompanionStance stance = CompanionStance::Follow;
    int companionLevel = 1;
    float companionXP = 0.0f;
    float companionXPToNext = 100.0f;
    Creature* companionTarget = nullptr;
    float outOfCombatTimer = 0.0f;

    AIState state = AIState::Wander;
    float stateTimer = 0.0f;
    float wanderAngle = 0.0f;
    float attackCooldown = 0.0f;
    float hurtFlashTimer = 0.0f;
    float animTime = 0.0f;
    float creeperFuse = 0.0f;
};

} // namespace Aetheria
