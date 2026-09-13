#include "SkillTreeUI.hpp"
#include "../core/Audio.hpp"
#include <cmath>
#include <algorithm>

namespace Aetheria {

SkillTreeUI::SkillTreeUI() {}

bool SkillTreeUI::drawButton(UIRenderer* ui, float x, float y, float w, float h,
                             const std::string& text, bool primary,
                             int mouseX, int mouseY, bool clicked) {
    bool hovered = (mouseX >= x && mouseX <= x + w && mouseY >= y && mouseY <= y + h);

    Vec4 bg = primary ?
        (hovered ? Vec4(0.20f, 0.65f, 0.95f, 0.95f) : Vec4(0.12f, 0.45f, 0.75f, 0.90f)) :
        (hovered ? Vec4(0.15f, 0.22f, 0.32f, 0.90f) : Vec4(0.08f, 0.12f, 0.18f, 0.85f));

    Vec4 border = primary ?
        (hovered ? Vec4(0.60f, 0.95f, 1.0f, 1.0f) : Vec4(0.35f, 0.75f, 0.95f, 0.85f)) :
        (hovered ? Vec4(0.40f, 0.60f, 0.85f, 0.85f) : Vec4(0.20f, 0.32f, 0.45f, 0.65f));

    ui->drawRect(x, y, w, h, bg);
    ui->drawRectOutline(x, y, w, h, hovered ? 1.5f : 1.0f, border);

    Vec4 textColor = primary ?
        (hovered ? Vec4(1.0f, 1.0f, 1.0f, 1.0f) : Vec4(0.92f, 0.96f, 1.0f, 0.95f)) :
        (hovered ? Vec4(1.0f, 0.92f, 0.45f, 1.0f) : Vec4(0.80f, 0.88f, 0.95f, 0.90f));

    ui->drawTextCentered(text, x + w * 0.5f, y + (h - 13.0f) * 0.5f, 1.25f, textColor);

    return hovered && clicked;
}

void SkillTreeUI::drawSkillCard(UIRenderer* ui, float x, float y, float w, float h,
                                SkillNode& skill, uint32_t& availablePoints, AudioEngine* audio,
                                int mouseX, int mouseY, bool clicked) {
    bool isMax = (skill.currentLevel >= skill.maxLevel);
    bool canAfford = (availablePoints >= static_cast<uint32_t>(skill.pointCost)) && !isMax;

    Vec4 bg = isMax ? Vec4(0.08f, 0.20f, 0.14f, 0.92f) :
              (skill.currentLevel > 0 ? Vec4(0.08f, 0.15f, 0.25f, 0.90f) : Vec4(0.05f, 0.08f, 0.12f, 0.85f));
    Vec4 border = isMax ? Vec4(0.35f, 0.95f, 0.45f, 0.85f) :
                  (skill.currentLevel > 0 ? Vec4(0.25f, 0.65f, 0.95f, 0.80f) : Vec4(0.18f, 0.28f, 0.40f, 0.60f));

    ui->drawRect(x, y, w, h, bg);
    ui->drawRectOutline(x, y, w, h, 1.0f, border);

    // Header Title & Hotkey
    std::string title = skill.name;
    Vec4 titleCol = isMax ? Vec4(0.40f, 1.0f, 0.50f, 1.0f) :
                    (skill.currentLevel > 0 ? Vec4(1.0f, 0.90f, 0.40f, 1.0f) : Vec4(0.90f, 0.95f, 1.0f, 1.0f));
    ui->drawText(title, x + 10.0f, y + 8.0f, 1.30f, titleCol);

    // Rank pill: [ 1 / 3 ]
    std::string rankStr = "[ " + std::to_string(skill.currentLevel) + " / " + std::to_string(skill.maxLevel) + " ]";
    float rankW = rankStr.length() * 6.0f * 1.15f;
    ui->drawText(rankStr, x + w - rankW - 100.0f, y + 8.0f, 1.15f, isMax ? Vec4(0.4f, 1.0f, 0.5f, 1.0f) : Vec4(0.7f, 0.85f, 1.0f, 0.9f));

    // Description & hotkey
    std::string hotkeyTag = skill.hotkey.empty() ? "" : ("Tus: " + skill.hotkey + "  |  ");
    std::string manaTag = skill.manaCost > 0 ? ("Mana: " + std::to_string(skill.manaCost) + "  |  ") : "";
    std::string descLine = hotkeyTag + manaTag + skill.desc;
    ui->drawText(descLine, x + 10.0f, y + 26.0f, 1.05f, {0.68f, 0.80f, 0.92f, 0.85f});

    // Upgrade Button on the right
    float btnW = 90.0f;
    float btnH = 26.0f;
    float btnX = x + w - btnW - 8.0f;
    float btnY = y + 10.0f;

    if (isMax) {
        ui->drawRect(btnX, btnY, btnW, btnH, {0.10f, 0.25f, 0.15f, 0.85f});
        ui->drawRectOutline(btnX, btnY, btnW, btnH, 1.0f, {0.35f, 0.90f, 0.45f, 0.7f});
        ui->drawTextCentered("MAX", btnX + btnW * 0.5f, btnY + 7.0f, 1.15f, {0.4f, 1.0f, 0.5f, 1.0f});
    } else {
        std::string btnText = "+1 PUAN";
        if (drawButton(ui, btnX, btnY, btnW, btnH, btnText, canAfford, mouseX, mouseY, clicked && canAfford)) {
            if (availablePoints >= static_cast<uint32_t>(skill.pointCost)) {
                availablePoints -= skill.pointCost;
                skill.currentLevel++;
                if (audio) {
                    audio->playSound(SoundID::LevelUp, 1.25f, 1.2f);
                }
            }
        }
    }
}

void SkillTreeUI::render(UIRenderer* ui, int screenWidth, int screenHeight,
                         Player& player, AudioEngine* audio,
                         int mouseX, int mouseY, bool mouseLeftClicked,
                         float totalTime) {
    if (!open) return;

    float sw = static_cast<float>(screenWidth);
    float sh = static_cast<float>(screenHeight);
    float cx = sw * 0.5f;
    float cy = sh * 0.5f;

    // Dark backdrop overlay
    ui->drawRect(0, 0, sw, sh, {0.02f, 0.03f, 0.05f, 0.72f});

    // Main Modal
    float panelW = 700.0f;
    float panelH = 530.0f;
    float panelX = cx - panelW * 0.5f;
    float panelY = cy - panelH * 0.5f;

    ui->drawRect(panelX, panelY, panelW, panelH, {0.04f, 0.07f, 0.12f, 0.96f});
    ui->drawRectOutline(panelX, panelY, panelW, panelH, 2.0f, {0.25f, 0.65f, 0.95f, 0.90f});
    ui->drawRectOutline(panelX + 4.0f, panelY + 4.0f, panelW - 8.0f, panelH - 8.0f, 1.0f, {0.15f, 0.35f, 0.55f, 0.50f});

    // Golden corner accents
    float cSize = 6.0f;
    Vec4 goldAccent(1.0f, 0.85f, 0.35f, 1.0f);
    ui->drawRect(panelX, panelY, cSize, cSize, goldAccent);
    ui->drawRect(panelX + panelW - cSize, panelY, cSize, cSize, goldAccent);
    ui->drawRect(panelX, panelY + panelH - cSize, cSize, cSize, goldAccent);
    ui->drawRect(panelX + panelW - cSize, panelY + panelH - cSize, cSize, cSize, goldAccent);

    // Header Title
    float topY = panelY + 14.0f;
    ui->drawTextCentered("CHRONICLES OF AETHERIA - YETENEK VE BUYU AGACI", cx, topY, 1.9f, {1.0f, 0.88f, 0.35f, 1.0f});

    // Skill Points Banner
    uint32_t sp = player.getSkillPoints();
    std::string spBanner = "KULLANILABILIR YETENEK PUANI: " + std::to_string(sp) + "  |  SEVIYE (LVL): " + std::to_string(player.getLevel());
    ui->drawTextCentered(spBanner, cx, topY + 24.0f, 1.30f, sp > 0 ? Vec4(0.35f, 0.95f, 0.45f, 1.0f) : Vec4(0.6f, 0.75f, 0.9f, 0.85f));

    // Divider line
    float divY = topY + 46.0f;
    ui->drawLine(panelX + 20.0f, divY, panelX + panelW - 20.0f, divY, 1.0f, {0.20f, 0.40f, 0.65f, 0.60f});

    // Two Columns: Left for Active Spells & Movement, Right for Passives
    float colW = (panelW - 60.0f) * 0.5f;
    float leftX = panelX + 20.0f;
    float rightX = leftX + colW + 20.0f;
    float startY = divY + 12.0f;

    // Left Column: Active Spells
    ui->drawText("AKTIF BUYULER & HAREKET:", leftX, startY, 1.35f, {0.4f, 0.85f, 1.0f, 1.0f});
    float cardY = startY + 20.0f;
    float cardH = 48.0f;
    float cardGap = 8.0f;

    auto& skills = player.getSkillTree().getSkills();
    int activeCount = 0;
    int passiveCount = 0;

    for (auto& s : skills) {
        if (s.isActiveSpell || s.id == SkillId::DoubleJump) {
            float cy = cardY + activeCount * (cardH + cardGap);
            drawSkillCard(ui, leftX, cy, colW, cardH, s, sp, audio, mouseX, mouseY, mouseLeftClicked);
            activeCount++;
        } else {
            float cy = cardY + passiveCount * (cardH + cardGap);
            drawSkillCard(ui, rightX, cy, colW, cardH, s, sp, audio, mouseX, mouseY, mouseLeftClicked);
            passiveCount++;
        }
    }

    // Right Column Header
    ui->drawText("PASIF GELISIMLER & GUCLENDIRMELER:", rightX, startY, 1.35f, {1.0f, 0.88f, 0.35f, 1.0f});

    player.setSkillPoints(sp);

    // =========================================================================
    // BOTTOM: RACE ABILITIES & PERKS
    // =========================================================================
    float bottomY = panelY + panelH - 85.0f;
    ui->drawLine(panelX + 20.0f, bottomY, panelX + panelW - 20.0f, bottomY, 1.0f, {0.20f, 0.40f, 0.65f, 0.60f});

    std::string race = player.getRace();
    std::string raceTitle = "IRK OZEL YETENEKLERI (" + race + "): ";
    std::string racePerk = "";
    Vec4 raceColor{0.8f, 0.9f, 1.0f, 1.0f};

    if (race == "Elf") {
        racePerk = "+20 Max Mana, +25% Mana Yenilenme, +15% Ok Hasari, Hizli Orman Adimlari.";
        raceColor = {0.35f, 0.95f, 0.55f, 1.0f};
    } else if (race == "Iblis" || race == "Demon") {
        racePerk = "Lav ve Ates Bagisikligi (%100 Yanma Direnci), +8 Karanlik Ates Hasari.";
        raceColor = {1.0f, 0.35f, 0.25f, 1.0f};
    } else if (race == "Vampir" || race == "Vampire") {
        racePerk = "Can Calma (Vurulan hasarin %20'si HP olarak emilir), Gece Ekstra Hasar & Hiz.";
        raceColor = {0.95f, 0.25f, 0.75f, 1.0f};
    } else if (race == "Slime") {
        racePerk = "Dusme Hasari Yok (No Fall Damage), Ziplayan Fizik (Bounce), +25% Ziplama Yuksekligi.";
        raceColor = {0.25f, 0.95f, 0.40f, 1.0f};
    } else {
        racePerk = "Dengeli Uyum: Seviye atlayinca +2 Yetenek Puani (Skill Point) kazanir.";
        raceColor = {0.4f, 0.85f, 1.0f, 1.0f};
    }

    ui->drawText(raceTitle + racePerk, panelX + 24.0f, bottomY + 12.0f, 1.15f, raceColor);

    // Close Button
    float closeW = 140.0f;
    float closeH = 34.0f;
    float closeX = cx - closeW * 0.5f;
    float closeY = panelY + panelH - closeH - 12.0f;

    if (drawButton(ui, closeX, closeY, closeW, closeH, "KAPAT [ESC / K]", false, mouseX, mouseY, mouseLeftClicked)) {
        open = false;
    }
}

} // namespace Aetheria
