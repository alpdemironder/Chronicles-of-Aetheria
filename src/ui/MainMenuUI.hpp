#pragma once

#include "UIRenderer.hpp"
#include <string>
#include <functional>

namespace Aetheria {

class MainMenuUI {
public:
    MainMenuUI();

    void render(UIRenderer* ui, int screenWidth, int screenHeight,
                int mouseX, int mouseY, bool mouseLeftClicked,
                float totalTime);

    void setOnPlay(std::function<void()> cb) { onPlay = cb; }
    void setOnOpenCalendar(std::function<void()> cb) { onOpenCalendar = cb; }
    void setOnOpenShaders(std::function<void()> cb) { onOpenShaders = cb; }
    void setOnOpenSettings(std::function<void()> cb) { onOpenSettings = cb; }
    void setOnQuit(std::function<void()> cb) { onQuit = cb; }

private:
    std::function<void()> onPlay = nullptr;
    std::function<void()> onOpenCalendar = nullptr;
    std::function<void()> onOpenShaders = nullptr;
    std::function<void()> onOpenSettings = nullptr;
    std::function<void()> onQuit = nullptr;

    bool drawButton(UIRenderer* ui, float x, float y, float w, float h,
                    const std::string& text, bool primary,
                    int mouseX, int mouseY, bool clicked);
};

} // namespace Aetheria
