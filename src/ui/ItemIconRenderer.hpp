#pragma once

#include "UIRenderer.hpp"
#include "../inventory/ItemStack.hpp"
#include <cstdint>

namespace Aetheria {

class ItemIconRenderer {
public:
    // Unified item & 3D block rendering entrypoint
    static void drawItem(UIRenderer* ui, float centerX, float centerY, float size, uint16_t itemId);

    static void drawItemStack(UIRenderer* ui, float centerX, float centerY, float size, const ItemStack& stack) {
        if (!stack.isEmpty()) {
            drawItem(ui, centerX, centerY, size, stack.id);
        }
    }

    // Authentic 3D Isometric Voxel Block Preview
    static void drawIsometricBlock(UIRenderer* ui, float cx, float cy, float size, uint16_t blockId);

    // Fresh Stylized RPG Item Icon Renderers
    static void drawSword(UIRenderer* ui, float cx, float cy, float size, int tier);
    static void drawPickaxe(UIRenderer* ui, float cx, float cy, float size, int tier);
    static void drawAxe(UIRenderer* ui, float cx, float cy, float size, int tier);
    static void drawShovel(UIRenderer* ui, float cx, float cy, float size, int tier);
    static void drawBow(UIRenderer* ui, float cx, float cy, float size);
    static void drawArrow(UIRenderer* ui, float cx, float cy, float size);

    static void drawArmorHelmet(UIRenderer* ui, float cx, float cy, float size, const Vec4& color, int tier);
    static void drawArmorChest(UIRenderer* ui, float cx, float cy, float size, const Vec4& color, int tier);
    static void drawArmorLegs(UIRenderer* ui, float cx, float cy, float size, const Vec4& color, int tier);
    static void drawArmorBoots(UIRenderer* ui, float cx, float cy, float size, const Vec4& color, int tier);

    static void drawIngot(UIRenderer* ui, float cx, float cy, float size, const Vec4& color);
    static void drawPotion(UIRenderer* ui, float cx, float cy, float size, const Vec4& liquidCol);
    static void drawFoodMeat(UIRenderer* ui, float cx, float cy, float size, bool cooked);
    static void drawBread(UIRenderer* ui, float cx, float cy, float size);
    static void drawApple(UIRenderer* ui, float cx, float cy, float size, bool golden);
    static void drawPalSphere(UIRenderer* ui, float cx, float cy, float size, uint16_t sphereId = 523);
    static void drawTorch(UIRenderer* ui, float cx, float cy, float size);
    static void drawMineral(UIRenderer* ui, float cx, float cy, float size, const Vec4& color, bool gem);
    static void drawStick(UIRenderer* ui, float cx, float cy, float size);
    static void drawTwine(UIRenderer* ui, float cx, float cy, float size, const Vec4& color);
    static void drawPelt(UIRenderer* ui, float cx, float cy, float size, const Vec4& color);
    static void drawBucket(UIRenderer* ui, float cx, float cy, float size);
    static void drawTechCore(UIRenderer* ui, float cx, float cy, float size, bool ancient);
};

} // namespace Aetheria
