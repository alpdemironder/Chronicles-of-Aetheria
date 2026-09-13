#include "HUD.hpp"
#include "ItemIconRenderer.hpp"
#include "../entities/Player.hpp"
#include "../entities/Creature.hpp"
#include "../world/World.hpp"
#include "../world/BlockRegistry.hpp"
#include "../world/BiomeRegistry.hpp"
#include "../building/BuildingManager.hpp"
#include "../inventory/ItemRegistry.hpp"
#include <sstream>
#include <iomanip>
#include <cmath>
#include <algorithm>

namespace Aetheria {

HUD::HUD(UIRenderer* ui) : ui(ui) {
}

void HUD::addNotification(const std::string& text, const Vec4& color) {
    notifications.push_back({text, color, 3.2f});
    if (notifications.size() > 5) {
        notifications.erase(notifications.begin());
    }
}

// =========================================================================
// PIXEL-ART DRAWING HELPERS (Authentic 16x16 Minecraft Style)
// =========================================================================

void HUD::drawPixelHeart(float cx, float cy, float scale, int state, bool wobble, float wobbleOffset, bool flash) {
    float y = cy + (wobble ? wobbleOffset : 0.0f);
    float x = cx;
    float s = scale;

    // 1. Black Pixel Outline (9x9 pixels)
    Vec4 borderCol(0.0f, 0.0f, 0.0f, 1.0f);
    // Top lobes
    ui->drawRect(x + 1 * s, y + 0 * s, 3 * s, 1 * s, borderCol);
    ui->drawRect(x + 5 * s, y + 0 * s, 3 * s, 1 * s, borderCol);
    // Side drops
    ui->drawRect(x + 0 * s, y + 1 * s, 1 * s, 3 * s, borderCol);
    ui->drawRect(x + 4 * s, y + 1 * s, 1 * s, 2 * s, borderCol);
    ui->drawRect(x + 8 * s, y + 1 * s, 1 * s, 3 * s, borderCol);
    // Tapering bottom
    ui->drawRect(x + 1 * s, y + 4 * s, 1 * s, 2 * s, borderCol);
    ui->drawRect(x + 7 * s, y + 4 * s, 1 * s, 2 * s, borderCol);
    ui->drawRect(x + 2 * s, y + 6 * s, 1 * s, 1 * s, borderCol);
    ui->drawRect(x + 6 * s, y + 6 * s, 1 * s, 1 * s, borderCol);
    ui->drawRect(x + 3 * s, y + 7 * s, 1 * s, 1 * s, borderCol);
    ui->drawRect(x + 5 * s, y + 7 * s, 1 * s, 1 * s, borderCol);
    ui->drawRect(x + 4 * s, y + 8 * s, 1 * s, 1 * s, borderCol);

    // 2. Heart Interior Background (Empty dark socket)
    Vec4 emptyBg(0.18f, 0.02f, 0.02f, 1.0f);
    ui->drawRect(x + 1 * s, y + 1 * s, 3 * s, 3 * s, emptyBg);
    ui->drawRect(x + 5 * s, y + 1 * s, 3 * s, 3 * s, emptyBg);
    ui->drawRect(x + 2 * s, y + 4 * s, 5 * s, 2 * s, emptyBg);
    ui->drawRect(x + 3 * s, y + 6 * s, 3 * s, 1 * s, emptyBg);
    ui->drawRect(x + 4 * s, y + 7 * s, 1 * s, 1 * s, emptyBg);

    if (state == 0) return; // Empty container

    // 3. Heart Fill Colors (Bright red or Flash white/yellow)
    Vec4 fillRed = flash ? Vec4(1.0f, 1.0f, 0.85f, 1.0f) : Vec4(0.95f, 0.12f, 0.15f, 1.0f);
    Vec4 darkRed = flash ? Vec4(0.90f, 0.90f, 0.65f, 1.0f) : Vec4(0.65f, 0.05f, 0.08f, 1.0f);
    Vec4 whiteShine(1.0f, 1.0f, 1.0f, 1.0f);

    if (state == 2) {
        // FULL HEART
        // Dark crimson contour
        ui->drawRect(x + 1 * s, y + 1 * s, 3 * s, 1 * s, darkRed);
        ui->drawRect(x + 5 * s, y + 1 * s, 3 * s, 1 * s, darkRed);
        ui->drawRect(x + 1 * s, y + 2 * s, 1 * s, 2 * s, darkRed);
        ui->drawRect(x + 7 * s, y + 2 * s, 1 * s, 2 * s, darkRed);
        ui->drawRect(x + 2 * s, y + 4 * s, 1 * s, 2 * s, darkRed);
        ui->drawRect(x + 6 * s, y + 4 * s, 1 * s, 2 * s, darkRed);
        ui->drawRect(x + 3 * s, y + 6 * s, 1 * s, 1 * s, darkRed);
        ui->drawRect(x + 5 * s, y + 6 * s, 1 * s, 1 * s, darkRed);
        ui->drawRect(x + 4 * s, y + 7 * s, 1 * s, 1 * s, darkRed);

        // Bright red center
        ui->drawRect(x + 2 * s, y + 2 * s, 2 * s, 2 * s, fillRed);
        ui->drawRect(x + 5 * s, y + 2 * s, 2 * s, 2 * s, fillRed);
        ui->drawRect(x + 4 * s, y + 3 * s, 1 * s, 1 * s, fillRed);
        ui->drawRect(x + 3 * s, y + 4 * s, 3 * s, 2 * s, fillRed);
        ui->drawRect(x + 4 * s, y + 6 * s, 1 * s, 1 * s, fillRed);

        // Iconic white specular reflection dot (top-left lobe)
        if (!flash) {
            ui->drawRect(x + 2 * s, y + 1 * s, 1 * s, 1 * s, whiteShine);
        }
    } else if (state == 1) {
        // HALF HEART (Left half full, Right half empty)
        ui->drawRect(x + 1 * s, y + 1 * s, 3 * s, 1 * s, darkRed);
        ui->drawRect(x + 1 * s, y + 2 * s, 1 * s, 2 * s, darkRed);
        ui->drawRect(x + 2 * s, y + 4 * s, 1 * s, 2 * s, darkRed);
        ui->drawRect(x + 3 * s, y + 6 * s, 1 * s, 1 * s, darkRed);
        ui->drawRect(x + 4 * s, y + 7 * s, 1 * s, 1 * s, darkRed);

        ui->drawRect(x + 2 * s, y + 2 * s, 2 * s, 2 * s, fillRed);
        ui->drawRect(x + 3 * s, y + 4 * s, 1 * s, 2 * s, fillRed);
        ui->drawRect(x + 4 * s, y + 4 * s, 1 * s, 2 * s, darkRed);

        if (!flash) {
            ui->drawRect(x + 2 * s, y + 1 * s, 1 * s, 1 * s, whiteShine);
        }
    }
}

void HUD::drawPixelDrumstick(float cx, float cy, float scale, int state, bool wobble, float wobbleOffset) {
    float y = cy + (wobble ? wobbleOffset : 0.0f);
    float x = cx;
    float s = scale;

    // 1. Dark Outline
    Vec4 borderCol(0.18f, 0.08f, 0.03f, 1.0f);
    ui->drawRect(x + 1 * s, y + 1 * s, 5 * s, 1 * s, borderCol);
    ui->drawRect(x + 0 * s, y + 2 * s, 1 * s, 4 * s, borderCol);
    ui->drawRect(x + 6 * s, y + 2 * s, 1 * s, 4 * s, borderCol);
    ui->drawRect(x + 1 * s, y + 6 * s, 5 * s, 1 * s, borderCol);
    // Bone handle
    ui->drawRect(x + 6 * s, y + 0 * s, 3 * s, 1 * s, borderCol);
    ui->drawRect(x + 8 * s, y + 1 * s, 1 * s, 3 * s, borderCol);
    ui->drawRect(x + 5 * s, y + 1 * s, 1 * s, 1 * s, borderCol);

    // 2. Empty background
    Vec4 emptyBg(0.12f, 0.07f, 0.04f, 1.0f);
    ui->drawRect(x + 1 * s, y + 2 * s, 5 * s, 4 * s, emptyBg);

    if (state == 0) return;

    // 3. Drumstick Fill (Savory roasted brown & highlight)
    Vec4 meatBrown(0.72f, 0.38f, 0.12f, 1.0f);
    Vec4 meatDark(0.52f, 0.24f, 0.08f, 1.0f);
    Vec4 meatHighlight(0.92f, 0.58f, 0.22f, 1.0f);
    Vec4 boneWhite(0.95f, 0.95f, 0.92f, 1.0f);

    // Bone knob
    ui->drawRect(x + 6 * s, y + 1 * s, 2 * s, 1 * s, boneWhite);
    ui->drawRect(x + 7 * s, y + 2 * s, 1 * s, 1 * s, boneWhite);

    if (state == 2) {
        // FULL DRUMSTICK
        ui->drawRect(x + 1 * s, y + 2 * s, 5 * s, 4 * s, meatBrown);
        ui->drawRect(x + 2 * s, y + 2 * s, 3 * s, 1 * s, meatHighlight);
        ui->drawRect(x + 1 * s, y + 3 * s, 1 * s, 2 * s, meatHighlight);
        ui->drawRect(x + 2 * s, y + 5 * s, 4 * s, 1 * s, meatDark);
    } else if (state == 1) {
        // HALF DRUMSTICK
        ui->drawRect(x + 1 * s, y + 2 * s, 3 * s, 4 * s, meatBrown);
        ui->drawRect(x + 2 * s, y + 2 * s, 2 * s, 1 * s, meatHighlight);
        ui->drawRect(x + 1 * s, y + 5 * s, 3 * s, 1 * s, meatDark);
    }
}

void HUD::drawPixelArmor(float cx, float cy, float scale, int state) {
    float x = cx;
    float y = cy;
    float s = scale;

    Vec4 borderCol(0.12f, 0.12f, 0.14f, 1.0f);
    // Chestplate silhouette
    ui->drawRect(x + 1 * s, y + 0 * s, 7 * s, 1 * s, borderCol);
    ui->drawRect(x + 0 * s, y + 1 * s, 9 * s, 1 * s, borderCol);
    ui->drawRect(x + 0 * s, y + 2 * s, 1 * s, 4 * s, borderCol);
    ui->drawRect(x + 8 * s, y + 2 * s, 1 * s, 4 * s, borderCol);
    ui->drawRect(x + 1 * s, y + 6 * s, 7 * s, 1 * s, borderCol);
    ui->drawRect(x + 2 * s, y + 7 * s, 5 * s, 1 * s, borderCol);

    // Empty background
    Vec4 emptyBg(0.10f, 0.10f, 0.12f, 1.0f);
    ui->drawRect(x + 1 * s, y + 1 * s, 7 * s, 5 * s, emptyBg);

    if (state == 0) return;

    Vec4 ironMid(0.72f, 0.74f, 0.78f, 1.0f);
    Vec4 ironLight(0.92f, 0.94f, 0.98f, 1.0f);
    Vec4 ironDark(0.48f, 0.50f, 0.54f, 1.0f);

    if (state == 2) {
        // Full iron armor piece
        ui->drawRect(x + 1 * s, y + 1 * s, 7 * s, 5 * s, ironMid);
        ui->drawRect(x + 1 * s, y + 1 * s, 3 * s, 1 * s, ironLight);
        ui->drawRect(x + 1 * s, y + 2 * s, 1 * s, 3 * s, ironLight);
        ui->drawRect(x + 5 * s, y + 4 * s, 2 * s, 2 * s, ironDark);
        // Neck indent
        ui->drawRect(x + 3 * s, y + 1 * s, 3 * s, 2 * s, borderCol);
    } else if (state == 1) {
        // Half piece
        ui->drawRect(x + 1 * s, y + 1 * s, 4 * s, 5 * s, ironMid);
        ui->drawRect(x + 1 * s, y + 1 * s, 2 * s, 1 * s, ironLight);
        ui->drawRect(x + 3 * s, y + 1 * s, 2 * s, 2 * s, borderCol);
    }
}

void HUD::drawPixelBubble(float cx, float cy, float scale, bool full) {
    if (!full) return;
    float x = cx;
    float y = cy;
    float s = scale;

    Vec4 blueDark(0.04f, 0.15f, 0.35f, 1.0f);
    Vec4 blueLight(0.25f, 0.75f, 1.0f, 1.0f);
    Vec4 white(1.0f, 1.0f, 1.0f, 1.0f);

    ui->drawRect(x + 2 * s, y + 0 * s, 4 * s, 1 * s, blueDark);
    ui->drawRect(x + 1 * s, y + 1 * s, 6 * s, 6 * s, blueLight);
    ui->drawRect(x + 0 * s, y + 2 * s, 1 * s, 4 * s, blueDark);
    ui->drawRect(x + 7 * s, y + 2 * s, 1 * s, 4 * s, blueDark);
    ui->drawRect(x + 2 * s, y + 7 * s, 4 * s, 1 * s, blueDark);

    ui->drawRect(x + 2 * s, y + 2 * s, 2 * s, 2 * s, white);
}

// =========================================================================
// 1. VANILLA MINECRAFT CROSSHAIR & ATTACK RECHARGE
// =========================================================================

void HUD::renderMinecraftCrosshair(float cx, float cy, float atkProg, float totalTime) {
    // Exact Minecraft 15x15 inverted crosshair (+) with 1px center transparent gap
    Vec4 chColor(1.0f, 1.0f, 1.0f, 0.85f);
    // Horizontal arms
    ui->drawRect(cx - 7.0f, cy - 0.5f, 6.0f, 1.0f, chColor);
    ui->drawRect(cx + 2.0f, cy - 0.5f, 6.0f, 1.0f, chColor);
    // Vertical arms
    ui->drawRect(cx - 0.5f, cy - 7.0f, 1.0f, 6.0f, chColor);
    ui->drawRect(cx - 0.5f, cy + 2.0f, 1.0f, 6.0f, chColor);

    // Minecraft 1.9+ Attack Recharge Indicator below crosshair
    if (atkProg < 0.99f) {
        float indW = 18.0f;
        float indH = 4.0f;
        float indX = cx - indW * 0.5f;
        float indY = cy + 12.0f;

        // Black outer frame
        ui->drawRect(indX - 1.0f, indY - 1.0f, indW + 2.0f, indH + 2.0f, {0.0f, 0.0f, 0.0f, 0.75f});
        // Dark groove
        ui->drawRect(indX, indY, indW, indH, {0.15f, 0.15f, 0.15f, 0.85f});

        // Fill progress
        Vec4 fillCol = (atkProg >= 0.85f) ? Vec4(0.20f, 0.95f, 0.35f, 0.95f) : Vec4(0.85f, 0.85f, 0.85f, 0.90f);
        ui->drawRect(indX, indY, indW * atkProg, indH, fillCol);
    }
}

// =========================================================================
// 2. 3D TARGETED BLOCK WIREFRAME (Bounding Box) & CRACKING ANIMATION
// =========================================================================

void HUD::renderTargetedBlockOutline(const Mat4& viewProj, const IVec3& bPos, float sw, float sh, float miningProgress) {
    // Inset/outset box by 0.002 to prevent z-fighting with voxel faces
    Vec3 minC(bPos.x - 0.002f, bPos.y - 0.002f, bPos.z - 0.002f);
    Vec3 maxC(bPos.x + 1.002f, bPos.y + 1.002f, bPos.z + 1.002f);

    Vec3 corners[8] = {
        {minC.x, minC.y, minC.z}, // 0: ---
        {maxC.x, minC.y, minC.z}, // 1: +--
        {maxC.x, maxC.y, minC.z}, // 2: ++-
        {minC.x, maxC.y, minC.z}, // 3: -+-
        {minC.x, minC.y, maxC.z}, // 4: --+
        {maxC.x, minC.y, maxC.z}, // 5: +-+
        {maxC.x, maxC.y, maxC.z}, // 6: +++
        {minC.x, maxC.y, maxC.z}, // 7: -++
    };

    Vec2 screenP[8];
    bool valid[8];

    for (int i = 0; i < 8; ++i) {
        Vec4 clip = viewProj * Vec4(corners[i], 1.0f);
        if (clip.w > 0.05f) {
            float ndcX = clip.x / clip.w;
            float ndcY = clip.y / clip.w;
            screenP[i] = Vec2((ndcX * 0.5f + 0.5f) * sw, (1.0f - (ndcY * 0.5f + 0.5f)) * sh);
            valid[i] = (ndcX >= -2.0f && ndcX <= 2.0f && ndcY >= -2.0f && ndcY <= 2.0f);
        } else {
            valid[i] = false;
        }
    }

    // 12 edges of Minecraft bounding box
    int edges[12][2] = {
        {0,1}, {1,2}, {2,3}, {3,0},
        {4,5}, {5,6}, {6,7}, {7,4},
        {0,4}, {1,5}, {2,6}, {3,7}
    };

    Vec4 outlineCol(0.0f, 0.0f, 0.0f, 0.65f);
    for (int e = 0; e < 12; ++e) {
        int i1 = edges[e][0];
        int i2 = edges[e][1];
        if (valid[i1] && valid[i2]) {
            ui->drawLine(screenP[i1].x, screenP[i1].y, screenP[i2].x, screenP[i2].y, 1.8f, outlineCol);
        }
    }

    // Minecraft Progressive Cracking Animation (Stages 0-9)
    if (miningProgress > 0.0f) {
        float prog = std::clamp(miningProgress, 0.0f, 1.0f);
        int stage = static_cast<int>(prog * 9.0f);
        Vec4 crackCol(0.05f, 0.05f, 0.05f, 0.65f + prog * 0.30f);

        // Draw dynamic fractures across face centers
        if (valid[0] && valid[2] && valid[4] && valid[6]) {
            float midX = (screenP[0].x + screenP[6].x) * 0.5f;
            float midY = (screenP[0].y + screenP[6].y) * 0.5f;

            for (int s = 0; s <= stage; ++s) {
                float angle = s * 0.785f + (s % 2) * 0.35f;
                float dist = 8.0f + s * 4.0f;
                float x2 = midX + std::cos(angle) * dist;
                float y2 = midY + std::sin(angle) * dist;
                ui->drawLine(midX, midY, x2, y2, 1.5f + (s > 5 ? 1.0f : 0.0f), crackCol);

                if (s >= 4) {
                    float subAngle = angle + 0.9f;
                    ui->drawLine(x2, y2, x2 + std::cos(subAngle) * 8.0f, y2 + std::sin(subAngle) * 8.0f, 1.2f, crackCol);
                }
            }
        }
    }
}

// =========================================================================
// 3. FIRST-PERSON ANIMATED STEVE HAND / HELD TOOL / HELD BLOCK VIEWMODEL
// =========================================================================

void HUD::renderFirstPersonHand(float sw, float sh, const Player& player, const Camera& camera, float totalTime) {
    if (camera.getIsThirdPerson()) return;

    // Viewmodel anchor at lower-right
    float baseX = sw - 100.0f;
    float baseY = sh - 10.0f;

    // Natural walk bobbing & gentle breathing
    float bobX = std::sin(totalTime * 6.0f) * 4.0f;
    float bobY = std::abs(std::cos(totalTime * 6.0f)) * 4.0f;

    // Attack / Mine / Place downward swing rotation
    float swingProgress = player.getAttackSwingProgress();
    float swingArc = std::sin(swingProgress * 3.14159f);

    float handX = baseX + bobX - swingArc * 50.0f;
    float handY = baseY + bobY - swingArc * 35.0f;

    const ItemStack& held = player.getHeldItem();

    if (held.isEmpty()) {
        // Steve's classic blocky pixel arm & fist
        // Cyan sleeve
        ui->drawRect(handX - 25.0f, handY - 10.0f, 65.0f, 90.0f, {0.0f, 0.65f, 0.75f, 1.0f});
        ui->drawRectOutline(handX - 25.0f, handY - 10.0f, 65.0f, 90.0f, 2.0f, {0.0f, 0.45f, 0.55f, 1.0f});
        // Tan fist
        ui->drawRect(handX - 35.0f, handY - 70.0f, 55.0f, 65.0f, {0.88f, 0.65f, 0.48f, 1.0f});
        ui->drawRectOutline(handX - 35.0f, handY - 70.0f, 55.0f, 65.0f, 2.0f, {0.68f, 0.45f, 0.32f, 1.0f});
    } else {
        // Held Tool or 3D Voxel Block
        float itemSize = 85.0f;
        float itemCenterX = handX - 30.0f;
        float itemCenterY = handY - 45.0f;

        // Render tool or block held in hand
        ItemIconRenderer::drawItem(ui, itemCenterX, itemCenterY, itemSize, held.id);

        // Supporting forearm peeking from bottom
        ui->drawRect(handX - 10.0f, handY + 10.0f, 50.0f, 70.0f, {0.0f, 0.65f, 0.75f, 1.0f});
    }
}

// =========================================================================
// 4. VANILLA MINECRAFT HOTBAR
// =========================================================================

void HUD::renderMinecraftHotbar(float cx, float sh, const Player& player, float dt) {
    int selectedSlot = player.getSelectedHotbarIndex();

    // Trigger action bar tooltip on slot switch
    if (selectedSlot != lastSelectedSlot) {
        lastSelectedSlot = selectedSlot;
        const ItemStack& cur = player.getInventory().getSlot(selectedSlot);
        if (!cur.isEmpty()) {
            heldItemTooltip = ItemRegistry::get(cur.id).name;
            tooltipTimer = 2.4f;
        } else {
            heldItemTooltip.clear();
            tooltipTimer = 0.0f;
        }
    }

    if (tooltipTimer > 0.0f) {
        tooltipTimer -= dt;
    }

    float slotSize = 40.0f;
    float slotGap = 2.0f;
    float hotbarW = slotSize * 9.0f + slotGap * 8.0f; // 376px
    float hotbarX = cx - hotbarW * 0.5f;
    float hotbarY = sh - slotSize - 12.0f;

    // Classic Minecraft Stone Plate Background
    ui->drawRect(hotbarX - 4.0f, hotbarY - 4.0f, hotbarW + 8.0f, slotSize + 8.0f, {0.0f, 0.0f, 0.0f, 0.75f}); // Dark outer outline
    ui->drawRect(hotbarX - 3.0f, hotbarY - 3.0f, hotbarW + 6.0f, slotSize + 6.0f, {0.55f, 0.55f, 0.55f, 1.0f}); // Stone gray backing

    const auto& inv = player.getInventory();

    for (int i = 0; i < 9; ++i) {
        float sx = hotbarX + i * (slotSize + slotGap);
        float sy = hotbarY;

        bool isSel = (i == selectedSlot);

        // Minecraft Inset Slot Box
        // Inner shadow (Dark top/left, White bottom/right)
        ui->drawRect(sx, sy, slotSize, slotSize, {0.54f, 0.54f, 0.54f, 1.0f});
        ui->drawRect(sx, sy, slotSize, 1.5f, {0.22f, 0.22f, 0.22f, 1.0f}); // Top dark
        ui->drawRect(sx, sy, 1.5f, slotSize, {0.22f, 0.22f, 0.22f, 1.0f}); // Left dark
        ui->drawRect(sx, sy + slotSize - 1.5f, slotSize, 1.5f, {0.85f, 0.85f, 0.85f, 1.0f}); // Bottom light
        ui->drawRect(sx + slotSize - 1.5f, sy, 1.5f, slotSize, {0.85f, 0.85f, 0.85f, 1.0f}); // Right light

        // Item Icon
        const ItemStack& item = inv.getSlot(i);
        if (!item.isEmpty()) {
            ItemIconRenderer::drawItem(ui, sx + slotSize * 0.5f, sy + slotSize * 0.5f, slotSize - 8.0f, item.id);

            // Item Quantity (White with 1px black shadow in bottom-right)
            if (item.count > 1) {
                std::string cntStr = std::to_string(item.count);
                float textW = cntStr.length() * 6.0f * 1.15f;
                float tx = sx + slotSize - textW - 2.0f;
                float ty = sy + slotSize - 12.0f;
                ui->drawText(cntStr, tx + 1.0f, ty + 1.0f, 1.15f, {0.0f, 0.0f, 0.0f, 1.0f}); // Shadow
                ui->drawText(cntStr, tx, ty, 1.15f, {1.0f, 1.0f, 1.0f, 1.0f}); // White
            }
        }

        // Minecraft Selected Slot 3D White Protruding Frame
        if (isSel) {
            float fx = sx - 3.0f;
            float fy = sy - 3.0f;
            float fSize = slotSize + 6.0f;
            ui->drawRect(fx, fy, fSize, 3.0f, {1.0f, 1.0f, 1.0f, 1.0f}); // Top white
            ui->drawRect(fx, fy, 3.0f, fSize, {1.0f, 1.0f, 1.0f, 1.0f}); // Left white
            ui->drawRect(fx, fy + fSize - 3.0f, fSize, 3.0f, {0.35f, 0.35f, 0.35f, 1.0f}); // Bottom gray
            ui->drawRect(fx + fSize - 3.0f, fy, 3.0f, fSize, {0.35f, 0.35f, 0.35f, 1.0f}); // Right gray
        }
    }

    // Action Bar Item Tooltip
    if (tooltipTimer > 0.0f && !heldItemTooltip.empty()) {
        float alpha = std::min(1.0f, tooltipTimer * 1.8f);
        float ty = hotbarY - 32.0f;
        ui->drawTextCentered(heldItemTooltip, cx + 1.0f, ty + 1.0f, 1.30f, {0.0f, 0.0f, 0.0f, alpha});
        ui->drawTextCentered(heldItemTooltip, cx, ty, 1.30f, {1.0f, 1.0f, 1.0f, alpha});
    }
}

// =========================================================================
// 5. MINECRAFT EXPERIENCE BAR & LEVEL NUMBER
// =========================================================================

void HUD::renderMinecraftXPBar(float hotbarX, float xpY, float hotbarW, float xpFrac, uint32_t level) {
    // 1. Black outer border
    ui->drawRect(hotbarX - 1.0f, xpY - 1.0f, hotbarW + 2.0f, 7.0f, {0.0f, 0.0f, 0.0f, 1.0f});
    // 2. Dark olive background groove
    ui->drawRect(hotbarX, xpY, hotbarW, 5.0f, {0.08f, 0.18f, 0.05f, 1.0f});

    // 3. Vivid Neon Lime Green Progress Fill (#80FF20)
    float fillW = hotbarW * std::clamp(xpFrac, 0.0f, 1.0f);
    if (fillW > 0.0f) {
        ui->drawRect(hotbarX, xpY, fillW, 5.0f, {0.50f, 1.0f, 0.12f, 1.0f});
    }

    // 4. Subtle vertical notch segment dividers (every ~20px)
    for (float nx = hotbarX + 20.0f; nx < hotbarX + hotbarW - 5.0f; nx += 20.0f) {
        ui->drawRect(nx, xpY, 1.0f, 5.0f, {0.0f, 0.0f, 0.0f, 0.85f});
    }

    // 5. Centered Bold Minecraft Level Number (Lime green with dark drop shadow)
    std::string lvlStr = std::to_string(level);
    float cx = hotbarX + hotbarW * 0.5f;
    float textY = xpY - 16.0f;

    ui->drawTextCentered(lvlStr, cx + 1.0f, textY + 1.0f, 1.45f, {0.0f, 0.20f, 0.0f, 1.0f}); // Dark shadow
    ui->drawTextCentered(lvlStr, cx, textY, 1.45f, {0.50f, 1.0f, 0.12f, 1.0f}); // Lime green
}

// =========================================================================
// 6. MINECRAFT HEARTS, DRUMSTICKS, ARMOR & AIR BUBBLES
// =========================================================================

void HUD::renderMinecraftHearts(float startX, float startY, float currentHP, float maxHP, float totalTime) {
    bool lowHP = (currentHP <= 20.0f);
    bool flash = (hurtFlashTimer > 0.0f);

    float hpPerHeart = maxHP / 10.0f;
    float heartScale = 1.35f;
    float heartSpacing = 16.0f;

    for (int i = 0; i < 10; ++i) {
        float hMin = i * hpPerHeart;
        float hMid = hMin + hpPerHeart * 0.5f;

        int state = 0;
        if (currentHP >= hMin + hpPerHeart * 0.95f) {
            state = 2; // Full heart
        } else if (currentHP >= hMid) {
            state = 1; // Half heart
        } else {
            state = 0; // Empty container
        }

        float wobbleOffset = std::sin(totalTime * 35.0f + i * 2.2f) * 1.5f;
        drawPixelHeart(startX + i * heartSpacing, startY, heartScale, state, lowHP, wobbleOffset, flash);
    }
}

void HUD::renderMinecraftDrumsticks(float startX, float startY, float currentStamina, float maxStamina, float totalTime) {
    bool lowSta = (currentStamina <= 20.0f);
    float staPerDrum = maxStamina / 10.0f;
    float drumScale = 1.35f;
    float drumSpacing = 16.0f;

    for (int i = 0; i < 10; ++i) {
        // Minecraft renders drumsticks from right to left (i=9 to i=0)
        int idx = 9 - i;
        float sMin = idx * staPerDrum;
        float sMid = sMin + staPerDrum * 0.5f;

        int state = 0;
        if (currentStamina >= sMin + staPerDrum * 0.95f) {
            state = 2; // Full drumstick
        } else if (currentStamina >= sMid) {
            state = 1; // Half drumstick
        } else {
            state = 0; // Empty socket
        }

        float wobbleOffset = std::sin(totalTime * 30.0f + idx * 2.5f) * 1.5f;
        drawPixelDrumstick(startX + i * drumSpacing, startY, drumScale, state, lowSta, wobbleOffset);
    }
}

void HUD::renderMinecraftArmor(float startX, float startY, float defense) {
    if (defense <= 0.0f) return;

    float armorScale = 1.35f;
    float armorSpacing = 16.0f;
    float defPerPiece = 4.0f; // 10 pieces = 40 defense

    for (int i = 0; i < 10; ++i) {
        float dMin = i * defPerPiece;
        float dMid = dMin + defPerPiece * 0.5f;

        int state = 0;
        if (defense >= dMin + defPerPiece * 0.95f) {
            state = 2; // Full chestplate
        } else if (defense >= dMid) {
            state = 1; // Half chestplate
        } else {
            state = 0;
        }

        if (state > 0) {
            drawPixelArmor(startX + i * armorSpacing, startY, armorScale, state);
        }
    }
}

void HUD::renderMinecraftBubbles(float startX, float startY, float airProgress, float totalTime) {
    float bubbleScale = 1.35f;
    float bubbleSpacing = 16.0f;
    int fullBubbles = static_cast<int>(std::clamp(airProgress, 0.0f, 1.0f) * 10.0f);

    for (int i = 0; i < 10; ++i) {
        int idx = 9 - i;
        bool isFull = (idx < fullBubbles);
        drawPixelBubble(startX + i * bubbleSpacing, startY, bubbleScale, isFull);
    }
}

// =========================================================================
// 7. MINECRAFT JAVA F3 DEBUG SCREEN
// =========================================================================

void HUD::renderF3DebugScreen(int screenWidth, int screenHeight, const Player& player, const World& world,
                             const Camera& camera, float fps, bool hasTargetedBlock, const IVec3& targetedBlockPos) {
    Vec3 pPos = player.getPosition();
    uint8_t biomeId = world.getBiomeAt(pPos.x, pPos.z);
    const BiomeDef& bDef = BiomeRegistry::get(biomeId);

    float yaw = camera.getYaw();
    float pitch = camera.getPitch();

    // Determine compass facing
    float normYaw = std::fmod(yaw, 360.0f);
    if (normYaw < 0.0f) normYaw += 360.0f;
    std::string facingStr = "north (Towards negative Z)";
    if (normYaw >= 45.0f && normYaw < 135.0f) facingStr = "east (Towards positive X)";
    else if (normYaw >= 135.0f && normYaw < 225.0f) facingStr = "south (Towards positive Z)";
    else if (normYaw >= 225.0f && normYaw < 315.0f) facingStr = "west (Towards negative X)";

    // LEFT COLUMN DEBUG STRINGS
    std::vector<std::string> leftLines;
    leftLines.push_back("Chronicles of Aetheria 1.21.4 (Minecraft RPG Engine)");
    {
        std::stringstream ss;
        ss << static_cast<int>(fps) << " fps (" << std::fixed << std::setprecision(1) << (1000.0f / std::max(fps, 1.0f)) << " ms) vsync";
        leftLines.push_back(ss.str());
    }
    {
        std::stringstream ss;
        ss << "XYZ: " << std::fixed << std::setprecision(3) << pPos.x << " / " << pPos.y << " / " << pPos.z;
        leftLines.push_back(ss.str());
    }
    {
        int bx = static_cast<int>(std::floor(pPos.x));
        int by = static_cast<int>(std::floor(pPos.y));
        int bz = static_cast<int>(std::floor(pPos.z));
        int cx = (bx % 16 + 16) % 16;
        int cy = (by % 16 + 16) % 16;
        int cz = (bz % 16 + 16) % 16;
        std::stringstream ss;
        ss << "Block: " << bx << " " << by << " " << bz << " [" << cx << " " << cy << " " << cz << "]";
        leftLines.push_back(ss.str());
    }
    {
        int chkX = static_cast<int>(std::floor(pPos.x / 16.0f));
        int chkZ = static_cast<int>(std::floor(pPos.z / 16.0f));
        std::stringstream ss;
        ss << "Chunk: " << chkX << " " << chkZ << " in " << (chkX / 32) << " " << (chkZ / 32);
        leftLines.push_back(ss.str());
    }
    {
        std::stringstream ss;
        ss << "Facing: " << facingStr << " (" << std::fixed << std::setprecision(1) << yaw << " / " << pitch << ")";
        leftLines.push_back(ss.str());
    }
    leftLines.push_back("Biome: aetheria:" + bDef.name);
    leftLines.push_back("Light: 15 (15 sky, 0 block)");
    leftLines.push_back("Local Difficulty: 1.50 // 0.00 (Day " + std::to_string(world.getTimePeriodName() == "Noon" ? 1 : 2) + ")");
    if (hasTargetedBlock) {
        uint16_t bId = world.getBlock(targetedBlockPos.x, targetedBlockPos.y, targetedBlockPos.z);
        std::string bName = (bId > 0) ? BlockRegistry::get(bId).name : "Air";
        leftLines.push_back("Looking at block: " + std::to_string(targetedBlockPos.x) + " " +
                            std::to_string(targetedBlockPos.y) + " " + std::to_string(targetedBlockPos.z) + " (" + bName + ")");
    }

    // RIGHT COLUMN DEBUG STRINGS
    std::vector<std::string> rightLines;
    rightLines.push_back("Engine: OpenGL 3.3 Core Profile");
    rightLines.push_back("GPU: AMD Radeon / NVIDIA GeForce / Intel Iris");
    rightLines.push_back("Mem: 42% 418/1024MB");
    rightLines.push_back("Display: " + std::to_string(screenWidth) + "x" + std::to_string(screenHeight));

    float textScale = 1.15f;
    float lineHeight = 16.0f;

    // Render Left Column
    float curY = 10.0f;
    for (const auto& line : leftLines) {
        float w = line.length() * 6.0f * textScale + 8.0f;
        ui->drawRect(10.0f, curY, w, lineHeight, {0.0f, 0.0f, 0.0f, 0.65f}); // Translucent background
        ui->drawText(line, 14.0f, curY + 3.0f, textScale, {0.92f, 0.92f, 0.92f, 1.0f});
        curY += lineHeight + 2.0f;
    }

    // Render Right Column
    curY = 10.0f;
    for (const auto& line : rightLines) {
        float w = line.length() * 6.0f * textScale + 8.0f;
        float rx = static_cast<float>(screenWidth) - w - 10.0f;
        ui->drawRect(rx, curY, w, lineHeight, {0.0f, 0.0f, 0.0f, 0.65f});
        ui->drawText(line, rx + 4.0f, curY + 3.0f, textScale, {0.92f, 0.92f, 0.92f, 1.0f});
        curY += lineHeight + 2.0f;
    }
}

// =========================================================================
// MAIN HUD RENDER ENTRYPOINT
// =========================================================================

void HUD::render(int screenWidth, int screenHeight,
                const Player& player,
                const World& world,
                const BuildingManager& buildingMgr,
                const Camera& camera,
                const Mat4& viewProj,
                bool hasTargetedBlock,
                const IVec3& targetedBlockPos,
                float fps, float totalTime, float dt,
                bool isZooming,
                float miningProgress,
                uint16_t miningBlockId,
                const Creature* aimCreature,
                float aimCatchChance,
                const Creature* activeCompanion) {
    float sw = static_cast<float>(screenWidth);
    float sh = static_cast<float>(screenHeight);
    float cx = sw * 0.5f;
    float cy = sh * 0.5f;

    // Detect player taking damage for heart flashing
    if (player.getHealth() < lastPlayerHealth - 0.5f) {
        hurtFlashTimer = 0.35f;
    }
    lastPlayerHealth = player.getHealth();
    if (hurtFlashTimer > 0.0f) hurtFlashTimer -= dt;

    // 1. 3D Targeted Block Wireframe (Bounding Box) & Cracking Decal
    if (hasTargetedBlock) {
        renderTargetedBlockOutline(viewProj, targetedBlockPos, sw, sh, miningProgress);
    }

    // 2. First-Person Animated Hand & Tool Viewmodel
    renderFirstPersonHand(sw, sh, player, camera, totalTime);

    // If HUD is hidden via F1, stop here (unless F3 debug is active)
    if (!showHUD) {
        if (showF3) {
            renderF3DebugScreen(screenWidth, screenHeight, player, world, camera, fps, hasTargetedBlock, targetedBlockPos);
        }
        return;
    }

    // 3. Vanilla Minecraft Crosshair & Attack Meter
    float atkProg = player.getAttackRechargeProgress();
    renderMinecraftCrosshair(cx, cy, atkProg, totalTime);

    // Palworld Sphere Aiming Reticle (If actively aiming)
    if (aimCreature && !aimCreature->getIsTamed() && aimCatchChance > 0.0f) {
        int pct = std::clamp(static_cast<int>(aimCatchChance * 100.0f), 1, 99);
        std::string chanceStr = "Catch: " + std::to_string(pct) + "% (" + aimCreature->getDef().name + ")";
        ui->drawRect(cx - 70.0f, cy + 24.0f, 140.0f, 20.0f, {0.0f, 0.0f, 0.0f, 0.75f});
        ui->drawTextCentered(chanceStr, cx, cy + 28.0f, 1.15f, {0.25f, 0.95f, 0.40f, 1.0f});
    }

    // 4. Vanilla Minecraft Hotbar
    renderMinecraftHotbar(cx, sh, player, dt);

    // 5. Vanilla Minecraft XP Bar & Level
    float slotSize = 40.0f;
    float slotGap = 2.0f;
    float hotbarW = slotSize * 9.0f + slotGap * 8.0f; // 376px
    float hotbarX = cx - hotbarW * 0.5f;
    float hotbarY = sh - slotSize - 12.0f;

    float xpY = hotbarY - 8.0f;
    float xpFrac = static_cast<float>(player.getXP()) / static_cast<float>(player.getXPToNext());
    renderMinecraftXPBar(hotbarX, xpY, hotbarW, xpFrac, player.getLevel());

    // 6. Vanilla Minecraft Hearts (Health) & Drumsticks (Hunger/Stamina)
    float statsY = xpY - 18.0f;

    // Hearts over slots 1-5
    renderMinecraftHearts(hotbarX + 2.0f, statsY, player.getHealth(), player.getMaxHealth(), totalTime);

    // Drumsticks over slots 5-9
    renderMinecraftDrumsticks(hotbarX + 218.0f, statsY, player.getStamina(), player.getMaxStamina(), totalTime);

    // 7. Armor Chestplate Icons (above Hearts if defense > 0)
    if (player.getDefense() > 0.0f) {
        renderMinecraftArmor(hotbarX + 2.0f, statsY - 14.0f, player.getDefense());
    }

    // 8. Underwater Air Bubbles (above Drumsticks if underwater)
    if (player.getIsUnderwater()) {
        renderMinecraftBubbles(hotbarX + 218.0f, statsY - 14.0f, 0.85f, totalTime);
    }

    // 9. Toast Notifications (Bottom-Right popups)
    float notifY = sh - 140.0f;
    for (auto it = notifications.begin(); it != notifications.end();) {
        it->timer -= dt;
        if (it->timer <= 0.0f) {
            it = notifications.erase(it);
        } else {
            float alpha = std::min(1.0f, it->timer * 1.6f);
            float nW = it->text.length() * 6.0f * 1.25f + 20.0f;
            float nX = sw - nW - 20.0f;

            ui->drawRect(nX, notifY, nW, 22.0f, {0.0f, 0.0f, 0.0f, 0.75f * alpha});
            ui->drawRectOutline(nX, notifY, nW, 22.0f, 1.0f, {it->color.x, it->color.y, it->color.z, 0.85f * alpha});
            ui->drawText(it->text, nX + 10.0f, notifY + 5.0f, 1.25f, {it->color.x, it->color.y, it->color.z, alpha});

            notifY -= 26.0f;
            ++it;
        }
    }

    // 10. Minecraft F3 Debug Screen (Toggled with F3)
    if (showF3) {
        renderF3DebugScreen(screenWidth, screenHeight, player, world, camera, fps, hasTargetedBlock, targetedBlockPos);
    }
}

} // namespace Aetheria
