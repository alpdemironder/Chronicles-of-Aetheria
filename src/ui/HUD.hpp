#pragma once

#include "UIRenderer.hpp"
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
    void toggleHUD() { showHUD = !showHUD; }
    bool isHUDVisible() const { return showHUD; }
    bool isF3Visible() const { return showF3; }

    void render(int screenWidth, int screenHeight,
                const Player& player,
                const World& world,
                const BuildingManager& buildingMgr,
                float fps, float totalTime, float dt = 0.016f,
                bool isZooming = false,
                float miningProgress = 0.0f,
                uint16_t miningBlockId = 0,
                const Creature* aimCreature = nullptr,
                float aimCatchChance = 0.0f,
                const Creature* activeCompanion = nullptr);

private:
    UIRenderer* ui = nullptr;
    std::vector<HUDNotification> notifications;
    bool showHUD = true;
    bool showF3 = false;
};

} // namespace Aetheria
