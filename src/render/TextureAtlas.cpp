#include "TextureAtlas.hpp"
#include "../world/BlockRegistry.hpp"
#include <cmath>
#include <algorithm>
#include <iostream>

namespace Aetheria {

static inline float pixelHash(int x, int y, int seed) {
    int n = x + y * 57 + seed * 131;
    n = (n << 13) ^ n;
    return 1.0f - ((n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0f;
}

// Seamless wrapping smooth value noise (Cubic Hermite interpolation)
static inline float smoothNoiseWrap(float x, float y, float period, int seed) {
    int p = static_cast<int>(period);
    if (p < 1) p = 16;
    int ix = static_cast<int>(std::floor(x));
    int iy = static_cast<int>(std::floor(y));

    float fx = x - ix;
    float fy = y - iy;

    int x0 = (ix % p + p) % p;
    int y0 = (iy % p + p) % p;
    int x1 = (x0 + 1) % p;
    int y1 = (y0 + 1) % p;

    // Smooth Hermite curve (3t^2 - 2t^3)
    float sx = fx * fx * (3.0f - 2.0f * fx);
    float sy = fy * fy * (3.0f - 2.0f * fy);

    float n00 = pixelHash(x0, y0, seed);
    float n10 = pixelHash(x1, y0, seed);
    float n01 = pixelHash(x0, y1, seed);
    float n11 = pixelHash(x1, y1, seed);

    float nx0 = n00 + sx * (n10 - n00);
    float nx1 = n01 + sx * (n11 - n01);

    return nx0 + sy * (nx1 - nx0);
}

// Multi-octave fractal noise with seamless wrapping
static inline float fbmWrap(float x, float y, int seed, int octaves = 2) {
    float val = 0.0f;
    float amp = 0.65f;
    float freq = 1.0f;
    for (int o = 0; o < octaves; ++o) {
        val += amp * smoothNoiseWrap(x * freq, y * freq, 16.0f, seed + o * 53);
        amp *= 0.5f;
        freq *= 2.0f;
    }
    return val;
}

// Subtle edge ambient shading for genuine 3D block presence
static inline float edgeAO(int x, int y, float strength = 0.06f) {
    float ao = 1.0f;
    if (y == 15) ao -= strength * 1.4f;
    else if (y == 14) ao -= strength * 0.6f;
    if (x == 15) ao -= strength * 1.2f;
    else if (x == 14) ao -= strength * 0.5f;
    if (y == 0) ao += strength * 1.2f;
    if (x == 0) ao += strength * 0.7f;
    return ao;
}

static inline void setPix(uint8_t* p, int x, int y, uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) {
    if (x < 0 || x >= 16 || y < 0 || y >= 16) return;
    int idx = (y * 16 + x) * 4;
    p[idx + 0] = r;
    p[idx + 1] = g;
    p[idx + 2] = b;
    p[idx + 3] = a;
}

static inline uint8_t clampU8(float val) {
    return static_cast<uint8_t>(std::clamp(val, 0.0f, 255.0f));
}

TextureAtlas::TextureAtlas() {}

TextureAtlas::~TextureAtlas() {
    if (textureID) {
        glDeleteTextures(1, &textureID);
        textureID = 0;
    }
}

bool TextureAtlas::init() {
    std::vector<uint8_t> pixelData(TEX_RES * TEX_RES * 4 * TOTAL_LAYERS, 255);
    generateAllTextures(pixelData);

    glGenTextures(1, &textureID);
    if (!textureID) {
        std::cerr << "Failed to generate OpenGL texture ID for TextureAtlas!" << std::endl;
        return false;
    }

    glBindTexture(GL_TEXTURE_2D_ARRAY, textureID);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA8, TEX_RES, TEX_RES, TOTAL_LAYERS, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixelData.data());

    // Generate hardware mipmaps for distance filtering
    if (glGenerateMipmap) {
        glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
    }

    // Authentic Minecraft 16x16 pixel-crisp nearest-neighbor filtering with mipmaps
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Enable Anisotropic Filtering up to 16x
    #ifndef GL_TEXTURE_MAX_ANISOTROPY_EXT
    #define GL_TEXTURE_MAX_ANISOTROPY_EXT 0x84FE
    #endif
    #ifndef GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT
    #define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT 0x84FF
    #endif
    GLfloat maxAniso = 1.0f;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAniso);
    if (maxAniso > 1.0f) {
        glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_ANISOTROPY_EXT, std::min(maxAniso, 16.0f));
    }

    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

    std::cout << "TextureAtlas initialized with " << TOTAL_LAYERS
              << " remastered 16x16 anti-aliased textures with Mipmapping & Anisotropic Filtering ("
              << std::min(maxAniso, 16.0f) << "x)!" << std::endl;
    return true;
}

void TextureAtlas::bind(GLuint unit) {
    if (textureID) {
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_2D_ARRAY, textureID);
    }
}

void TextureAtlas::generateAllTextures(std::vector<uint8_t>& pixelData) {
    // 1. Generate textures for all registered blocks (1 to 369 including saplings)
    for (uint16_t bId = 1; bId <= 369; ++bId) {
        uint8_t* layerPtr = &pixelData[(bId * TEX_RES * TEX_RES) * 4];
        generateBlockTexture(bId, layerPtr);
    }

    // 2. Generate Palworld modular structures
    for (int st = 370; st <= 385; ++st) {
        uint8_t* layerPtr = &pixelData[(st * TEX_RES * TEX_RES) * 4];
        generateStructureTexture(st, layerPtr);
    }

    // 3. Generate Creature skin textures
    for (int cr = 390; cr <= 400; ++cr) {
        uint8_t* layerPtr = &pixelData[(cr * TEX_RES * TEX_RES) * 4];
        generateCreatureTexture(cr, layerPtr);
    }

    // 4. Generate Multi-Face Block Textures (405 to 420)
    for (int mf = 405; mf <= 420; ++mf) {
        uint8_t* layerPtr = &pixelData[(mf * TEX_RES * TEX_RES) * 4];
        generateMultiFaceTexture(mf, layerPtr);
    }
}

// =============================================================================
// REMASTERED GEMSTONES & CRYSTALS (IDs 161 to 205)
// =============================================================================
static void generateRemasteredGemTexture(uint16_t bId, int x, int y, float baseR, float baseG, float baseB, float& r, float& g, float& b) {
    float dx = std::abs(x - 7.5f);
    float dy = std::abs(y - 7.5f);
    float maxDist = std::max(dx, dy);
    float manhattan = dx + dy;

    // 1. Brilliant Cut Faceting & Geometry
    bool isTable = (dx <= 3.2f && dy <= 3.2f && manhattan <= 5.0f);
    bool isMainBevel = (x == y || (x + y == 15));
    bool isCrossRidge = (x == 7 || x == 8 || y == 7 || y == 8);
    bool isGirdleEdge = (x == 0 || x == 15 || y == 0 || y == 15);
    bool isInnerGirdle = (x == 1 || x == 14 || y == 1 || y == 14);

    // Facet Quad Lighting
    int facetSector = 0;
    if (x >= 8 && y < 8) facetSector = 1;
    else if (x >= 8 && y >= 8) facetSector = 2;
    else if (x < 8 && y >= 8) facetSector = 3;

    float facetLight = 1.0f;
    if (isTable) {
        facetLight = 1.38f; // Radiant luminous table core
    } else if (facetSector == 0) {
        facetLight = 1.24f; // Top-Left incident highlight
    } else if (facetSector == 1) {
        facetLight = 1.08f; // Top-Right secondary
    } else if (facetSector == 2) {
        facetLight = 0.68f; // Bottom-Right deep refraction shadow
    } else {
        facetLight = 0.88f; // Bottom-Left ambient catch
    }

    // Alternating triangular star kite facets
    if ((x * 3 + y * 5) % 4 == 0) facetLight *= 1.18f;
    else if ((x * 5 + y * 3) % 4 == 1) facetLight *= 0.86f;

    if (isMainBevel || isCrossRidge) {
        facetLight *= 1.32f; // Razor-sharp facet ridge
    }
    if (isGirdleEdge) {
        facetLight *= 0.62f; // Deep chiseled setting rim
    } else if (isInnerGirdle) {
        facetLight *= 1.12f;
    }

    r = baseR * facetLight;
    g = baseG * facetLight;
    b = baseB * facetLight;

    // Gem-Specific Remastered Effects
    if (bId == 161) { // Diamond: High-dispersion white-cyan brilliance
        float prism = ((x + y) % 3 == 0) ? 35.0f : 0.0f;
        r = std::min(r * 1.15f + prism, 255.0f);
        g = std::min(g * 1.18f + prism, 255.0f);
        b = std::min(b * 1.25f + prism, 255.0f);
    } else if (bId == 162) { // Emerald: Concentric step-cut facets
        int step = static_cast<int>(maxDist);
        float stepShade = (step % 2 == 0) ? 1.25f : 0.80f;
        r *= stepShade * 0.7f; g *= stepShade * 1.25f; b *= stepShade * 0.85f;
    } else if (bId == 167) { // Opal: Iridescent Spectral Rainbow Dispersion
        float hueAngle = (x * 0.45f + y * 0.65f);
        r = 175.0f + std::sin(hueAngle) * 75.0f;
        g = 185.0f + std::sin(hueAngle + 2.09f) * 65.0f;
        b = 210.0f + std::sin(hueAngle + 4.18f) * 45.0f;
        if (isTable) { r += 45.0f; g += 45.0f; b += 45.0f; }
    } else if (bId == 168) { // Amber: Translucent honey resin with inclusion
        if (x >= 6 && x <= 8 && y >= 6 && y <= 8) {
            r = 70.0f; g = 30.0f; b = 10.0f; // Ancient fossil insect inclusion
        }
    } else if (bId == 170) { // Onyx: Jet-black mirror with white specular facet lines
        float blackBase = 18.0f + (15.0f - maxDist) * 3.5f;
        r = blackBase; g = blackBase; b = blackBase + 6.0f;
        if (isMainBevel || isTable) { r += 130.0f; g += 130.0f; b += 140.0f; }
    } else if (bId == 172) { // Tourmaline: Watermelon dual chromatic gradient
        float tGrad = y / 15.0f;
        r = baseR * (1.3f - tGrad * 0.8f);
        g = baseG * (0.3f + tGrad * 1.4f);
        b = baseB * 0.8f;
    }

    // 4-Point Specular Star Glint at (4,4)
    int gx = 4, gy = 4;
    if (x == gx && y == gy) {
        r = 255.0f; g = 255.0f; b = 255.0f; // Pure white star center
    } else if ((std::abs(x - gx) <= 1 && y == gy) || (std::abs(y - gy) <= 1 && x == gx)) {
        r = std::min(r * 1.5f + 160.0f, 255.0f);
        g = std::min(g * 1.5f + 160.0f, 255.0f);
        b = std::min(b * 1.5f + 175.0f, 255.0f);
    }

    // Secondary sparkle at (11,10)
    if (x == 11 && y == 10) {
        r = 250.0f; g = 250.0f; b = 255.0f;
    } else if ((x == 10 && y == 10) || (x == 12 && y == 10) || (x == 11 && y == 9) || (x == 11 && y == 11)) {
        r = std::min(r + 75.0f, 255.0f);
        g = std::min(g + 75.0f, 255.0f);
        b = std::min(b + 90.0f, 255.0f);
    }
}

static void generateElementalCrystalTexture(uint16_t bId, int x, int y, float baseR, float baseG, float baseB, float& r, float& g, float& b) {
    float dx = (x - 7.5f);
    float dy = (y - 7.5f);
    float dist = std::sqrt(dx * dx + dy * dy);
    float angle = std::atan2(dy, dx);

    int idx = bId - 186; // 0..19
    float spikes = std::cos(angle * 6.0f) * 2.0f;
    float coreFactor = std::clamp(1.0f - (dist - spikes) / 8.5f, 0.0f, 1.0f);
    float glow = 0.65f + coreFactor * 0.95f;

    r = baseR; g = baseG; b = baseB;

    switch (idx) {
    case 0: // Flame: rising tongues
        if (y < 8 + (x % 3) * 2) { r = 255.0f; g = 190.0f + (7 - y) * 9.0f; b = 40.0f; }
        break;
    case 1: // Frost: 6-pointed snowflake
        if (std::abs(dx) <= 1.0f || std::abs(dy) <= 1.0f || std::abs(dx - dy) <= 1.0f || std::abs(dx + dy) <= 1.0f) {
            r = 240.0f; g = 252.0f; b = 255.0f;
        }
        break;
    case 2: // Tempest: electric zigzag
        if (std::abs(y - (x + ((x % 3 == 0) ? 2 : -1))) <= 1) {
            r = 255.0f; g = 255.0f; b = 180.0f;
        }
        break;
    case 4: // Radiant Light: solar cross
        if (dist < 3.2f || std::abs(dx) == 0.0f || std::abs(dy) == 0.0f) {
            r = 255.0f; g = 255.0f; b = 235.0f;
        }
        break;
    case 5: // Shadow Void: black hole center with violet corona
        if (dist < 3.5f) { r = 12.0f; g = 2.0f; b = 20.0f; }
        else if (dist < 7.0f) { r = 170.0f; g = 40.0f; b = 245.0f; }
        break;
    case 6: // Arcane Mana: concentric circles
        if (std::abs(dist - 4.2f) < 1.0f || std::abs(dist - 6.8f) < 0.8f) {
            r = 240.0f; g = 130.0f; b = 255.0f;
        }
        break;
    case 15: // Prismatic: full rainbow dispersion
        r = 145.0f + std::sin(angle * 2.0f) * 110.0f;
        g = 145.0f + std::sin(angle * 2.0f + 2.09f) * 110.0f;
        b = 145.0f + std::sin(angle * 2.0f + 4.18f) * 110.0f;
        break;
    default:
        if (std::abs(dx) == std::abs(dy) || x == 7 || y == 7) {
            r = std::min(r * 1.4f + 45.0f, 255.0f);
            g = std::min(g * 1.4f + 45.0f, 255.0f);
            b = std::min(b * 1.4f + 55.0f, 255.0f);
        }
        break;
    }

    r *= glow; g *= glow; b *= glow;
}

// =============================================================================
// AUTHENTIC MINECRAFT VANILLA BLOCK TEXTURES (IDs 1 to 365)
// =============================================================================

// Classic Minecraft Cobblestone boulder matrix
static const uint8_t mcCobbleMatrix[16][16] = {
    {2,2,3,1,0,0,1,2,2,3,1,0,0,1,2,2},
    {2,3,4,2,1,0,1,3,4,3,2,1,0,2,3,2},
    {1,2,3,2,1,0,0,2,3,2,1,0,0,1,2,1},
    {0,1,1,1,0,0,0,1,1,1,0,0,1,1,1,0},
    {0,0,0,0,1,2,3,2,1,0,1,2,3,2,1,0},
    {1,2,3,1,2,3,4,3,2,1,2,4,4,3,2,1},
    {2,3,4,2,2,3,3,2,1,0,1,3,4,3,2,1},
    {1,2,3,1,1,2,1,0,0,0,0,1,2,2,1,0},
    {0,1,1,0,0,0,0,1,2,3,1,0,0,1,1,0},
    {0,0,1,2,3,2,1,2,4,4,2,1,0,0,0,0},
    {1,2,3,4,4,3,2,3,4,3,2,1,1,2,2,1},
    {2,3,4,3,3,2,1,2,3,2,1,0,2,3,3,2},
    {1,2,2,1,1,0,0,1,1,1,0,0,1,2,2,1},
    {0,1,1,0,0,1,2,3,2,1,0,0,0,1,1,0},
    {0,0,0,1,2,3,4,4,3,2,1,1,0,0,0,0},
    {1,1,2,3,4,4,3,3,2,1,2,2,1,1,1,1}
};

