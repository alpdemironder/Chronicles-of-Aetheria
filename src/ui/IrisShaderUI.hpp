#pragma once

#include "UIRenderer.hpp"
#include "../render/IrisShaderManager.hpp"
#include <functional>

namespace Aetheria {

class IrisShaderUI {
public:
    IrisShaderUI(IrisShaderManager* manager);

    bool isOpen() const { return open; }
    void toggle() { open = !open; }
    void setOpen(bool state) { open = state; }

    void render(UIRenderer* ui, int screenWidth, int screenHeight,
                int mouseX, int mouseY, bool mouseLeftDown, bool mouseLeftClicked);

private:
    IrisShaderManager* shaderManager = nullptr;
    bool open = false;

    // Helper UI controls
    bool drawButton(UIRenderer* ui, float x, float y, float w, float h,
                    const std::string& text, bool active,
                    int mouseX, int mouseY, bool clicked);

    void drawToggle(UIRenderer* ui, float x, float y, float w, float h,
                    const std::string& label, bool value,
                    int mouseX, int mouseY, bool clicked,
                    const std::function<void(bool)>& onToggle);
};

} // namespace Aetheria
