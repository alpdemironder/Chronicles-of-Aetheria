#pragma once

#include <cstdint>
#include <string>
#include "../core/Math.hpp"

namespace Aetheria {

struct BiomeDef {
    uint8_t id = 0;
    std::string name = "Unknown";
    std::string description = "";
    float temperature = 0.5f;   // -1.0 (freezing) to +1.0 (scorching)
    float moisture = 0.5f;      // -1.0 (arid desert) to +1.0 (rainforest/ocean)
    float baseElevation = 32.0f;
    float roughness = 8.0f;
    uint16_t surfaceBlock = 1;     // e.g. Grass
    uint16_t subSurfaceBlock = 2;  // e.g. Dirt
    uint16_t deepStoneBlock = 36;  // e.g. Stone
    uint16_t logBlock = 71;        // Oak Log
    uint16_t leafBlock = 73;       // Oak Leaves
    float treeDensity = 0.05f;
    float crystalDensity = 0.0f;
    Vec4 skyColor{0.45f, 0.65f, 0.95f, 1.0f};
    Vec4 fogColor{0.70f, 0.82f, 0.95f, 1.0f};
    float fogDistance = 90.0f;
};

} // namespace Aetheria