// Fill with Stone (ID 36) base - Smooth cloudy Jappa Minecraft stone with subtle micro-grit
static void fillMinecraftStone(uint8_t* out, int seed = 36) {
    for (int y = 0; y < 16; ++y) {
        for (int x = 0; x < 16; ++x) {
            float smooth = fbmWrap(x * 0.22f, y * 0.22f, seed, 2);
            float grit = pixelHash(x, y, seed + 100) * 0.10f;
            float val = smooth * 0.88f + grit;
            float ao = edgeAO(x, y, 0.04f);

            uint8_t g;
            if (val > 0.38f) g = 142;
            else if (val > 0.08f) g = 128;
            else if (val > -0.22f) g = 116;
            else if (val > -0.55f) g = 104;
            else g = 92;

            g = clampU8(g * ao);
            setPix(out, x, y, g, g, g);
        }
    }
}

// Fill with Deepslate base - Smooth horizontal slate strata
static void fillMinecraftDeepslate(uint8_t* out, int seed = 45) {
    for (int y = 0; y < 16; ++y) {
        for (int x = 0; x < 16; ++x) {
            float wave = std::sin(y * 0.85f + smoothNoiseWrap(x * 0.35f, y * 0.2f, 16.0f, seed) * 1.4f);
            float smooth = fbmWrap(x * 0.25f, y * 0.25f, seed + 10, 2);
            float val = wave * 0.5f + smooth * 0.5f;
            uint8_t g;
            if (val > 0.40f) g = 74;
            else if (val > 0.05f) g = 60;
            else if (val > -0.35f) g = 48;
            else g = 36;
            setPix(out, x, y, g, g, g + 3);
        }
    }
}

// Fill with 4-board Wooden Planks - Smooth horizontal wood grain with board bevels
static void fillMinecraftPlanks(uint8_t* out, const Vec4& mainCol, const Vec4& seamCol, const Vec4& hiCol, int seed = 72) {
    for (int y = 0; y < 16; ++y) {
        int boardIdx = y / 4;
        bool isSeam = (y % 4 == 3);
        bool isHi = (y % 4 == 0);

        for (int x = 0; x < 16; ++x) {
            bool isNail = ((boardIdx % 2 == 0 && (x == 2 || x == 14)) || (boardIdx % 2 == 1 && (x == 1 || x == 13))) && (y % 4 == 1);
            float grain = smoothNoiseWrap(x * 0.6f, y * 0.15f, 16.0f, seed + boardIdx * 19);
            float wn = grain * 8.0f;

            if (isSeam) {
                setPix(out, x, y, clampU8(seamCol.x * 255.0f + wn * 0.4f),
                                  clampU8(seamCol.y * 255.0f + wn * 0.4f),
                                  clampU8(seamCol.z * 255.0f + wn * 0.4f));
            } else if (isHi) {
                setPix(out, x, y, clampU8(hiCol.x * 255.0f + wn * 0.6f),
                                  clampU8(hiCol.y * 255.0f + wn * 0.6f),
                                  clampU8(hiCol.z * 255.0f + wn * 0.6f));
            } else if (isNail) {
                setPix(out, x, y, 52, 48, 46); // Dark iron nail
            } else {
                setPix(out, x, y, clampU8(mainCol.x * 255.0f + wn),
                                  clampU8(mainCol.y * 255.0f + wn),
                                  clampU8(mainCol.z * 255.0f + wn));
            }
        }
    }
}

// =============================================================================
// MINECRAFT STONE BRICKS SUITE (IDs 241 to 244)
// =============================================================================
static void fillMinecraftStoneBricks(uint8_t* out, int variant, int seed = 241) {
    // 1. Generate base 2x2 running bond ashlar stone bricks
    for (int y = 0; y < 16; ++y) {
        int row = y / 8; // Row 0 (0..7), Row 1 (8..15)
        bool isHorizMortar = (y == 7 || y == 15);
        bool isHorizBevelTop = (y == 0 || y == 8);
        bool isHorizBevelBot = (y == 6 || y == 14);

        for (int x = 0; x < 16; ++x) {
            bool isVertMortar = false;
            bool isVertBevelLeft = false;
            bool isVertBevelRight = false;

            if (row == 0) {
                isVertMortar = (x == 7 || x == 15);
                isVertBevelLeft = (x == 0 || x == 8);
                isVertBevelRight = (x == 6 || x == 14);
            } else {
                // Staggered by 4 pixels (running bond)
                isVertMortar = (x == 3 || x == 11);
                isVertBevelLeft = (x == 4 || x == 12 || x == 0);
                isVertBevelRight = (x == 2 || x == 10 || x == 15);
            }

            bool isMortar = isHorizMortar || isVertMortar;

            // Cloudy stone base
            float smooth = fbmWrap(x * 0.22f, y * 0.22f, seed + row * 17, 2);
            float grit = pixelHash(x, y, seed) * 0.08f;
            float val = smooth * 0.85f + grit;

            uint8_t g;
            if (isMortar) {
                g = 52; // Deep dark mortar groove
            } else if (isHorizBevelTop || isVertBevelLeft) {
                // Top/Left sunlit bevel highlight
                g = clampU8(148 + val * 18.0f);
            } else if (isHorizBevelBot || isVertBevelRight) {
                // Bottom/Right shadow bevel
                g = clampU8(96 + val * 16.0f);
            } else {
                // Main brick face
                if (val > 0.35f) g = 138;
                else if (val > 0.0f) g = 124;
                else if (val > -0.35f) g = 112;
                else g = 100;
            }

            setPix(out, x, y, g, g, g);
        }
    }

    // 2. Variants
    if (variant == 1) { // Mossy Stone Bricks
        // Creeping lush green moss tendrils across mortar and bricks
        for (int y = 0; y < 16; ++y) {
            for (int x = 0; x < 16; ++x) {
                float mn = smoothNoiseWrap(x * 0.35f, y * 0.35f, 16.0f, 242);
                bool isMossPatch = (mn > 0.15f && (y >= 4 && y <= 11 && x >= 2 && x <= 9)) ||
                                   (mn > 0.25f && (y >= 10 && x >= 8)) ||
                                   (mn > 0.05f && (y == 7 && x >= 3 && x <= 12));
                if (isMossPatch) {
                    if (mn > 0.35f) setPix(out, x, y, 122, 185, 48);      // Vivid lime moss
                    else if (mn > 0.20f) setPix(out, x, y, 84, 142, 38);  // Mid green
                    else setPix(out, x, y, 52, 95, 26);                   // Deep dark moss
                }
            }
        }
    } else if (variant == 2) { // Cracked Stone Bricks
        // Branching fracture cracks
        auto isCrack = [](int x, int y) -> int {
            // Main diagonal fracture
            if ((x == 4 && y == 0) || (x == 5 && (y >= 1 && y <= 2)) || (x == 6 && (y >= 3 && y <= 4)) ||
                (x == 7 && (y >= 5 && y <= 7)) || (x == 6 && (y >= 8 && y <= 9)) ||
                (x == 5 && (y >= 10 && y <= 12)) || (x == 6 && (y >= 13 && y <= 15))) return 1;
            // Branch
            if ((y == 5 && x == 8) || (y == 6 && (x >= 9 && x <= 10)) || (y == 7 && (x >= 11 && x <= 13))) return 1;
            // Highlight pixel adjacent to crack
            if ((x == 6 && y == 1) || (x == 7 && y == 3) || (x == 7 && y == 9) || (x == 6 && y == 11)) return 2;
            return 0;
        };

        for (int y = 0; y < 16; ++y) {
            for (int x = 0; x < 16; ++x) {
                int c = isCrack(x, y);
                if (c == 1) setPix(out, x, y, 35, 35, 38); // Void fissure shadow
                else if (c == 2) setPix(out, x, y, 168, 168, 172); // Chipped stone highlight
            }
        }
    } else if (variant == 3) { // Chiseled Crypt Stone
        // Concentric square relief framing an inner raised boss
        for (int y = 0; y < 16; ++y) {
            for (int x = 0; x < 16; ++x) {
                bool isBorder = (x == 0 || x == 15 || y == 0 || y == 15);
                bool isGroove = (x == 2 || x == 13 || y == 2 || y == 13);
                bool isInnerBorder = (x == 3 || x == 12 || y == 3 || y == 12);
                bool isCenterBoss = (x >= 6 && x <= 9 && y >= 6 && y <= 9);
                bool isCenterCore = (x >= 7 && x <= 8 && y >= 7 && y <= 8);

                if (isBorder) setPix(out, x, y, 145, 145, 150);
                else if (isGroove) setPix(out, x, y, 42, 42, 45); // Recessed shadow groove
                else if (isInnerBorder) setPix(out, x, y, 135, 135, 140);
                else if (isCenterCore) setPix(out, x, y, 165, 165, 172); // Raised boss medallion
                else if (isCenterBoss) setPix(out, x, y, 115, 115, 120);
                else setPix(out, x, y, 100, 100, 105);
            }
        }
    }
}

// =============================================================================
// MINECRAFT GLOWSTONE & SEA LANTERN LUMINARIES
// =============================================================================
static void fillMinecraftGlowstone(uint8_t* out) {
    // Golden amber crystalline clusters with incandescent core
    for (int y = 0; y < 16; ++y) {
        for (int x = 0; x < 16; ++x) {
            float n1 = smoothNoiseWrap(x * 0.45f, y * 0.45f, 16.0f, 344);
            float n2 = smoothNoiseWrap(x * 0.9f, y * 0.9f, 16.0f, 345);
            float val = n1 * 0.65f + n2 * 0.35f;

            bool isSeam = (val < -0.35f) || (x == 4 && y >= 2 && y <= 7) || (x == 11 && y >= 8 && y <= 13) || (y == 8 && x >= 4 && x <= 11);
            bool isBrightCore = (val > 0.42f);
            bool isHotSpot = (val > 0.65f) || (x == 7 && y == 5) || (x == 8 && y == 11) || (x == 12 && y == 4) || (x == 3 && y == 12);

            if (isHotSpot) {
                setPix(out, x, y, 255, 255, 195); // Incandescent white-yellow sparkle
            } else if (isBrightCore) {
                setPix(out, x, y, 255, 225, 75);  // Brilliant gold
            } else if (isSeam) {
                setPix(out, x, y, 135, 75, 18);   // Dark amber crevice
            } else if (val > 0.05f) {
                setPix(out, x, y, 242, 180, 42);  // Rich amber gold
            } else {
                setPix(out, x, y, 198, 130, 26);  // Deep golden orange
            }
        }
    }
}

static void fillMinecraftSeaLantern(uint8_t* out) {
    // Luminous aquamarine tile with concentric frame lines and cyan glints
    for (int y = 0; y < 16; ++y) {
        for (int x = 0; x < 16; ++x) {
            bool isBorder = (x == 0 || x == 15 || y == 0 || y == 15);
            bool isInnerFrame = (x == 2 || x == 13 || y == 2 || y == 13) ||
                                (x == 5 || x == 10 || y == 5 || y == 10);
            float n = smoothNoiseWrap(x * 0.4f, y * 0.4f, 16.0f, 257);

            if (isBorder) {
                setPix(out, x, y, 85, 155, 148); // Aquamarine seam
            } else if (isInnerFrame) {
                setPix(out, x, y, 135, 205, 195); // Subtle structural frame
            } else {
                // Luminous cyan-white glowing body
                if (n > 0.35f || (x >= 7 && x <= 8 && y >= 7 && y <= 8)) {
                    setPix(out, x, y, 235, 255, 252); // Luminous white-cyan core
                } else if (n > 0.0f) {
                    setPix(out, x, y, 185, 238, 232); // Soft cyan
                } else {
                    setPix(out, x, y, 152, 218, 210); // Pale seafoam
                }
            }
        }
    }
}

