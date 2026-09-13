#pragma once

#include "UIRenderer.hpp"
#include "../skills/SkillTree.hpp"
#include "../entities/Player.hpp"
#include <string>
#include <functional>

namespace Aetheria {

class SkillTreeUI {
public:
    SkillTreeUI();

    void render(UIRenderer* ui, int screenWidth, int screenHeight,
                Player& player, AudioEngine* audio,
                int mouseX, int mouseY, bool mouseLeftClicked,
                float totalTime);

    bool isOpen() const { return open; }
    void setOpen(bool o) { open = o; }
    void toggle() { open = !open; }

private:
    bool open = false;

    bool drawButton(UIRenderer* ui, float x, float y, float w, float h,
                    const std::string& text, bool primary,
                    int mouseX, int mouseY, bool clicked);

    void drawSkillCard(UIRenderer* ui, float x, float y, float w, float h,
                       SkillNode& skill, uint32_t& availablePoints, AudioEngine* audio,
                       int mouseX, int mouseY, bool clicked);
};

} // namespace Aetheria
