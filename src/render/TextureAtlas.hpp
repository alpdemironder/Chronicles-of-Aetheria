#pragma once

#include "GLHeaders.hpp"
#include <vector>
#include <cstdint>

namespace Aetheria {

constexpr int TEX_RES = 16;       // 16x16 pixels per texture
constexpr int TOTAL_LAYERS = 512; // 365 blocks + structures + creatures

// Texture layer index mappings
constexpr int TEX_LAYER_WOOD_PLANK   = 370;
constexpr int TEX_LAYER_STONE_COBBLE = 371;
constexpr int TEX_LAYER_WOOD_WALL    = 372;
constexpr int TEX_LAYER_STONE_BRICK  = 373;
constexpr int TEX_LAYER_WOOD_ROOF    = 374;
constexpr int TEX_LAYER_STONE_ROOF   = 375;
constexpr int TEX_LAYER_WOOD_STAIRS  = 376;
constexpr int TEX_LAYER_STONE_STAIRS = 377;
constexpr int TEX_LAYER_PALBOX_CORE  = 378;
constexpr int TEX_LAYER_WORKBENCH    = 379;
constexpr int TEX_LAYER_CHEST        = 380;
constexpr int TEX_LAYER_BED          = 381;

constexpr int TEX_LAYER_CREATURE_FUR         = 390;
constexpr int TEX_LAYER_CREATURE_SCALE       = 391;
constexpr int TEX_LAYER_CREATURE_BOSS        = 392;
constexpr int TEX_LAYER_CREATURE_PIXIE       = 393;
constexpr int TEX_LAYER_CREATURE_BONE        = 394;
constexpr int TEX_LAYER_CREATURE_HYTALE_EYE   = 395;
constexpr int TEX_LAYER_CREATURE_HYTALE_RUNE  = 396;
constexpr int TEX_LAYER_CREATURE_HYTALE_ARMOR = 397;
constexpr int TEX_LAYER_CREATURE_HYTALE_CLOTH = 398;

// Multi-Face Block Texture Layers (Authentic Minecraft Block Facets)
constexpr int TEX_LAYER_GRASS_TOP       = 405;
constexpr int TEX_LAYER_LOG_OAK_TOP     = 406;
constexpr int TEX_LAYER_LOG_BIRCH_TOP   = 407;
constexpr int TEX_LAYER_LOG_SPRUCE_TOP  = 408;
constexpr int TEX_LAYER_CRAFTING_TOP    = 409;
constexpr int TEX_LAYER_FURNACE_TOP     = 410;
constexpr int TEX_LAYER_BOOKSHELF_TOP   = 411;
constexpr int TEX_LAYER_TNT_TOP         = 412;
constexpr int TEX_LAYER_TNT_BOTTOM      = 413;
constexpr int TEX_LAYER_LOG_CHERRY_TOP  = 414;
constexpr int TEX_LAYER_LOG_MAPLE_TOP   = 415;
constexpr int TEX_LAYER_LOG_JUNGLE_TOP  = 416;
constexpr int TEX_LAYER_LOG_ACACIA_TOP  = 417;
constexpr int TEX_LAYER_LOG_BAMBOO_TOP  = 418;
constexpr int TEX_LAYER_LOG_ASTRAL_TOP  = 419;
constexpr int TEX_LAYER_SANDSTONE_TOP   = 420;

inline uint16_t getBlockFaceTexture(uint16_t blockId, int faceIndex) {
    switch (blockId) {
        case 1: // Grass Block
            if (faceIndex == 0) return TEX_LAYER_GRASS_TOP;
            if (faceIndex == 1) return 2; // Dirt bottom
            return 1; // Grass side (dirt + jagged grass overhang)
        case 54: // Sandstone
            if (faceIndex == 0) return TEX_LAYER_SANDSTONE_TOP;
            return 54;
        case 71: // Oak Log
            if (faceIndex == 0 || faceIndex == 1) return TEX_LAYER_LOG_OAK_TOP;
            return 71;
        case 74: // Birch Log
            if (faceIndex == 0 || faceIndex == 1) return TEX_LAYER_LOG_BIRCH_TOP;
            return 74;
        case 77: // Spruce Log
            if (faceIndex == 0 || faceIndex == 1) return TEX_LAYER_LOG_SPRUCE_TOP;
            return 77;
        case 80: // Pine Log
            if (faceIndex == 0 || faceIndex == 1) return TEX_LAYER_LOG_SPRUCE_TOP;
            return 80;
        case 83: // Jungle Log
            if (faceIndex == 0 || faceIndex == 1) return TEX_LAYER_LOG_JUNGLE_TOP;
            return 83;
        case 86: // Acacia Log
            if (faceIndex == 0 || faceIndex == 1) return TEX_LAYER_LOG_ACACIA_TOP;
            return 86;
        case 89: // Dark Oak Log
            if (faceIndex == 0 || faceIndex == 1) return TEX_LAYER_LOG_SPRUCE_TOP;
            return 89;
        case 92: // Mangrove Log
            if (faceIndex == 0 || faceIndex == 1) return TEX_LAYER_LOG_SPRUCE_TOP;
            return 92;
        case 95: // Cherry Log
            if (faceIndex == 0 || faceIndex == 1) return TEX_LAYER_LOG_CHERRY_TOP;
            return 95;
        case 98: // Maple Log
            if (faceIndex == 0 || faceIndex == 1) return TEX_LAYER_LOG_MAPLE_TOP;
            return 98;
        case 101: // Willow Log
            if (faceIndex == 0 || faceIndex == 1) return TEX_LAYER_LOG_OAK_TOP;
            return 101;
        case 104: // Fungal Log
            if (faceIndex == 0 || faceIndex == 1) return TEX_LAYER_LOG_CHERRY_TOP;
            return 104;
        case 107: // Petrified Log
            if (faceIndex == 0 || faceIndex == 1) return TEX_LAYER_LOG_SPRUCE_TOP;
            return 107;
        case 110: // Bamboo Stalk
            if (faceIndex == 0 || faceIndex == 1) return TEX_LAYER_LOG_BAMBOO_TOP;
            return 110;
        case 113: // Palm Log
            if (faceIndex == 0 || faceIndex == 1) return TEX_LAYER_LOG_OAK_TOP;
            return 113;
        case 116: // Astral Log
            if (faceIndex == 0 || faceIndex == 1) return TEX_LAYER_LOG_ASTRAL_TOP;
            return 116;
        case 331: // Crafting Table
            if (faceIndex == 0) return TEX_LAYER_CRAFTING_TOP;
            if (faceIndex == 1) return 72; // Planks bottom
            return 331; // Crafting side with hanging tools
        case 332: // Smelting Furnace
            if (faceIndex == 0) return TEX_LAYER_FURNACE_TOP;
            if (faceIndex == 2 || faceIndex == 3) return 332; // Front arch
            return 37; // Cobblestone sides
        case 337: // Wooden Storage Chest
        case 333:
            if (faceIndex == 0 || faceIndex == 1) return 72; // Planks top and bottom
            return TEX_LAYER_CHEST; // Authentic chest side & latch texture
        case 347: // Bookshelf
            if (faceIndex == 0 || faceIndex == 1) return 72; // Planks top/bottom
            return 347; // Bookshelf side
        case 348: // TNT
            if (faceIndex == 0) return TEX_LAYER_TNT_TOP;
            if (faceIndex == 1) return TEX_LAYER_TNT_BOTTOM;
            return 348;
        default:
            return blockId;
    }
}

class TextureAtlas {
public:
    TextureAtlas();
    ~TextureAtlas();

    bool init();
    void bind(GLuint unit = 0);

    GLuint getTextureID() const { return textureID; }

private:
    void generateAllTextures(std::vector<uint8_t>& pixelData);
    void generateBlockTexture(uint16_t blockId, uint8_t* outPixels);
    void generateMultiFaceTexture(int layerIndex, uint8_t* outPixels);
    void generateStructureTexture(int layerIndex, uint8_t* outPixels);
    void generateCreatureTexture(int layerIndex, uint8_t* outPixels);

    GLuint textureID = 0;
};

} // namespace Aetheria
