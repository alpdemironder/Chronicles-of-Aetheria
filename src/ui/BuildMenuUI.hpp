#pragma once

#include "UIRenderer.hpp"
#include "../building/StructurePiece.hpp"
#include <vector>

namespace Aetheria {

class Window;
class BuildingManager;

class BuildMenuUI {
public:
    BuildMenuUI(UIRenderer* ui);

    void toggle() { isOpen = !isOpen; }
    void open() { isOpen = true; }
    void close() { isOpen = false; }
    bool getIsOpen() const { return isOpen; }

    void handleInput(const Window& window, BuildingManager& buildingMgr);
    void render(int screenWidth, int screenHeight, const BuildingManager& buildingMgr);

private:
    UIRenderer* ui = nullptr;
    bool isOpen = false;
    int currentCategory = 0;
    int selectedIndex = 0;
};

} // namespace Aetheria
