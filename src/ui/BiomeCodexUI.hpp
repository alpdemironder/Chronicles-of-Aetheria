#pragma once

#include "UIRenderer.hpp"
#include "../world/BiomeRegistry.hpp"

namespace Aetheria {

class Window;

class BiomeCodexUI {
public:
    BiomeCodexUI(UIRenderer* ui);

    void toggle() { isOpen = !isOpen; }
    void open() { isOpen = true; }
    void close() { isOpen = false; }
    bool getIsOpen() const { return isOpen; }

    void handleInput(const Window& window);
    void render(int screenWidth, int screenHeight);

private:
    UIRenderer* ui = nullptr;
    bool isOpen = false;
    int selectedIndex = 1; // 1 to 35
};

} // namespace Aetheria