// =============================================================================
// MODERN MINECRAFT SIGNATURE ORE MASKS & PIXEL ART
// =============================================================================
static void fillMinecraftOre(uint8_t* out, int oreType, bool isDeepslate, float baseR, float baseG, float baseB) {
    if (isDeepslate) fillMinecraftDeepslate(out, 45);
    else fillMinecraftStone(out, 36);

    // Modern Minecraft 1.17+ Unique Shape Matrices:
    // level: 0 = stone, 1 = dark rim / outline, 2 = body, 3 = glint / highlight, 4 = special (patina / pyrite)
    auto getOrePixel = [oreType](int x, int y) -> int {
        switch (oreType) {
        case 121: { // Coal Ore: Chunky rounded spots
            bool inSpot1 = (x >= 2 && x <= 5 && y >= 2 && y <= 5 && !(x == 2 && y == 2) && !(x == 5 && y == 5));
            bool inSpot2 = (x >= 9 && x <= 13 && y >= 3 && y <= 7 && !(x == 9 && y == 3) && !(x == 13 && y == 7));
            bool inSpot3 = (x >= 3 && x <= 7 && y >= 9 && y <= 13 && !(x == 3 && y == 13) && !(x == 7 && y == 9));
            bool inSpot4 = (x >= 10 && x <= 14 && y >= 10 && y <= 14 && !(x == 10 && y == 10) && !(x == 14 && y == 14));
            if (!inSpot1 && !inSpot2 && !inSpot3 && !inSpot4) return 0;
            if ((x == 3 && y == 3) || (x == 11 && y == 4) || (x == 5 && y == 10) || (x == 12 && y == 11)) return 3; // Glint
            if ((x >= 3 && x <= 4 && y >= 3 && y <= 4) || (x >= 10 && x <= 12 && y >= 4 && y <= 6) ||
                (x >= 4 && x <= 6 && y >= 10 && y <= 12) || (x >= 11 && x <= 13 && y >= 11 && y <= 13)) return 2; // Body
            return 1; // Rim
        }
        case 122: { // Copper Ore: Teardrop nuggets with oxidized turquoise patina
            // Patina verdigris spots
            if ((x == 3 && y == 6) || (x == 4 && y == 7) || (x == 10 && y == 8) || (x == 11 && y == 8) || (x == 13 && y == 13)) return 4;
            // Droplet 1
            if (x >= 3 && x <= 6 && y >= 3 && y <= 6) {
                if (x == 4 && y == 4) return 3;
                if (x >= 4 && x <= 5 && y >= 4 && y <= 5) return 2;
                return 1;
            }
            // Droplet 2
            if (x >= 9 && x <= 13 && y >= 9 && y <= 13 && !(x == 13 && y == 9)) {
                if (x == 11 && y == 10) return 3;
                if (x >= 10 && x <= 12 && y >= 10 && y <= 12) return 2;
                return 1;
            }
            // Small nugget 3
            if (x >= 10 && x <= 12 && y >= 2 && y <= 4) {
                if (x == 11 && y == 3) return 3;
                return 2;
            }
            return 0;
        }
        case 124: { // Iron Ore: Iconic diagonal stepped streaks
            // Streak 1 (bottom-left to center)
            if ((x == 2 && y == 13) || (x == 3 && (y == 12 || y == 11)) || (x == 4 && (y == 11 || y == 10)) ||
                (x == 5 && (y == 10 || y == 9)) || (x == 6 && (y == 8 || y == 7)) || (x == 7 && (y == 7 || y == 6))) {
                if ((x == 3 && y == 11) || (x == 5 && y == 9) || (x == 6 && y == 7)) return 3;
                return 2;
            }
            if ((x == 1 && y == 13) || (x == 2 && y == 12) || (x == 4 && y == 9) || (x == 7 && y == 5) || (x == 8 && y == 6)) return 1;
            // Streak 2 (center to top-right)
            if ((x == 8 && y == 11) || (x == 9 && (y == 10 || y == 9)) || (x == 10 && (y == 9 || y == 8)) ||
                (x == 11 && (y == 8 || y == 7)) || (x == 12 && (y == 6 || y == 5)) || (x == 13 && (y == 5 || y == 4))) {
                if ((x == 9 && y == 10) || (x == 11 && y == 7) || (x == 12 && y == 5)) return 3;
                return 2;
            }
            if ((x == 8 && y == 12) || (x == 10 && y == 10) || (x == 13 && y == 6) || (x == 14 && y == 4)) return 1;
            // Cluster 3
            if (x >= 3 && x <= 5 && y >= 3 && y <= 5) {
                if (x == 4 && y == 4) return 3;
                return 2;
            }
            return 0;
        }
        case 126: { // Gold Ore: Scattered sparkling nuggets across rock
            if ((x == 4 && y == 4) || (x == 12 && y == 3) || (x == 8 && y == 8) || (x == 3 && y == 12) || (x == 12 && y == 11)) return 3;
            if ((x >= 3 && x <= 5 && y >= 3 && y <= 5) ||
                (x >= 11 && x <= 13 && y >= 2 && y <= 4) ||
                (x >= 7 && x <= 9 && y >= 7 && y <= 9) ||
                (x >= 2 && x <= 4 && y >= 11 && y <= 13) ||
                (x >= 11 && x <= 13 && y >= 10 && y <= 12)) return 2;
            if ((x == 2 && y == 4) || (x == 6 && y == 4) || (x == 10 && y == 3) || (x == 14 && y == 3) ||
                (x == 6 && y == 8) || (x == 10 && y == 8) || (x == 1 && y == 12) || (x == 5 && y == 12)) return 1;
            return 0;
        }
        case 128: { // Cobalt / Lapis Ore: Jagged vein with gold pyrite specks
            // Pyrite flecks
            if ((x == 5 && y == 9) || (x == 10 && y == 6)) return 4;
            // Jagged vein
            if ((x == 3 && (y == 12 || y == 11)) || (x == 4 && (y == 11 || y == 10)) ||
                (x == 5 && (y == 10 || y == 9))  || (x == 6 && (y == 9 || y == 8)) ||
                (x == 7 && (y == 8 || y == 7))   || (x == 8 && (y == 8 || y == 7)) ||
                (x == 9 && (y == 7 || y == 6))   || (x == 10 && (y == 6 || y == 5)) ||
                (x == 11 && (y == 5 || y == 4))  || (x == 12 && (y == 4 || y == 3))) {
                if ((x == 4 && y == 10) || (x == 7 && y == 8) || (x == 9 && y == 6) || (x == 11 && y == 4)) return 3;
                return 2;
            }
            if ((x == 2 && y == 12) || (x == 5 && y == 11) || (x == 8 && y == 9) || (x == 11 && y == 6) || (x == 13 && y == 3)) return 1;
            return 0;
        }
        case 130: { // Redstone / Adamantite: Dense glowing crystalline clusters
            if ((x == 4 && y == 3) || (x == 11 && y == 4) || (x == 5 && y == 10) || (x == 12 && y == 11)) return 3;
            if ((x >= 3 && x <= 5 && y >= 3 && y <= 5 && !(x == 3 && y == 3) && !(x == 5 && y == 5)) ||
                (x >= 10 && x <= 12 && y >= 3 && y <= 5 && !(x == 10 && y == 5) && !(x == 12 && y == 3)) ||
                (x >= 4 && x <= 6 && y >= 9 && y <= 11 && !(x == 4 && y == 11) && !(x == 6 && y == 9)) ||
                (x >= 11 && x <= 13 && y >= 10 && y <= 12 && !(x == 11 && y == 10) && !(x == 13 && y == 12))) return 2;
            if ((x == 2 && y == 4) || (x == 6 && y == 4) || (x == 9 && y == 4) || (x == 13 && y == 4) ||
                (x == 3 && y == 10) || (x == 7 && y == 10) || (x == 10 && y == 11) || (x == 14 && y == 11)) return 1;
            return 0;
        }
        case 129: { // Mythril / Diamond Cyan: Radiant crystal rifts
            if ((x == 4 && y == 4) || (x == 11 && y == 4) || (x == 6 && y == 9) || (x == 12 && y == 12)) return 3;
            if ((x >= 3 && x <= 5 && y >= 3 && y <= 5) || (x >= 10 && x <= 12 && y >= 3 && y <= 5) ||
                (x >= 5 && x <= 7 && y >= 8 && y <= 10) || (x >= 11 && x <= 13 && y >= 11 && y <= 13)) return 2;
            if ((x == 2 && y == 4) || (x == 6 && y == 4) || (x == 9 && y == 4) || (x == 13 && y == 4) ||
                (x == 4 && y == 9) || (x == 8 && y == 9) || (x == 10 && y == 12) || (x == 14 && y == 12)) return 1;
            return 0;
        }
        default: { // Other ores: Classic Minecraft cluster shape
            if ((x == 4 && y == 3) || (x == 11 && y == 5) || (x == 5 && y == 10) || (x == 12 && y == 12)) return 3;
            if ((x >= 3 && x <= 5 && y >= 2 && y <= 4) || (x >= 10 && x <= 12 && y >= 4 && y <= 6) ||
                (x >= 4 && x <= 6 && y >= 9 && y <= 11) || (x >= 11 && x <= 13 && y >= 11 && y <= 13)) return 2;
            if ((x == 2 && y == 3) || (x == 6 && y == 3) || (x == 9 && y == 5) || (x == 13 && y == 5) ||
                (x == 3 && y == 10) || (x == 7 && y == 10) || (x == 10 && y == 12) || (x == 14 && y == 12)) return 1;
            return 0;
        }
        }
    };

    for (int y = 0; y < 16; ++y) {
        for (int x = 0; x < 16; ++x) {
            int lvl = getOrePixel(x, y);
            if (lvl == 0) continue;

            switch (oreType) {
            case 121: // Coal Ore
                if (lvl == 3) setPix(out, x, y, 76, 76, 82);
                else if (lvl == 2) setPix(out, x, y, 44, 46, 50);
                else setPix(out, x, y, 22, 22, 24);
                break;
            case 122: // Copper Ore
                if (lvl == 4) setPix(out, x, y, 92, 198, 165); // Patina turquoise
                else if (lvl == 3) setPix(out, x, y, 248, 155, 95);
                else if (lvl == 2) setPix(out, x, y, 215, 115, 65);
                else setPix(out, x, y, 165, 80, 42);
                break;
            case 123: // Tin Ore
                if (lvl == 3) setPix(out, x, y, 235, 235, 242);
                else if (lvl == 2) setPix(out, x, y, 195, 195, 205);
                else setPix(out, x, y, 145, 145, 155);
                break;
            case 124: // Iron Ore
                if (lvl == 3) setPix(out, x, y, 248, 208, 185);
                else if (lvl == 2) setPix(out, x, y, 218, 175, 145);
                else setPix(out, x, y, 165, 122, 98);
                break;
            case 125: // Silver Ore
                if (lvl == 3) setPix(out, x, y, 255, 255, 255);
                else if (lvl == 2) setPix(out, x, y, 230, 235, 245);
                else setPix(out, x, y, 175, 180, 195);
                break;
            case 126: // Gold Ore
                if (lvl == 3) setPix(out, x, y, 255, 252, 160);
                else if (lvl == 2) setPix(out, x, y, 252, 220, 55);
                else setPix(out, x, y, 195, 145, 25);
                break;
            case 127: // Platinum Ore
                if (lvl == 3) setPix(out, x, y, 255, 255, 255);
                else if (lvl == 2) setPix(out, x, y, 218, 230, 242);
                else setPix(out, x, y, 165, 180, 198);
                break;
            case 128: // Cobalt / Lapis Ore
                if (lvl == 4) setPix(out, x, y, 248, 210, 55); // Pyrite gold fleck
                else if (lvl == 3) setPix(out, x, y, 75, 135, 255);
                else if (lvl == 2) setPix(out, x, y, 35, 80, 215);
                else setPix(out, x, y, 18, 45, 130);
                break;
            case 129: // Mythril / Diamond-Cyan Ore
                if (lvl == 3) setPix(out, x, y, 245, 255, 255);
                else if (lvl == 2) setPix(out, x, y, 75, 235, 245);
                else setPix(out, x, y, 25, 135, 165);
                break;
            case 130: // Adamantite / Redstone Ore
                if (lvl == 3) setPix(out, x, y, 255, 145, 165);
                else if (lvl == 2) setPix(out, x, y, 230, 32, 48);
                else setPix(out, x, y, 142, 16, 26);
                break;
            default:
                if (lvl == 3) setPix(out, x, y, 255, 255, 255);
                else if (lvl == 2) setPix(out, x, y, clampU8(baseR * 1.3f), clampU8(baseG * 1.3f), clampU8(baseB * 1.3f));
                else setPix(out, x, y, clampU8(baseR * 0.75f), clampU8(baseG * 0.75f), clampU8(baseB * 0.75f));
                break;
            }
        }
    }
}

