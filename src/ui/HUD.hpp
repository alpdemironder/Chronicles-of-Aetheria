#pragma once

#include "UIRenderer.hpp"
#include "../core/Camera.hpp"
#include <string>
#include <vector>

namespace Aetheria {

class Player;
class World;
class BuildingManager;
class Creature;

struct HUDNotification {
    std::string text;
    Vec4 color{1.0f, 1.0f, 1.0f, 1.0f};
    float timer = 3.2f;
};

class HUD {
public:
    HUD(UIRenderer* ui);

    void addNotification(const std::string& text, const Vec4& color = {0.35f, 0.95f, 0.45f, 1.0f});

    void toggleF3() { showF3 = !showF3; }
    bool isF3Open() const { return showF3; }
    void setF3Open(bool o) { showF3 = o; }

    void toggleHUD() { showHUD = !showHUD; }
    bool isHUDVisible() const { return showHUD; }
    void setHUDVisible(bool v) { showHUD = v; }

    void render(int screenWidth, int screenHeight,
                const Player& player,
                const World& world,
                const BuildingManager& buildingMgr,
                const Camera& camera,
                const Mat4& viewProj,
                bool hasTargetedBlock,
                const IVec3& targetedBlockPos,
                float fps, float totalTime, float dt = 0.016f,
                bool isZooming = false,
                float miningProgress = 0.0f,
                uint16_t miningBlockId = 0,
                const Creature* aimCreature = nullptr,
                float aimCatchChance = 0.0f,
                const Creature* activeCompanion = nullptr);

private:
    void renderMinecraftCrosshair(float cx, float cy, float atkProg, float totalTime);
    void renderTargetedBlockOutline(const Mat4& viewProj, const IVec3& bPos, float sw, float sh, float miningProgress);
    void renderMinecraftHotbar(float cx, float sh, const Player& player, float dt);
    void renderMinecraftHearts(float startX, float startY, float currentHP, float maxHP, float totalTime);
    void renderMinecraftDrumsticks(float startX, float startY, float currentStamina, float maxStamina, float totalTime);
    void renderMinecraftArmor(float startX, float startY, float defense);
    void renderMinecraftBubbles(float startX, float startY, float airProgress, float totalTime);
    void renderMinecraftXPBar(float hotbarX, float xpY, float hotbarW, float xpFrac, uint32_t level);
    void renderFirstPersonHand(float sw, float sh, const Player& player, const Camera& camera, float totalTime);
    void renderF3DebugScreen(int screenWidth, int screenHeight, const Player& player, const World& world,
                             const Camera& camera, float fps, bool hasTargetedBlock, const IVec3& targetedBlockPos);

    // Pixel art drawing helpers
    void drawPixelHeart(float cx, float cy, float scale, int state, bool wobble, float wobbleOffset, bool flash);
    void drawPixelDrumstick(float cx, float cy, float scale, int state, bool wobble, float wobbleOffset);
    void drawPixelArmor(float cx, float cy, float scale, int state);
    void drawPixelBubble(float cx, float cy, float scale, bool full);

    UIRenderer* ui = nullptr;
    std::vector<HUDNotification> notifications;

    bool showF3 = false;
    bool showHUD = true;

    int lastSelectedSlot = -1;
    std::string heldItemTooltip;
    float tooltipTimer = 0.0f;
    float lastPlayerHealth = 100.0f;
    float hurtFlashTimer = 0.0f;
};

} // namespace Aetheria
