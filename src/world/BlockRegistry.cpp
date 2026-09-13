#include "BlockRegistry.hpp"
#include <iostream>

namespace Aetheria {

std::vector<BlockDef> BlockRegistry::blocks;
bool BlockRegistry::initialized = false;

void BlockRegistry::registerBlock(const BlockDef& def) {
    if (def.id >= blocks.size()) {
        blocks.resize(def.id + 1);
    }
    blocks[def.id] = def;
}

const BlockDef& BlockRegistry::get(uint16_t id) {
    if (!initialized) init();
    if (id < blocks.size()) {
        return blocks[id];
    }
    return blocks[0]; // Air
}

const std::vector<BlockDef>& BlockRegistry::getAll() {
    if (!initialized) init();
    return blocks;
}

size_t BlockRegistry::getCount() {
    if (!initialized) init();
    return blocks.size() > 0 ? blocks.size() - 1 : 0;
}

void BlockRegistry::init() {
    if (initialized) return;
    initialized = true;

    blocks.clear();
    blocks.resize(370);

    // ID 0: Air
    blocks[0] = { 0, "Air", BlockCategory::Terrain, 0.0f, 0, 0, true, false, {0,0,0,0}, 0, 0 };

    // --- 1 to 35: Terrain & Soils ---
    registerBlock({ 1, "Grass Block", BlockCategory::Terrain, 0.6f, 3, 0, false, true, {0.35f, 0.72f, 0.25f, 1.0f}, 2, 1 });
    registerBlock({ 2, "Dirt", BlockCategory::Terrain, 0.5f, 3, 0, false, true, {0.52f, 0.37f, 0.26f, 1.0f}, 2, 1 });
    registerBlock({ 3, "Coarse Dirt", BlockCategory::Terrain, 0.5f, 3, 0, false, true, {0.48f, 0.33f, 0.22f, 1.0f}, 3, 1 });
    registerBlock({ 4, "Podzol", BlockCategory::Terrain, 0.5f, 3, 0, false, true, {0.40f, 0.28f, 0.16f, 1.0f}, 4, 1 });
    registerBlock({ 5, "Mycelium", BlockCategory::Terrain, 0.6f, 3, 2, false, true, {0.56f, 0.48f, 0.62f, 1.0f}, 2, 1 });
    registerBlock({ 6, "Peat", BlockCategory::Terrain, 0.5f, 3, 0, false, true, {0.30f, 0.24f, 0.18f, 1.0f}, 6, 1 });
    registerBlock({ 7, "Mud", BlockCategory::Terrain, 0.5f, 3, 0, false, true, {0.38f, 0.30f, 0.25f, 1.0f}, 7, 1 });
    registerBlock({ 8, "Silt", BlockCategory::Terrain, 0.4f, 3, 0, false, true, {0.60f, 0.55f, 0.48f, 1.0f}, 8, 1 });
    registerBlock({ 9, "Loam", BlockCategory::Terrain, 0.5f, 3, 0, false, true, {0.45f, 0.35f, 0.25f, 1.0f}, 9, 1 });
    registerBlock({ 10, "Clay", BlockCategory::Terrain, 0.6f, 3, 0, false, true, {0.64f, 0.66f, 0.72f, 1.0f}, 10, 4 });
    registerBlock({ 11, "Sand", BlockCategory::Terrain, 0.5f, 3, 0, false, true, {0.86f, 0.82f, 0.62f, 1.0f}, 11, 1 });
    registerBlock({ 12, "Red Sand", BlockCategory::Terrain, 0.5f, 3, 0, false, true, {0.78f, 0.42f, 0.24f, 1.0f}, 12, 1 });
    registerBlock({ 13, "White Sand", BlockCategory::Terrain, 0.5f, 3, 0, false, true, {0.92f, 0.92f, 0.88f, 1.0f}, 13, 1 });
    registerBlock({ 14, "Gravel", BlockCategory::Terrain, 0.6f, 3, 0, false, true, {0.55f, 0.55f, 0.55f, 1.0f}, 14, 1 });
    registerBlock({ 15, "Permafrost", BlockCategory::Terrain, 1.2f, 1, 0, false, true, {0.60f, 0.70f, 0.78f, 1.0f}, 15, 1 });
    registerBlock({ 16, "Snow Block", BlockCategory::Terrain, 0.2f, 3, 0, false, true, {0.95f, 0.96f, 0.98f, 1.0f}, 16, 1 });
    registerBlock({ 17, "Water", BlockCategory::Terrain, 0.0f, 0, 0, true, false, {0.18f, 0.48f, 0.88f, 0.65f}, 0, 0, true });
    registerBlock({ 18, "Ice", BlockCategory::Terrain, 0.5f, 1, 0, true, true, {0.70f, 0.85f, 0.95f, 0.85f}, 18, 1, false });
    registerBlock({ 19, "Packed Ice", BlockCategory::Terrain, 0.8f, 1, 0, false, true, {0.65f, 0.80f, 0.92f, 1.0f}, 19, 1, false });
    registerBlock({ 20, "Blue Ice", BlockCategory::Terrain, 1.5f, 1, 1, false, true, {0.45f, 0.70f, 0.96f, 1.0f}, 20, 1, false });
    registerBlock({ 21, "Scorched Earth", BlockCategory::Terrain, 0.8f, 3, 1, false, true, {0.28f, 0.18f, 0.14f, 1.0f}, 21, 1 });
    registerBlock({ 22, "Soul Soil", BlockCategory::Terrain, 0.6f, 3, 2, false, true, {0.32f, 0.24f, 0.20f, 1.0f}, 22, 1 });
    registerBlock({ 23, "Netherrack", BlockCategory::Terrain, 0.4f, 1, 0, false, true, {0.50f, 0.15f, 0.15f, 1.0f}, 23, 1 });
    registerBlock({ 24, "End Stone", BlockCategory::Terrain, 1.5f, 1, 0, false, true, {0.88f, 0.89f, 0.68f, 1.0f}, 24, 1 });
    registerBlock({ 25, "Lush Turf", BlockCategory::Terrain, 0.6f, 3, 1, false, true, {0.20f, 0.80f, 0.35f, 1.0f}, 25, 1 });
    registerBlock({ 26, "Moss Block", BlockCategory::Terrain, 0.4f, 3, 0, false, true, {0.30f, 0.52f, 0.22f, 1.0f}, 26, 1 });
    registerBlock({ 27, "Coral Sand", BlockCategory::Terrain, 0.5f, 3, 0, false, true, {0.95f, 0.75f, 0.70f, 1.0f}, 27, 1 });
    registerBlock({ 28, "Black Sand", BlockCategory::Terrain, 0.5f, 3, 0, false, true, {0.18f, 0.18f, 0.20f, 1.0f}, 28, 1 });
    registerBlock({ 29, "Dune Sand", BlockCategory::Terrain, 0.5f, 3, 0, false, true, {0.84f, 0.76f, 0.52f, 1.0f}, 29, 1 });
    registerBlock({ 30, "Crimson Turf", BlockCategory::Terrain, 0.6f, 3, 3, false, true, {0.70f, 0.15f, 0.25f, 1.0f}, 30, 1 });
    registerBlock({ 31, "Warped Turf", BlockCategory::Terrain, 0.6f, 3, 3, false, true, {0.12f, 0.62f, 0.58f, 1.0f}, 31, 1 });
    registerBlock({ 32, "Molten Lava", BlockCategory::Terrain, 0.0f, 0, 15, false, false, {0.98f, 0.42f, 0.08f, 1.0f}, 0, 0, true });
    registerBlock({ 33, "Abyssal Silt", BlockCategory::Terrain, 0.5f, 3, 0, false, true, {0.15f, 0.20f, 0.25f, 1.0f}, 33, 1 });
    registerBlock({ 34, "Frozen Gravel", BlockCategory::Terrain, 0.8f, 3, 0, false, true, {0.60f, 0.68f, 0.75f, 1.0f}, 34, 1 });
    registerBlock({ 35, "Sacred Earth", BlockCategory::Terrain, 0.8f, 3, 5, false, true, {0.85f, 0.80f, 0.50f, 1.0f}, 35, 1 });

    // --- 36 to 70: Geological Stones & Strata ---
    registerBlock({ 36, "Stone", BlockCategory::Stone, 1.5f, 1, 0, false, true, {0.55f, 0.55f, 0.55f, 1.0f}, 37, 1 });
    registerBlock({ 37, "Cobblestone", BlockCategory::Stone, 2.0f, 1, 0, false, true, {0.48f, 0.48f, 0.48f, 1.0f}, 37, 1 });
    registerBlock({ 38, "Mossy Cobblestone", BlockCategory::Stone, 2.0f, 1, 0, false, true, {0.42f, 0.50f, 0.40f, 1.0f}, 38, 1 });
    registerBlock({ 39, "Granite", BlockCategory::Stone, 1.8f, 1, 0, false, true, {0.65f, 0.45f, 0.40f, 1.0f}, 39, 1 });
    registerBlock({ 40, "Polished Granite", BlockCategory::Stone, 1.8f, 1, 0, false, true, {0.70f, 0.48f, 0.42f, 1.0f}, 40, 1 });
    registerBlock({ 41, "Diorite", BlockCategory::Stone, 1.8f, 1, 0, false, true, {0.75f, 0.75f, 0.75f, 1.0f}, 41, 1 });
    registerBlock({ 42, "Polished Diorite", BlockCategory::Stone, 1.8f, 1, 0, false, true, {0.80f, 0.80f, 0.80f, 1.0f}, 42, 1 });
    registerBlock({ 43, "Andesite", BlockCategory::Stone, 1.8f, 1, 0, false, true, {0.50f, 0.52f, 0.50f, 1.0f}, 43, 1 });
    registerBlock({ 44, "Polished Andesite", BlockCategory::Stone, 1.8f, 1, 0, false, true, {0.55f, 0.58f, 0.55f, 1.0f}, 44, 1 });
    registerBlock({ 45, "Deepslate", BlockCategory::Stone, 2.5f, 1, 0, false, true, {0.28f, 0.28f, 0.30f, 1.0f}, 46, 1 });
    registerBlock({ 46, "Cobbled Deepslate", BlockCategory::Stone, 2.8f, 1, 0, false, true, {0.24f, 0.24f, 0.26f, 1.0f}, 46, 1 });
    registerBlock({ 47, "Tuff", BlockCategory::Stone, 1.5f, 1, 0, false, true, {0.42f, 0.44f, 0.40f, 1.0f}, 47, 1 });
    registerBlock({ 48, "Calcite", BlockCategory::Stone, 1.2f, 1, 0, false, true, {0.88f, 0.88f, 0.84f, 1.0f}, 48, 1 });
    registerBlock({ 49, "Dripstone", BlockCategory::Stone, 1.6f, 1, 0, false, true, {0.52f, 0.42f, 0.36f, 1.0f}, 49, 1 });
    registerBlock({ 50, "Basalt", BlockCategory::Stone, 2.2f, 1, 0, false, true, {0.30f, 0.30f, 0.32f, 1.0f}, 50, 1 });
    registerBlock({ 51, "Smooth Basalt", BlockCategory::Stone, 2.2f, 1, 0, false, true, {0.26f, 0.26f, 0.28f, 1.0f}, 51, 1 });
    registerBlock({ 52, "Obsidian", BlockCategory::Stone, 10.0f, 1, 0, false, true, {0.12f, 0.08f, 0.18f, 1.0f}, 52, 1 });
    registerBlock({ 53, "Cryo-Obsidian", BlockCategory::Stone, 12.0f, 1, 4, false, true, {0.10f, 0.20f, 0.35f, 1.0f}, 53, 1 });
    registerBlock({ 54, "Sandstone", BlockCategory::Stone, 1.2f, 1, 0, false, true, {0.84f, 0.80f, 0.60f, 1.0f}, 54, 1 });
    registerBlock({ 55, "Chiseled Sandstone", BlockCategory::Stone, 1.2f, 1, 0, false, true, {0.82f, 0.78f, 0.58f, 1.0f}, 55, 1 });
    registerBlock({ 56, "Red Sandstone", BlockCategory::Stone, 1.2f, 1, 0, false, true, {0.74f, 0.38f, 0.20f, 1.0f}, 56, 1 });
    registerBlock({ 57, "Marble", BlockCategory::Stone, 2.0f, 1, 0, false, true, {0.92f, 0.92f, 0.94f, 1.0f}, 57, 1 });
    registerBlock({ 58, "Slate", BlockCategory::Stone, 2.0f, 1, 0, false, true, {0.38f, 0.40f, 0.44f, 1.0f}, 58, 1 });
    registerBlock({ 59, "Schist", BlockCategory::Stone, 2.0f, 1, 0, false, true, {0.46f, 0.48f, 0.42f, 1.0f}, 59, 1 });
    registerBlock({ 60, "Pumice", BlockCategory::Stone, 0.8f, 1, 0, false, true, {0.68f, 0.66f, 0.64f, 1.0f}, 60, 1 });
    registerBlock({ 61, "Brimstone", BlockCategory::Stone, 2.5f, 1, 2, false, true, {0.65f, 0.55f, 0.15f, 1.0f}, 61, 1 });
    registerBlock({ 62, "Voidstone", BlockCategory::Stone, 4.0f, 1, 1, false, true, {0.15f, 0.05f, 0.20f, 1.0f}, 62, 1 });
    registerBlock({ 63, "Starstone", BlockCategory::Stone, 5.0f, 1, 8, false, true, {0.25f, 0.35f, 0.65f, 1.0f}, 63, 1 });
    registerBlock({ 64, "Moon Rock", BlockCategory::Stone, 2.5f, 1, 3, false, true, {0.78f, 0.80f, 0.85f, 1.0f}, 64, 1 });
    registerBlock({ 65, "Meteorite Crust", BlockCategory::Stone, 6.0f, 1, 1, false, true, {0.22f, 0.15f, 0.10f, 1.0f}, 65, 1 });
    registerBlock({ 66, "Gilded Blackstone", BlockCategory::Stone, 3.0f, 1, 2, false, true, {0.20f, 0.18f, 0.20f, 1.0f}, 66, 1 });
    registerBlock({ 67, "Prismarine", BlockCategory::Stone, 2.2f, 1, 0, false, true, {0.35f, 0.65f, 0.60f, 1.0f}, 67, 1 });
    registerBlock({ 68, "Dark Prismarine", BlockCategory::Stone, 2.4f, 1, 0, false, true, {0.20f, 0.35f, 0.32f, 1.0f}, 68, 1 });
    registerBlock({ 69, "Prismarine Bricks", BlockCategory::Stone, 2.2f, 1, 0, false, true, {0.40f, 0.70f, 0.65f, 1.0f}, 69, 1 });
    registerBlock({ 70, "Eldritch Monolith Stone", BlockCategory::Stone, 8.0f, 1, 5, false, true, {0.10f, 0.25f, 0.18f, 1.0f}, 70, 1 });

    // --- 71 to 120: Woods, Leaves & Flora (16 Tree Species * 3 + specials) ---
    const char* treeNames[16] = {
        "Oak", "Birch", "Spruce", "Pine", "Jungle", "Acacia", "Dark Oak", "Mangrove",
        "Cherry", "Maple", "Willow", "Fungal", "Petrified", "Bamboo", "Palm", "Astral"
    };
    Vec4 logColors[16] = {
        {0.50f, 0.38f, 0.25f, 1.0f}, {0.85f, 0.85f, 0.82f, 1.0f}, {0.38f, 0.28f, 0.18f, 1.0f}, {0.42f, 0.30f, 0.20f, 1.0f},
        {0.48f, 0.34f, 0.18f, 1.0f}, {0.58f, 0.40f, 0.28f, 1.0f}, {0.25f, 0.18f, 0.12f, 1.0f}, {0.45f, 0.32f, 0.22f, 1.0f},
        {0.62f, 0.42f, 0.42f, 1.0f}, {0.55f, 0.32f, 0.18f, 1.0f}, {0.35f, 0.38f, 0.25f, 1.0f}, {0.48f, 0.25f, 0.45f, 1.0f},
        {0.45f, 0.45f, 0.48f, 1.0f}, {0.45f, 0.65f, 0.25f, 1.0f}, {0.60f, 0.45f, 0.30f, 1.0f}, {0.25f, 0.45f, 0.75f, 1.0f}
    };
    Vec4 plankColors[16] = {
        {0.65f, 0.50f, 0.32f, 1.0f}, {0.82f, 0.75f, 0.60f, 1.0f}, {0.48f, 0.36f, 0.24f, 1.0f}, {0.52f, 0.38f, 0.25f, 1.0f},
        {0.60f, 0.44f, 0.28f, 1.0f}, {0.72f, 0.48f, 0.28f, 1.0f}, {0.35f, 0.24f, 0.16f, 1.0f}, {0.58f, 0.35f, 0.28f, 1.0f},
        {0.85f, 0.65f, 0.65f, 1.0f}, {0.72f, 0.45f, 0.25f, 1.0f}, {0.48f, 0.50f, 0.35f, 1.0f}, {0.65f, 0.38f, 0.60f, 1.0f},
        {0.55f, 0.55f, 0.58f, 1.0f}, {0.62f, 0.75f, 0.38f, 1.0f}, {0.75f, 0.58f, 0.38f, 1.0f}, {0.38f, 0.60f, 0.90f, 1.0f}
    };
    Vec4 leafColors[16] = {
        {0.25f, 0.65f, 0.18f, 1.0f}, {0.35f, 0.72f, 0.22f, 1.0f}, {0.18f, 0.45f, 0.22f, 1.0f}, {0.16f, 0.42f, 0.20f, 1.0f},
        {0.15f, 0.60f, 0.15f, 1.0f}, {0.45f, 0.62f, 0.15f, 1.0f}, {0.18f, 0.40f, 0.12f, 1.0f}, {0.22f, 0.55f, 0.25f, 1.0f},
        {0.95f, 0.65f, 0.78f, 1.0f}, {0.85f, 0.40f, 0.12f, 1.0f}, {0.32f, 0.58f, 0.30f, 1.0f}, {0.75f, 0.25f, 0.55f, 1.0f},
        {0.45f, 0.45f, 0.42f, 1.0f}, {0.35f, 0.75f, 0.20f, 1.0f}, {0.28f, 0.68f, 0.22f, 1.0f}, {0.45f, 0.75f, 0.98f, 1.0f}
    };

    for (int i = 0; i < 16; ++i) {
        uint16_t logId = 71 + i * 3;
        uint16_t plankId = 72 + i * 3;
        uint16_t leafId = 73 + i * 3;

        std::string name = treeNames[i];
        registerBlock({ logId, name + " Log", BlockCategory::Wood, 2.0f, 2, 0, false, true, logColors[i], logId, 1 });
        registerBlock({ plankId, name + " Planks", BlockCategory::Wood, 1.8f, 2, 0, false, true, plankColors[i], plankId, 1 });
        registerBlock({ leafId, name + " Leaves", BlockCategory::Wood, 0.2f, 0, (i == 15 ? (uint8_t)6 : (uint8_t)0), true, true, leafColors[i], leafId, 1 });
    }
    registerBlock({ 119, "Giant Red Mushroom Cap", BlockCategory::Wood, 0.3f, 2, 2, false, true, {0.85f, 0.20f, 0.20f, 1.0f}, 119, 1 });
    registerBlock({ 120, "Giant Brown Mushroom Cap", BlockCategory::Wood, 0.3f, 2, 2, false, true, {0.58f, 0.42f, 0.28f, 1.0f}, 120, 1 });

    // --- 121 to 160: Ores & Refined Minerals ---
    const char* oreNames[15] = {
        "Coal", "Copper", "Tin", "Iron", "Silver", "Gold", "Platinum", "Cobalt",
        "Mythril", "Adamantite", "Orichalcum", "Titanium", "Runite", "Meteorite", "Star Metal"
    };
    Vec4 oreColors[15] = {
        {0.20f, 0.20f, 0.20f, 1.0f}, {0.82f, 0.48f, 0.28f, 1.0f}, {0.68f, 0.68f, 0.72f, 1.0f}, {0.78f, 0.70f, 0.62f, 1.0f},
        {0.88f, 0.90f, 0.95f, 1.0f}, {0.96f, 0.82f, 0.20f, 1.0f}, {0.85f, 0.88f, 0.92f, 1.0f}, {0.20f, 0.35f, 0.85f, 1.0f},
        {0.25f, 0.70f, 0.85f, 1.0f}, {0.85f, 0.25f, 0.35f, 1.0f}, {0.88f, 0.45f, 0.65f, 1.0f}, {0.70f, 0.72f, 0.78f, 1.0f},
        {0.18f, 0.55f, 0.45f, 1.0f}, {0.35f, 0.22f, 0.18f, 1.0f}, {0.45f, 0.65f, 0.98f, 1.0f}
    };

    for (int i = 0; i < 15; ++i) {
        uint16_t oreId = 121 + i;
        registerBlock({ oreId, std::string(oreNames[i]) + " Ore", BlockCategory::Ores, 3.0f + i * 0.3f, 1, 0, false, true, oreColors[i], oreId, 1 });
    }

    // 136 to 140: Deepslate Ores
    registerBlock({ 136, "Deepslate Coal Ore", BlockCategory::Ores, 4.0f, 1, 0, false, true, {0.22f, 0.22f, 0.24f, 1.0f}, 121, 1 });
    registerBlock({ 137, "Deepslate Iron Ore", BlockCategory::Ores, 4.5f, 1, 0, false, true, {0.38f, 0.34f, 0.30f, 1.0f}, 124, 1 });
    registerBlock({ 138, "Deepslate Gold Ore", BlockCategory::Ores, 5.0f, 1, 0, false, true, {0.55f, 0.48f, 0.25f, 1.0f}, 126, 1 });
    registerBlock({ 139, "Deepslate Mythril Ore", BlockCategory::Ores, 6.0f, 1, 2, false, true, {0.22f, 0.42f, 0.50f, 1.0f}, 129, 1 });
    registerBlock({ 140, "Deepslate Adamantite Ore", BlockCategory::Ores, 7.0f, 1, 2, false, true, {0.48f, 0.22f, 0.28f, 1.0f}, 130, 1 });

    // 141 to 155: Refined Solid Blocks
    for (int i = 0; i < 15; ++i) {
        uint16_t blockId = 141 + i;
        registerBlock({ blockId, std::string("Block of ") + oreNames[i], BlockCategory::Ores, 5.0f + i * 0.4f, 1, (i >= 8 ? (uint8_t)4 : (uint8_t)0), false, true, oreColors[i] * 1.15f, blockId, 1 });
    }

    // 156 to 160: Raw Ore Clusters
    registerBlock({ 156, "Raw Iron Cluster", BlockCategory::Ores, 3.5f, 1, 0, false, true, {0.70f, 0.60f, 0.50f, 1.0f}, 156, 1 });
    registerBlock({ 157, "Raw Gold Cluster", BlockCategory::Ores, 3.8f, 1, 0, false, true, {0.90f, 0.75f, 0.15f, 1.0f}, 157, 1 });
    registerBlock({ 158, "Raw Copper Cluster", BlockCategory::Ores, 3.2f, 1, 0, false, true, {0.78f, 0.45f, 0.25f, 1.0f}, 158, 1 });
    registerBlock({ 159, "Raw Mythril Cluster", BlockCategory::Ores, 5.0f, 1, 3, false, true, {0.20f, 0.65f, 0.80f, 1.0f}, 159, 1 });
    registerBlock({ 160, "Raw Adamantite Cluster", BlockCategory::Ores, 5.5f, 1, 3, false, true, {0.80f, 0.20f, 0.30f, 1.0f}, 160, 1 });

    // --- 161 to 205: Gems & Elemental Crystals (45 blocks) ---
    const char* gemNames[15] = {
        "Diamond", "Emerald", "Ruby", "Sapphire", "Amethyst", "Topaz", "Opal", "Amber",
        "Jade", "Onyx", "Aquamarine", "Tourmaline", "Tanzanite", "Garnet", "Alexandrite"
    };
    Vec4 gemColors[15] = {
        {0.45f, 0.95f, 0.95f, 1.0f}, {0.15f, 0.85f, 0.35f, 1.0f}, {0.95f, 0.15f, 0.25f, 1.0f}, {0.15f, 0.35f, 0.95f, 1.0f},
        {0.75f, 0.35f, 0.90f, 1.0f}, {0.95f, 0.75f, 0.20f, 1.0f}, {0.88f, 0.92f, 0.95f, 1.0f}, {0.95f, 0.60f, 0.10f, 1.0f},
        {0.25f, 0.75f, 0.45f, 1.0f}, {0.12f, 0.12f, 0.15f, 1.0f}, {0.40f, 0.85f, 0.85f, 1.0f}, {0.90f, 0.35f, 0.65f, 1.0f},
        {0.35f, 0.30f, 0.80f, 1.0f}, {0.70f, 0.15f, 0.20f, 1.0f}, {0.35f, 0.75f, 0.65f, 1.0f}
    };

    for (int i = 0; i < 15; ++i) {
        uint16_t gemId = 161 + i;
        registerBlock({ gemId, std::string(gemNames[i]) + " Block", BlockCategory::Crystals, 5.0f, 1, 4, false, true, gemColors[i], gemId, 1 });
    }

    for (int i = 0; i < 10; ++i) {
        uint16_t clusterId = 176 + i;
        registerBlock({ clusterId, std::string(gemNames[i]) + " Cluster", BlockCategory::Crystals, 4.0f, 1, 6, true, true, gemColors[i] * 1.1f, clusterId, 2 });
    }

    const char* elementalCrystals[20] = {
        "Flame Crystal", "Frost Crystal", "Tempest Crystal", "Terra Crystal", "Radiant Light Crystal",
        "Shadow Void Crystal", "Arcane Mana Crystal", "Chrono Time Crystal", "Blood Crystal", "Astral Zenith Crystal",
        "Poison Spore Crystal", "Venom Fang Crystal", "Holy Sun Crystal", "Lunar Moon Crystal", "Void Star Crystal",
        "Prismatic Crystal", "Resonance Crystal", "Sonic Wave Crystal", "Magnetic Crystal", "Soul Spark Crystal"
    };
    Vec4 elemColors[20] = {
        {0.98f, 0.40f, 0.10f, 1.0f}, {0.40f, 0.85f, 0.98f, 1.0f}, {0.95f, 0.95f, 0.30f, 1.0f}, {0.55f, 0.75f, 0.25f, 1.0f},
        {0.98f, 0.98f, 0.85f, 1.0f}, {0.20f, 0.05f, 0.30f, 1.0f}, {0.65f, 0.25f, 0.95f, 1.0f}, {0.30f, 0.75f, 0.90f, 1.0f},
        {0.85f, 0.08f, 0.12f, 1.0f}, {0.35f, 0.55f, 0.98f, 1.0f}, {0.45f, 0.80f, 0.20f, 1.0f}, {0.55f, 0.15f, 0.65f, 1.0f},
        {0.98f, 0.85f, 0.20f, 1.0f}, {0.70f, 0.80f, 0.95f, 1.0f}, {0.15f, 0.10f, 0.25f, 1.0f}, {0.90f, 0.70f, 0.95f, 1.0f},
        {0.25f, 0.90f, 0.70f, 1.0f}, {0.40f, 0.60f, 0.85f, 1.0f}, {0.75f, 0.35f, 0.45f, 1.0f}, {0.30f, 0.85f, 0.90f, 1.0f}
    };
    for (int i = 0; i < 20; ++i) {
        uint16_t elemId = 186 + i;
        registerBlock({ elemId, elementalCrystals[i], BlockCategory::Crystals, 4.5f, 1, 12, false, true, elemColors[i], elemId, 1 });
    }

    // --- 206 to 240: Magical & Ancient Runestones (35 blocks) ---
    const char* runeTypes[15] = {
        "Fire", "Frost", "Storm", "Earth", "Life", "Decay", "Light", "Shadow",
        "Arcana", "Gravity", "Space", "Time", "Mind", "Spirit", "Chaos"
    };
    for (int i = 0; i < 15; ++i) {
        uint16_t runeId = 206 + i;
        registerBlock({ runeId, std::string("Runestone of ") + runeTypes[i], BlockCategory::Magic, 6.0f, 1, 8, false, true, {0.35f + (i % 3) * 0.2f, 0.25f + ((i / 3) % 3) * 0.2f, 0.55f, 1.0f}, runeId, 1 });
    }

    const char* magicStructures[20] = {
        "Enchanting Altar", "Arcane Pedestal", "Leyline Conduit", "Celestial Portal Frame", "Void Rift Anchor",
        "Eldritch Obelisk", "Sun Altar", "Moon Shrine", "Mana Fountain", "Blood Altar",
        "Dragon Altar", "Phoenix Pyre", "Titan Beacon", "Astral Ward", "Warding Totem",
        "Spirit Crucible", "Soul Siphon", "Void Monolith", "Nexus Core", "Aether Anchor"
    };
    for (int i = 0; i < 20; ++i) {
        uint16_t structId = 221 + i;
        registerBlock({ structId, magicStructures[i], BlockCategory::Magic, 8.0f, 1, 10, false, true, {0.45f, 0.30f, 0.65f, 1.0f}, structId, 1 });
    }

    // --- 241 to 280: Dungeon & Temple Architecture (40 blocks) ---
    const char* dungeonBlocks[40] = {
        "Ancient Stone Bricks", "Mossy Stone Bricks", "Cracked Stone Bricks", "Chiseled Crypt Stone",
        "Crypt Slab Block", "Gilded Crypt Tile", "Necrotic Bone Tile", "Skulls Block", "Skeleton Mosaic", "Cursed Stone",
        "Sun Temple Sandstone", "Sun Hieroglyph Brick", "Pharaoh Gold Inlay", "Sunken Temple Prismarine", "Dark Atlantis Tile",
        "Coral Encrusted Brick", "Sea Lantern Tile", "Siren Pearl Tile", "Volcanic Fortress Brick", "Magma Tile",
        "Hellfire Slate", "Brimstone Cobble", "Demon Forge Tile", "Frost Fortress Tile", "Ice Citadel Brick",
        "Frozen Crypt Slab", "Sky Palace Marble", "Angelic Column", "Celestial Mosaic", "Void Fortress Block",
        "Abyssal Slate", "Nether Citadel Brick", "Shadow Tile", "Labyrinth Tile", "Minotaur Stone",
        "Dwarven Vault Brick", "Runed Dwarven Slab", "Dragon Lair Rock", "Ancient Roman Tile", "Gothic Cathedral Brick"
    };
    for (int i = 0; i < 40; ++i) {
        uint16_t dungId = 241 + i;
        registerBlock({ dungId, dungeonBlocks[i], BlockCategory::Dungeon, 4.0f, 1, (i == 16 ? (uint8_t)14 : (uint8_t)0), false, true, {0.40f + (i % 5) * 0.1f, 0.38f + ((i / 5) % 4) * 0.1f, 0.42f, 1.0f}, dungId, 1 });
    }

    // --- 281 to 330: Artisan Dyes & Colored Building Blocks (50 blocks) ---
    const char* colorNames[16] = {
        "White", "Orange", "Magenta", "Light Blue", "Yellow", "Lime", "Pink", "Gray",
        "Light Gray", "Cyan", "Purple", "Blue", "Brown", "Green", "Red", "Black"
    };
    Vec4 colors16[16] = {
        {0.95f, 0.95f, 0.95f, 1.0f}, {0.92f, 0.50f, 0.15f, 1.0f}, {0.80f, 0.30f, 0.75f, 1.0f}, {0.40f, 0.70f, 0.90f, 1.0f},
        {0.95f, 0.85f, 0.20f, 1.0f}, {0.45f, 0.80f, 0.15f, 1.0f}, {0.92f, 0.60f, 0.70f, 1.0f}, {0.35f, 0.35f, 0.38f, 1.0f},
        {0.60f, 0.60f, 0.62f, 1.0f}, {0.20f, 0.60f, 0.65f, 1.0f}, {0.55f, 0.25f, 0.75f, 1.0f}, {0.20f, 0.30f, 0.75f, 1.0f},
        {0.45f, 0.30f, 0.20f, 1.0f}, {0.30f, 0.50f, 0.20f, 1.0f}, {0.75f, 0.20f, 0.20f, 1.0f}, {0.12f, 0.12f, 0.14f, 1.0f}
    };

    // 281-296: Terracotta
    for (int i = 0; i < 16; ++i) {
        uint16_t id = 281 + i;
        registerBlock({ id, std::string(colorNames[i]) + " Terracotta", BlockCategory::Colored, 2.5f, 1, 0, false, true, colors16[i] * 0.8f, id, 1 });
    }
    // 297-312: Glazed Ceramic Tiles
    for (int i = 0; i < 16; ++i) {
        uint16_t id = 297 + i;
        registerBlock({ id, std::string(colorNames[i]) + " Glazed Tile", BlockCategory::Colored, 2.8f, 1, 0, false, true, colors16[i] * 1.05f, id, 1 });
    }
    // 313-328: Smooth Concrete
    for (int i = 0; i < 16; ++i) {
        uint16_t id = 313 + i;
        registerBlock({ id, std::string(colorNames[i]) + " Concrete", BlockCategory::Colored, 3.0f, 1, 0, false, true, colors16[i], id, 1 });
    }
    registerBlock({ 329, "Gilded Concrete", BlockCategory::Colored, 3.5f, 1, 2, false, true, {0.95f, 0.85f, 0.40f, 1.0f}, 329, 1 });
    registerBlock({ 330, "Polished Marble Tile", BlockCategory::Colored, 3.0f, 1, 0, false, true, {0.92f, 0.92f, 0.95f, 1.0f}, 330, 1 });

    // --- 331 to 350: Mechanical, Utility & Lighting (20 blocks) ---
    registerBlock({ 331, "Crafting Workstation", BlockCategory::Mechanical, 2.5f, 2, 0, false, true, {0.62f, 0.45f, 0.28f, 1.0f}, 331, 1 });
    registerBlock({ 332, "Smelting Furnace", BlockCategory::Mechanical, 3.5f, 1, 4, false, true, {0.45f, 0.45f, 0.45f, 1.0f}, 332, 1 });
    registerBlock({ 333, "Weapon Forge", BlockCategory::Mechanical, 4.0f, 1, 6, false, true, {0.35f, 0.35f, 0.40f, 1.0f}, 333, 1 });
    registerBlock({ 334, "Alchemy Lab", BlockCategory::Mechanical, 2.5f, 1, 5, false, true, {0.30f, 0.50f, 0.60f, 1.0f}, 334, 1 });
    registerBlock({ 335, "Blacksmith Anvil", BlockCategory::Mechanical, 5.0f, 1, 0, false, true, {0.25f, 0.25f, 0.28f, 1.0f}, 335, 1 });
    registerBlock({ 336, "Rune Enchanter", BlockCategory::Mechanical, 5.0f, 1, 8, false, true, {0.40f, 0.25f, 0.55f, 1.0f}, 336, 1 });
    registerBlock({ 337, "Wooden Storage Chest", BlockCategory::Mechanical, 2.0f, 2, 0, false, true, {0.55f, 0.40f, 0.22f, 1.0f}, 337, 1 });
    registerBlock({ 338, "Iron Reinforced Vault", BlockCategory::Mechanical, 6.0f, 1, 0, false, true, {0.60f, 0.60f, 0.65f, 1.0f}, 338, 1 });
    registerBlock({ 339, "Trap Spikes", BlockCategory::Mechanical, 1.5f, 1, 0, false, true, {0.50f, 0.50f, 0.52f, 1.0f}, 339, 1 });
    registerBlock({ 340, "Wooden Trapdoor", BlockCategory::Mechanical, 2.0f, 2, 0, true, true, {0.58f, 0.42f, 0.25f, 1.0f}, 340, 1 });
    registerBlock({ 341, "Iron Grate", BlockCategory::Mechanical, 4.0f, 1, 0, true, true, {0.45f, 0.45f, 0.48f, 1.0f}, 341, 1 });
    registerBlock({ 342, "Clear Glass", BlockCategory::Mechanical, 0.3f, 0, 0, true, true, {0.85f, 0.92f, 0.95f, 0.4f}, 342, 1 });
    registerBlock({ 343, "Tinted Glass", BlockCategory::Mechanical, 0.3f, 0, 0, true, true, {0.35f, 0.35f, 0.40f, 0.6f}, 343, 1 });
    registerBlock({ 344, "Glowstone Lamp", BlockCategory::Mechanical, 1.0f, 0, 15, false, true, {0.95f, 0.85f, 0.45f, 1.0f}, 344, 1 });
    registerBlock({ 345, "Brass Lantern", BlockCategory::Mechanical, 1.2f, 1, 14, false, true, {0.85f, 0.65f, 0.25f, 1.0f}, 345, 1 });
    registerBlock({ 346, "Campfire", BlockCategory::Mechanical, 1.0f, 2, 13, false, true, {0.85f, 0.40f, 0.15f, 1.0f}, 346, 1 });
    registerBlock({ 347, "Bookshelf", BlockCategory::Mechanical, 1.5f, 2, 0, false, true, {0.50f, 0.35f, 0.22f, 1.0f}, 347, 1 });
    registerBlock({ 348, "Palworld Sphere Workbench", BlockCategory::Mechanical, 3.0f, 2, 4, false, true, {0.35f, 0.65f, 0.75f, 1.0f}, 348, 1 });
    registerBlock({ 349, "Pal Creature Bed", BlockCategory::Mechanical, 1.5f, 2, 0, false, true, {0.75f, 0.45f, 0.35f, 1.0f}, 349, 1 });
    registerBlock({ 350, "Feed Box", BlockCategory::Mechanical, 2.0f, 2, 0, false, true, {0.65f, 0.48f, 0.28f, 1.0f}, 350, 1 });

    // --- 351 to 365: Exotic & Mythic Relics (15 blocks) ---
    const char* exoticNames[15] = {
        "Cosmic Stardust", "Nebular Foam", "Aether Cloud", "Dragon Scale Tile", "Phoenix Ash Block",
        "Titan Core", "Void Singularity", "Chrono Shard Block", "Aurora Weave", "World Tree Bark",
        "Divine Marble", "Abyssal Pearl Block", "Etheric Membrane", "Living Wood", "Philosopher's Stone Block"
    };
    Vec4 exoticColors[15] = {
        {0.65f, 0.85f, 0.98f, 1.0f}, {0.85f, 0.45f, 0.95f, 1.0f}, {0.92f, 0.95f, 0.98f, 0.85f}, {0.75f, 0.15f, 0.25f, 1.0f},
        {0.95f, 0.45f, 0.15f, 1.0f}, {0.35f, 0.65f, 0.85f, 1.0f}, {0.10f, 0.02f, 0.18f, 1.0f}, {0.45f, 0.85f, 0.75f, 1.0f},
        {0.25f, 0.95f, 0.65f, 1.0f}, {0.45f, 0.60f, 0.30f, 1.0f}, {0.98f, 0.98f, 0.90f, 1.0f}, {0.20f, 0.75f, 0.80f, 1.0f},
        {0.70f, 0.85f, 0.95f, 0.75f}, {0.35f, 0.70f, 0.35f, 1.0f}, {0.85f, 0.20f, 0.35f, 1.0f}
    };
    for (int i = 0; i < 15; ++i) {
        uint16_t exId = 351 + i;
        registerBlock({ exId, exoticNames[i], BlockCategory::Exotic, 10.0f, 1, 12, (i == 2 || i == 12), true, exoticColors[i], exId, 1 });
    }

    // --- 366 to 369: Tree Saplings & Shoots (Placeable on soil, grows into trees) ---
    registerBlock({ 366, "Oak Sapling", BlockCategory::Wood, 0.05f, 0, 0, true, false, {0.30f, 0.72f, 0.25f, 1.0f}, 366, 1 });
    registerBlock({ 367, "Birch Sapling", BlockCategory::Wood, 0.05f, 0, 0, true, false, {0.45f, 0.82f, 0.30f, 1.0f}, 367, 1 });
    registerBlock({ 368, "Spruce Sapling", BlockCategory::Wood, 0.05f, 0, 0, true, false, {0.18f, 0.48f, 0.25f, 1.0f}, 368, 1 });
    registerBlock({ 369, "Cherry Sapling", BlockCategory::Wood, 0.05f, 0, 0, true, false, {0.95f, 0.65f, 0.80f, 1.0f}, 369, 1 });

    std::cout << "Registered all " << getCount() << " unique block definitions into BlockRegistry!" << std::endl;
}

} // namespace Aetheria
