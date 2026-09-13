#include "BiomeRegistry.hpp"
#include <iostream>
#include <cmath>
#include <limits>

namespace Aetheria {

std::vector<BiomeDef> BiomeRegistry::biomes;
bool BiomeRegistry::initialized = false;

void BiomeRegistry::registerBiome(const BiomeDef& def) {
    if (def.id >= biomes.size()) {
        biomes.resize(def.id + 1);
    }
    biomes[def.id] = def;
}

const BiomeDef& BiomeRegistry::get(uint8_t id) {
    if (!initialized) init();
    if (id < biomes.size() && biomes[id].id == id) {
        return biomes[id];
    }
    return biomes[1]; // Fallback to Verdant Plains
}

const std::vector<BiomeDef>& BiomeRegistry::getAll() {
    if (!initialized) init();
    return biomes;
}

size_t BiomeRegistry::getCount() {
    if (!initialized) init();
    return biomes.size() > 0 ? biomes.size() - 1 : 0;
}

uint8_t BiomeRegistry::sampleBiome(float temp, float moisture, float elevation) {
    if (!initialized) init();

    uint8_t bestId = 1;
    float bestDist = std::numeric_limits<float>::max();

    // Find nearest climate point among the 35 biomes
    for (size_t i = 1; i < biomes.size(); ++i) {
        const auto& b = biomes[i];
        float dt = temp - b.temperature;
        float dm = moisture - b.moisture;
        float de = (elevation - b.baseElevation) / 64.0f;

        float dist = dt * dt * 1.5f + dm * dm * 1.5f + de * de * 0.8f;
        if (dist < bestDist) {
            bestDist = dist;
            bestId = static_cast<uint8_t>(i);
        }
    }

    return bestId;
}

void BiomeRegistry::init() {
    if (initialized) return;
    initialized = true;

    biomes.clear();
    biomes.resize(36);

    // 1. Verdant Plains
    registerBiome({ 1, "Verdant Plains", "Rolling lush green hills and tranquil wildflower fields",
        0.2f, 0.3f, 32.0f, 6.0f, 1, 2, 36, 71, 73, 0.03f, 0.0f,
        {0.45f, 0.65f, 0.95f, 1.0f}, {0.70f, 0.82f, 0.95f, 1.0f}, 95.0f });

    // 2. Ancient Oak Forest
    registerBiome({ 2, "Ancient Oak Forest", "Vast dense canopy of ancient oaks and mossy logs",
        0.1f, 0.5f, 34.0f, 8.0f, 1, 2, 36, 71, 73, 0.15f, 0.0f,
        {0.40f, 0.60f, 0.90f, 1.0f}, {0.60f, 0.75f, 0.85f, 1.0f}, 80.0f });

    // 3. Golden Autumn Grove
    registerBiome({ 3, "Golden Autumn Grove", "Rich foliage of golden yellow and fiery orange maples",
        0.0f, 0.4f, 33.0f, 7.0f, 4, 2, 36, 98, 100, 0.12f, 0.0f,
        {0.50f, 0.62f, 0.88f, 1.0f}, {0.85f, 0.72f, 0.60f, 1.0f}, 85.0f });

    // 4. Silver Birch Woods
    registerBiome({ 4, "Silver Birch Woods", "Tranquil open forest of slender silver birch trees",
        0.0f, 0.2f, 32.0f, 6.0f, 1, 2, 36, 74, 76, 0.10f, 0.0f,
        {0.48f, 0.68f, 0.95f, 1.0f}, {0.75f, 0.85f, 0.95f, 1.0f}, 90.0f });

    // 5. Snowy Pine Taiga
    registerBiome({ 5, "Snowy Pine Taiga", "Cold conifer forests blanketed in deep glistening snow",
        -0.6f, 0.4f, 38.0f, 10.0f, 16, 2, 36, 80, 82, 0.12f, 0.0f,
        {0.60f, 0.72f, 0.92f, 1.0f}, {0.80f, 0.88f, 0.98f, 1.0f}, 70.0f });

    // 6. Frostbite Tundra
    registerBiome({ 6, "Frostbite Tundra", "Barren sub-zero plains with icy permafrost and biting winds",
        -0.9f, -0.2f, 30.0f, 4.0f, 16, 15, 36, 80, 82, 0.01f, 0.0f,
        {0.65f, 0.75f, 0.95f, 1.0f}, {0.85f, 0.92f, 1.00f, 1.0f}, 60.0f });

    // 7. Glacial Ice Spire Peaks
    registerBiome({ 7, "Glacial Ice Spire Peaks", "Soaring jagged peaks of eternal packed and blue ice",
        -1.0f, 0.1f, 52.0f, 24.0f, 18, 19, 36, 0, 0, 0.0f, 0.08f,
        {0.50f, 0.65f, 0.95f, 1.0f}, {0.70f, 0.85f, 1.00f, 1.0f}, 50.0f });

    // 8. Arid Sunbaked Desert
    registerBiome({ 8, "Arid Sunbaked Desert", "Endless rolling golden sand dunes under a relentless sun",
        0.9f, -0.9f, 31.0f, 5.0f, 11, 54, 54, 0, 0, 0.0f, 0.0f,
        {0.65f, 0.75f, 0.95f, 1.0f}, {0.90f, 0.85f, 0.70f, 1.0f}, 110.0f });

    // 9. Redrock Canyon & Mesas
    registerBiome({ 9, "Redrock Canyon & Mesas", "Spectacular stratified canyons and terracotta plateaus",
        0.8f, -0.6f, 42.0f, 18.0f, 12, 56, 56, 0, 0, 0.0f, 0.0f,
        {0.60f, 0.65f, 0.90f, 1.0f}, {0.85f, 0.60f, 0.45f, 1.0f}, 95.0f });

    // 10. Golden Savannah
    registerBiome({ 10, "Golden Savannah", "Expansive sunlit grasslands dotted with flat-topped acacias",
        0.7f, -0.3f, 33.0f, 5.0f, 1, 2, 36, 86, 88, 0.04f, 0.0f,
        {0.55f, 0.70f, 0.95f, 1.0f}, {0.85f, 0.80f, 0.65f, 1.0f}, 100.0f });

    // 11. Lush Tropical Jungle
    registerBiome({ 11, "Lush Tropical Jungle", "Immense ancient mahogany trees and hanging canopies",
        0.8f, 0.9f, 36.0f, 14.0f, 25, 2, 36, 83, 85, 0.25f, 0.0f,
        {0.35f, 0.65f, 0.85f, 1.0f}, {0.55f, 0.80f, 0.70f, 1.0f}, 70.0f });

    // 12. Tranquil Bamboo Basin
    registerBiome({ 12, "Tranquil Bamboo Basin", "Dense serene groves of towering golden-green bamboo stalks",
        0.4f, 0.6f, 32.0f, 7.0f, 1, 2, 36, 110, 112, 0.20f, 0.0f,
        {0.42f, 0.68f, 0.90f, 1.0f}, {0.65f, 0.85f, 0.75f, 1.0f}, 85.0f });

    // 13. Murky Bayou Swamp
    registerBiome({ 13, "Murky Bayou Swamp", "Stagnant marshy wetlands with hanging willow moss",
        0.4f, 0.8f, 27.0f, 3.0f, 7, 7, 36, 101, 103, 0.08f, 0.0f,
        {0.35f, 0.50f, 0.60f, 1.0f}, {0.45f, 0.55f, 0.40f, 1.0f}, 60.0f });

    // 14. Mangrove Estuary
    registerBiome({ 14, "Mangrove Estuary", "Tangled prop roots and brackish waterways along delta shallows",
        0.6f, 0.85f, 28.0f, 4.0f, 7, 10, 36, 92, 94, 0.12f, 0.0f,
        {0.40f, 0.60f, 0.80f, 1.0f}, {0.50f, 0.70f, 0.60f, 1.0f}, 75.0f });

    // 15. Coral Reef Coastline
    registerBiome({ 15, "Coral Reef Coastline", "Warm azure waters with dazzling vibrant coral formations",
        0.7f, 0.5f, 26.0f, 4.0f, 27, 11, 36, 113, 115, 0.02f, 0.05f,
        {0.30f, 0.70f, 0.95f, 1.0f}, {0.60f, 0.85f, 0.95f, 1.0f}, 100.0f });

    // 16. Abyssal Ocean Trench
    registerBiome({ 16, "Abyssal Ocean Trench", "Sunken abyss where mysterious prismarine ruins reside",
        -0.2f, 1.0f, 12.0f, 6.0f, 33, 67, 45, 0, 0, 0.0f, 0.02f,
        {0.10f, 0.15f, 0.35f, 1.0f}, {0.15f, 0.25f, 0.45f, 1.0f}, 45.0f });

    // 17. Granite Highlands & Crags
    registerBiome({ 17, "Granite Highlands & Crags", "Towering barren granite monoliths and windy precipices",
        -0.3f, -0.1f, 55.0f, 22.0f, 39, 36, 36, 77, 79, 0.02f, 0.0f,
        {0.45f, 0.60f, 0.90f, 1.0f}, {0.65f, 0.75f, 0.85f, 1.0f}, 80.0f });

    // 18. Volcanic Caldera & Ashlands
    registerBiome({ 18, "Volcanic Caldera & Ashlands", "Smoldering basalt crags, rivers of magma, and ash clouds",
        1.0f, -0.5f, 40.0f, 16.0f, 50, 20, 52, 0, 0, 0.0f, 0.05f,
        {0.30f, 0.15f, 0.15f, 1.0f}, {0.45f, 0.25f, 0.18f, 1.0f}, 55.0f });

    // 19. Giant Fungal Isle
    registerBiome({ 19, "Giant Fungal Isle", "Enchanted mycelium fields covered in towering mushroom trees",
        0.3f, 0.7f, 31.0f, 6.0f, 5, 2, 36, 104, 106, 0.15f, 0.04f,
        {0.45f, 0.40f, 0.65f, 1.0f}, {0.65f, 0.55f, 0.75f, 1.0f}, 75.0f });

    // 20. Cherry Blossom Sanctuary
    registerBiome({ 20, "Cherry Blossom Sanctuary", "Cascading showers of vibrant pink blossom petals",
        0.2f, 0.4f, 35.0f, 9.0f, 1, 2, 36, 95, 97, 0.14f, 0.0f,
        {0.60f, 0.70f, 0.95f, 1.0f}, {0.95f, 0.80f, 0.88f, 1.0f}, 90.0f });

    // 21. Mystical Enchanted Forest
    registerBiome({ 21, "Mystical Enchanted Forest", "Bioluminescent flora, crystal pools, and fairy trees",
        0.1f, 0.7f, 34.0f, 8.0f, 25, 2, 36, 116, 118, 0.16f, 0.10f,
        {0.25f, 0.45f, 0.85f, 1.0f}, {0.55f, 0.40f, 0.85f, 1.0f}, 70.0f });

    // 22. Necrotic Cursed Mire
    registerBiome({ 22, "Necrotic Cursed Mire", "Blighted soil where specters roam amid weeping thorns",
        -0.1f, 0.3f, 29.0f, 5.0f, 22, 23, 45, 107, 109, 0.05f, 0.02f,
        {0.20f, 0.15f, 0.25f, 1.0f}, {0.35f, 0.25f, 0.40f, 1.0f}, 50.0f });

    // 23. Amethyst Geode Valley
    registerBiome({ 23, "Amethyst Geode Valley", "Open crystalline chasms crowned with giant amethyst clusters",
        0.0f, 0.0f, 32.0f, 12.0f, 48, 47, 45, 0, 0, 0.0f, 0.20f,
        {0.35f, 0.30f, 0.65f, 1.0f}, {0.60f, 0.45f, 0.85f, 1.0f}, 75.0f });

    // 24. Scorched Wasteland
    registerBiome({ 24, "Scorched Wasteland", "Devastated radioactive ground riddled with glowing cracks",
        0.9f, -0.8f, 33.0f, 6.0f, 21, 20, 61, 0, 0, 0.0f, 0.03f,
        {0.40f, 0.30f, 0.20f, 1.0f}, {0.60f, 0.45f, 0.25f, 1.0f}, 65.0f });

    // 25. Frostfire Glacier
    registerBiome({ 25, "Frostfire Glacier", "Blue fire dancing eternally across crystalline glacier ice",
        -0.8f, 0.6f, 48.0f, 18.0f, 19, 18, 53, 0, 0, 0.0f, 0.12f,
        {0.40f, 0.65f, 0.98f, 1.0f}, {0.55f, 0.80f, 1.00f, 1.0f}, 60.0f });

    // 26. Petrified Stone Forest
    registerBiome({ 26, "Petrified Stone Forest", "Ancient trees turned to indestructible fossilized agate",
        0.3f, -0.2f, 35.0f, 8.0f, 3, 36, 45, 107, 109, 0.08f, 0.05f,
        {0.50f, 0.55f, 0.70f, 1.0f}, {0.70f, 0.68f, 0.72f, 1.0f}, 85.0f });

    // 27. Desert Oasis
    registerBiome({ 27, "Desert Oasis", "Lush emerald spring and date palms in the heart of dunes",
        0.85f, -0.4f, 29.0f, 4.0f, 1, 11, 54, 113, 115, 0.08f, 0.0f,
        {0.55f, 0.75f, 0.98f, 1.0f}, {0.80f, 0.85f, 0.75f, 1.0f}, 95.0f });

    // 28. Floating Celestial Isles
    registerBiome({ 28, "Floating Celestial Isles", "Gravity-defying islands high in the cloud realm",
        0.0f, 0.1f, 58.0f, 20.0f, 35, 63, 63, 116, 118, 0.06f, 0.15f,
        {0.65f, 0.80f, 1.00f, 1.0f}, {0.85f, 0.92f, 1.00f, 1.0f}, 110.0f });

    // 29. Corrupted Void Crater
    registerBiome({ 29, "Corrupted Void Crater", "Reality tearing apart into bottomless pitch-black singularity",
        -0.5f, -0.7f, 25.0f, 14.0f, 62, 62, 62, 0, 0, 0.0f, 0.10f,
        {0.08f, 0.02f, 0.15f, 1.0f}, {0.18f, 0.05f, 0.28f, 1.0f}, 45.0f });

    // 30. Windblown Steppes
    registerBiome({ 30, "Windblown Steppes", "High wide grasslands with rushing mountain breezes",
        -0.1f, -0.4f, 40.0f, 9.0f, 1, 2, 36, 77, 79, 0.02f, 0.0f,
        {0.50f, 0.65f, 0.95f, 1.0f}, {0.75f, 0.80f, 0.90f, 1.0f}, 105.0f });

    // 31. Subterranean Crystal Cavern
    registerBiome({ 31, "Subterranean Crystal Cavern", "Immense underground vault illuminated by luminous gems",
        0.1f, 0.2f, 20.0f, 8.0f, 45, 45, 45, 0, 0, 0.0f, 0.25f,
        {0.15f, 0.25f, 0.40f, 1.0f}, {0.35f, 0.50f, 0.70f, 1.0f}, 50.0f });

    // 32. Deep Molten Core
    registerBiome({ 32, "Deep Molten Core", "Underground ocean of lava with floating obsidian shelves",
        1.0f, 0.0f, 15.0f, 10.0f, 52, 50, 61, 0, 0, 0.0f, 0.15f,
        {0.25f, 0.05f, 0.05f, 1.0f}, {0.55f, 0.18f, 0.08f, 1.0f}, 40.0f });

    // 33. Sunken Atlantis Ruins
    registerBiome({ 33, "Sunken Atlantis Ruins", "Submerged marble palaces and overgrown sea lantern columns",
        0.3f, 0.95f, 18.0f, 6.0f, 69, 67, 57, 0, 0, 0.0f, 0.08f,
        {0.15f, 0.45f, 0.65f, 1.0f}, {0.25f, 0.60f, 0.75f, 1.0f}, 60.0f });

    // 34. Alpine Wildflower Meadow
    registerBiome({ 34, "Alpine Wildflower Meadow", "High mountain meadow carpeted in thousands of blue blossoms",
        -0.4f, 0.2f, 46.0f, 12.0f, 1, 2, 36, 77, 79, 0.05f, 0.0f,
        {0.55f, 0.72f, 1.00f, 1.0f}, {0.78f, 0.88f, 1.00f, 1.0f}, 95.0f });

    // 35. Astral Zenith Sanctuary
    registerBiome({ 35, "Astral Zenith Sanctuary", "Sacred plateau illuminated by shimmering celestial auroras",
        0.0f, 0.0f, 50.0f, 10.0f, 35, 63, 63, 116, 118, 0.08f, 0.30f,
        {0.12f, 0.18f, 0.45f, 1.0f}, {0.35f, 0.50f, 0.85f, 1.0f}, 90.0f });

    std::cout << "Registered all " << getCount() << " unique biomes into BiomeRegistry!" << std::endl;
}

} // namespace Aetheria