void TextureAtlas::generateBlockTexture(uint16_t bId, uint8_t* out) {
    const BlockDef& def = BlockRegistry::get(bId);
    float baseR = def.color.x * 255.0f;
    float baseG = def.color.y * 255.0f;
    float baseB = def.color.z * 255.0f;

    // Remastered Gemstones (161 to 175)
    if (bId >= 161 && bId <= 175) {
        for (int y = 0; y < 16; ++y) {
            for (int x = 0; x < 16; ++x) {
                float r, g, b;
                generateRemasteredGemTexture(bId, x, y, baseR, baseG, baseB, r, g, b);
                setPix(out, x, y, clampU8(r), clampU8(g), clampU8(b), 255);
            }
        }
        return;
    }

    // Remastered Gem Clusters (176 to 185)
    if (bId >= 176 && bId <= 185) {
        for (int y = 0; y < 16; ++y) {
            for (int x = 0; x < 16; ++x) {
                float dx = std::abs(x - 7.5f);
                float dy = (15.0f - y);
                bool isSpike = (dx < (dy * 0.45f));
                float r, g, b;
                if (isSpike) {
                    float facet = (x % 2 == 0) ? 1.35f : 0.85f;
                    r = baseR * facet; g = baseG * facet; b = baseB * facet;
                    if (y <= 3) { r = 255.0f; g = 255.0f; b = 255.0f; }
                } else {
                    float rockN = pixelHash(x, y, bId) * 0.15f;
                    r = 95.0f * (0.9f + rockN); g = 95.0f * (0.9f + rockN); b = 105.0f * (0.9f + rockN);
                }
                setPix(out, x, y, clampU8(r), clampU8(g), clampU8(b), 255);
            }
        }
        return;
    }

    // Remastered Elemental Crystals (186 to 205)
    if (bId >= 186 && bId <= 205) {
        for (int y = 0; y < 16; ++y) {
            for (int x = 0; x < 16; ++x) {
                float r, g, b;
                generateElementalCrystalTexture(bId, x, y, baseR, baseG, baseB, r, g, b);
                setPix(out, x, y, clampU8(r), clampU8(g), clampU8(b), 255);
            }
        }
        return;
    }

    // =========================================================================
    // 1. TERRAIN & SOILS (1 to 35)
    // =========================================================================
    if (bId == 1) { // Grass Block (Side Face) - Authentic jagged grass overhang on dirt
        static const int grassOverhang[16] = {3, 2, 4, 3, 2, 4, 3, 2, 3, 4, 2, 3, 4, 3, 2, 3};
        for (int y = 0; y < 16; ++y) {
            for (int x = 0; x < 16; ++x) {
                if (y < grassOverhang[x]) {
                    float gn = smoothNoiseWrap(x * 0.5f, y * 0.5f, 16.0f, 101);
                    if (y == 0 && gn > 0.2f) setPix(out, x, y, 138, 198, 68); // Lime highlight
                    else if (gn > 0.0f) setPix(out, x, y, 114, 172, 58);      // Mid lush green
                    else if (gn > -0.4f) setPix(out, x, y, 90, 144, 46);      // Deep grass green
                    else setPix(out, x, y, 68, 116, 34);                       // Dark grass shadow
                } else {
                    bool isShadow = (y == grassOverhang[x]); // Soft cast shadow directly under grass fringe
                    float dn = smoothNoiseWrap(x * 0.35f, y * 0.35f, 16.0f, 202);
                    uint8_t r, g, b;
                    if (dn > 0.35f)      { r = 138; g = 100; b = 64; } // Light earth clump
                    else if (dn > 0.0f)  { r = 118; g = 85;  b = 54; } // Main brown dirt
                    else if (dn > -0.4f) { r = 98;  g = 68;  b = 43; } // Dark dirt
                    else                 { r = 78;  g = 54;  b = 34; } // Crevice shadow

                    if (isShadow) {
                        r = clampU8(r * 0.75f);
                        g = clampU8(g * 0.75f);
                        b = clampU8(b * 0.75f);
                    }
                    setPix(out, x, y, r, g, b);
                }
            }
        }
        return;
    }
    else if (bId == 2) { // Dirt - Authentic Minecraft smooth dirt clumping
        for (int y = 0; y < 16; ++y) {
            for (int x = 0; x < 16; ++x) {
                float dn = smoothNoiseWrap(x * 0.35f, y * 0.35f, 16.0f, 202) * 0.7f +
                           smoothNoiseWrap(x * 0.7f, y * 0.7f, 16.0f, 203) * 0.3f;
                if (dn > 0.40f) setPix(out, x, y, 142, 104, 66);
                else if (dn > 0.10f) setPix(out, x, y, 122, 88, 56);
                else if (dn > -0.25f) setPix(out, x, y, 102, 72, 46);
                else if (dn > -0.60f) setPix(out, x, y, 82, 58, 36);
                else setPix(out, x, y, 66, 46, 28);
            }
        }
        return;
    }
    else if (bId == 3) { // Coarse Dirt - Dirt with stone pebble bits
        for (int y = 0; y < 16; ++y) {
            for (int x = 0; x < 16; ++x) {
                float dn = smoothNoiseWrap(x * 0.35f, y * 0.35f, 16.0f, 303);
                bool isPebble = (x == 5 && y == 3) || (x == 6 && y == 3) ||
                                (x == 11 && y == 9) || (x == 12 && y == 9) ||
                                (x == 2 && y == 12);
                if (isPebble) setPix(out, x, y, 120, 115, 110);
                else if (dn > 0.25f) setPix(out, x, y, 122, 88, 56);
                else if (dn > -0.25f) setPix(out, x, y, 98, 68, 42);
                else setPix(out, x, y, 74, 50, 30);
            }
        }
        return;
    }
    else if (bId == 4) { // Podzol - Pine needle mulch top
        for (int y = 0; y < 16; ++y) {
            for (int x = 0; x < 16; ++x) {
                float pn = smoothNoiseWrap(x * 0.35f, y * 0.35f, 16.0f, 404);
                if (pn > 0.35f) setPix(out, x, y, 98, 66, 36);
                else if (pn > -0.15f) setPix(out, x, y, 76, 50, 28);
                else if (pn > -0.55f) setPix(out, x, y, 58, 36, 20);
                else setPix(out, x, y, 42, 26, 14);
            }
        }
        return;
    }
    else if (bId == 5) { // Mycelium - Fungal violet earth
        for (int y = 0; y < 16; ++y) {
            for (int x = 0; x < 16; ++x) {
                float mn = smoothNoiseWrap(x * 0.35f, y * 0.35f, 16.0f, 505);
                if (mn > 0.35f) setPix(out, x, y, 162, 144, 174);
                else if (mn > -0.10f) setPix(out, x, y, 142, 124, 152);
                else if (mn > -0.50f) setPix(out, x, y, 118, 100, 126);
                else setPix(out, x, y, 96, 80, 104);
            }
        }
        return;
    }
    else if (bId == 10) { // Clay - Bluish-gray smooth clay
        for (int y = 0; y < 16; ++y) {
            for (int x = 0; x < 16; ++x) {
                float cn = smoothNoiseWrap(x * 0.3f, y * 0.3f, 16.0f, 10);
                uint8_t g = clampU8(162.0f + cn * 12.0f);
                setPix(out, x, y, g - 6, g - 2, g + 8);
            }
        }
        return;
    }
    else if (bId == 11) { // Sand - Fine yellow desert sand ripples
        for (int y = 0; y < 16; ++y) {
            for (int x = 0; x < 16; ++x) {
                float wave = std::sin(y * 0.75f + std::sin(x * 0.45f) * 0.7f);
                float sn = smoothNoiseWrap(x * 0.3f, y * 0.3f, 16.0f, 11);
                float val = wave * 0.35f + sn * 0.65f;
                if (val > 0.35f) setPix(out, x, y, 236, 226, 176);
                else if (val > -0.10f) setPix(out, x, y, 222, 210, 156);
                else if (val > -0.50f) setPix(out, x, y, 206, 192, 140);
                else setPix(out, x, y, 190, 175, 124);
            }
        }
        return;
    }
    else if (bId == 12) { // Red Sand - Terracotta desert sand
        for (int y = 0; y < 16; ++y) {
            for (int x = 0; x < 16; ++x) {
                float wave = std::sin(y * 0.75f + std::sin(x * 0.45f) * 0.7f);
                float sn = smoothNoiseWrap(x * 0.3f, y * 0.3f, 16.0f, 12);
                float val = wave * 0.35f + sn * 0.65f;
                if (val > 0.35f) setPix(out, x, y, 216, 124, 60);
                else if (val > -0.10f) setPix(out, x, y, 196, 102, 46);
                else if (val > -0.50f) setPix(out, x, y, 176, 84, 36);
                else setPix(out, x, y, 154, 68, 28);
            }
        }
        return;
    }
    else if (bId == 14) { // Gravel - Grayscale stone pebbles
        for (int y = 0; y < 16; ++y) {
            for (int x = 0; x < 16; ++x) {
                float gn = smoothNoiseWrap(x * 0.45f, y * 0.45f, 16.0f, 14);
                float grit = pixelHash(x, y, 14) * 0.12f;
                float val = gn * 0.88f + grit;
                if (val > 0.40f) setPix(out, x, y, 152, 144, 142);
                else if (val > 0.10f) setPix(out, x, y, 132, 124, 122);
                else if (val > -0.25f) setPix(out, x, y, 112, 104, 102);
                else if (val > -0.60f) setPix(out, x, y, 92, 84, 82);
                else setPix(out, x, y, 74, 66, 64);
            }
        }
        return;
    }
    else if (bId == 16) { // Snow Block - Crisp pristine snow with soft blue shadows
        for (int y = 0; y < 16; ++y) {
            for (int x = 0; x < 16; ++x) {
                float n = smoothNoiseWrap(x * 0.3f, y * 0.3f, 16.0f, 16);
                if (n > 0.30f) setPix(out, x, y, 255, 255, 255);
                else if (n > -0.25f) setPix(out, x, y, 245, 248, 252);
                else setPix(out, x, y, 232, 238, 248);
            }
        }
        return;
    }
    else if (bId == 17) { // Water - Translucent aquatic pool with animated caustics pattern
        for (int y = 0; y < 16; ++y) {
            for (int x = 0; x < 16; ++x) {
                float caustic = std::sin(x * 0.8f + y * 0.4f) * std::cos(y * 0.8f - x * 0.3f);
                float wn = smoothNoiseWrap(x * 0.4f, y * 0.4f, 16.0f, 17);
                float val = caustic * 0.7f + wn * 0.3f;
                if (val > 0.35f) {
                    setPix(out, x, y, 120, 215, 255, 205);
                } else if (val > 0.05f) {
                    setPix(out, x, y, 48, 140, 235, 195);
                } else if (val > -0.35f) {
                    setPix(out, x, y, 28, 100, 215, 190);
                } else {
                    setPix(out, x, y, 16, 70, 185, 185);
                }
            }
        }
        return;
    }
    else if (bId >= 18 && bId <= 20) { // Ice, Packed Ice, Blue Ice
        for (int y = 0; y < 16; ++y) {
            for (int x = 0; x < 16; ++x) {
                bool isFracture = (x == y || x + y == 12 || x - y == 6);
                if (bId == 18) { // Ice
                    if (isFracture) setPix(out, x, y, 235, 248, 255, 220);
                    else setPix(out, x, y, 160, 205, 245, 200);
                } else if (bId == 19) { // Packed Ice
                    if (isFracture) setPix(out, x, y, 215, 235, 255);
                    else setPix(out, x, y, 145, 185, 235);
                } else { // Blue Ice
                    if (isFracture) setPix(out, x, y, 190, 220, 255);
                    else setPix(out, x, y, 100, 160, 250);
                }
            }
        }
        return;
    }
    else if (bId == 22) { // Soul Sand / Soul Soil
        for (int y = 0; y < 16; ++y) {
            for (int x = 0; x < 16; ++x) {
                bool isEye = ((x == 4 || x == 11) && (y == 4 || y == 11));
                bool isMouth = (y == 7 && x >= 5 && x <= 10);
                if (isEye || isMouth) setPix(out, x, y, 42, 28, 20);
                else {
                    float n = smoothNoiseWrap(x * 0.4f, y * 0.4f, 16.0f, 22);
                    setPix(out, x, y, clampU8(78 + n * 10), clampU8(56 + n * 8), clampU8(42 + n * 6));
                }
            }
        }
        return;
    }
    else if (bId == 23) { // Netherrack - Authentic craggy dark bloodstone with porous cavities
        for (int y = 0; y < 16; ++y) {
            for (int x = 0; x < 16; ++x) {
                float n1 = smoothNoiseWrap(x * 0.45f, y * 0.45f, 16.0f, 23);
                float n2 = pixelHash(x, y, 23) * 0.14f;
                float val = n1 * 0.86f + n2;
                if (val > 0.42f)      setPix(out, x, y, 158, 52, 54); // Bright crag ridge
                else if (val > 0.12f) setPix(out, x, y, 122, 34, 38); // Body bloodstone
                else if (val > -0.22f)setPix(out, x, y, 92,  22, 26); // Shadow stone
                else if (val > -0.55f)setPix(out, x, y, 64,  14, 18); // Porous cavity
                else                  setPix(out, x, y, 42,  8,  12); // Deep ash pit
            }
        }
        return;
    }
    else if (bId == 24) { // End Stone - Authentic inverted pale yellow cratered moon rock
        for (int y = 0; y < 16; ++y) {
            for (int x = 0; x < 16; ++x) {
                float en1 = smoothNoiseWrap(x * 0.38f, y * 0.38f, 16.0f, 24);
                float en2 = smoothNoiseWrap(x * 0.85f, y * 0.85f, 16.0f, 124) * 0.35f;
                float grit = pixelHash(x, y, 24) * 0.08f;
                float val = en1 * 0.65f + en2 + grit;
                if (val > 0.40f)      setPix(out, x, y, 242, 246, 195); // Crater rim highlight
                else if (val > 0.08f) setPix(out, x, y, 222, 226, 172); // Main cream body
                else if (val > -0.28f)setPix(out, x, y, 198, 202, 146); // Shadow slope
                else if (val > -0.60f)setPix(out, x, y, 172, 176, 120); // Deep crater floor
                else                  setPix(out, x, y, 148, 152, 98);  // Crater pit
            }
        }
        return;
    }
    else if (bId == 32) { // Molten Lava - Radiant swirling magma with dark cooling crust and white-hot veins
        for (int y = 0; y < 16; ++y) {
            for (int x = 0; x < 16; ++x) {
                float swirl = std::sin(x * 0.6f + y * 0.7f) + std::cos(y * 0.5f - x * 0.4f);
                float ln = smoothNoiseWrap(x * 0.35f, y * 0.35f, 16.0f, 32);
                float val = swirl * 0.6f + ln * 0.4f;
                if (val > 0.85f) {
                    setPix(out, x, y, 255, 245, 180, 255); // White-hot thermal core
                } else if (val > 0.30f) {
                    setPix(out, x, y, 255, 140, 20, 255);  // Blazing orange lava
                } else if (val > -0.30f) {
                    setPix(out, x, y, 220, 50, 10, 255);   // Deep crimson magma
                } else {
                    setPix(out, x, y, 110, 24, 12, 255);   // Cooling basalt crust
                }
            }
        }
        return;
    }

    // =========================================================================
    // 2. GEOLOGICAL STONES (36 to 70)
    // =========================================================================
    if (bId == 36) { // Stone - Authentic smooth gray stone
        fillMinecraftStone(out, 36);
        return;
    }
    else if (bId == 37) { // Cobblestone - Authentic Minecraft interlocking boulder matrix
        for (int y = 0; y < 16; ++y) {
            for (int x = 0; x < 16; ++x) {
                int lvl = mcCobbleMatrix[y][x];
                float smooth = smoothNoiseWrap(x * 0.4f, y * 0.4f, 16.0f, 37);
                float n = smooth * 6.0f;
                if (lvl == 0) setPix(out, x, y, clampU8(52 + n), clampU8(52 + n), clampU8(54 + n));      // Dark mortar
                else if (lvl == 1) setPix(out, x, y, clampU8(76 + n), clampU8(76 + n), clampU8(78 + n)); // Stone edge
                else if (lvl == 2) setPix(out, x, y, clampU8(104 + n), clampU8(104 + n), clampU8(106 + n));// Mid stone
                else if (lvl == 3) setPix(out, x, y, clampU8(128 + n), clampU8(128 + n), clampU8(130 + n));// Light stone
                else setPix(out, x, y, clampU8(154 + n), clampU8(154 + n), clampU8(156 + n));              // Highlight
            }
        }
        return;
    }
    else if (bId == 38) { // Mossy Cobblestone - Cobblestone with organic creeping moss
        for (int y = 0; y < 16; ++y) {
            for (int x = 0; x < 16; ++x) {
                int lvl = mcCobbleMatrix[y][x];
                float smooth = smoothNoiseWrap(x * 0.4f, y * 0.4f, 16.0f, 38);
                float mossN = smoothNoiseWrap(x * 0.3f, y * 0.3f, 16.0f, 88);
                bool isMoss = (mossN > 0.05f) && (lvl <= 2);
                float n = smooth * 6.0f;
                if (isMoss) {
                    if (mossN > 0.35f) setPix(out, x, y, 96, 158, 54);
                    else setPix(out, x, y, 68, 120, 40);
                } else {
                    if (lvl == 0) setPix(out, x, y, clampU8(52 + n), clampU8(52 + n), clampU8(54 + n));
                    else if (lvl == 1) setPix(out, x, y, clampU8(76 + n), clampU8(76 + n), clampU8(78 + n));
                    else if (lvl == 2) setPix(out, x, y, clampU8(104 + n), clampU8(104 + n), clampU8(106 + n));
                    else if (lvl == 3) setPix(out, x, y, clampU8(128 + n), clampU8(128 + n), clampU8(130 + n));
                    else setPix(out, x, y, clampU8(154 + n), clampU8(154 + n), clampU8(156 + n));
                }
            }
        }
        return;
    }
    else if (bId == 39 || bId == 40) { // Granite & Polished Granite
        for (int y = 0; y < 16; ++y) {
            for (int x = 0; x < 16; ++x) {
                float gn = smoothNoiseWrap(x * 0.4f, y * 0.4f, 16.0f, 39);
                if (gn < -0.35f) setPix(out, x, y, 92, 64, 54);
                else if (gn > 0.30f) setPix(out, x, y, 195, 145, 130);
                else setPix(out, x, y, 160, 112, 100);
            }
        }
        return;
    }
    else if (bId == 41 || bId == 42) { // Diorite & Polished Diorite
        for (int y = 0; y < 16; ++y) {
            for (int x = 0; x < 16; ++x) {
                float dn = smoothNoiseWrap(x * 0.4f, y * 0.4f, 16.0f, 41);
                if (dn > 0.35f) setPix(out, x, y, 225, 225, 228);
                else if (dn > 0.0f) setPix(out, x, y, 190, 190, 195);
                else if (dn > -0.35f) setPix(out, x, y, 150, 150, 155);
                else setPix(out, x, y, 115, 115, 120);
            }
        }
        return;
    }
    else if (bId == 43 || bId == 44) { // Andesite & Polished Andesite
        for (int y = 0; y < 16; ++y) {
            for (int x = 0; x < 16; ++x) {
                float an = smoothNoiseWrap(x * 0.35f, y * 0.35f, 16.0f, 43);
                uint8_t g = clampU8(125.0f + an * 18.0f);
                setPix(out, x, y, g, g + 2, g);
            }
        }
        return;
    }
    else if (bId == 45 || bId == 46) { // Deepslate
        fillMinecraftDeepslate(out, 45);
        return;
    }
    else if (bId == 47) { // Tuff - Volcanic ash rock with dark olivine inclusions
        for (int y = 0; y < 16; ++y) {
            for (int x = 0; x < 16; ++x) {
                float tn = smoothNoiseWrap(x * 0.35f, y * 0.35f, 16.0f, 47);
                bool isSpeckle = (x * 7 + y * 13) % 9 == 0;
                if (isSpeckle) setPix(out, x, y, 70, 72, 65);
                else if (tn > 0.25f) setPix(out, x, y, 120, 125, 115);
                else if (tn > -0.15f) setPix(out, x, y, 102, 106, 98);
                else setPix(out, x, y, 84, 88, 80);
            }
        }
        return;
    }
    else if (bId == 48) { // Calcite - Creamy white crystalline limestone
        for (int y = 0; y < 16; ++y) {
            for (int x = 0; x < 16; ++x) {
                float cn = smoothNoiseWrap(x * 0.35f, y * 0.35f, 16.0f, 48);
                bool isVein = (x - y == 3 || x - y == -4);
                if (isVein) setPix(out, x, y, 205, 202, 192);
                else if (cn > 0.2f) setPix(out, x, y, 242, 240, 232);
                else if (cn > -0.2f) setPix(out, x, y, 228, 226, 218);
                else setPix(out, x, y, 212, 210, 202);
            }
        }
        return;
    }
    else if (bId == 49) { // Dripstone - Earthy brown stalactite stone with vertical drip flutes
        for (int y = 0; y < 16; ++y) {
            for (int x = 0; x < 16; ++x) {
                float dn = smoothNoiseWrap(x * 0.3f, y * 0.2f, 16.0f, 49) * 6.0f;
                bool isGroove = (x == 2 || x == 7 || x == 12);
                if (isGroove) setPix(out, x, y, clampU8(105 + dn), clampU8(78 + dn * 0.7f), clampU8(62 + dn * 0.5f));
                else setPix(out, x, y, clampU8(142 + dn), clampU8(112 + dn * 0.8f), clampU8(92 + dn * 0.6f));
            }
        }
        return;
    }
    else if (bId == 50) { // Basalt - Dark columnar basalt
        for (int y = 0; y < 16; ++y) {
            for (int x = 0; x < 16; ++x) {
                bool isColumn = (x == 3 || x == 7 || x == 11 || x == 15);
                float bn = smoothNoiseWrap(x * 0.4f, y * 0.2f, 16.0f, 50) * 6.0f;
                if (isColumn) setPix(out, x, y, clampU8(45 + bn), clampU8(45 + bn), clampU8(48 + bn));
                else setPix(out, x, y, clampU8(78 + bn), clampU8(78 + bn), clampU8(84 + bn));
            }
        }
        return;
    }
    else if (bId == 51) { // Smooth Basalt - Polished dark stone
        for (int y = 0; y < 16; ++y) {
            for (int x = 0; x < 16; ++x) {
                float bn = smoothNoiseWrap(x * 0.35f, y * 0.35f, 16.0f, 51) * 5.0f;
                setPix(out, x, y, clampU8(62 + bn), clampU8(62 + bn), clampU8(66 + bn));
            }
        }
        return;
    }
    else if (bId == 52 || bId == 53) { // Obsidian & Crying Obsidian - Authentic conchoidal volcanic glass
        for (int y = 0; y < 16; ++y) {
            for (int x = 0; x < 16; ++x) {
                // Conchoidal fracture facets
                float on = smoothNoiseWrap(x * 0.35f, y * 0.35f, 16.0f, 52);
                bool isFacetRidge = (x == y || x + y == 15 || (x * 2 + y * 5) % 9 == 0);
                bool isGlint = ((x == 3 && y == 3) || (x == 11 && y == 4) || (x == 6 && y == 10) || (x == 12 && y == 12));

                if (bId == 53) { // Cryo-Obsidian / Crying Obsidian - Ethereal weeping magenta tears
                    // Glowing crying cracks & tear pools
                    bool isTear = ((x == 4 || x == 5) && (y >= 2 && y <= 6)) ||
                                  ((x == 5 || x == 6) && (y >= 7 && y <= 10)) ||
                                  ((x == 6) && (y >= 11 && y <= 13)) ||
                                  ((x >= 10 && x <= 12) && (y >= 8 && y <= 11)) ||
                                  (x == 11 && y == 12);
                    bool isTearCore = (x == 5 && (y == 4 || y == 8)) || (x == 11 && y == 9);

                    if (isTearCore) {
                        setPix(out, x, y, 255, 175, 255); // Blinding neon magenta-white core
                    } else if (isTear) {
                        setPix(out, x, y, 225, 45, 215);  // Electric purple crying stream
                    } else if (isFacetRidge) {
                        setPix(out, x, y, 78, 22, 95);    // Obsidian facet shadow
                    } else if (on > 0.1f) {
                        setPix(out, x, y, 42, 16, 58);
                    } else {
                        setPix(out, x, y, 20, 10, 30);    // Deep obsidian black
                    }
                } else { // Authentic Obsidian - Deep plum-black volcanic glass with sharp violet sheen
                    if (isGlint) {
                        setPix(out, x, y, 145, 95, 195);  // Razor-sharp specular glint
                    } else if (isFacetRidge) {
                        setPix(out, x, y, 82, 44, 125);   // Facet ridge highlight
                    } else if (on > 0.20f) {
                        setPix(out, x, y, 52, 28, 80);    // Plum-violet glass
                    } else if (on > -0.20f) {
                        setPix(out, x, y, 32, 18, 52);    // Mid dark volcanic glass
                    } else {
                        setPix(out, x, y, 16, 10, 26);    // Deep dark obsidian shadow
                    }
                }
            }
        }
        return;
    }
    else if (bId == 54) { // Sandstone - Smooth horizontal strata
        for (int y = 0; y < 16; ++y) {
            bool isSeam = (y == 3 || y == 11 || y == 15);
            for (int x = 0; x < 16; ++x) {
                float n = smoothNoiseWrap(x * 0.35f, y * 0.2f, 16.0f, 54) * 6.0f;
                if (isSeam) setPix(out, x, y, clampU8(185 + n), clampU8(172 + n), clampU8(125 + n));
                else setPix(out, x, y, clampU8(218 + n), clampU8(205 + n), clampU8(155 + n));
            }
        }
        return;
    }
    else if (bId == 55) { // Chiseled Sandstone - Carved hieroglyphic face
        for (int y = 0; y < 16; ++y) {
            for (int x = 0; x < 16; ++x) {
                bool isBorder = (x == 0 || x == 15 || y == 0 || y == 15);
                bool isEye = ((x == 4 || x == 11) && (y >= 5 && y <= 7));
                bool isMouth = (y == 11 && x >= 5 && x <= 10) || (y == 12 && (x == 5 || x == 10));
                if (isBorder || isEye || isMouth) setPix(out, x, y, 165, 150, 105);
                else setPix(out, x, y, 218, 205, 155);
            }
        }
        return;
    }
    else if (bId == 56) { // Red Sandstone - Layered terracotta strata
        for (int y = 0; y < 16; ++y) {
            bool isSeam = (y == 3 || y == 11 || y == 15);
            for (int x = 0; x < 16; ++x) {
                float n = smoothNoiseWrap(x * 0.35f, y * 0.2f, 16.0f, 56) * 6.0f;
                if (isSeam) setPix(out, x, y, clampU8(165 + n), clampU8(75 + n * 0.5f), clampU8(32 + n * 0.3f));
                else setPix(out, x, y, clampU8(198 + n), clampU8(98 + n * 0.5f), clampU8(45 + n * 0.3f));
            }
        }
        return;
    }
    else if (bId == 57) { // Marble - White stone with delicate gray veins
        for (int y = 0; y < 16; ++y) {
            for (int x = 0; x < 16; ++x) {
                float mn = smoothNoiseWrap(x * 0.35f, y * 0.35f, 16.0f, 57);
                bool isVein = (x + y * 2 == 14 || x - y == 5);
                if (isVein) setPix(out, x, y, 195, 195, 202);
                else if (mn > 0.2f) setPix(out, x, y, 248, 248, 252);
                else setPix(out, x, y, 235, 235, 240);
            }
        }
        return;
    }
    else if (bId == 58) { // Slate - Dark layered blue-gray roofing stone
        for (int y = 0; y < 16; ++y) {
            bool isBand = (y % 3 == 0);
            for (int x = 0; x < 16; ++x) {
                float sn = smoothNoiseWrap(x * 0.35f, y * 0.2f, 16.0f, 58) * 6.0f;
                if (isBand) setPix(out, x, y, clampU8(75 + sn), clampU8(80 + sn), clampU8(92 + sn));
                else setPix(out, x, y, clampU8(95 + sn), clampU8(100 + sn), clampU8(114 + sn));
            }
        }
        return;
    }
    else if (bId == 67 || bId == 68 || bId == 69) { // Prismarine varieties
        for (int y = 0; y < 16; ++y) {
            for (int x = 0; x < 16; ++x) {
                if (bId == 69) { // Prismarine Bricks (Brick grid)
                    bool isMortar = (y % 4 == 0 || (x + ((y / 4) % 2) * 4) % 8 == 0);
                    if (isMortar) setPix(out, x, y, 45, 95, 88);
                    else setPix(out, x, y, 102, 178, 165);
                } else if (bId == 68) { // Dark Prismarine
                    float dn = smoothNoiseWrap(x * 0.35f, y * 0.35f, 16.0f, 68) * 8.0f;
                    setPix(out, x, y, clampU8(48 + dn), clampU8(88 + dn), clampU8(82 + dn));
                } else { // Prismarine
                    float pn = smoothNoiseWrap(x * 0.35f, y * 0.35f, 16.0f, 67);
                    if (pn > 0.3f) setPix(out, x, y, 125, 195, 175);
                    else if (pn > -0.15f) setPix(out, x, y, 92, 165, 150);
                    else setPix(out, x, y, 68, 135, 125);
                }
            }
        }
        return;
    }

    // =========================================================================
    // 3. WOODS, PLANKS & CUTOUT LEAVES (71 to 120)
    // 16 distinct tree species: Log (71+i*3), Planks (72+i*3), Leaves (73+i*3)
    // =========================================================================
    if (bId >= 71 && bId <= 118) {
        int speciesIdx = (bId - 71) / 3;
        int typeIdx = (bId - 71) % 3; // 0=Log, 1=Plank, 2=Leaf

        // A. ALL 16 WOODEN PLANKS (typeIdx == 1)
        if (typeIdx == 1) {
            static const Vec4 pMain[16] = {
                {0.68f, 0.50f, 0.28f, 1.0f}, // 0: Oak
                {0.84f, 0.78f, 0.58f, 1.0f}, // 1: Birch
                {0.45f, 0.33f, 0.20f, 1.0f}, // 2: Spruce
                {0.50f, 0.36f, 0.22f, 1.0f}, // 3: Pine
                {0.68f, 0.48f, 0.34f, 1.0f}, // 4: Jungle
                {0.72f, 0.42f, 0.22f, 1.0f}, // 5: Acacia
                {0.30f, 0.20f, 0.12f, 1.0f}, // 6: Dark Oak
                {0.55f, 0.28f, 0.22f, 1.0f}, // 7: Mangrove
                {0.88f, 0.65f, 0.68f, 1.0f}, // 8: Cherry
                {0.72f, 0.46f, 0.25f, 1.0f}, // 9: Maple
                {0.45f, 0.48f, 0.32f, 1.0f}, // 10: Willow
                {0.65f, 0.32f, 0.58f, 1.0f}, // 11: Fungal
                {0.55f, 0.55f, 0.58f, 1.0f}, // 12: Petrified
                {0.64f, 0.75f, 0.36f, 1.0f}, // 13: Bamboo
                {0.75f, 0.58f, 0.38f, 1.0f}, // 14: Palm
                {0.35f, 0.55f, 0.88f, 1.0f}  // 15: Astral
            };
            Vec4 m = pMain[speciesIdx];
            Vec4 s(m.x * 0.55f, m.y * 0.55f, m.z * 0.55f, 1.0f);
            Vec4 h(std::min(m.x * 1.18f, 1.0f), std::min(m.y * 1.18f, 1.0f), std::min(m.z * 1.18f, 1.0f), 1.0f);
            fillMinecraftPlanks(out, m, s, h, bId);
            return;
        }

        // B. ALL 16 WOOD LOG BARKS (typeIdx == 0)
        if (typeIdx == 0) {
            for (int y = 0; y < 16; ++y) {
                for (int x = 0; x < 16; ++x) {
                    float n = smoothNoiseWrap(x * 0.4f, y * 0.2f, 16.0f, bId) * 8.0f;
                    bool isGroove = (x == 1 || x == 5 || x == 9 || x == 13 || (x == 2 && y % 3 == 0));

                    switch (speciesIdx) {
                    case 0: // Oak
                        if (isGroove) setPix(out, x, y, clampU8(62 + n), clampU8(45 + n * 0.7f), clampU8(26 + n * 0.5f));
                        else setPix(out, x, y, clampU8(112 + n), clampU8(84 + n * 0.8f), clampU8(50 + n * 0.6f));
                        break;
                    case 1: { // Birch
                        bool isNotch = (y == 2 && x >= 3 && x <= 6) || (y == 7 && x >= 9 && x <= 13) ||
                                       (y == 12 && x >= 1 && x <= 4) || (y == 14 && x >= 8 && x <= 11);
                        if (isNotch) setPix(out, x, y, clampU8(48 + n * 0.6f), clampU8(48 + n * 0.6f), clampU8(48 + n * 0.6f));
                        else setPix(out, x, y, clampU8(224 + n * 0.5f), clampU8(224 + n * 0.5f), clampU8(220 + n * 0.5f));
                        break;
                    }
                    case 2: // Spruce
                    case 3: // Pine
                        if (isGroove) setPix(out, x, y, clampU8(45 + n), clampU8(32 + n * 0.7f), clampU8(20 + n * 0.5f));
                        else setPix(out, x, y, clampU8(78 + n), clampU8(58 + n * 0.8f), clampU8(38 + n * 0.6f));
                        break;
                    case 4: { // Jungle: warm brown with green moss patches
                        bool isMoss = (x * 5 + y * 7) % 11 <= 2;
                        if (isMoss) setPix(out, x, y, 78, 128, 48);
                        else if (isGroove) setPix(out, x, y, clampU8(58 + n), clampU8(40 + n * 0.7f), clampU8(24 + n * 0.5f));
                        else setPix(out, x, y, clampU8(118 + n), clampU8(88 + n * 0.8f), clampU8(52 + n * 0.6f));
                        break;
                    }
                    case 5: // Acacia: gray-brown bark with fiery orange crevices
                        if (isGroove) setPix(out, x, y, clampU8(155 + n), clampU8(68 + n * 0.5f), clampU8(25 + n * 0.3f));
                        else setPix(out, x, y, clampU8(115 + n), clampU8(108 + n), clampU8(102 + n));
                        break;
                    case 6: // Dark Oak: deep espresso dark bark
                        if (isGroove) setPix(out, x, y, clampU8(28 + n * 0.5f), clampU8(20 + n * 0.4f), clampU8(14 + n * 0.3f));
                        else setPix(out, x, y, clampU8(58 + n * 0.8f), clampU8(42 + n * 0.7f), clampU8(28 + n * 0.5f));
                        break;
                    case 7: // Mangrove: muddy burgundy-red bark
                        if (isGroove) setPix(out, x, y, clampU8(65 + n), clampU8(30 + n * 0.5f), clampU8(25 + n * 0.4f));
                        else setPix(out, x, y, clampU8(112 + n), clampU8(62 + n * 0.6f), clampU8(48 + n * 0.5f));
                        break;
                    case 8: { // Cherry: mahogany-purple bark with horizontal silver lenticels
                        bool isLenticel = (y == 3 && x >= 2 && x <= 6) || (y == 9 && x >= 8 && x <= 13) || (y == 13 && x >= 1 && x <= 5);
                        if (isLenticel) setPix(out, x, y, 185, 172, 175);
                        else if (isGroove) setPix(out, x, y, clampU8(62 + n), clampU8(38 + n * 0.6f), clampU8(45 + n * 0.7f));
                        else setPix(out, x, y, clampU8(98 + n), clampU8(62 + n * 0.7f), clampU8(70 + n * 0.8f));
                        break;
                    }
                    case 9: // Maple: golden russet bark
                        if (isGroove) setPix(out, x, y, clampU8(82 + n), clampU8(48 + n * 0.6f), clampU8(24 + n * 0.4f));
                        else setPix(out, x, y, clampU8(138 + n), clampU8(88 + n * 0.7f), clampU8(46 + n * 0.5f));
                        break;
                    case 10: // Willow: swamp olive-drab bark
                        if (isGroove) setPix(out, x, y, clampU8(52 + n), clampU8(54 + n), clampU8(38 + n * 0.7f));
                        else setPix(out, x, y, clampU8(92 + n), clampU8(98 + n), clampU8(68 + n * 0.8f));
                        break;
                    case 11: // Fungal: deep violet stalk with cyan specks
                        if ((x == 3 && y == 5) || (x == 11 && y == 12)) setPix(out, x, y, 60, 240, 240);
                        else if (isGroove) setPix(out, x, y, clampU8(68 + n), clampU8(28 + n * 0.4f), clampU8(72 + n));
                        else setPix(out, x, y, clampU8(115 + n), clampU8(48 + n * 0.5f), clampU8(125 + n));
                        break;
                    case 12: // Petrified: layered agate stone wood
                        if ((y + x) % 4 == 0) setPix(out, x, y, clampU8(95 + n), clampU8(95 + n), clampU8(105 + n));
                        else setPix(out, x, y, clampU8(135 + n), clampU8(132 + n), clampU8(138 + n));
                        break;
                    case 13: { // Bamboo: culm with horizontal dark rings
                        bool isNode = (y == 2 || y == 10);
                        if (isNode) setPix(out, x, y, 78, 115, 42);
                        else setPix(out, x, y, clampU8(128 + n * 0.6f), clampU8(175 + n * 0.8f), clampU8(68 + n * 0.4f));
                        break;
                    }
                    case 14: { // Palm: diamond crisscross fibrous pattern
                        bool isDiamond = (x + y) % 4 == 0 || (x - y + 16) % 4 == 0;
                        if (isDiamond) setPix(out, x, y, clampU8(92 + n), clampU8(65 + n * 0.7f), clampU8(38 + n * 0.5f));
                        else setPix(out, x, y, clampU8(145 + n), clampU8(108 + n * 0.8f), clampU8(68 + n * 0.6f));
                        break;
                    }
                    default: // 15: Astral: midnight cosmic bark with stars
                        if ((x * 7 + y * 13) % 17 == 0) setPix(out, x, y, 220, 245, 255); // Star speck
                        else if (isGroove) setPix(out, x, y, clampU8(24 + n * 0.5f), clampU8(32 + n * 0.6f), clampU8(78 + n));
                        else setPix(out, x, y, clampU8(48 + n * 0.8f), clampU8(65 + n), clampU8(142 + n));
                        break;
                    }
                }
            }
            return;
        }

        // C. ALL 16 CUTOUT FOLIAGE LEAVES (typeIdx == 2)
        // Handcrafted with transparent cutout holes (a = 0)
        if (typeIdx == 2) {
            static const uint8_t leafBaseRGB[16][3] = {
                {74, 155, 38},   // 0: Oak (lush green)
                {115, 185, 48},  // 1: Birch (bright lime)
                {38, 92, 45},    // 2: Spruce (dark evergreen)
                {32, 85, 52},    // 3: Pine (blue-green conifer)
                {42, 165, 45},   // 4: Jungle (rich tropical green)
                {98, 142, 38},   // 5: Acacia (olive savannah)
                {30, 88, 32},    // 6: Dark Oak (deep forest green)
                {45, 125, 62},   // 7: Mangrove (waxy dark teal)
                {248, 142, 185}, // 8: Cherry (delicate sakura pink)
                {225, 88, 25},   // 9: Maple (autumn fiery orange/red)
                {92, 138, 85},   // 10: Willow (pale sage weeping green)
                {175, 48, 145},  // 11: Fungal (magenta/violet spores)
                {85, 145, 140},  // 12: Petrified (crystalline teal)
                {88, 185, 42},   // 13: Bamboo (bright lime blades)
                {68, 162, 52},   // 14: Palm (tropical fan frond green)
                {95, 185, 245}   // 15: Astral (shimmering starlight cyan)
            };

            const uint8_t* base = leafBaseRGB[speciesIdx];

            for (int y = 0; y < 16; ++y) {
                for (int x = 0; x < 16; ++x) {
                    // Species-specific cutout pattern
                    bool isHole = false;
                    if (speciesIdx == 8) { // Cherry: floral petal cutouts
                        isHole = ((x * 3 + y * 5) % 4 == 0) && ((x + y) % 3 != 0);
                    } else if (speciesIdx == 9) { // Maple: star-shaped cutouts
                        isHole = ((x * 5 + y * 7) % 5 == 0) && ((x ^ y) % 2 == 0);
                    } else if (speciesIdx == 2 || speciesIdx == 3) { // Conifer needles
                        isHole = (x % 3 == 0 && y % 2 == 1) || (y % 4 == 0 && x % 2 == 1);
                    } else { // Standard leaf cluster cutouts
                        isHole = ((x * 3 + y * 7) % 5 == 0 && (x + y * 2) % 3 == 0);
                    }

                    if (isHole) {
                        setPix(out, x, y, 0, 0, 0, 0); // Transparent cutout hole!
                    } else {
                        float ln = smoothNoiseWrap(x * 0.45f, y * 0.45f, 16.0f, bId);
                        float shade = (ln > 0.4f) ? 1.25f : ((ln > -0.2f) ? 1.0f : ((ln > -0.6f) ? 0.78f : 0.58f));

                        // Special color variation for Autumn Maple (blend red, orange, gold)
                        if (speciesIdx == 9) {
                            if (ln > 0.35f) setPix(out, x, y, 248, 185, 35, 255);       // Golden yellow
                            else if (ln > -0.2f) setPix(out, x, y, 228, 92, 28, 255);   // Fiery orange
                            else setPix(out, x, y, 185, 32, 22, 255);                   // Crimson red
                        }
                        // Cherry petals highlight
                        else if (speciesIdx == 8) {
                            if (ln > 0.45f) setPix(out, x, y, 255, 225, 235, 255); // Pale blush white
                            else if (ln > -0.2f) setPix(out, x, y, 248, 142, 185, 255);
                            else setPix(out, x, y, 205, 88, 138, 255);
                        }
                        else {
                            setPix(out, x, y,
                                   clampU8(base[0] * shade),
                                   clampU8(base[1] * shade),
                                   clampU8(base[2] * shade), 255);
                        }
                    }
                }
            }
            return;
        }
    }

    // Mushrooms (119 and 120)
    if (bId == 119) { // Giant Red Mushroom Cap - Crimson red with white circular polka dots
        for (int y = 0; y < 16; ++y) {
            for (int x = 0; x < 16; ++x) {
                float n = pixelHash(x, y, 119) * 8.0f;
                bool isSpot = ((x >= 2 && x <= 4 && y >= 2 && y <= 4 && !(x == 2 && y == 2) && !(x == 4 && y == 4)) ||
                               (x >= 10 && x <= 12 && y >= 3 && y <= 5 && !(x == 10 && y == 3)) ||
                               (x >= 4 && x <= 6 && y >= 10 && y <= 12 && !(x == 6 && y == 12)) ||
                               (x >= 11 && x <= 13 && y >= 10 && y <= 12));
                if (isSpot) setPix(out, x, y, 245, 245, 245);
                else setPix(out, x, y, clampU8(215 + n), clampU8(38 + n * 0.4f), clampU8(38 + n * 0.4f));
            }
        }
        return;
    }
    else if (bId == 120) { // Giant Brown Mushroom Cap - Earthy chestnut brown with pale rim
        for (int y = 0; y < 16; ++y) {
            for (int x = 0; x < 16; ++x) {
                float n = pixelHash(x, y, 120) * 10.0f;
                bool isBorder = (x == 0 || x == 15 || y == 0 || y == 15);
                if (isBorder) setPix(out, x, y, clampU8(175 + n), clampU8(140 + n * 0.7f), clampU8(105 + n * 0.5f));
                else setPix(out, x, y, clampU8(135 + n), clampU8(92 + n * 0.7f), clampU8(58 + n * 0.5f));
            }
        }
        return;
    }

    // =========================================================================
    // 4. ORES & MINERAL BLOCKS (121 to 160)
    // =========================================================================
    if (bId >= 121 && bId <= 140) {
        bool isDeepslate = (bId >= 136);
        int oreType = (bId >= 136) ? (bId - 136 + 121) : bId;
        fillMinecraftOre(out, oreType, isDeepslate, baseR, baseG, baseB);
        return;
    }
    else if (bId >= 141 && bId <= 145) { // Stamped Mineral Blocks
        for (int y = 0; y < 16; ++y) {
            for (int x = 0; x < 16; ++x) {
                bool isBorder = (x == 0 || x == 15 || y == 0 || y == 15);
                bool isBevel = (x == 1 || y == 1);
                bool isShade = (x == 14 || y == 14);
                float r = baseR, g = baseG, b = baseB;
                if (isBorder) { r *= 0.70f; g *= 0.70f; b *= 0.70f; }
                else if (isBevel) { r = std::min(r * 1.30f + 25.0f, 255.0f); g = std::min(g * 1.30f + 25.0f, 255.0f); b = std::min(b * 1.30f + 25.0f, 255.0f); }
                else if (isShade) { r *= 0.80f; g *= 0.80f; b *= 0.80f; }
                setPix(out, x, y, clampU8(r), clampU8(g), clampU8(b));
            }
        }
        return;
    }

    // =========================================================================
    // 8. MECHANICAL, UTILITY & LIGHTING (331 to 350)
    // =========================================================================
    if (bId == 331) { // Crafting Table (Side Face) - Planks with hanging handsaw and hammer
        fillMinecraftPlanks(out, {0.68f, 0.50f, 0.28f, 1.0f}, {0.35f, 0.24f, 0.12f, 1.0f}, {0.78f, 0.62f, 0.38f, 1.0f}, 331);
        // Top cloth strip (Navy blue #263859)
        for (int x = 0; x < 16; ++x) {
            setPix(out, x, 0, 38, 56, 92);
            setPix(out, x, 1, 48, 72, 118);
        }
        // Handsaw blade on left (x: 2..6, y: 4..12)
        for (int y = 4; y <= 12; ++y) {
            setPix(out, 3, y, 175, 180, 185); // Saw steel
            if (y % 2 == 0) setPix(out, 2, y, 145, 150, 155); // Saw teeth
        }
        // Saw wooden handle
        setPix(out, 3, 3, 115, 68, 32);
        setPix(out, 4, 3, 115, 68, 32);

        // Claw Hammer on right (x: 10..13, y: 4..13)
        for (int y = 6; y <= 13; ++y) setPix(out, 11, y, 125, 80, 42); // Wooden haft
        for (int x = 9; x <= 13; ++x) setPix(out, x, 5, 165, 170, 175); // Steel hammer head
        setPix(out, 13, 4, 140, 145, 150); // Claw tip
        return;
    }
    else if (bId == 332) { // Furnace (Front Face) - Cobblestone arch with burning ember grate
        for (int y = 0; y < 16; ++y) {
            for (int x = 0; x < 16; ++x) {
                int lvl = mcCobbleMatrix[y][x];
                float n = pixelHash(x, y, 332) * 8.0f;
                // Cobblestone frame
                setPix(out, x, y, clampU8(100 + n), clampU8(100 + n), clampU8(100 + n));
                if (lvl == 0) setPix(out, x, y, 50, 50, 50);
            }
        }
        // Archway opening (x: 3..12, y: 6..13)
        for (int y = 6; y <= 13; ++y) {
            for (int x = 3; x <= 12; ++x) {
                bool isArchBorder = (y == 6 || x == 3 || x == 12);
                if (isArchBorder) {
                    setPix(out, x, y, 42, 42, 42); // Chiseled stone border
                } else if (y >= 10) {
                    // Glowing fire embers
                    if (y == 13) setPix(out, x, y, 255, 60, 15);
                    else if (y == 12) setPix(out, x, y, 255, 145, 25);
                    else if (y == 11) setPix(out, x, y, 255, 220, 50);
                    else setPix(out, x, y, 180, 50, 15);
                } else {
                    setPix(out, x, y, 20, 20, 20); // Dark interior cavity
                }
            }
        }
        return;
    }
    else if (bId == 347) { // Bookshelf (Side Face) - 2 shelves of colorful books
        // Oak frame (y: 0, 7, 15)
        for (int y = 0; y < 16; ++y) {
            for (int x = 0; x < 16; ++x) {
                if (y == 0 || y == 7 || y == 15 || x == 0 || x == 15) {
                    setPix(out, x, y, 160, 115, 65);
                } else {
                    setPix(out, x, y, 65, 45, 28); // Shadow inside shelf
                }
            }
        }
        // Shelf 1 (y: 1..6) - Book spines
        static const uint8_t shelf1Cols[5][3] = {{185,42,42}, {45,85,185}, {48,155,62}, {195,145,35}, {145,55,160}};
        for (int x = 1; x <= 14; ++x) {
            int bIdx = (x - 1) / 3;
            for (int y = 1; y <= 6; ++y) {
                if (y == 1) setPix(out, x, y, 220, 215, 195); // Paper edge
                else setPix(out, x, y, shelf1Cols[bIdx % 5][0], shelf1Cols[bIdx % 5][1], shelf1Cols[bIdx % 5][2]);
            }
        }
        // Shelf 2 (y: 8..14)
        static const uint8_t shelf2Cols[5][3] = {{48,155,62}, {185,42,42}, {195,145,35}, {45,85,185}, {125,75,45}};
        for (int x = 1; x <= 14; ++x) {
            int bIdx = (x - 1) / 3;
            for (int y = 8; y <= 14; ++y) {
                if (y == 8) setPix(out, x, y, 220, 215, 195);
                else setPix(out, x, y, shelf2Cols[bIdx % 5][0], shelf2Cols[bIdx % 5][1], shelf2Cols[bIdx % 5][2]);
            }
        }
        return;
    }
    else if (bId == 348) { // TNT (Side Face) - Red dynamite with center white TNT banner
        for (int y = 0; y < 16; ++y) {
            for (int x = 0; x < 16; ++x) {
                bool isSeam = (x == 3 || x == 7 || x == 11 || x == 15);
                if (y >= 6 && y <= 9) {
                    // White paper label band
                    setPix(out, x, y, 242, 242, 242);
                } else {
                    // Red dynamite tube
                    float n = pixelHash(x, y, 348) * 10.0f;
                    if (isSeam) setPix(out, x, y, 145, 25, 15);
                    else setPix(out, x, y, clampU8(205 + n), clampU8(38 + n * 0.5f), clampU8(24 + n * 0.5f));
                }
            }
        }
        // Pixelated "TNT" lettering on white label (y: 6..9)
        // 'T' at x: 2,3,4
        setPix(out, 2, 6, 18, 18, 18); setPix(out, 3, 6, 18, 18, 18); setPix(out, 4, 6, 18, 18, 18);
        setPix(out, 3, 7, 18, 18, 18); setPix(out, 3, 8, 18, 18, 18); setPix(out, 3, 9, 18, 18, 18);
        // 'N' at x: 6,7,8,9
        setPix(out, 6, 6, 18, 18, 18); setPix(out, 6, 7, 18, 18, 18); setPix(out, 6, 8, 18, 18, 18); setPix(out, 6, 9, 18, 18, 18);
        setPix(out, 7, 7, 18, 18, 18); setPix(out, 8, 8, 18, 18, 18);
        setPix(out, 9, 6, 18, 18, 18); setPix(out, 9, 7, 18, 18, 18); setPix(out, 9, 8, 18, 18, 18); setPix(out, 9, 9, 18, 18, 18);
        // 'T' at x: 11,12,13
        setPix(out, 11, 6, 18, 18, 18); setPix(out, 12, 6, 18, 18, 18); setPix(out, 13, 6, 18, 18, 18);
        setPix(out, 12, 7, 18, 18, 18); setPix(out, 12, 8, 18, 18, 18); setPix(out, 12, 9, 18, 18, 18);
        return;
    }
    else if (bId == 342 || bId == 343) { // Clear Glass (342) & Tinted Glass (343)
        for (int y = 0; y < 16; ++y) {
            for (int x = 0; x < 16; ++x) {
                bool isBorder = (x == 0 || x == 15 || y == 0 || y == 15);
                bool isGlint1 = (x == 3 && y == 2) || (x == 4 && y == 3) || (x == 5 && y == 4);
                bool isGlint2 = (x == 10 && y == 11) || (x == 11 && y == 12) || (x == 12 && y == 13);
                if (isBorder) {
                    if (bId == 342) setPix(out, x, y, 220, 235, 245, 230);
                    else setPix(out, x, y, 80, 80, 95, 240);
                } else if (isGlint1 || isGlint2) {
                    setPix(out, x, y, 255, 255, 255, 180);
                } else {
                    if (bId == 342) setPix(out, x, y, 210, 235, 248, 75); // Soft translucent glass pane!
                    else setPix(out, x, y, 40, 40, 50, 160);
                }
            }
        }
        return;
    }
    else if (bId >= 366 && bId <= 369) { // Tree Saplings (Oak, Birch, Spruce, Cherry)
        // Clear transparent background (alpha cutout in shader)
        for (int y = 0; y < 16; ++y) {
            for (int x = 0; x < 16; ++x) {
                setPix(out, x, y, 0, 0, 0, 0);
            }
        }
        // Base wooden stem & branches
        for (int y = 10; y <= 15; ++y) {
            setPix(out, 7, y, 92, 62, 34, 255);
            setPix(out, 8, y, 115, 80, 48, 255);
        }
        setPix(out, 6, 9, 85, 55, 30, 255);
        setPix(out, 9, 9, 105, 72, 42, 255);

        // Species-specific foliage palette
        uint8_t lr = 75, lg = 175, lb = 45; // Oak (Vibrant lush green)
        if (bId == 367) { lr = 125; lg = 210; lb = 55; }       // Birch (Bright lime/yellow-green)
        else if (bId == 368) { lr = 35; lg = 115; lb = 50; }   // Spruce (Deep dark pine needle green)
        else if (bId == 369) { lr = 245; lg = 145; lb = 190; } // Cherry (Delicate cherry blossom pink)

        // Center foliage cluster
        for (int dy = 2; dy <= 8; ++dy) {
            for (int dx = 5; dx <= 10; ++dx) {
                float n = pixelHash(dx, dy, bId);
                uint8_t cr = clampU8(lr + n * 20.0f);
                uint8_t cg = clampU8(lg + n * 24.0f);
                uint8_t cb = clampU8(lb + n * 18.0f);
                setPix(out, dx, dy, cr, cg, cb, 255);
            }
        }
        // Left leaf branch
        for (int dy = 4; dy <= 9; ++dy) {
            for (int dx = 2; dx <= 5; ++dx) {
                if (dy == 4 && dx == 2) continue;
                float n = pixelHash(dx, dy, bId + 11);
                setPix(out, dx, dy, clampU8(lr - 15.0f + n * 18.0f), clampU8(lg - 12.0f + n * 20.0f), clampU8(lb - 8.0f + n * 14.0f), 255);
            }
        }
        // Right leaf branch
        for (int dy = 3; dy <= 8; ++dy) {
            for (int dx = 10; dx <= 13; ++dx) {
                if (dy == 3 && dx == 13) continue;
                float n = pixelHash(dx, dy, bId + 23);
                setPix(out, dx, dy, clampU8(lr + 18.0f + n * 16.0f), clampU8(lg + 16.0f + n * 22.0f), clampU8(lb + 12.0f + n * 16.0f), 255);
            }
        }
        return;
    }

    // =========================================================================
    // 5. STONE BRICKS SUITE & ARCHITECTURE (241 to 244)
    // =========================================================================
    if (bId >= 241 && bId <= 244) {
        fillMinecraftStoneBricks(out, bId - 241, bId);
        return;
    }

    // =========================================================================
    // 6. SPECIAL DUNGEON & TEMPLE BLOCKS (257, 259, 260)
    // =========================================================================
    if (bId == 257) { // Sea Lantern Tile
        fillMinecraftSeaLantern(out);
        return;
    }
    else if (bId == 259) { // Volcanic Fortress Brick (Nether Fortress Bricks)
        for (int y = 0; y < 16; ++y) {
            int row = y / 4;
            for (int x = 0; x < 16; ++x) {
                bool isMortar = (y % 4 == 3) || ((row % 2 == 0) ? (x == 7 || x == 15) : (x == 3 || x == 11));
                float n = pixelHash(x, y, 259) * 0.12f;
                if (isMortar) setPix(out, x, y, 32, 10, 14);
                else setPix(out, x, y, clampU8(78 + n * 16.0f), clampU8(22 + n * 8.0f), clampU8(28 + n * 8.0f));
            }
        }
        return;
    }
    else if (bId == 260) { // Magma Tile - Dark cooled crust with radiant lava fissures
        for (int y = 0; y < 16; ++y) {
            for (int x = 0; x < 16; ++x) {
                float n = smoothNoiseWrap(x * 0.4f, y * 0.4f, 16.0f, 260);
                bool isVein = (x == y || x + y == 14 || (x * 3 + y * 7) % 11 == 0);
                if (isVein && n > 0.0f) {
                    setPix(out, x, y, 255, 145, 25); // Glowing hot magma crack
                } else if (isVein) {
                    setPix(out, x, y, 215, 55, 15);  // Deep crimson vein
                } else {
                    setPix(out, x, y, 48, 20, 16);   // Dark basalt crust
                }
            }
        }
        return;
    }

    // =========================================================================
    // 7. GLOWSTONE LAMP (344)
    // =========================================================================
    if (bId == 344) {
        fillMinecraftGlowstone(out);
        return;
    }

    // Standard Fallback for other block numbers - Smooth multi-octave tonal variation with soft edge bevel
    for (int y = 0; y < 16; ++y) {
        for (int x = 0; x < 16; ++x) {
            float smooth = fbmWrap(x * 0.25f, y * 0.25f, bId, 2);
            float grit = pixelHash(x, y, bId) * 0.05f;
            float noise = smooth * 0.12f + grit;
            float ao = edgeAO(x, y, 0.04f);
            float r = baseR * (1.0f + noise) * ao;
            float g = baseG * (1.0f + noise) * ao;
            float b = baseB * (1.0f + noise) * ao;
            setPix(out, x, y, clampU8(r), clampU8(g), clampU8(b), 255);
        }
    }
}

