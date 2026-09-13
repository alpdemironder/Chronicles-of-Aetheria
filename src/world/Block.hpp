#pragma once

#include <cstdint>
#include <string>
#include "../core/Math.hpp"

namespace Aetheria {

enum class BlockCategory : uint8_t {
    Terrain,
    Stone,
    Wood,
    Ores,
    Crystals,
    Magic,
    Dungeon,
    Colored,
    Mechanical,
    Exotic
};

struct BlockDef {
    uint16_t id = 0;
    std::string name = "Air";
    BlockCategory category = BlockCategory::Terrain;
    float hardness = 1.0f;
    uint8_t toolRequired = 0; // 0=None, 1=Pickaxe, 2=Axe, 3=Shovel, 4=Wand
    uint8_t lightEmission = 0;
    bool isTransparent = false;
    bool isSolid = true;
    Vec4 color{1.0f, 1.0f, 1.0f, 1.0f}; // RGBA base color
    uint16_t dropItemId = 0;
    uint8_t dropCount = 1;
    bool isLiquid = false;
};

} // namespace Aetheria
