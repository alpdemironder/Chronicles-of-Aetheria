#pragma once

#include "UIRenderer.hpp"
#include "../world/Block.hpp"
#include <vector>

namespace Aetheria {

class Window;
class Player;

class BlockCatalogUI {
public:
    BlockCatalogUI(UIRenderer* ui);

    void toggle() { isOpen = !isOpen; }
    void open() { isOpen = true; }
    void close() { isOpen = false; }
    bool getIsOpen() const { return isOpen; }

    void handleInput(const Window& window, Player& player);
    void render(int screenWidth, int screenHeight, const Player& player);

private:
    UIRenderer* ui = nullptr;
    bool isOpen = false;
    int currentCategory = 0;
    int currentPage = 0;
    int selectedIndex = 0;
};

} // namespace Aetheria