// =============================================================================
// AUTHENTIC MULTI-FACE BLOCK TEXTURES (Layers 405 to 420)
// =============================================================================
void TextureAtlas::generateMultiFaceTexture(int mf, uint8_t* out) {
    if (mf == TEX_LAYER_GRASS_TOP) { // Grass Top - Lush scattered green lawn
        for (int y = 0; y < 16; ++y) {
            for (int x = 0; x < 16; ++x) {
                float gn = fbmWrap(x * 0.25f, y * 0.25f, 405, 2);
                float grit = pixelHash(x, y, 405) * 0.08f;
                float val = gn * 0.92f + grit;
                if (val > 0.40f) setPix(out, x, y, 138, 198, 68);       // Lime highlight
                else if (val > 0.10f) setPix(out, x, y, 114, 172, 58);  // Lush green
                else if (val > -0.25f) setPix(out, x, y, 90, 144, 46);  // Mid green
                else if (val > -0.60f) setPix(out, x, y, 70, 118, 36);  // Forest green
                else setPix(out, x, y, 52, 88, 28);                     // Deep shadow
            }
        }
    }
    else if (mf == TEX_LAYER_LOG_OAK_TOP || mf == TEX_LAYER_LOG_BIRCH_TOP || mf == TEX_LAYER_LOG_SPRUCE_TOP) {
        // Log Tops - Concentric annual tree growth rings
        float rHeart = (mf == TEX_LAYER_LOG_BIRCH_TOP) ? 210.0f : 175.0f;
        float gHeart = (mf == TEX_LAYER_LOG_BIRCH_TOP) ? 195.0f : 140.0f;
        float bHeart = (mf == TEX_LAYER_LOG_BIRCH_TOP) ? 145.0f : 85.0f;

        for (int y = 0; y < 16; ++y) {
            for (int x = 0; x < 16; ++x) {
                float dx = x - 7.5f;
                float dy = y - 7.5f;
                float dist = std::sqrt(dx * dx + dy * dy);

                if (dist > 6.4f) {
                    // Outer bark ring
                    if (mf == TEX_LAYER_LOG_BIRCH_TOP) setPix(out, x, y, 220, 220, 215);
                    else if (mf == TEX_LAYER_LOG_SPRUCE_TOP) setPix(out, x, y, 55, 40, 25);
                    else setPix(out, x, y, 92, 68, 38);
                } else {
                    // Concentric growth ring lines (dist approx 2.0, 4.2, 5.8)
                    bool isRing = (std::abs(dist - 2.0f) < 0.45f || std::abs(dist - 4.2f) < 0.45f);
                    float wn = smoothNoiseWrap(x * 0.35f, y * 0.35f, 16.0f, mf) * 6.0f;
                    if (isRing) {
                        setPix(out, x, y, clampU8(rHeart * 0.72f + wn), clampU8(gHeart * 0.72f + wn), clampU8(bHeart * 0.72f + wn));
                    } else {
                        setPix(out, x, y, clampU8(rHeart + wn), clampU8(gHeart + wn), clampU8(bHeart + wn));
                    }
                }
            }
        }
    }
    else if (mf == TEX_LAYER_CRAFTING_TOP) { // Crafting Table Top - 3x3 Grid with measuring tools
        fillMinecraftPlanks(out, {0.78f, 0.62f, 0.38f, 1.0f}, {0.45f, 0.32f, 0.18f, 1.0f}, {0.88f, 0.74f, 0.48f, 1.0f}, 409);
        // 3x3 checkered marked grid lines in center (x: 2..13, y: 2..13)
        for (int y = 2; y <= 13; ++y) {
            for (int x = 2; x <= 13; ++x) {
                bool isGridLine = (x == 2 || x == 6 || x == 10 || x == 13 ||
                                   y == 2 || y == 6 || y == 10 || y == 13);
                if (isGridLine) setPix(out, x, y, 95, 65, 35);
            }
        }
        // Corner metal brackets
        setPix(out, 0, 0, 160, 165, 170); setPix(out, 1, 0, 160, 165, 170); setPix(out, 0, 1, 160, 165, 170);
        setPix(out, 15, 0, 160, 165, 170); setPix(out, 14, 0, 160, 165, 170); setPix(out, 15, 1, 160, 165, 170);
        setPix(out, 0, 15, 160, 165, 170); setPix(out, 1, 15, 160, 165, 170); setPix(out, 0, 14, 160, 165, 170);
        setPix(out, 15, 15, 160, 165, 170); setPix(out, 14, 15, 160, 165, 170); setPix(out, 15, 14, 160, 165, 170);
    }
    else if (mf == TEX_LAYER_FURNACE_TOP) { // Furnace Top - Smooth stone with vent ring
        fillMinecraftStone(out, 410);
        // Circular vent in center
        for (int y = 0; y < 16; ++y) {
            for (int x = 0; x < 16; ++x) {
                float dx = x - 7.5f, dy = y - 7.5f;
                float dist = std::sqrt(dx * dx + dy * dy);
                if (std::abs(dist - 4.5f) < 0.6f) setPix(out, x, y, 62, 62, 62);
            }
        }
    }
    else if (mf == TEX_LAYER_TNT_TOP) { // TNT Top - Red border with central fuse ring
        for (int y = 0; y < 16; ++y) {
            for (int x = 0; x < 16; ++x) {
                float dx = x - 7.5f, dy = y - 7.5f;
                float dist = std::sqrt(dx * dx + dy * dy);
                if (dist < 1.8f) setPix(out, x, y, 50, 50, 50);       // Black fuse cord
                else if (dist < 4.5f) setPix(out, x, y, 235, 235, 235); // White label center
                else setPix(out, x, y, 205, 38, 24);                   // Red dynamite rim
            }
        }
    }
    else if (mf == TEX_LAYER_TNT_BOTTOM) { // TNT Bottom - Red dynamite ends
        for (int y = 0; y < 16; ++y) {
            for (int x = 0; x < 16; ++x) {
                bool isSeam = (x == 3 || x == 7 || x == 11 || x == 15 || y == 3 || y == 7 || y == 11 || y == 15);
                if (isSeam) setPix(out, x, y, 145, 25, 15);
                else setPix(out, x, y, 195, 35, 22);
            }
        }
    }
    else if (mf == TEX_LAYER_LOG_CHERRY_TOP || mf == TEX_LAYER_LOG_MAPLE_TOP ||
             mf == TEX_LAYER_LOG_JUNGLE_TOP || mf == TEX_LAYER_LOG_ACACIA_TOP ||
             mf == TEX_LAYER_LOG_BAMBOO_TOP || mf == TEX_LAYER_LOG_ASTRAL_TOP) {
        float rHeart = 175.0f, gHeart = 140.0f, bHeart = 85.0f;
        uint8_t rBark = 92, gBark = 68, bBark = 38;

        if (mf == TEX_LAYER_LOG_CHERRY_TOP) {
            rHeart = 225.0f; gHeart = 165.0f; bHeart = 175.0f;
            rBark = 98; gBark = 62; bBark = 70;
        } else if (mf == TEX_LAYER_LOG_MAPLE_TOP) {
            rHeart = 215.0f; gHeart = 145.0f; bHeart = 75.0f;
            rBark = 138; gBark = 88; bBark = 46;
        } else if (mf == TEX_LAYER_LOG_JUNGLE_TOP) {
            rHeart = 185.0f; gHeart = 135.0f; bHeart = 85.0f;
            rBark = 78; gBark = 128; bBark = 48;
        } else if (mf == TEX_LAYER_LOG_ACACIA_TOP) {
            rHeart = 205.0f; gHeart = 115.0f; bHeart = 45.0f;
            rBark = 115; gBark = 108; bBark = 102;
        } else if (mf == TEX_LAYER_LOG_BAMBOO_TOP) {
            rHeart = 195.0f; gHeart = 215.0f; bHeart = 125.0f;
            rBark = 78; gBark = 115; bBark = 42;
        } else if (mf == TEX_LAYER_LOG_ASTRAL_TOP) {
            rHeart = 85.0f; gHeart = 145.0f; bHeart = 225.0f;
            rBark = 48; gBark = 65; bBark = 142;
        }

        for (int y = 0; y < 16; ++y) {
            for (int x = 0; x < 16; ++x) {
                float dx = x - 7.5f;
                float dy = y - 7.5f;
                float dist = std::sqrt(dx * dx + dy * dy);

                if (dist > 6.4f) {
                    setPix(out, x, y, rBark, gBark, bBark);
                } else if (mf == TEX_LAYER_LOG_BAMBOO_TOP && dist < 2.5f) {
                    setPix(out, x, y, 45, 68, 25);
                } else {
                    bool isRing = (std::abs(dist - 2.0f) < 0.45f || std::abs(dist - 4.2f) < 0.45f);
                    float wn = smoothNoiseWrap(x * 0.35f, y * 0.35f, 16.0f, mf) * 6.0f;
                    if (isRing) {
                        setPix(out, x, y, clampU8(rHeart * 0.72f + wn), clampU8(gHeart * 0.72f + wn), clampU8(bHeart * 0.72f + wn));
                    } else {
                        setPix(out, x, y, clampU8(rHeart + wn), clampU8(gHeart + wn), clampU8(bHeart + wn));
                    }
                }
            }
        }
    }
    else if (mf == TEX_LAYER_SANDSTONE_TOP) { // Sandstone Top - Smooth sand stone face
        for (int y = 0; y < 16; ++y) {
            for (int x = 0; x < 16; ++x) {
                float n = smoothNoiseWrap(x * 0.35f, y * 0.35f, 16.0f, 420) * 6.0f;
                setPix(out, x, y, clampU8(224 + n), clampU8(212 + n), clampU8(162 + n));
            }
        }
    }
}

