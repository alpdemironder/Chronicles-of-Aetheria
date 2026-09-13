#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace Aetheria {

enum class SkillId {
    DoubleJump,
    Fireball,
    FrostNova,
    HolyHeal,
    IronSkin,
    Regeneration,
    ManaSurge,
    Swiftness,
    Berserker
};

struct SkillNode {
    SkillId id;
    std::string name;
    std::string desc;
    std::string hotkey;
    int currentLevel = 0;
    int maxLevel = 1;
    int pointCost = 1;
    int manaCost = 0;
    bool isActiveSpell = false;
};

class SkillTree {
public:
    SkillTree() {
        init();
    }

    void init() {
        skills.clear();
        // 1. Mobility
        skills.push_back({
            SkillId::DoubleJump,
            "Cift Ziplama (Double Jump)",
            "Havada ikinci kez ziplama yetenegi (15 Stamina).",
            "SPACE (Havada)",
            0, 1, 1, 0, false
        });

        // 2. Active Spells
        skills.push_back({
            SkillId::Fireball,
            "Alev Topu (Fireball)",
            "Ileriye patlayici alev topu firlatir (45 Hasar + Alan Yakma).",
            "[Q]",
            0, 3, 1, 25, true
        });

        skills.push_back({
            SkillId::FrostNova,
            "Buz Firtinasi (Frost Nova)",
            "Cevredeki canavarlari dondurur ve yavaslatir (30 Hasar).",
            "[Z]",
            0, 3, 1, 30, true
        });

        skills.push_back({
            SkillId::HolyHeal,
            "Kutsal Sifa (Holy Heal)",
            "Kutsal isikla aninda 40 HP saglik yeniler.",
            "[H]",
            0, 3, 1, 40, true
        });

        // 3. Passive Skills
        skills.push_back({
            SkillId::IronSkin,
            "Celik Deri (Iron Skin)",
            "Karaktere rank basina +4 Zirh/Defans kazandirir.",
            "PASIF",
            0, 3, 1, 0, false
        });

        skills.push_back({
            SkillId::Regeneration,
            "Hizli Iyilesme (Regen)",
            "Her 2 saniyede bir pasif olarak +1.5 Can yeniler.",
            "PASIF",
            0, 3, 1, 0, false
        });

        skills.push_back({
            SkillId::ManaSurge,
            "Mana Akisi (Mana Surge)",
            "Rank basina +30 Max Mana ve +40% Mana dolum hizi.",
            "PASIF",
            0, 3, 1, 0, false
        });

        skills.push_back({
            SkillId::Swiftness,
            "Ruzgar Adimlari (Swift)",
            "+15% Kosu ve depar hareket hizi.",
            "PASIF",
            0, 2, 1, 0, false
        });

        skills.push_back({
            SkillId::Berserker,
            "Savas Ofkesi (Berserk)",
            "Rank basina +12% Silah ve buyu saldiri hasari.",
            "PASIF",
            0, 3, 1, 0, false
        });
    }

    std::vector<SkillNode>& getSkills() { return skills; }
    const std::vector<SkillNode>& getSkills() const { return skills; }

    SkillNode* getSkill(SkillId id) {
        for (auto& s : skills) {
            if (s.id == id) return &s;
        }
        return nullptr;
    }

    const SkillNode* getSkill(SkillId id) const {
        for (const auto& s : skills) {
            if (s.id == id) return &s;
        }
        return nullptr;
    }

    bool hasSkill(SkillId id) const {
        const SkillNode* s = getSkill(id);
        return s && s->currentLevel > 0;
    }

    int getSkillLevel(SkillId id) const {
        const SkillNode* s = getSkill(id);
        return s ? s->currentLevel : 0;
    }

    bool upgradeSkill(SkillId id, uint32_t& availablePoints) {
        SkillNode* s = getSkill(id);
        if (!s) return false;
        if (s->currentLevel >= s->maxLevel) return false;
        if (availablePoints < static_cast<uint32_t>(s->pointCost)) return false;

        availablePoints -= s->pointCost;
        s->currentLevel++;
        return true;
    }

    // Passive bonuses calculations
    float getBonusDefense() const {
        return static_cast<float>(getSkillLevel(SkillId::IronSkin)) * 4.0f;
    }

    float getHealthRegenPerSecond() const {
        return static_cast<float>(getSkillLevel(SkillId::Regeneration)) * 0.75f;
    }

    float getBonusMaxMana() const {
        return static_cast<float>(getSkillLevel(SkillId::ManaSurge)) * 30.0f;
    }

    float getManaRegenMultiplier() const {
        return 1.0f + static_cast<float>(getSkillLevel(SkillId::ManaSurge)) * 0.40f;
    }

    float getSpeedMultiplier() const {
        return 1.0f + static_cast<float>(getSkillLevel(SkillId::Swiftness)) * 0.15f;
    }

    float getDamageMultiplier() const {
        return 1.0f + static_cast<float>(getSkillLevel(SkillId::Berserker)) * 0.12f;
    }

private:
    std::vector<SkillNode> skills;
};

} // namespace Aetheria
