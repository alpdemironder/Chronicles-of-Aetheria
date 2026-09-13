#pragma once

#include "Biome.hpp"
#include <vector>

namespace Aetheria {

class BiomeRegistry {
public:
    static void init();
    static const BiomeDef& get(uint8_t id);
    static const std::vector<BiomeDef>& getAll();
    static uint8_t sampleBiome(float temp, float moisture, float elevation);
    static size_t getCount();

private:
    static void registerBiome(const BiomeDef& def);
    static std::vector<BiomeDef> biomes;
    static bool initialized;
};

} // namespace Aetheria