// =============================================================================
// MODULAR STRUCTURE TEXTURES (370 to 385)
// =============================================================================
void TextureAtlas::generateStructureTexture(int st, uint8_t* out) {
    for (int y = 0; y < 16; ++y) {
        for (int x = 0; x < 16; ++x) {
            uint8_t r = 160, g = 110, b = 65;

            if (st == TEX_LAYER_WOOD_PLANK || st == TEX_LAYER_WOOD_WALL || st == TEX_LAYER_WOOD_STAIRS) {
                bool isSeam = (y % 4 == 3);
                bool isTopBevel = (y % 4 == 0);
                bool isNail = ((x == 1 || x == 14) && (y % 4 == 1));

                float woodNoise = pixelHash(x, y, st) * 15.0f;
                r = clampU8(175.0f + woodNoise);
                g = clampU8(120.0f + woodNoise * 0.8f);
                b = clampU8(70.0f + woodNoise * 0.6f);

                if (isSeam) { r = 85; g = 55; b = 30; }
                else if (isTopBevel) { r = 205; g = 145; b = 90; }
                else if (isNail) { r = 60; g = 60; b = 65; }
            }
            else if (st == TEX_LAYER_STONE_COBBLE || st == TEX_LAYER_STONE_BRICK || st == TEX_LAYER_STONE_STAIRS) {
                int row = y / 4;
                int xOff = (row % 2 == 0) ? 0 : 4;
                bool isMortar = (y % 4 == 0 || (x + xOff) % 8 == 0);

                float stoneNoise = pixelHash(x, y, st) * 18.0f;
                r = clampU8(140.0f + stoneNoise);
                g = clampU8(142.0f + stoneNoise);
                b = clampU8(148.0f + stoneNoise);

                if (isMortar) { r = 70; g = 70; b = 75; }
                else if (y % 4 == 1 || (x + xOff) % 8 == 1) { r += 25; g += 25; b += 25; }
                else if (y % 4 == 3 || (x + xOff) % 8 == 7) { r -= 25; g -= 25; b -= 25; }
            }
            else if (st == TEX_LAYER_PALBOX_CORE) {
                float dist = std::sqrt(static_cast<float>((x - 7.5f) * (x - 7.5f) + (y - 7.5f) * (y - 7.5f)));
                if (dist < 4.0f) {
                    r = 60; g = 210; b = 255;
                    if (dist < 2.0f) { r = 210; g = 245; b = 255; }
                } else {
                    bool isRivet = (x <= 2 && y <= 2) || (x >= 13 && y <= 2) || (x <= 2 && y >= 13) || (x >= 13 && y >= 13);
                    if (isRivet) { r = 240; g = 195; b = 40; }
                    else {
                        r = 35 + (x % 2) * 8;
                        g = 55 + (y % 2) * 8;
                        b = 85 + ((x + y) % 2) * 10;
                    }
                }
            }
            else if (st == TEX_LAYER_CHEST) {
                bool isIronCorner = (x <= 1 || x >= 14 || y <= 1 || y >= 14);
                bool isLatch = (x >= 6 && x <= 9 && y >= 6 && y <= 9);
                if (isLatch) { r = 245; g = 205; b = 50; }
                else if (isIronCorner) { r = 90; g = 95; b = 105; }
                else { r = 160; g = 105; b = 50; }
            }
            else {
                r = 130 + (x % 3) * 10;
                g = 130 + (y % 3) * 10;
                b = 135;
            }

            setPix(out, x, y, r, g, b, 255);
        }
    }
}

