#pragma once

#include "Entity.hpp"
#include "../core/Camera.hpp"
#include "../inventory/Inventory.hpp"
#include "../inventory/ItemRegistry.hpp"
#include <cstdint>

namespace Aetheria {

class Window;
class AudioEngine;

class Player : public Entity {
public:
    Player(const Vec3& startPos);

    void handleInput(const Window& window, Camera& camera, AudioEngine* audio, float dt, bool isBuildingMode = false);
    void update(World* world, AudioEngine* audio, float dt);

    void takeDamage(float amount, AudioEngine* audio);
    void heal(float amount) { health = std::min(maxHealth, health + amount); }
    void gainXP(uint32_t amount, AudioEngine* audio);

    // Movement & Combat Mechanics
    bool getIsSprinting() const { return isSprinting; }
    bool getIsDashing() const { return dashTimer > 0.0f; }
    float getDashCooldown() const { return dashCooldown; }
    bool tryDash(const Vec3& wishDir, const Vec3& lookForward, AudioEngine* audio);

    // Fluid & Swimming Mechanics
    bool getIsInWater() const { return inWater; }
    bool getIsUnderwater() const { return isUnderwater; }
    bool getIsInLava() const { return inLava; }
    bool getIsUnderLava() const { return isUnderLava; }

    // RPG Stats
    uint32_t getLevel() const { return level; }
    uint32_t getXP() const { return xp; }
    uint32_t getXPToNext() const { return xpToNextLevel; }
    float getHealth() const { return health; }
    float getMaxHealth() const { return maxHealth; }
    float getMana() const { return mana; }
    float getMaxMana() const { return maxMana; }
    float getStamina() const { return stamina; }
    float getMaxStamina() const { return maxStamina; }
    float getAttackPower() const;
    float getDefense() const { return inventory.getTotalDefense(); }

    const std::string& getName() const { return name; }
    void setName(const std::string& n) { name = n; }
    const std::string& getCharacterClass() const { return characterClass; }
    void setCharacterClass(const std::string& cls) { characterClass = cls; }

    int getSelectedHotbarIndex() const { return selectedHotbarIndex; }
    void setSelectedHotbarIndex(int idx);
    void cycleHotbar(int delta);

    Inventory& getInventory() { return inventory; }
    const Inventory& getInventory() const { return inventory; }

    uint16_t getHeldBlockId() const;
    const ItemStack& getHeldItem() const;
    void setHotbarSlot(int index, uint16_t blockId);

    bool tryUseHeldItem(AudioEngine* audio);

    bool isAttacking() const { return attackSwingTimer > 0.0f; }
    float getAttackSwingProgress() const { return attackSwingTimer / 0.25f; }
    void triggerAttack();

    // Minecraft 1.9+ Attack Cooldown & Recharge Meter
    float getAttackRechargeProgress() const;
    float getAttackCooldownDuration() const;
    bool isAttackFullyCharged() const { return getAttackRechargeProgress() >= 0.95f; }
    void resetAttackCooldown() { attackCooldownTimer = 0.0f; }

private:
    uint32_t level = 1;
    uint32_t xp = 0;
    uint32_t xpToNextLevel = 100;

    std::string name = "Alp";
    std::string characterClass = "Savasci";

    float health = 100.0f;
    float maxHealth = 100.0f;
    float mana = 100.0f;
    float maxMana = 100.0f;
    float stamina = 100.0f;
    float maxStamina = 100.0f;
    float baseAttackPower = 12.0f;

    int selectedHotbarIndex = 0;
    Inventory inventory;

    float footstepTimer = 0.0f;
    float attackSwingTimer = 0.0f;
    float attackCooldownTimer = 99.0f; // Seconds since last attack or item swap (starts ready)

    // Movement state
    bool isSprinting = false;
    float timeSinceLastWPress = 99.0f;
    float dashCooldown = 0.0f;
    float dashTimer = 0.0f;
    Vec3 dashDir{0.0f, 0.0f, 0.0f};

    // Fluid mechanics state
    bool inWater = false;
    bool isUnderwater = false;
    bool inLava = false;
    bool isUnderLava = false;
    float lavaBurnTimer = 0.0f;
};

} // namespace Aetheria
