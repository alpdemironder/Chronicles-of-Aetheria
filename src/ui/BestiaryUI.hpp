#pragma once

#include "UIRenderer.hpp"
#include "../entities/CreatureRegistry.hpp"

namespace Aetheria {

class Window;

class BestiaryUI {
public:
    BestiaryUI(UIRenderer* ui);

    void toggle() { isOpen = !isOpen; }
    void open() { isOpen = true; }
    void close() { isOpen = false; }
    bool getIsOpen() const { return isOpen; }

    void handleInput(const Window& window);
    void render(int screenWidth, int screenHeight);

private:
    UIRenderer* ui = nullptr;
    bool isOpen = false;
    int selectedIndex = 0;
};

} // namespace Aetheria