// =============================================================================
// SMOOTH & EDGY CREATURE TEXTURES (390 to 400)
// =============================================================================
void TextureAtlas::generateCreatureTexture(int cr, uint8_t* out) {
    for (int y = 0; y < 16; ++y) {
        for (int x = 0; x < 16; ++x) {
            // 1. EDGY PREDATOR EYES & BATTLE VISAGE (Layer 395)
            if (cr == TEX_LAYER_CREATURE_HYTALE_EYE) {
                // Sleek dark matte face plate
                uint8_t r = 32, g = 36, b = 44;

                // Aggressive angled predator eyes (sharp diagonal slits slanting down towards center)
                // Left eye: (x in 2..6, y in 5..8); Right eye: (x in 9..13, y in 5..8)
                int lDist = (6 - x) + (y - 5); // Diagonal slash
                int rDist = (x - 9) + (y - 5);
                bool isLeftSlit = (x >= 2 && x <= 6 && y >= 6 && y <= 8 && std::abs(lDist - 3) <= 1);
                bool isRightSlit = (x >= 9 && x <= 13 && y >= 6 && y <= 8 && std::abs(rDist - 3) <= 1);

                // Heavy angular brow ridge
                bool isLeftBrow = (y == 5 && x >= 2 && x <= 7);
                bool isRightBrow = (y == 5 && x >= 8 && x <= 13);

                // White-hot predator slit pupils
                bool isLeftPupil = (x == 4 && y == 7);
                bool isRightPupil = (x == 11 && y == 7);

                // Sharp predator fangs / jaw vents
                bool isFang = (y == 13 && (x == 4 || x == 11)) || (y == 14 && (x == 4 || x == 11));
                bool isMouthSlit = (y == 12 && x >= 5 && x <= 10);

                // Crimson war markings / carbon stripes
                bool isWarMark = ((x == 1 || x == 14) && y >= 4 && y <= 11) ||
                                 ((x == 2 || x == 13) && y >= 9 && y <= 11);

                if (isLeftPupil || isRightPupil) {
                    r = 255; g = 255; b = 255; // White-hot focal slit
                } else if (isLeftSlit || isRightSlit) {
                    r = 255; g = 190; b = 20;  // Glowing amber-gold predator eye
                } else if (isLeftBrow || isRightBrow) {
                    r = 14; g = 16; b = 20;    // Aggressive black brow ridge
                } else if (isFang) {
                    r = 240; g = 240; b = 230; // Sharp ivory fangs
                } else if (isMouthSlit) {
                    r = 16; g = 18; b = 22;    // Dark mouth seam
                } else if (isWarMark) {
                    r = 200; g = 35; b = 45;   // Crimson war marking
                }

                setPix(out, x, y, r, g, b, 255);
                continue;
            }

            // 2. HIGH-TECH EDGY ENERGY CORE / RUNES (Layer 396)
            if (cr == TEX_LAYER_CREATURE_HYTALE_RUNE) {
                float dx = x - 7.5f;
                float dy = y - 7.5f;
                float d = std::sqrt(dx * dx + dy * dy);

                // Sharp faceted diamond core with concentric energy rings
                float manhattan = std::abs(dx) + std::abs(dy);
                bool isDiamondEdge = (std::abs(manhattan - 5.0f) < 0.6f);
                bool isCenterCore = (manhattan < 2.2f);
                bool isRadialRay = ((x == 7 || x == 8) && y >= 1 && y <= 14) ||
                                   ((y == 7 || y == 8) && x >= 1 && x <= 14);

                uint8_t r = 12, g = 35, b = 60; // Deep space navy background
                if (isCenterCore) {
                    r = 255; g = 255; b = 255; // Blinding energy center
                } else if (isDiamondEdge || isRadialRay) {
                    r = 0; g = 235; b = 255;   // Electric neon cyan energy line
                } else {
                    float glow = std::clamp(1.0f - (d / 8.5f), 0.0f, 1.0f);
                    r = static_cast<uint8_t>(10.0f + glow * 40.0f);
                    g = static_cast<uint8_t>(60.0f + glow * 150.0f);
                    b = static_cast<uint8_t>(90.0f + glow * 165.0f);
                }
                setPix(out, x, y, r, g, b, 255);
                continue;
            }

            // 3. EDGY BRUSHED TITANIUM & HONED BLADE STEEL (Layer 397)
            if (cr == TEX_LAYER_CREATURE_HYTALE_ARMOR) {
                // Sleek specular reflection streak along diagonal
                float diag = (x + y) / 30.0f;
                float shine = std::exp(-std::pow((diag - 0.5f) * 6.0f, 2.0f));

                bool isOuterBorder = (x == 0 || x == 15 || y == 0 || y == 15);
                bool isChamfer = (x == 1 || x == 14 || y == 1 || y == 14);
                bool isCenterSpine = (x == y);

                uint8_t r = static_cast<uint8_t>(90.0f + shine * 140.0f);
                uint8_t g = static_cast<uint8_t>(98.0f + shine * 145.0f);
                uint8_t b = static_cast<uint8_t>(112.0f + shine * 140.0f);

                if (isCenterSpine) {
                    r = 240; g = 245; b = 255; // Honed razor spine reflection
                } else if (isOuterBorder) {
                    r = 25; g = 28; b = 36;    // Deep gunmetal outer chamfer
                } else if (isChamfer) {
                    r = 0; g = 210; b = 245;   // Neon cyan accent border trim
                }

                setPix(out, x, y, r, g, b, 255);
                continue;
            }

            // 4. EDGY TACTICAL COMBAT WEAVE (Layer 398)
            if (cr == TEX_LAYER_CREATURE_HYTALE_CLOTH) {
                // Carbon fiber diagonal weave
                bool isWeave = ((x + y) % 4 < 2);
                bool isHarness = (y >= 6 && y <= 9);
                bool isBuckle = (isHarness && x >= 6 && x <= 9);

                uint8_t r = isWeave ? 42 : 32;
                uint8_t g = isWeave ? 46 : 36;
                uint8_t b = isWeave ? 56 : 44;

                if (isBuckle) {
                    r = 0; g = 220; b = 240; // High-tech cyan buckle
                } else if (isHarness) {
                    r = 20; g = 22; b = 28;  // Reinforced tactical strap
                }
                setPix(out, x, y, r, g, b, 255);
                continue;
            }

            // 5. SMOOTH & EDGY CREATURE MATERIALS
            float baseR = 190.0f, baseG = 160.0f, baseB = 120.0f;

            if (cr == TEX_LAYER_CREATURE_FUR) {
                // Smooth directional gradient with razor specular streaks (No pixelated noise)
                float gradY = static_cast<float>(y) / 15.0f;
                baseR = 135.0f + gradY * 65.0f;
                baseG = 115.0f + gradY * 55.0f;
                baseB = 95.0f + gradY * 45.0f;
                // Razor directional fur gleams
                if (x % 5 == (y % 3)) {
                    baseR += 30.0f; baseG += 25.0f; baseB += 20.0f;
                }
            } else if (cr == TEX_LAYER_CREATURE_SCALE) {
                // Smooth Hexagonal Chitin Armor (Sleek predatory scales)
                float hx = (x % 6) - 3.0f;
                float hy = (y % 6) - 3.0f;
                float dist = std::sqrt(hx * hx + hy * hy);
                float plateShade = std::clamp(1.0f - (dist / 3.8f), 0.0f, 1.0f);
                baseR = 30.0f + plateShade * 65.0f;
                baseG = 140.0f + plateShade * 95.0f;
                baseB = 90.0f + plateShade * 75.0f;
            } else if (cr == TEX_LAYER_CREATURE_BOSS) {
                // Matte Obsidian Carbon with glowing razor magma fissures
                baseR = 30.0f; baseG = 32.0f; baseB = 40.0f;
                // Razor energy veins
                if (x == y || (x + y == 15 && x >= 4 && x <= 11)) {
                    baseR = 255.0f; baseG = 95.0f; baseB = 15.0f; // Searing magma fissure
                } else if (std::abs(x - y) == 1) {
                    baseR = 180.0f; baseG = 50.0f; baseB = 10.0f; // Magma heat glow
                }
            } else if (cr == TEX_LAYER_CREATURE_PIXIE) {
                // Ultra-smooth prismatic iridescent gradient
                float grad = static_cast<float>(x + y) / 30.0f;
                baseR = 80.0f + grad * 120.0f;
                baseG = 210.0f - grad * 60.0f;
                baseB = 255.0f;
            } else if (cr == TEX_LAYER_CREATURE_BONE) {
                // Polished Ivory with smooth gradient and razor suture lines
                float gradY = static_cast<float>(y) / 15.0f;
                baseR = 230.0f - gradY * 30.0f;
                baseG = 225.0f - gradY * 30.0f;
                baseB = 210.0f - gradY * 30.0f;
                if (x == 7 && (y >= 4 && y <= 12)) {
                    baseR = 40.0f; baseG = 42.0f; baseB = 48.0f; // Razor cranial suture
                }
            }

            setPix(out, x, y, clampU8(baseR), clampU8(baseG), clampU8(baseB), 255);
        }
    }
}

} // namespace Aetheria
