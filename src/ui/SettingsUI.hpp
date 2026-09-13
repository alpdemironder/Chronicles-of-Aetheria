#pragma once

#include "UIRenderer.hpp"
#include "../core/Settings.hpp"
#include <functional>

namespace Aetheria {

enum class SettingsTab {
    Graphics,
    Audio,
    Controls,
    Gameplay
};

class SettingsUI {
public:
    SettingsUI();

    bool isOpen() const { return open; }
    void toggle() { open = !open; }
    void setOpen(bool state) { open = state; }
    void setOpenIrisCallback(std::function<void()> cb) { onOpenIris = cb; }
    void setQuitToTitleCallback(std::function<void()> cb) { onQuitToTitle = cb; }

    void render(UIRenderer* ui, int screenWidth, int screenHeight,
                int mouseX, int mouseY, bool mouseLeftDown, bool mouseLeftClicked);

private:
    std::function<void()> onOpenIris = nullptr;
    std::function<void()> onQuitToTitle = nullptr;
    bool open = false;
    SettingsTab currentTab = SettingsTab::Graphics;

    // Helper UI controls
    bool drawButton(UIRenderer* ui, float x, float y, float w, float h,
                    const std::string& text, bool active,
                    int mouseX, int mouseY, bool clicked);

    void drawSlider(UIRenderer* ui, float x, float y, float w, float h,
                    const std::string& label, const std::string& valText,
                    float& value, float minVal, float maxVal,
                    int mouseX, int mouseY, bool mouseDown);

    void drawToggle(UIRenderer* ui, float x, float y, float w, float h,
                    const std::string& label, bool& value,
                    int mouseX, int mouseY, bool clicked);
};

} // namespace Aetheria
