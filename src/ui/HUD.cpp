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

namespace Aetheria {

HUD::HUD(UIRenderer* ui) : ui(ui) {
}

void HUD::addNotification(const std::string& text, const Vec4& color) {
    notifications.push_back({text, color, 3.2f});
    if (notifications.size() > 5) {
        notifications.erase(notifications.begin());
    }
}

void HUD::render(int screenWidth, int screenHeight,
                const Player& player,
                const World& world,
                const BuildingManager& buildingMgr,
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

    // =========================================================================
    // 1. CLEAN MODERN CROSSHAIR & MINECRAFT-STYLE MINING RETICLE
    // =========================================================================
    float prog = std::clamp(miningProgress, 0.0f, 1.0f);
    float jitterX = (prog > 0.70f) ? (std::sin(totalTime * 50.0f) * 1.5f) : 0.0f;
    float jitterY = (prog > 0.70f) ? (std::cos(totalTime * 55.0f) * 1.5f) : 0.0f;

    if (isZooming) {
        // Telescopic Zoom Overlay
        ui->drawRect(0, 0, sw, 30.0f, {0.0f, 0.0f, 0.0f, 0.55f});
        ui->drawRect(0, sh - 30.0f, sw, 30.0f, {0.0f, 0.0f, 0.0f, 0.55f});

        // Fine sniper crosshair
        Vec4 retColor(0.2f, 0.9f, 1.0f, 0.85f);
        ui->drawLine(cx - 24.0f, cy, cx - 6.0f, cy, 1.2f, retColor);
        ui->drawLine(cx + 6.0f, cy, cx + 24.0f, cy, 1.2f, retColor);
        ui->drawLine(cx, cy - 24.0f, cx, cy - 6.0f, 1.2f, retColor);
        ui->drawLine(cx, cy + 6.0f, cx, cy + 24.0f, 1.2f, retColor);
        ui->drawRect(cx - 1.0f, cy - 1.0f, 2.0f, 2.0f, {1.0f, 1.0f, 1.0f, 0.95f});
        ui->drawTextCentered("[ ZOOM 3.1x ]", cx, cy + 32.0f, 1.3f, {0.3f, 0.95f, 1.0f, 0.95f});
    } else {
        // Minimalist Clean Crosshair (Crisp lines with center gap)
        Vec4 chColor(1.0f, 1.0f, 1.0f, 0.80f);
        if (prog > 0.0f) {
            chColor = {1.0f, 0.75f - prog * 0.35f, 0.25f, 0.95f}; // Warms to fiery orange during mining
        }
        ui->drawRect(cx + jitterX - 7.0f, cy + jitterY - 0.5f, 5.0f, 1.0f, chColor);
        ui->drawRect(cx + jitterX + 3.0f, cy + jitterY - 0.5f, 5.0f, 1.0f, chColor);
        ui->drawRect(cx + jitterX - 0.5f, cy + jitterY - 7.0f, 1.0f, 5.0f, chColor);
        ui->drawRect(cx + jitterX - 0.5f, cy + jitterY + 3.0f, 1.0f, 5.0f, chColor);
        ui->drawRect(cx + jitterX - 0.5f, cy + jitterY - 0.5f, 1.0f, 1.0f, {1.0f, 1.0f, 1.0f, 0.95f});

        // ---------------------------------------------------------------------
        // MINECRAFT-STYLE MINING CRACK STAGES & SEGMENTED RADIAL RETICLE
        // ---------------------------------------------------------------------
        if (prog > 0.0f) {
            // 10 Radial Crack Stages around crosshair
            for (int i = 0; i < 10; ++i) {
                float angle = -1.57079f + (static_cast<float>(i) / 10.0f) * 6.28318f;
                float r1 = 13.0f;
                float r2 = 18.0f;
                float x1 = cx + jitterX + std::cos(angle) * r1;
                float y1 = cy + jitterY + std::sin(angle) * r1;
                float x2 = cx + jitterX + std::cos(angle) * r2;
                float y2 = cy + jitterY + std::sin(angle) * r2;

                float segFrac = (i + 1) / 10.0f;
                Vec4 segColor;
                if (prog >= segFrac) {
                    segColor = {1.0f, 0.85f - prog * 0.55f, 0.15f, 0.95f}; // Radiant yellow -> fiery orange -> bright red
                } else {
                    segColor = {0.20f, 0.25f, 0.32f, 0.40f}; // Inactive slot
                }
                ui->drawLine(x1, y1, x2, y2, 2.0f, segColor);
            }

            // Outer fracture stress diamond / box
            float stressSz = 8.0f + prog * 10.0f;
            ui->drawRectOutline(cx + jitterX - stressSz * 0.5f, cy + jitterY - stressSz * 0.5f, stressSz, stressSz, 1.0f,
                                {1.0f, 0.45f, 0.15f, prog * 0.75f});

            // Sleek Edgy Mining Status Pill below reticle
            std::string bName = (miningBlockId > 0) ? BlockRegistry::get(miningBlockId).name : "Block";
            int pct = static_cast<int>(prog * 100.0f);
            std::string mineText = bName + "  " + std::to_string(pct) + "%";

            float cardW = 120.0f;
            float cardH = 22.0f;
            float cardX = cx - cardW * 0.5f;
            float cardY = cy + 24.0f;

            ui->drawEdgyPanel(cardX, cardY, cardW, cardH, 3.0f,
                              {0.04f, 0.06f, 0.08f, 0.85f},
                              {0.95f, 0.65f, 0.20f, 0.80f}, 1.2f);
            ui->drawTextCentered(mineText, cx, cardY + 3.0f, 1.15f, {1.0f, 0.95f, 0.85f, 0.95f});

            // Mini progress bar inside panel
            ui->drawEdgyProgressBar(cardX + 6.0f, cardY + 14.0f, cardW - 12.0f, 4.0f, prog,
                                    {1.0f, 0.85f, 0.25f, 1.0f}, {0.95f, 0.30f, 0.12f, 1.0f},
                                    {0.12f, 0.14f, 0.18f, 0.90f}, 1.5f);
        } else {
            // -----------------------------------------------------------------
            // MINECRAFT 1.9+ ATTACK RECHARGE INDICATOR BELOW CROSSHAIR
            // -----------------------------------------------------------------
            float atkProg = player.getAttackRechargeProgress();
            if (atkProg < 0.99f) {
                float indW = 20.0f;
                float indH = 4.5f;
                float indX = cx - indW * 0.5f;
                float indY = cy + 15.0f;

                // Translucent dark background groove
                ui->drawRect(indX - 1.0f, indY - 1.0f, indW + 2.0f, indH + 2.0f, {0.04f, 0.06f, 0.09f, 0.85f});
                ui->drawRectOutline(indX - 1.0f, indY - 1.0f, indW + 2.0f, indH + 2.0f, 1.0f, {0.20f, 0.30f, 0.42f, 0.80f});

                // Fill bar: transitions from cool cyan-blue to radiant green when ready
                Vec4 fillCol = (atkProg >= 0.85f) ? Vec4(0.20f, 0.95f, 0.45f, 0.95f) :
                               Vec4(0.35f, 0.75f, 0.95f, 0.90f);
                ui->drawRect(indX, indY, indW * atkProg, indH, fillCol);
            }
        }

        // ---------------------------------------------------------------------
        // DYNAMIC CAPTURE PROBABILITY RETICLE (When aiming with Pal Sphere)
        // ---------------------------------------------------------------------
        if (aimCreature && !aimCreature->getIsTamed() && aimCatchChance > 0.0f) {
            float brDist = 18.0f + std::sin(totalTime * 6.0f) * 1.5f;
            Vec4 brCol = {0.0f, 0.92f, 1.0f, 0.85f};
            // 4 Targeting corner brackets around crosshair
            ui->drawLine(cx - brDist, cy - brDist, cx - brDist + 6.0f, cy - brDist, 1.5f, brCol);
            ui->drawLine(cx - brDist, cy - brDist, cx - brDist, cy - brDist + 6.0f, 1.5f, brCol);

            ui->drawLine(cx + brDist, cy - brDist, cx + brDist - 6.0f, cy - brDist, 1.5f, brCol);
            ui->drawLine(cx + brDist, cy - brDist, cx + brDist, cy - brDist + 6.0f, 1.5f, brCol);

            ui->drawLine(cx - brDist, cy + brDist, cx - brDist + 6.0f, cy + brDist, 1.5f, brCol);
            ui->drawLine(cx - brDist, cy + brDist, cx - brDist, cy + brDist - 6.0f, 1.5f, brCol);

            ui->drawLine(cx + brDist, cy + brDist, cx + brDist - 6.0f, cy + brDist, 1.5f, brCol);
            ui->drawLine(cx + brDist, cy + brDist, cx + brDist, cy + brDist - 6.0f, 1.5f, brCol);

            // Capture Chance Pill Card below crosshair
            int pct = std::clamp(static_cast<int>(aimCatchChance * 100.0f), 1, 99);
            Vec4 rateCol = (pct >= 70) ? Vec4(0.25f, 0.95f, 0.40f, 1.0f) :
                           (pct >= 40 ? Vec4(1.0f, 0.82f, 0.20f, 1.0f) : Vec4(0.95f, 0.30f, 0.30f, 1.0f));

            float cardW = 170.0f;
            float cardH = 38.0f;
            float cardX = cx - cardW * 0.5f;
            float cardY = cy + 26.0f;

            ui->drawRect(cardX, cardY, cardW, cardH, {0.04f, 0.06f, 0.09f, 0.88f});
            ui->drawRectOutline(cardX, cardY, cardW, cardH, 1.2f, rateCol);

            std::string mobInfo = aimCreature->getDef().name + "  " +
                                  std::to_string(static_cast<int>(aimCreature->getHealth() / aimCreature->getMaxHealth() * 100.0f)) + "% HP";
            ui->drawTextCentered(mobInfo, cx, cardY + 4.0f, 1.10f, {0.9f, 0.95f, 1.0f, 0.95f});

            std::string chanceStr = "CATCH CHANCE: " + std::to_string(pct) + "%";
            ui->drawTextCentered(chanceStr, cx, cardY + 16.0f, 1.20f, rateCol);

            ui->drawTextCentered("[R-Click: Throw Sphere]", cx, cardY + 27.0f, 0.95f, {0.65f, 0.85f, 1.0f, 0.85f});
        }
    }

    // =========================================================================
    // 2. SLEEK FLOATING TOP STATUS PILL (Biome, Celestial Time, Coordinates, FPS)
    // =========================================================================
    Vec3 pPos = player.getPosition();
    uint8_t biomeId = world.getBiomeAt(pPos.x, pPos.z);
    const BiomeDef& biome = BiomeRegistry::get(biomeId);

    std::string celestialTag = world.isNight() ? "[MOON " : "[SUN ";
    celestialTag += world.getFormattedTime() + " " + world.getTimePeriodName() + "]";

    std::stringstream ssTop;
    ssTop << biome.name << "  //  "
          << celestialTag << "  //  "
          << static_cast<int>(pPos.x) << ", "
          << static_cast<int>(pPos.y) << ", "
          << static_cast<int>(pPos.z) << "  //  "
          << static_cast<int>(fps) << " FPS";
    std::string topStr = ssTop.str();

    float topTextW = static_cast<float>(topStr.length()) * 6.0f * 1.30f;
    float topW = topTextW + 36.0f;
    float topH = 26.0f;
    float topX = cx - topW * 0.5f;
    float topY = 10.0f;

    // Clean translucent pill chassis
    ui->drawRect(topX, topY, topW, topH, {0.05f, 0.07f, 0.10f, 0.75f});
    ui->drawRectOutline(topX, topY, topW, topH, 1.0f, {0.20f, 0.38f, 0.55f, 0.65f});
    ui->drawTextCentered(topStr, cx, topY + 7.0f, 1.30f, {0.90f, 0.95f, 1.0f, 0.95f});

    if (buildingMgr.hasBaseCamp()) {
        float distToBase = (pPos - buildingMgr.getBaseCampCenter()).length();
        std::string baseStatus = (distToBase <= buildingMgr.getBaseCampRadius()) ? "[BASE CAMP SECURE]" : "[OUTSIDE BASE]";
        float bTextW = static_cast<float>(baseStatus.length()) * 6.0f * 1.2f;
        float bx = topX + topW + 12.0f;
        ui->drawRect(bx, topY, bTextW + 18.0f, topH, {0.05f, 0.08f, 0.12f, 0.70f});
        ui->drawRectOutline(bx, topY, bTextW + 18.0f, topH, 1.0f, {0.18f, 0.50f, 0.40f, 0.70f});
        ui->drawText(baseStatus, bx + 9.0f, topY + 7.0f, 1.20f, {0.35f, 0.95f, 0.70f, 0.95f});
    }

    // =========================================================================
    // 3. CLEAN MODERN RPG STAT DECK (Bottom-Left)
    // =========================================================================
    float deckW = 280.0f;
    float deckH = 104.0f;
    float deckX = 20.0f;
    float deckY = sh - deckH - 20.0f;

    // Translucent modern glass panel
    ui->drawRect(deckX, deckY, deckW, deckH, {0.05f, 0.07f, 0.10f, 0.78f});
    ui->drawRectOutline(deckX, deckY, deckW, deckH, 1.2f, {0.18f, 0.30f, 0.45f, 0.70f});

    // Corner accents
    ui->drawRect(deckX, deckY, 3.0f, 3.0f, {0.3f, 0.8f, 1.0f, 0.8f});
    ui->drawRect(deckX + deckW - 3.0f, deckY, 3.0f, 3.0f, {0.3f, 0.8f, 1.0f, 0.8f});
    ui->drawRect(deckX, deckY + deckH - 3.0f, 3.0f, 3.0f, {0.3f, 0.8f, 1.0f, 0.8f});
    ui->drawRect(deckX + deckW - 3.0f, deckY + deckH - 3.0f, 3.0f, 3.0f, {0.3f, 0.8f, 1.0f, 0.8f});

    float barX = deckX + 12.0f;
    float barW = 160.0f;
    float barH = 10.0f;

    // Player Name & Class Header
    std::string playerTitle = player.getName() + " [" + player.getCharacterClass() + "]";
    ui->drawText(playerTitle, barX, deckY + 8.0f, 1.25f, {1.0f, 0.88f, 0.35f, 1.0f});

    // Health Bar
    float hpY = deckY + 26.0f;
    ui->drawEdgyProgressBar(barX, hpY, barW, barH,
                            player.getHealth() / player.getMaxHealth(),
                            {0.95f, 0.18f, 0.25f, 1.0f}, {1.0f, 0.50f, 0.15f, 1.0f},
                            {0.14f, 0.05f, 0.07f, 0.85f}, 2.0f);
    std::stringstream ssHP;
    ssHP << "HP " << static_cast<int>(player.getHealth());
    if (player.getDefense() > 0.0f) {
        ssHP << " [+" << static_cast<int>(player.getDefense()) << "]";
    }
    ui->drawText(ssHP.str(), barX + barW + 8.0f, hpY + 1.0f, 1.15f, {1.0f, 0.88f, 0.65f, 1.0f});

    // Mana Bar
    float mpY = hpY + 17.0f;
    ui->drawEdgyProgressBar(barX, mpY, barW, barH,
                            player.getMana() / player.getMaxMana(),
                            {0.0f, 0.88f, 1.0f, 1.0f}, {0.15f, 0.45f, 0.95f, 1.0f},
                            {0.05f, 0.10f, 0.18f, 0.85f}, 2.0f);
    std::stringstream ssMP;
    ssMP << "MP " << static_cast<int>(player.getMana());
    ui->drawText(ssMP.str(), barX + barW + 8.0f, mpY + 1.0f, 1.15f, {0.4f, 0.90f, 1.0f, 1.0f});

    // Stamina Bar
    float staY = mpY + 17.0f;
    ui->drawEdgyProgressBar(barX, staY, barW, barH,
                            player.getStamina() / player.getMaxStamina(),
                            {0.30f, 0.95f, 0.40f, 1.0f}, {0.12f, 0.68f, 0.28f, 1.0f},
                            {0.05f, 0.15f, 0.08f, 0.85f}, 2.0f);
    std::stringstream ssSTA;
    ssSTA << "STA " << static_cast<int>(player.getStamina());
    ui->drawText(ssSTA.str(), barX + barW + 8.0f, staY + 1.0f, 1.15f, {0.4f, 1.0f, 0.6f, 1.0f});

    // XP / Level Bar
    float xpY = staY + 17.0f;
    ui->drawEdgyProgressBar(barX, xpY, barW, 8.0f,
                            static_cast<float>(player.getXP()) / static_cast<float>(player.getXPToNext()),
                            {1.0f, 0.85f, 0.20f, 1.0f}, {0.95f, 0.60f, 0.10f, 1.0f},
                            {0.15f, 0.12f, 0.04f, 0.85f}, 1.5f);
    std::stringstream ssLVL;
    ssLVL << "LVL " << player.getLevel();
    ui->drawText(ssLVL.str(), barX + barW + 8.0f, xpY - 1.0f, 1.10f, {1.0f, 0.90f, 0.45f, 1.0f});

    // Movement Mode Badge & Dash Indicator
    float badgeY = deckY + deckH + 6.0f;
    float nextBadgeX = deckX;
    if (player.getIsCrouching()) {
        ui->drawRect(nextBadgeX, badgeY, 68.0f, 18.0f, {0.18f, 0.14f, 0.05f, 0.85f});
        ui->drawRectOutline(nextBadgeX, badgeY, 68.0f, 18.0f, 1.0f, {1.0f, 0.80f, 0.20f, 0.85f});
        ui->drawText("[SNEAK]", nextBadgeX + 7.0f, badgeY + 4.0f, 1.05f, {1.0f, 0.88f, 0.35f, 1.0f});
        nextBadgeX += 74.0f;
    } else if (player.getIsSprinting()) {
        ui->drawRect(nextBadgeX, badgeY, 74.0f, 18.0f, {0.05f, 0.18f, 0.08f, 0.85f});
        ui->drawRectOutline(nextBadgeX, badgeY, 74.0f, 18.0f, 1.0f, {0.25f, 0.95f, 0.45f, 0.85f});
        ui->drawText("[SPRINT]", nextBadgeX + 7.0f, badgeY + 4.0f, 1.05f, {0.35f, 1.0f, 0.55f, 1.0f});
        nextBadgeX += 80.0f;
    }

    if (player.getDashCooldown() <= 0.0f && player.getStamina() >= 20.0f) {
        ui->drawRect(nextBadgeX, badgeY, 86.0f, 18.0f, {0.05f, 0.15f, 0.22f, 0.85f});
        ui->drawRectOutline(nextBadgeX, badgeY, 86.0f, 18.0f, 1.0f, {0.25f, 0.85f, 1.0f, 0.85f});
        ui->drawText("[R] DASH", nextBadgeX + 7.0f, badgeY + 4.0f, 1.05f, {0.40f, 0.95f, 1.0f, 1.0f});
    } else if (player.getDashCooldown() > 0.0f) {
        std::stringstream ssCD;
        ssCD << "[R] " << std::fixed << std::setprecision(1) << player.getDashCooldown() << "s";
        ui->drawRect(nextBadgeX, badgeY, 76.0f, 18.0f, {0.10f, 0.10f, 0.14f, 0.75f});
        ui->drawRectOutline(nextBadgeX, badgeY, 76.0f, 18.0f, 1.0f, {0.35f, 0.45f, 0.55f, 0.65f});
        ui->drawText(ssCD.str(), nextBadgeX + 7.0f, badgeY + 4.0f, 1.05f, {0.65f, 0.75f, 0.85f, 0.9f});
    }

    // =========================================================================
    // 4. CLEAN FLOATING GLASS HOTBAR (Bottom-Center)
    // =========================================================================
    int selectedSlot = player.getSelectedHotbarIndex();
    float slotSize = 48.0f;
    float gap = 5.0f;
    float hotbarW = slotSize * 9.0f + gap * 8.0f;
    float hotbarX = cx - hotbarW * 0.5f;
    float hotbarY = sh - slotSize - 16.0f;

    // Translucent backing plate
    ui->drawRect(hotbarX - 6.0f, hotbarY - 6.0f, hotbarW + 12.0f, slotSize + 12.0f, {0.04f, 0.06f, 0.09f, 0.78f});
    ui->drawRectOutline(hotbarX - 6.0f, hotbarY - 6.0f, hotbarW + 12.0f, slotSize + 12.0f, 1.2f, {0.18f, 0.28f, 0.40f, 0.65f});

    const auto& inv = player.getInventory();

    for (int i = 0; i < 9; ++i) {
        float sx = hotbarX + i * (slotSize + gap);
        float sy = hotbarY;

        bool isSel = (i == selectedSlot);

        // Slot Tile
        Vec4 slotBg = isSel ? Vec4(0.10f, 0.24f, 0.38f, 0.90f) : Vec4(0.07f, 0.10f, 0.14f, 0.85f);
        ui->drawRect(sx, sy, slotSize, slotSize, slotBg);

        if (isSel) {
            // Glowing border & top accent cap
            ui->drawRectOutline(sx, sy, slotSize, slotSize, 2.0f, {0.0f, 0.92f, 1.0f, 0.98f});
            ui->drawRect(sx, sy, slotSize, 3.0f, {1.0f, 1.0f, 1.0f, 0.95f});
        } else {
            ui->drawRectOutline(sx, sy, slotSize, slotSize, 1.0f, {0.20f, 0.28f, 0.38f, 0.55f});
        }

        // Slot Item Icon
        const ItemStack& item = inv.getSlot(i);
        if (!item.isEmpty()) {
            ItemIconRenderer::drawItem(ui, sx + slotSize * 0.5f, sy + slotSize * 0.5f, slotSize - 6.0f, item.id);

            // Item Quantity Badge
            if (item.count > 1) {
                std::string cntStr = std::to_string(item.count);
                float cntW = cntStr.length() * 6.0f * 1.15f;
                ui->drawRect(sx + slotSize - cntW - 3.0f, sy + slotSize - 13.0f, cntW + 3.0f, 12.0f, {0.04f, 0.05f, 0.07f, 0.85f});
                ui->drawText(cntStr, sx + slotSize - cntW - 1.0f, sy + slotSize - 11.0f, 1.15f, {1.0f, 1.0f, 0.88f, 1.0f});
            }
        }

        // Slot number label top-left
        ui->drawText(std::to_string(i + 1), sx + 4.0f, sy + 3.0f, 1.0f, isSel ? Vec4(0.0f, 0.95f, 1.0f, 1.0f) : Vec4(0.5f, 0.58f, 0.7f, 0.7f));
    }

    // Floating Active Item Name Card
    const ItemStack& selItem = inv.getSlot(selectedSlot);
    if (!selItem.isEmpty()) {
        const auto& def = ItemRegistry::get(selItem.id);
        std::stringstream ssHeld;
        ssHeld << def.name << "  x" << selItem.count;
        if (def.attackDamage > 0.0f) ssHeld << "  [ATK +" << static_cast<int>(def.attackDamage) << "]";
        std::string heldStr = ssHeld.str();
        float textW = static_cast<float>(heldStr.length()) * 6.0f * 1.30f;
        float cardW = textW + 20.0f;
        float cardX = cx - cardW * 0.5f;
        float cardY = hotbarY - 26.0f;

        ui->drawRect(cardX, cardY, cardW, 20.0f, {0.04f, 0.06f, 0.09f, 0.82f});
        ui->drawRectOutline(cardX, cardY, cardW, 20.0f, 1.0f, {0.20f, 0.40f, 0.60f, 0.70f});
        ui->drawTextCentered(heldStr, cx, cardY + 5.0f, 1.30f, {1.0f, 1.0f, 0.95f, 1.0f});
    }

    // =========================================================================
    // 5. BUILD / DISMANTLE STATUS BANNER (Center Overlay when active)
    // =========================================================================
    if (buildingMgr.getIsBuilding()) {
        const auto& sDef = StructureRegistry::get(buildingMgr.getSelectedStructure());
        std::string bText = "BUILD: " + sDef.name + "  [LClick: Place | R: Rotate | X: Exit]";
        float bW = bText.length() * 6.0f * 1.30f + 24.0f;
        float bx = cx - bW * 0.5f;
        ui->drawRect(bx, 44.0f, bW, 24.0f, {0.08f, 0.22f, 0.38f, 0.88f});
        ui->drawRectOutline(bx, 44.0f, bW, 24.0f, 1.2f, {0.20f, 0.75f, 1.0f, 0.90f});
        ui->drawTextCentered(bText, cx, 50.0f, 1.30f, {1.0f, 1.0f, 1.0f, 1.0f});
    } else if (buildingMgr.getIsDismantling()) {
        std::string dText = "DISMANTLE MODE: Aim & Left-Click to Recycle  [X: Exit]";
        float dW = dText.length() * 6.0f * 1.30f + 24.0f;
        float dx = cx - dW * 0.5f;
        ui->drawRect(dx, 44.0f, dW, 24.0f, {0.40f, 0.08f, 0.12f, 0.88f});
        ui->drawRectOutline(dx, 44.0f, dW, 24.0f, 1.2f, {0.95f, 0.25f, 0.35f, 0.90f});
        ui->drawTextCentered(dText, cx, 50.0f, 1.30f, {1.0f, 1.0f, 1.0f, 1.0f});
    }

    // =========================================================================
    // 6. TOAST NOTIFICATIONS (Clean, bottom-right popups)
    // =========================================================================
    float notifY = sh - 150.0f;
    for (auto it = notifications.begin(); it != notifications.end();) {
        it->timer -= dt;
        if (it->timer <= 0.0f) {
            it = notifications.erase(it);
        } else {
            float alpha = std::min(1.0f, it->timer * 1.6f);
            float nW = it->text.length() * 6.0f * 1.25f + 20.0f;
            float nX = sw - nW - 20.0f;

            ui->drawRect(nX, notifY, nW, 22.0f, {0.04f, 0.06f, 0.09f, 0.85f * alpha});
            ui->drawRectOutline(nX, notifY, nW, 22.0f, 1.0f, {it->color.x, it->color.y, it->color.z, 0.85f * alpha});
            ui->drawText(it->text, nX + 10.0f, notifY + 5.0f, 1.25f, {it->color.x, it->color.y, it->color.z, alpha});

            notifY -= 26.0f;
            ++it;
        }
    }

    // =========================================================================
    // 7. ACTIVE COMPANION STATUS DECK (Top-Right)
    // =========================================================================
    if (activeCompanion && !activeCompanion->getIsDead()) {
        float compW = 224.0f;
        float compH = 54.0f;
        float compX = sw - compW - 20.0f;
        float compY = 44.0f;

        // Clean modern dark glass panel
        ui->drawRect(compX, compY, compW, compH, {0.04f, 0.06f, 0.09f, 0.85f});
        ui->drawRectOutline(compX, compY, compW, compH, 1.2f, {0.18f, 0.40f, 0.65f, 0.75f});

        // Corner accents
        ui->drawRect(compX, compY, 3.0f, 3.0f, {0.0f, 0.9f, 1.0f, 0.9f});
        ui->drawRect(compX + compW - 3.0f, compY, 3.0f, 3.0f, {0.0f, 0.9f, 1.0f, 0.9f});

        // Title: [PET] Wolf Lv.2
        std::string title = "[PET] " + activeCompanion->getDef().name + " Lv." + std::to_string(activeCompanion->getCompanionLevel());
        ui->drawText(title, compX + 8.0f, compY + 5.0f, 1.15f, {0.3f, 0.9f, 1.0f, 1.0f});

        // Stance tag
        std::string stText = "[" + activeCompanion->getStanceName() + "]";
        Vec4 stColor = (activeCompanion->getStance() == CompanionStance::Follow) ? Vec4(0.2f, 0.9f, 1.0f, 1.0f) :
                       ((activeCompanion->getStance() == CompanionStance::Stay) ? Vec4(1.0f, 0.8f, 0.2f, 1.0f) : Vec4(0.3f, 0.95f, 0.5f, 1.0f));
        float stW = stText.length() * 6.0f * 0.95f;
        ui->drawText(stText, compX + compW - stW - 8.0f, compY + 5.0f, 0.95f, stColor);

        // Health Bar
        float chp = activeCompanion->getHealth();
        float mhp = activeCompanion->getMaxHealth();
        ui->drawEdgyProgressBar(compX + 8.0f, compY + 20.0f, compW - 16.0f, 8.0f,
                                chp / mhp,
                                {0.2f, 0.85f, 1.0f, 1.0f}, {0.05f, 0.55f, 0.95f, 1.0f},
                                {0.06f, 0.12f, 0.18f, 0.85f}, 1.5f);

        std::string hpStr = std::to_string(static_cast<int>(chp)) + " / " + std::to_string(static_cast<int>(mhp)) + " HP";
        ui->drawText(hpStr, compX + 8.0f, compY + 34.0f, 0.95f, {0.75f, 0.85f, 0.95f, 0.9f});

        ui->drawText("[V] Stance", compX + compW - 68.0f, compY + 34.0f, 0.95f, {1.0f, 0.9f, 0.4f, 0.95f});
    }
}

} // namespace Aetheria
