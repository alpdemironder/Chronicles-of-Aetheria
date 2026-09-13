#include "Player.hpp"
#include "../core/Window.hpp"
#include "../core/Audio.hpp"
#include "../world/World.hpp"
#include "../world/BlockRegistry.hpp"
#include <cmath>
#include <algorithm>
#include <iostream>

namespace Aetheria {

Player::Player(const Vec3& startPos)
    : Entity(startPos, Vec3(0.6f, 1.8f, 0.6f)) {
}

void Player::setSelectedHotbarIndex(int idx) {
    if (idx >= 0 && idx < 9 && idx != selectedHotbarIndex) {
        selectedHotbarIndex = idx;
        attackCooldownTimer = 0.0f; // Reset attack charge on weapon swap
    }
}

void Player::cycleHotbar(int delta) {
    int oldIdx = selectedHotbarIndex;
    selectedHotbarIndex = (selectedHotbarIndex - delta + 9) % 9;
    if (selectedHotbarIndex != oldIdx) {
        attackCooldownTimer = 0.0f; // Reset attack charge on weapon swap
    }
}

void Player::setHotbarSlot(int index, uint16_t blockId) {
    if (index >= 0 && index < 9) {
        inventory.setSlot(index, {blockId, 64, 64});
    }
}

uint16_t Player::getHeldBlockId() const {
    return inventory.getSlot(selectedHotbarIndex).id;
}

const ItemStack& Player::getHeldItem() const {
    return inventory.getSlot(selectedHotbarIndex);
}

void Player::triggerAttack() {
    attackSwingTimer = 0.25f;
    attackCooldownTimer = 0.0f;
}

float Player::getAttackCooldownDuration() const {
    const ItemStack& held = getHeldItem();
    if (held.isEmpty()) return 0.25f; // Bare hands: 4.0 attacks/sec -> 0.25s
    const auto& def = ItemRegistry::get(held.id);
    if (def.attackSpeed <= 0.05f) return 0.50f;
    return 1.0f / def.attackSpeed;
}

float Player::getAttackRechargeProgress() const {
    float dur = getAttackCooldownDuration();
    if (dur <= 0.001f) return 1.0f;
    return std::clamp(attackCooldownTimer / dur, 0.0f, 1.0f);
}

float Player::getAttackPower() const {
    const ItemStack& held = getHeldItem();
    float weaponDmg = baseAttackPower;
    if (!held.isEmpty()) {
        const auto& def = ItemRegistry::get(held.id);
        if (def.attackDamage > weaponDmg) {
            weaponDmg = def.attackDamage;
        }
    }
    float base = weaponDmg + static_cast<float>(level - 1) * 2.0f;

    // Racial bonuses
    if (isDemon()) base += 4.0f;
    if (isElf()) base += 2.0f;

    // Minecraft 1.9+ Attack Charge Damage Scaling:
    float charge = getAttackRechargeProgress();
    float damageMultiplier = 0.20f + 0.80f * (charge * charge);

    // Skill Tree Berserker passive
    damageMultiplier *= skillTree.getDamageMultiplier();

    return base * damageMultiplier;
}

bool Player::tryUseHeldItem(AudioEngine* audio) {
    ItemStack& held = inventory.getSlot(selectedHotbarIndex);
    if (held.isEmpty()) return false;

    const auto& def = ItemRegistry::get(held.id);
    bool consumed = false;

    if (def.healAmount > 0.0f) {
        if (health < maxHealth) {
            health = std::min(maxHealth, health + def.healAmount);
            consumed = true;
            if (audio) audio->playSound(SoundID::LevelUp, 1.2f, 0.8f);
        }
    } else if (def.manaAmount > 0.0f) {
        if (mana < maxMana) {
            mana = std::min(maxMana, mana + def.manaAmount);
            consumed = true;
            if (audio) audio->playSound(SoundID::LevelUp, 1.4f, 0.8f);
        }
    } else if (def.staminaAmount > 0.0f) {
        if (stamina < maxStamina) {
            stamina = std::min(maxStamina, stamina + def.staminaAmount);
            consumed = true;
            if (audio) audio->playSound(SoundID::Footstep, 0.8f, 1.0f);
        }
    }

    if (consumed) {
        held.count--;
        if (held.count == 0) held.clear();
        return true;
    }

    return false;
}

bool Player::tryDash(const Vec3& wishDir, const Vec3& lookForward, AudioEngine* audio) {
    if (dashCooldown <= 0.0f && stamina >= 20.0f) {
        stamina -= 20.0f;
        dashCooldown = 0.85f;
        dashTimer = 0.22f;

        Vec3 dir = wishDir;
        if (dir.lengthSq() < 0.01f) {
            dir = lookForward;
            dir.y = 0.0f;
        }
        if (dir.lengthSq() > 0.01f) {
            dir = dir.normalized();
        } else {
            dir = Vec3(0, 0, 1);
        }
        dashDir = dir;

        velocity.x = dashDir.x * 22.0f;
        velocity.z = dashDir.z * 22.0f;
        if (onGround) {
            velocity.y = 2.2f;
        }

        if (audio) {
            audio->playSound(SoundID::Dash, 1.1f, 0.9f);
        }
        return true;
    }
    return false;
}

void Player::handleInput(const Window& window, Camera& camera, AudioEngine* audio, float dt, bool isBuildingMode) {
    // 1. Mouse look (only if cursor is locked in game mode)
    if (window.isCursorLocked()) {
        float sensitivity = 0.12f * camera.getZoomRatio();
        camera.addYawPitch(window.getMouseDeltaX() * sensitivity, -window.getMouseDeltaY() * sensitivity);
    }

    // 2. Hotbar numeric keys 1-9
    for (int k = 0; k < 9; ++k) {
        if (window.isKeyPressed('1' + k)) {
            setSelectedHotbarIndex(k);
        }
    }

    // 3. Mouse wheel scroll
    int wheel = window.getMouseWheelDelta();
    if (wheel != 0) {
        cycleHotbar(wheel);
    }

    // 4. WASD Movement Direction
    float yawRad = camera.getYaw() * DEG2RAD;
    Vec3 forward(std::cos(yawRad), 0.0f, std::sin(yawRad));
    Vec3 right(-std::sin(yawRad), 0.0f, std::cos(yawRad));

    Vec3 wishDir{0, 0, 0};
    if (window.isKeyDown('W')) wishDir += forward;
    if (window.isKeyDown('S')) wishDir -= forward;
    if (window.isKeyDown('D')) wishDir += right;
    if (window.isKeyDown('A')) wishDir -= right;

    bool isMoving = (wishDir.lengthSq() > 0.001f);
    if (isMoving) wishDir = wishDir.normalized();

    // 5. Crouch with Shift (Minecraft sneak)
    bool shiftDown = window.isKeyDown(VK_SHIFT);
    isCrouching = shiftDown;
    if (isCrouching) {
        isSprinting = false; // Cannot sprint while crouching
        size.y = 1.45f;
    } else {
        size.y = 1.80f;
    }
    camera.updateCrouch(isCrouching, dt);

    // 6. Sprinting by double-tapping "W" (or holding Left Control)
    timeSinceLastWPress += dt;
    if (window.isKeyPressed('W')) {
        if (timeSinceLastWPress <= 0.28f && stamina > 5.0f && !isCrouching) {
            isSprinting = true;
        }
        timeSinceLastWPress = 0.0f;
    }
    if (window.isKeyDown(VK_CONTROL) && window.isKeyDown('W') && stamina > 5.0f && !isCrouching) {
        isSprinting = true;
    }

    // Cancel sprinting when W is released, movement stops, or crouching
    if (isSprinting) {
        if (!window.isKeyDown('W') || !isMoving || isCrouching || stamina <= 0.0f) {
            isSprinting = false;
        }
    }

    // 7. Dash with "R"
    if (window.isKeyPressed('R') && !isBuildingMode) {
        tryDash(wishDir, camera.getForward(), audio);
    }

    // Cooldown timers
    if (dashCooldown > 0.0f) {
        dashCooldown = std::max(0.0f, dashCooldown - dt);
    }

    // Stamina drain and regeneration
    if (isSprinting) {
        stamina = std::max(0.0f, stamina - 20.0f * dt);
    } else if (dashTimer <= 0.0f) {
        float regenBase = isCrouching ? 22.0f : 15.0f;
        stamina = std::min(maxStamina, stamina + (regenBase + static_cast<float>(level - 1) * 0.75f) * dt);
    }

    // Movement speed calculation
    float speed;
    if (inLava) {
        speed = isDemon() ? 3.6f : 1.8f;
    } else if (inWater) {
        speed = isSprinting ? 6.0f : 3.6f;
    } else {
        speed = isCrouching ? 2.4f : (isSprinting ? 8.8f : 5.0f);
        if (isElf()) speed *= 1.10f;
    }
    speed *= skillTree.getSpeedMultiplier();

    // Horizontal acceleration & Dash momentum
    if (dashTimer > 0.0f) {
        dashTimer -= dt;
        velocity.x = dashDir.x * 21.0f;
        velocity.z = dashDir.z * 21.0f;
    } else {
        velocity.x += (wishDir.x * speed - velocity.x) * 12.0f * dt;
        velocity.z += (wishDir.z * speed - velocity.z) * 12.0f * dt;
    }

    // Jump & Fluid Swim Controls
    if (inWater) {
        // Space swims upwards effortlessly
        if (window.isKeyDown(VK_SPACE)) {
            velocity.y = std::min(velocity.y + 16.0f * dt, 4.8f);
        }
        // Shift dives downwards smoothly
        if (shiftDown) {
            velocity.y = std::max(velocity.y - 16.0f * dt, -5.2f);
        }
    } else if (inLava) {
        // Thick viscous paddling in molten magma
        if (window.isKeyDown(VK_SPACE)) {
            velocity.y = std::min(velocity.y + (isDemon() ? 14.0f : 9.0f) * dt, isDemon() ? 4.2f : 2.5f);
        }
    } else {
        // Normal ground jump & Double Jump mechanics
        if (window.isKeyPressed(VK_SPACE)) {
            if (onGround) {
                velocity.y = isCrouching ? 6.5f : (isSprinting ? 9.6f : 9.2f);
                if (isSlime()) velocity.y *= 1.25f; // Slime bouncy jump
                onGround = false;
                canDoubleJump = true;
            } else if (canDoubleJump && skillTree.hasSkill(SkillId::DoubleJump) && stamina >= 15.0f) {
                // Mid-air Double Jump!
                velocity.y = 8.8f;
                canDoubleJump = false;
                stamina = std::max(0.0f, stamina - 15.0f);
                if (audio) audio->playSound(SoundID::LevelUp, 1.6f, 0.6f);
            }
        } else if (window.isKeyDown(VK_SPACE) && onGround) {
            velocity.y = isCrouching ? 6.5f : (isSprinting ? 9.6f : 9.2f);
            if (isSlime()) velocity.y *= 1.25f;
            onGround = false;
            canDoubleJump = true;
        }
    }
}

void Player::update(World* world, AudioEngine* audio, float dt) {
    // 1. Detect fluid states (feet, waist, eyes)
    int px = static_cast<int>(std::floor(position.x));
    int pz = static_cast<int>(std::floor(position.z));
    int footY = static_cast<int>(std::floor(position.y + 0.15f));
    int waistY = static_cast<int>(std::floor(position.y + 0.85f));
    int eyeY = static_cast<int>(std::floor(position.y + 1.62f));

    uint16_t bFoot = world ? world->getBlock(px, footY, pz) : 0;
    uint16_t bWaist = world ? world->getBlock(px, waistY, pz) : 0;
    uint16_t bEye = world ? world->getBlock(px, eyeY, pz) : 0;

    inWater = (bFoot == 17 || bWaist == 17);
    isUnderwater = (bEye == 17);

    inLava = (bFoot == 32 || bWaist == 32);
    isUnderLava = (bEye == 32);

    // 2. Apply fluid dynamics, buoyancy & viscous drag
    if (inWater) {
        hasGravity = false;
        // Buoyancy: gentle downward sinking
        velocity.y -= 5.0f * dt;
        if (velocity.y < -8.5f) velocity.y = -8.5f;

        // Water drag
        float dragH = std::clamp(1.0f - 3.6f * dt, 0.0f, 1.0f);
        float dragV = std::clamp(1.0f - 2.8f * dt, 0.0f, 1.0f);
        velocity.x *= dragH;
        velocity.z *= dragH;
        velocity.y *= dragV;

        lavaBurnTimer = 0.0f;
    } else if (inLava) {
        hasGravity = false;
        // Molten lava viscosity
        velocity.y -= 2.2f * dt;
        if (velocity.y < -4.0f) velocity.y = -4.0f;

        float drag = std::clamp(1.0f - 7.5f * dt, 0.0f, 1.0f);
        velocity *= drag;

        // Lava thermal damage
        if (isDemon()) {
            lavaBurnTimer = 0.0f; // Demon is completely immune to lava burns
        } else {
            lavaBurnTimer += dt;
            if (lavaBurnTimer >= 0.45f) {
                lavaBurnTimer = 0.0f;
                takeDamage(12.0f, audio);
            }
        }
    } else {
        hasGravity = true;
        lavaBurnTimer = 0.0f;
    }

    Entity::update(world, dt);

    if (onGround) {
        canDoubleJump = true;
    }

    // Passive health regeneration from Skill Tree
    float hpRegen = skillTree.getHealthRegenPerSecond();
    if (hpRegen > 0.0f && health < maxHealth) {
        health = std::min(maxHealth, health + hpRegen * dt);
    }

    // Mana regeneration
    float manaRegenRate = 4.0f * skillTree.getManaRegenMultiplier();
    if (isElf()) manaRegenRate *= 1.50f; // Elf mana affinity
    if (mana < maxMana) {
        mana = std::min(maxMana, mana + manaRegenRate * dt);
    }

    if (attackSwingTimer > 0.0f) {
        attackSwingTimer -= dt;
    }
    attackCooldownTimer += dt;

    // Footsteps and swimming water splashes
    float horizontalSpeed = std::sqrt(velocity.x * velocity.x + velocity.z * velocity.z);
    if (inWater) {
        if (horizontalSpeed > 0.8f || std::abs(velocity.y) > 0.8f) {
            footstepTimer += dt;
            if (footstepTimer >= 0.48f) {
                footstepTimer = 0.0f;
                if (audio) audio->playSound(SoundID::Footstep, 0.55f + (rand() % 12) / 100.0f, 0.28f);
            }
        }
    } else if (onGround && horizontalSpeed > 0.8f && !isCrouching) {
        float stepRate = isSprinting ? 3.6f : 4.5f;
        footstepTimer += dt * (horizontalSpeed / stepRate);
        if (footstepTimer >= 0.38f) {
            footstepTimer = 0.0f;
            if (audio) audio->playSound(SoundID::Footstep, 0.95f + (rand() % 15) / 100.0f, isSprinting ? 0.55f : 0.4f);
        }
    } else {
        footstepTimer = 0.2f;
    }
}

void Player::takeDamage(float amount, AudioEngine* audio) {
    // Invulnerability frames during dash dodge!
    if (dashTimer > 0.0f) {
        return;
    }

    // Slime rubbery body absorbs 50% physical impact & fall damage
    if (isSlime()) {
        amount *= 0.50f;
    }

    float def = getDefense();
    float reduction = std::min(0.80f, def * 0.035f);
    float actualDmg = amount * (1.0f - reduction);

    health = std::max(0.0f, health - actualDmg);
    if (audio) {
        audio->playSound(SoundID::PlayerHurt, 1.0f, 1.0f);
    }
}

void Player::gainXP(uint32_t amount, AudioEngine* audio) {
    xp += amount;
    while (xp >= xpToNextLevel) {
        xp -= xpToNextLevel;
        level++;
        xpToNextLevel = static_cast<uint32_t>(xpToNextLevel * 1.5f);
        maxHealth += 20.0f;
        health = maxHealth;
        maxMana += 15.0f;
        mana = maxMana;
        maxStamina += 15.0f;
        stamina = maxStamina;
        baseAttackPower += 4.0f;

        // Award Skill Points (Human gains 2, others gain 1)
        uint32_t spAward = isHuman() ? 2 : 1;
        skillPoints += spAward;

        // Level-based inventory expansion (+3 slots unlocked per level up!)
        uint32_t newUnlocked = std::min(static_cast<uint32_t>(Inventory::MAIN_AND_HOTBAR), 18u + (level - 1) * 3u);
        inventory.setUnlockedSlotCount(newUnlocked);

        if (audio) {
            audio->playSound(SoundID::LevelUp, 1.0f, 1.0f);
        }
        std::cout << "LEVEL UP! Player is now Level " << level << "! Max Stamina: " << maxStamina 
                  << " | Unlocked Storage Slots: " << newUnlocked 
                  << " | Awarded SP: +" << spAward << " (Total SP: " << skillPoints << ")!" << std::endl;
    }
}

} // namespace Aetheria
