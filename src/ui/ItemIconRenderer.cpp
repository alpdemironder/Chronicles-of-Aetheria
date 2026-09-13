#include "ItemIconRenderer.hpp"
#include "../world/BlockRegistry.hpp"
#include "../inventory/ItemRegistry.hpp"
#include <cmath>
#include <algorithm>

namespace Aetheria {

// Helper to scale colors for shading
static inline Vec4 shadeCol(const Vec4& c, float factor) {
    return Vec4(std::clamp(c.x * factor, 0.0f, 1.0f),
                std::clamp(c.y * factor, 0.0f, 1.0f),
                std::clamp(c.z * factor, 0.0f, 1.0f),
                c.w);
}

// Tool Tier Metallic Colors
static inline Vec4 getTierMetalCol(int tier) {
    switch (tier) {
        case 1: return {0.62f, 0.44f, 0.24f, 1.0f}; // Wood
        case 2: return {0.60f, 0.62f, 0.65f, 1.0f}; // Stone
        case 3: return {0.88f, 0.90f, 0.95f, 1.0f}; // Iron
        case 4: return {1.0f, 0.84f, 0.18f, 1.0f};  // Gold
        case 5: return {0.30f, 0.95f, 0.98f, 1.0f}; // Diamond
        case 6: return {0.95f, 0.35f, 0.95f, 1.0f}; // Aetherium
        default: return {0.85f, 0.88f, 0.92f, 1.0f};
    }
}

// =========================================================================
// 1. AUTHENTIC 3D ISOMETRIC VOXEL BLOCK RENDERER
// =========================================================================
void ItemIconRenderer::drawIsometricBlock(UIRenderer* ui, float cx, float cy, float size, uint16_t blockId) {
    if (blockId == 0) return;

    const BlockDef& bDef = BlockRegistry::get(blockId);
    Vec4 baseCol = bDef.color;

    // Isometric Cube Geometry
    float R = size * 0.44f;
    float hx = R * 0.866f;  // cos(30 deg)
    float hy = R * 0.500f;  // sin(30 deg)
    float dh = R * 0.88f;   // vertical height of side faces
    float y0 = cy - dh * 0.38f;

    // Corner vertices
    // Top face
    float t0x = cx,      t0y = y0 - hy;       // Top apex
    float t1x = cx + hx, t1y = y0;            // Right corner
    float t2x = cx,      t2y = y0 + hy;       // Center bottom corner
    float t3x = cx - hx, t3y = y0;            // Left corner

    // Bottom corners
    float b1x = t1x, b1y = t1y + dh;          // Right bottom
    float b2x = t2x, b2y = t2y + dh;          // Center bottom
    float b3x = t3x, b3y = t3y + dh;          // Left bottom

    // Face Colors with 3-Point Studio Lighting
    Vec4 topCol   = shadeCol(baseCol, 1.05f);
    Vec4 leftCol  = shadeCol(baseCol, 0.76f);
    Vec4 rightCol = shadeCol(baseCol, 0.54f);

    // Block-specific custom palettes
    bool isGrass = (blockId == 1 || blockId == 25);
    bool isCraftingTable = (blockId == 331);
    bool isFurnace = (blockId == 332);
    bool isChest = (blockId == 333);
    bool isLog = (blockId >= 71 && blockId <= 118 && ((blockId - 71) % 3 == 0));
    bool isPlank = (blockId >= 71 && blockId <= 118 && ((blockId - 72) % 3 == 0));
    bool isGlass = (blockId >= 181 && blockId <= 195);
    bool isOre = (blockId >= 121 && blockId <= 130);
    bool isBrick = (blockId >= 151 && blockId <= 165);

    if (isGrass) {
        topCol = {0.32f, 0.74f, 0.22f, 1.0f};  // Rich green turf top
        leftCol = {0.48f, 0.33f, 0.22f, 1.0f}; // Dirt side (medium)
        rightCol = {0.38f, 0.25f, 0.16f, 1.0f};// Dirt side (shadowed)
    } else if (isCraftingTable) {
        topCol = {0.72f, 0.52f, 0.32f, 1.0f};
        leftCol = {0.52f, 0.36f, 0.20f, 1.0f};
        rightCol = {0.42f, 0.28f, 0.15f, 1.0f};
    } else if (isFurnace) {
        topCol = {0.50f, 0.50f, 0.52f, 1.0f};
        leftCol = {0.40f, 0.40f, 0.42f, 1.0f};
        rightCol = {0.28f, 0.28f, 0.30f, 1.0f};
    } else if (isChest) {
        topCol = {0.68f, 0.48f, 0.25f, 1.0f};
        leftCol = {0.55f, 0.38f, 0.18f, 1.0f};
        rightCol = {0.42f, 0.28f, 0.12f, 1.0f};
    }

    // --- RENDER 3 CUBE FACES ---
    // Top Face (Rhombus)
    ui->addQuad4P(t0x, t0y, t1x, t1y, t2x, t2y, t3x, t3y, topCol);
    // Left Face (Slanted Quad)
    ui->addQuad4P(t3x, t3y, t2x, t2y, b2x, b2y, b3x, b3y, leftCol);
    // Right Face (Slanted Quad)
    ui->addQuad4P(t2x, t2y, t1x, t1y, b1x, b1y, b2x, b2y, rightCol);

    // --- BLOCK TEXTURE DETAILS & ORNAMENTATION ---
    if (isGrass) {
        // Lush grass overhang fringes hanging down from top of side faces
        Vec4 grassFringeCol = {0.28f, 0.68f, 0.18f, 1.0f};
        float fH = dh * 0.35f;
        // Left face grass drip
        ui->addQuad4P(t3x, t3y, t2x, t2y, t2x, t2y + fH * 0.85f, t3x, t3y + fH, grassFringeCol);
        // Grass tassel triangles
        ui->addTriangle(t3x + hx * 0.3f, t3y + hy * 0.3f + fH,
                        t3x + hx * 0.5f, t3y + hy * 0.5f + fH + 2.5f,
                        t3x + hx * 0.7f, t3y + hy * 0.7f + fH, grassFringeCol);

        // Right face grass drip
        Vec4 grassFringeShade = {0.22f, 0.52f, 0.15f, 1.0f};
        ui->addQuad4P(t2x, t2y, t1x, t1y, t1x, t1y + fH, t2x, t2y + fH * 0.85f, grassFringeShade);
        ui->addTriangle(t2x + hx * 0.3f, t2y - hy * 0.3f + fH,
                        t2x + hx * 0.5f, t2y - hy * 0.5f + fH + 2.5f,
                        t2x + hx * 0.7f, t2y - hy * 0.7f + fH, grassFringeShade);
    } else if (isCraftingTable) {
        // Inlaid 3x3 Grid on Tabletop
        Vec4 gridCol = {0.35f, 0.22f, 0.12f, 0.85f};
        ui->drawLine(t0x, t0y + hy * 0.66f, t2x, t2y - hy * 0.66f, 1.0f, gridCol);
        ui->drawLine(t3x + hx * 0.33f, t3y - hy * 0.33f, t1x - hx * 0.33f, t1y + hy * 0.33f, 1.0f, gridCol);
        ui->drawLine(t3x + hx * 0.66f, t3y + hy * 0.33f, t1x - hx * 0.66f, t1y - hy * 0.33f, 1.0f, gridCol);

        // Left Face: Carpenter's Tools (Saw & Hammer silhouette)
        Vec4 toolCol = {0.22f, 0.15f, 0.10f, 0.95f};
        float midLx = (t3x + t2x) * 0.5f;
        float midLy = (t3y + t2y) * 0.5f + dh * 0.40f;
        ui->drawLine(midLx - 4, midLy - 4, midLx + 4, midLy + 4, 1.8f, toolCol);
        ui->drawLine(midLx - 3, midLy + 4, midLx + 3, midLy - 2, 1.5f, {0.75f, 0.75f, 0.80f, 0.9f});

        // Right Face: Steel reinforcement corners
        Vec4 rivetCol = {0.25f, 0.25f, 0.28f, 0.95f};
        ui->drawRect(t1x - 3, t1y + 4, 2, 2, rivetCol);
        ui->drawRect(b1x - 3, b1y - 6, 2, 2, rivetCol);
    } else if (isFurnace) {
        // Left Face: Arched Furnace Hearth with Glowing Fire Embers
        float fLx = (t3x + t2x) * 0.5f;
        float fLy = (t3y + t2y) * 0.5f + dh * 0.50f;
        float mouthW = hx * 0.55f;
        float mouthH = dh * 0.50f;

        // Dark cavity
        ui->drawRect(fLx - mouthW * 0.5f, fLy - mouthH * 0.5f, mouthW, mouthH, {0.08f, 0.06f, 0.06f, 0.98f});
        ui->drawRectOutline(fLx - mouthW * 0.5f, fLy - mouthH * 0.5f, mouthW, mouthH, 1.0f, {0.18f, 0.18f, 0.20f, 0.95f});

        // Glowing hot coals & flames inside
        ui->drawRect(fLx - mouthW * 0.35f, fLy + mouthH * 0.15f, mouthW * 0.70f, mouthH * 0.28f, {0.95f, 0.25f, 0.05f, 1.0f});
        ui->addTriangle(fLx - 3, fLy + mouthH * 0.15f, fLx, fLy - 2, fLx + 3, fLy + mouthH * 0.15f, {1.0f, 0.85f, 0.10f, 1.0f});
    } else if (isLog) {
        // Annual growth rings on Top Face
        Vec4 ringCol = shadeCol(baseCol, 0.75f);
        ui->drawLine(t0x, t0y + hy * 0.4f, t2x, t2y - hy * 0.4f, 1.0f, ringCol);
        ui->drawLine(t3x + hx * 0.4f, t3y, t1x - hx * 0.4f, t1y, 1.0f, ringCol);
        // Center pith
        ui->drawRect(cx - 1.5f, y0 - 1.0f, 3.0f, 2.0f, shadeCol(baseCol, 0.45f));

        // Vertical Bark Furrows on Left and Right faces
        Vec4 barkLine = shadeCol(baseCol, 0.50f);
        ui->drawLine(t3x + hx * 0.33f, t3y + hy * 0.33f, t3x + hx * 0.33f, t3y + hy * 0.33f + dh, 1.0f, barkLine);
        ui->drawLine(t3x + hx * 0.66f, t3y + hy * 0.66f, t3x + hx * 0.66f, t3y + hy * 0.66f + dh, 1.0f, barkLine);
        ui->drawLine(t2x + hx * 0.50f, t2y - hy * 0.50f, t2x + hx * 0.50f, t2y - hy * 0.50f + dh, 1.0f, shadeCol(baseCol, 0.35f));
    } else if (isPlank) {
        // Horizontal wood plank slats on Left and Right faces
        Vec4 seamCol = shadeCol(baseCol, 0.45f);
        ui->drawLine(t3x, t3y + dh * 0.33f, t2x, t2y + dh * 0.33f, 1.0f, seamCol);
        ui->drawLine(t3x, t3y + dh * 0.66f, t2x, t2y + dh * 0.66f, 1.0f, seamCol);
        ui->drawLine(t2x, t2y + dh * 0.33f, t1x, t1y + dh * 0.33f, 1.0f, shadeCol(baseCol, 0.35f));
        ui->drawLine(t2x, t2y + dh * 0.66f, t1x, t1y + dh * 0.66f, 1.0f, shadeCol(baseCol, 0.35f));
    } else if (isOre) {
        // Distinct glittering crystalline ore chunks embedded in stone
        Vec4 oreGlintCol;
        if (blockId == 127) oreGlintCol = {0.35f, 0.98f, 1.0f, 1.0f};      // Diamond
        else if (blockId == 124) oreGlintCol = {1.0f, 0.85f, 0.20f, 1.0f};  // Gold
        else if (blockId == 122) oreGlintCol = {0.85f, 0.70f, 0.58f, 1.0f}; // Iron
        else if (blockId == 128) oreGlintCol = {0.15f, 0.95f, 0.40f, 1.0f}; // Emerald
        else if (blockId == 125) oreGlintCol = {0.95f, 0.20f, 0.20f, 1.0f}; // Redstone
        else oreGlintCol = {0.12f, 0.12f, 0.14f, 1.0f};                     // Coal

        // Top Face Gem Specks
        ui->drawRect(cx - 3, y0 - 3, 4, 3, oreGlintCol);
        ui->drawRect(cx + 4, y0 - 5, 3, 2, oreGlintCol);
        ui->drawRect(cx - 5, y0 + 2, 3, 3, oreGlintCol);
        // Left Face Gem Specks
        float mLx = (t3x + t2x) * 0.5f;
        float mLy = (t3y + t2y) * 0.5f + dh * 0.45f;
        ui->drawRect(mLx - 3, mLy - 4, 4, 4, oreGlintCol);
        ui->drawRect(mLx + 4, mLy + 3, 3, 3, oreGlintCol);
        // Right Face Gem Specks
        float mRx = (t2x + t1x) * 0.5f;
        float mRy = (t2y + t1y) * 0.5f + dh * 0.45f;
        ui->drawRect(mRx - 2, mRy - 2, 4, 3, shadeCol(oreGlintCol, 0.8f));
    } else if (isBrick) {
        // Staggered brick courses
        Vec4 mortarCol = {0.85f, 0.82f, 0.78f, 0.7f};
        ui->drawLine(t3x, t3y + dh * 0.5f, t2x, t2y + dh * 0.5f, 1.0f, mortarCol);
        ui->drawLine(t2x, t2y + dh * 0.5f, t1x, t1y + dh * 0.5f, 1.0f, mortarCol);
    } else if (isGlass) {
        // Diagonal white specular sheen streaks across ice-blue pane
        Vec4 glintCol = {1.0f, 1.0f, 1.0f, 0.75f};
        ui->drawLine(t3x + hx * 0.4f, t3y + dh * 0.2f, t3x + hx * 0.8f, t3y + dh * 0.6f, 1.5f, glintCol);
        ui->drawLine(t3x + hx * 0.6f, t3y + dh * 0.2f, t3x + hx * 0.9f, t3y + dh * 0.5f, 1.0f, glintCol);
    }

    // --- 3D BEVEL EDGES & SPECULAR CONTOUR LINES ---
    Vec4 edgeHighlight = {1.0f, 1.0f, 1.0f, 0.28f};
    Vec4 edgeShadow    = {0.0f, 0.0f, 0.0f, 0.38f};

    // Sunlit top edge highlights
    ui->drawLine(t0x, t0y, t1x, t1y, 1.0f, edgeHighlight);
    ui->drawLine(t0x, t0y, t3x, t3y, 1.0f, edgeHighlight);
    // Center ridge line
    ui->drawLine(t2x, t2y, b2x, b2y, 1.2f, edgeShadow);
    // Bottom contour silhouette
    ui->drawLine(b3x, b3y, b2x, b2y, 1.0f, edgeShadow);
    ui->drawLine(b2x, b2y, b1x, b1y, 1.0f, edgeShadow);
}

// =========================================================================
// 2. WEAPONS: SWORDS (ALL TIERS)
// =========================================================================
void ItemIconRenderer::drawSword(UIRenderer* ui, float cx, float cy, float size, int tier) {
    float s = size * 0.38f;
    Vec4 bladeCol = getTierMetalCol(tier);
    Vec4 bladeHighlight = shadeCol(bladeCol, 1.25f);
    Vec4 bladeShadow = shadeCol(bladeCol, 0.72f);
    Vec4 hiltCol = (tier >= 4) ? Vec4(1.0f, 0.84f, 0.2f, 1.0f) : Vec4(0.48f, 0.32f, 0.18f, 1.0f);

    // Diagonal from bottom-left to top-right
    float hX = cx - s * 0.65f;
    float hY = cy + s * 0.65f;
    float tX = cx + s * 0.75f;
    float tY = cy - s * 0.75f;

    // 1. Pommel
    ui->drawRect(hX - 2.5f, hY + 1.0f, 5.0f, 5.0f, hiltCol);

    // 2. Handle / Grip
    ui->drawLine(hX, hY, hX + s * 0.30f, hY - s * 0.30f, 3.0f, {0.35f, 0.22f, 0.12f, 1.0f});

    // 3. Crossguard
    float gX = hX + s * 0.32f;
    float gY = hY - s * 0.32f;
    ui->drawLine(gX - s * 0.28f, gY - s * 0.28f, gX + s * 0.28f, gY + s * 0.28f, 3.2f, hiltCol);
    ui->drawRect(gX - s * 0.30f, gY - s * 0.30f, 3.0f, 3.0f, bladeHighlight);
    ui->drawRect(gX + s * 0.28f, gY + s * 0.28f, 3.0f, 3.0f, bladeHighlight);

    // 4. Double-Edged Blade
    float bStartX = gX + s * 0.05f;
    float bStartY = gY - s * 0.05f;
    // Upper bright edge
    ui->addTriangle(bStartX - 2.0f, bStartY - 2.0f, tX, tY, bStartX, bStartY, bladeHighlight);
    // Lower shadowed edge
    ui->addTriangle(bStartX + 2.0f, bStartY + 2.0f, tX, tY, bStartX, bStartY, bladeShadow);
    // Center fuller spine
    ui->drawLine(bStartX, bStartY, tX - 3.0f, tY + 3.0f, 1.0f, bladeShadow);
}

// =========================================================================
// 3. TOOLS: PICKAXE
// =========================================================================
void ItemIconRenderer::drawPickaxe(UIRenderer* ui, float cx, float cy, float size, int tier) {
    float s = size * 0.38f;
    Vec4 headCol = getTierMetalCol(tier);
    Vec4 shaftCol = {0.55f, 0.38f, 0.22f, 1.0f};

    // Wooden shaft diagonal
    ui->drawLine(cx - s * 0.70f, cy + s * 0.70f, cx + s * 0.45f, cy - s * 0.45f, 3.0f, shaftCol);

    // Curved crescent pick head at top-right
    float hX = cx + s * 0.35f;
    float hY = cy - s * 0.35f;
    float tip1X = hX - s * 0.65f, tip1Y = hY - s * 0.15f;
    float tip2X = hX + s * 0.15f, tip2Y = hY + s * 0.65f;

    ui->addTriangle(hX, hY, tip1X, tip1Y, hX - s * 0.20f, hY - s * 0.35f, headCol);
    ui->addTriangle(hX, hY, tip2X, tip2Y, hX + s * 0.35f, hY + s * 0.20f, headCol);
    ui->drawRect(hX - 2.0f, hY - 2.0f, 5.0f, 5.0f, shadeCol(headCol, 1.25f));
}

// =========================================================================
// 4. TOOLS: AXE
// =========================================================================
void ItemIconRenderer::drawAxe(UIRenderer* ui, float cx, float cy, float size, int tier) {
    float s = size * 0.38f;
    Vec4 headCol = getTierMetalCol(tier);
    Vec4 shaftCol = {0.55f, 0.38f, 0.22f, 1.0f};

    // Shaft
    ui->drawLine(cx - s * 0.65f, cy + s * 0.65f, cx + s * 0.50f, cy - s * 0.50f, 3.0f, shaftCol);

    // Heavy bearded axe blade
    float hX = cx + s * 0.30f;
    float hY = cy - s * 0.30f;
    ui->addQuad4P(hX, hY,
                  hX - s * 0.45f, hY - s * 0.20f,
                  hX - s * 0.10f, hY + s * 0.45f,
                  hX + s * 0.15f, hY + s * 0.15f, headCol);
    // Beveled cutting edge
    ui->drawLine(hX - s * 0.45f, hY - s * 0.20f, hX - s * 0.10f, hY + s * 0.45f, 1.5f, shadeCol(headCol, 1.35f));
}

// =========================================================================
// 5. TOOLS: SHOVEL
// =========================================================================
void ItemIconRenderer::drawShovel(UIRenderer* ui, float cx, float cy, float size, int tier) {
    float s = size * 0.38f;
    Vec4 bladeCol = getTierMetalCol(tier);
    Vec4 shaftCol = {0.55f, 0.38f, 0.22f, 1.0f};

    // Shaft
    ui->drawLine(cx - s * 0.60f, cy + s * 0.60f, cx + s * 0.25f, cy - s * 0.25f, 2.5f, shaftCol);

    // D-Grip at bottom-left
    ui->drawRectOutline(cx - s * 0.70f, cy + s * 0.55f, 6.0f, 6.0f, 1.2f, shaftCol);

    // Scoop Blade at top-right
    float bX = cx + s * 0.45f;
    float bY = cy - s * 0.45f;
    ui->addTriangle(bX - 5.0f, bY + 5.0f, bX + 6.0f, bY - 6.0f, bX + 8.0f, bY + 2.0f, bladeCol);
    ui->addTriangle(bX - 5.0f, bY + 5.0f, bX + 6.0f, bY - 6.0f, bX - 2.0f, bY - 8.0f, shadeCol(bladeCol, 1.2f));
}

// =========================================================================
// 6. WEAPONS: BOW & ARROW
// =========================================================================
void ItemIconRenderer::drawBow(UIRenderer* ui, float cx, float cy, float size) {
    float s = size * 0.40f;
    Vec4 staveCol = {0.62f, 0.42f, 0.22f, 1.0f};
    Vec4 stringCol = {0.92f, 0.92f, 0.95f, 0.9f};

    // C-shaped curved stave
    ui->drawLine(cx - s * 0.5f, cy - s * 0.6f, cx + s * 0.2f, cy - s * 0.3f, 2.5f, staveCol);
    ui->drawLine(cx + s * 0.2f, cy - s * 0.3f, cx + s * 0.4f, cy, 3.0f, {0.35f, 0.22f, 0.12f, 1.0f}); // grip
    ui->drawLine(cx + s * 0.4f, cy, cx + s * 0.2f, cy + s * 0.3f, 3.0f, staveCol);
    ui->drawLine(cx + s * 0.2f, cy + s * 0.3f, cx - s * 0.5f, cy + s * 0.6f, 2.5f, staveCol);

    // Taut string
    ui->drawLine(cx - s * 0.5f, cy - s * 0.6f, cx - s * 0.5f, cy + s * 0.6f, 1.0f, stringCol);
}

void ItemIconRenderer::drawArrow(UIRenderer* ui, float cx, float cy, float size) {
    float s = size * 0.40f;
    // Shaft
    ui->drawLine(cx - s * 0.6f, cy + s * 0.6f, cx + s * 0.5f, cy - s * 0.5f, 2.0f, {0.70f, 0.55f, 0.35f, 1.0f});

    // Arrowhead at top-right
    float aX = cx + s * 0.5f;
    float aY = cy - s * 0.5f;
    ui->addTriangle(aX, aY - 6.0f, aX + 6.0f, aY, aX - 2.0f, aY + 2.0f, {0.35f, 0.35f, 0.38f, 1.0f});

    // Split feathers at nock
    float nX = cx - s * 0.5f;
    float nY = cy + s * 0.5f;
    ui->drawLine(nX, nY, nX - 4.0f, nY + 1.0f, 1.5f, {0.95f, 0.95f, 0.95f, 0.95f});
    ui->drawLine(nX, nY, nX + 1.0f, nY + 4.0f, 1.5f, {0.95f, 0.95f, 0.95f, 0.95f});
}

// =========================================================================
// 7. ARMOR PIECES (HELMET, CHEST, LEGS, BOOTS)
// =========================================================================
void ItemIconRenderer::drawArmorHelmet(UIRenderer* ui, float cx, float cy, float size, const Vec4& color, int tier) {
    float s = size * 0.38f;
    Vec4 mainCol = color;
    Vec4 highCol = shadeCol(color, 1.25f);
    Vec4 darkCol = shadeCol(color, 0.65f);

    // Domed skull cap
    ui->addQuad4P(cx - s * 0.6f, cy - s * 0.1f,
                  cx - s * 0.3f, cy - s * 0.7f,
                  cx + s * 0.3f, cy - s * 0.7f,
                  cx + s * 0.6f, cy - s * 0.1f, highCol);

    // Cheek guards & visor brow
    ui->drawRect(cx - s * 0.6f, cy - s * 0.1f, s * 1.2f, s * 0.35f, mainCol);
    ui->drawRect(cx - s * 0.6f, cy + s * 0.25f, s * 0.35f, s * 0.40f, darkCol);
    ui->drawRect(cx + s * 0.25f, cy + s * 0.25f, s * 0.35f, s * 0.40f, darkCol);

    // Eye slit / T-visor opening
    ui->drawRect(cx - s * 0.35f, cy + s * 0.05f, s * 0.70f, 2.5f, {0.08f, 0.08f, 0.10f, 0.95f});
}

void ItemIconRenderer::drawArmorChest(UIRenderer* ui, float cx, float cy, float size, const Vec4& color, int tier) {
    float s = size * 0.38f;
    Vec4 highCol = shadeCol(color, 1.25f);
    Vec4 darkCol = shadeCol(color, 0.70f);

    // Broad shoulder pauldrons
    ui->drawRect(cx - s * 0.85f, cy - s * 0.6f, s * 0.45f, s * 0.35f, highCol);
    ui->drawRect(cx + s * 0.40f, cy - s * 0.6f, s * 0.45f, s * 0.35f, highCol);

    // Sculpted pectoral breastplate
    ui->addQuad4P(cx - s * 0.55f, cy - s * 0.5f,
                  cx + s * 0.55f, cy - s * 0.5f,
                  cx + s * 0.40f, cy + s * 0.3f,
                  cx - s * 0.40f, cy + s * 0.3f, color);

    // Waist taper & belt
    ui->drawRect(cx - s * 0.40f, cy + s * 0.3f, s * 0.80f, s * 0.25f, darkCol);
    ui->drawRect(cx - 3.0f, cy + s * 0.32f, 6.0f, 4.0f, {0.95f, 0.82f, 0.20f, 1.0f}); // buckle
}

void ItemIconRenderer::drawArmorLegs(UIRenderer* ui, float cx, float cy, float size, const Vec4& color, int tier) {
    float s = size * 0.38f;
    Vec4 darkCol = shadeCol(color, 0.70f);

    // Waistband
    ui->drawRect(cx - s * 0.55f, cy - s * 0.65f, s * 1.1f, s * 0.25f, shadeCol(color, 1.15f));

    // Left leg
    ui->addQuad4P(cx - s * 0.55f, cy - s * 0.4f,
                  cx - s * 0.10f, cy - s * 0.4f,
                  cx - s * 0.15f, cy + s * 0.7f,
                  cx - s * 0.50f, cy + s * 0.7f, color);

    // Right leg
    ui->addQuad4P(cx + s * 0.10f, cy - s * 0.4f,
                  cx + s * 0.55f, cy - s * 0.4f,
                  cx + s * 0.50f, cy + s * 0.7f,
                  cx + s * 0.15f, cy + s * 0.7f, darkCol);

    // Knee poleyn guards
    ui->drawRect(cx - s * 0.45f, cy + s * 0.1f, s * 0.30f, 3.5f, shadeCol(color, 1.3f));
    ui->drawRect(cx + s * 0.18f, cy + s * 0.1f, s * 0.30f, 3.5f, shadeCol(color, 1.3f));
}

void ItemIconRenderer::drawArmorBoots(UIRenderer* ui, float cx, float cy, float size, const Vec4& color, int tier) {
    float s = size * 0.38f;
    Vec4 highCol = shadeCol(color, 1.25f);
    Vec4 darkCol = shadeCol(color, 0.65f);

    // Left Boot
    ui->drawRect(cx - s * 0.65f, cy - s * 0.4f, s * 0.35f, s * 0.55f, color);
    ui->drawRect(cx - s * 0.85f, cy + s * 0.15f, s * 0.55f, s * 0.35f, darkCol); // toe forward
    ui->drawRect(cx - s * 0.65f, cy - s * 0.5f, s * 0.35f, s * 0.15f, highCol); // cuff

    // Right Boot
    ui->drawRect(cx + s * 0.20f, cy - s * 0.4f, s * 0.35f, s * 0.55f, color);
    ui->drawRect(cx + s * 0.20f, cy + s * 0.15f, s * 0.55f, s * 0.35f, darkCol); // toe forward
    ui->drawRect(cx + s * 0.20f, cy - s * 0.5f, s * 0.35f, s * 0.15f, highCol); // cuff
}

// =========================================================================
// 8. MATERIALS: BULLION INGOTS
// =========================================================================
void ItemIconRenderer::drawIngot(UIRenderer* ui, float cx, float cy, float size, const Vec4& color) {
    float w = size * 0.38f;
    float h = size * 0.24f;
    Vec4 topCol   = shadeCol(color, 1.20f);
    Vec4 frontCol = shadeCol(color, 0.85f);
    Vec4 rightCol = shadeCol(color, 0.65f);

    // 3D Trapezoidal Bullion Bar
    // Top face
    ui->addQuad4P(cx - w * 0.6f, cy - h * 0.5f,
                  cx + w * 0.4f, cy - h * 0.5f,
                  cx + w * 0.2f, cy - h * 0.05f,
                  cx - w * 0.8f, cy - h * 0.05f, topCol);

    // Front face
    ui->addQuad4P(cx - w * 0.8f, cy - h * 0.05f,
                  cx + w * 0.2f, cy - h * 0.05f,
                  cx + w * 0.05f, cy + h * 0.55f,
                  cx - w * 0.95f, cy + h * 0.55f, frontCol);

    // Right face
    ui->addQuad4P(cx + w * 0.2f, cy - h * 0.05f,
                  cx + w * 0.4f, cy - h * 0.5f,
                  cx + w * 0.6f, cy + h * 0.15f,
                  cx + w * 0.05f, cy + h * 0.55f, rightCol);

    // Specular diagonal shine streak on top face
    ui->drawLine(cx - w * 0.3f, cy - h * 0.4f, cx - w * 0.1f, cy - h * 0.15f, 1.2f, {1, 1, 1, 0.75f});
}

// =========================================================================
// 9. CONSUMABLES: POTIONS, FOOD, PAL SPHERE, TORCH
// =========================================================================
void ItemIconRenderer::drawPotion(UIRenderer* ui, float cx, float cy, float size, const Vec4& liquidCol) {
    float s = size * 0.36f;

    // Glass bulb body
    ui->drawRect(cx - s * 0.6f, cy - s * 0.2f, s * 1.2f, s * 0.95f, {0.85f, 0.92f, 0.98f, 0.35f});
    ui->drawRectOutline(cx - s * 0.6f, cy - s * 0.2f, s * 1.2f, s * 0.95f, 1.2f, {0.9f, 0.95f, 1.0f, 0.85f});

    // Glowing Magical Liquid
    ui->drawRect(cx - s * 0.5f, cy + s * 0.05f, s * 1.0f, s * 0.65f, liquidCol);
    // Bubble specks
    ui->drawRect(cx - 3, cy + s * 0.2f, 2, 2, {1, 1, 1, 0.8f});
    ui->drawRect(cx + 2, cy + s * 0.4f, 2, 2, {1, 1, 1, 0.8f});

    // Narrow bottle neck
    ui->drawRect(cx - s * 0.22f, cy - s * 0.65f, s * 0.44f, s * 0.48f, {0.85f, 0.92f, 0.98f, 0.5f});
    ui->drawRectOutline(cx - s * 0.22f, cy - s * 0.65f, s * 0.44f, s * 0.48f, 1.0f, {0.9f, 0.95f, 1.0f, 0.85f});

    // Wooden Cork Stopper
    ui->drawRect(cx - s * 0.28f, cy - s * 0.80f, s * 0.56f, s * 0.20f, {0.55f, 0.35f, 0.18f, 1.0f});

    // Curved white glass reflection glint
    ui->drawLine(cx - s * 0.45f, cy - s * 0.1f, cx - s * 0.45f, cy + s * 0.55f, 1.5f, {1, 1, 1, 0.85f});
}

void ItemIconRenderer::drawFoodMeat(UIRenderer* ui, float cx, float cy, float size, bool cooked) {
    float s = size * 0.38f;
    Vec4 meatCol = cooked ? Vec4(0.72f, 0.40f, 0.20f, 1.0f) : Vec4(0.88f, 0.28f, 0.28f, 1.0f);

    // Thick steak cutlet
    ui->addQuad4P(cx - s * 0.6f, cy - s * 0.3f,
                  cx + s * 0.4f, cy - s * 0.5f,
                  cx + s * 0.6f, cy + s * 0.4f,
                  cx - s * 0.3f, cy + s * 0.6f, meatCol);

    // White bone shank
    ui->drawLine(cx - s * 0.45f, cy + s * 0.45f, cx - s * 0.75f, cy + s * 0.75f, 3.5f, {0.95f, 0.95f, 0.92f, 1.0f});
    ui->drawRect(cx - s * 0.85f, cy + s * 0.70f, 4.0f, 4.0f, {0.95f, 0.95f, 0.92f, 1.0f});

    if (cooked) {
        // Charred grill marks
        Vec4 grillCol = {0.35f, 0.18f, 0.10f, 0.95f};
        ui->drawLine(cx - s * 0.2f, cy - s * 0.3f, cx + s * 0.3f, cy + s * 0.2f, 1.5f, grillCol);
        ui->drawLine(cx - s * 0.4f, cy - s * 0.05f, cx + s * 0.1f, cy + s * 0.45f, 1.5f, grillCol);
    }
}

void ItemIconRenderer::drawBread(UIRenderer* ui, float cx, float cy, float size) {
    float s = size * 0.40f;
    Vec4 crustCol = {0.78f, 0.55f, 0.25f, 1.0f};

    // Rounded loaf
    ui->addQuad4P(cx - s * 0.7f, cy,
                  cx, cy - s * 0.45f,
                  cx + s * 0.7f, cy,
                  cx, cy + s * 0.35f, crustCol);

    // Scoring cuts across loaf
    Vec4 cutCol = {0.92f, 0.75f, 0.45f, 1.0f};
    ui->drawLine(cx - s * 0.35f, cy - s * 0.1f, cx - s * 0.15f, cy + s * 0.15f, 1.8f, cutCol);
    ui->drawLine(cx - s * 0.05f, cy - s * 0.2f, cx + s * 0.15f, cy + s * 0.05f, 1.8f, cutCol);
    ui->drawLine(cx + s * 0.25f, cy - s * 0.3f, cx + s * 0.45f, cy - s * 0.05f, 1.8f, cutCol);
}

void ItemIconRenderer::drawApple(UIRenderer* ui, float cx, float cy, float size, bool golden) {
    float s = size * 0.38f;
    Vec4 appleCol = golden ? Vec4(1.0f, 0.85f, 0.18f, 1.0f) : Vec4(0.92f, 0.18f, 0.20f, 1.0f);

    // Plump apple body
    ui->drawRect(cx - s * 0.55f, cy - s * 0.4f, s * 1.1f, s * 1.0f, appleCol);
    ui->drawRectOutline(cx - s * 0.55f, cy - s * 0.4f, s * 1.1f, s * 1.0f, 1.2f, shadeCol(appleCol, 0.7f));

    // Wooden Stem
    ui->drawLine(cx, cy - s * 0.4f, cx + 2.5f, cy - s * 0.75f, 2.0f, {0.45f, 0.30f, 0.15f, 1.0f});

    // Green Leaf
    ui->addTriangle(cx + 2.0f, cy - s * 0.65f, cx + 7.0f, cy - s * 0.85f, cx + 5.0f, cy - s * 0.55f, {0.25f, 0.85f, 0.25f, 1.0f});

    // White shine spot
    ui->drawRect(cx - s * 0.35f, cy - s * 0.25f, 2.5f, 3.5f, {1, 1, 1, 0.75f});
}

void ItemIconRenderer::drawPalSphere(UIRenderer* ui, float cx, float cy, float size, uint16_t sphereId) {
    float r = size * 0.35f;

    Vec4 topColor = {0.15f, 0.45f, 0.85f, 1.0f};      // Pal Sphere Blue
    Vec4 outlineColor = {0.05f, 0.15f, 0.30f, 0.95f};
    Vec4 coreColor = {0.0f, 0.95f, 1.0f, 1.0f};       // Cyan energy

    if (sphereId == 526) {
        topColor = {0.10f, 0.82f, 0.58f, 1.0f};       // Mega Sphere Emerald
        outlineColor = {0.80f, 0.50f, 0.15f, 0.95f};  // Bronze-gold trim
        coreColor = {1.0f, 0.85f, 0.20f, 1.0f};       // Amber energy
    } else if (sphereId == 527) {
        topColor = {0.68f, 0.18f, 0.88f, 1.0f};       // Giga Sphere Royal Violet
        outlineColor = {0.95f, 0.80f, 0.15f, 0.95f};  // Masterwork gold trim
        coreColor = {0.20f, 1.0f, 0.95f, 1.0f};       // Radiant turquoise core
    }

    // Outer Sphere
    ui->drawRect(cx - r, cy - r, r * 2.0f, r * 2.0f, topColor);
    ui->drawRectOutline(cx - r, cy - r, r * 2.0f, r * 2.0f, 1.5f, outlineColor);

    // White lower half band
    ui->drawRect(cx - r, cy, r * 2.0f, r, {0.92f, 0.94f, 0.96f, 1.0f});

    // Black equator seam
    ui->drawRect(cx - r, cy - 1.5f, r * 2.0f, 3.0f, {0.12f, 0.14f, 0.18f, 1.0f});

    // Glowing Central Energy Core button
    ui->drawRect(cx - 3.5f, cy - 3.5f, 7.0f, 7.0f, coreColor);
    ui->drawRectOutline(cx - 3.5f, cy - 3.5f, 7.0f, 7.0f, 1.0f, {1, 1, 1, 1});
}

void ItemIconRenderer::drawTorch(UIRenderer* ui, float cx, float cy, float size) {
    float s = size * 0.38f;

    // Wooden stake
    ui->drawLine(cx - s * 0.35f, cy + s * 0.7f, cx + s * 0.25f, cy - s * 0.2f, 3.2f, {0.52f, 0.36f, 0.20f, 1.0f});

    // Burning flame head
    float fX = cx + s * 0.30f;
    float fY = cy - s * 0.35f;
    // Outer orange body
    ui->addTriangle(fX - 4, fY + 4, fX, fY - 8, fX + 4, fY + 4, {0.95f, 0.40f, 0.05f, 1.0f});
    // Inner yellow hot core
    ui->addTriangle(fX - 2, fY + 3, fX, fY - 5, fX + 2, fY + 3, {1.0f, 0.90f, 0.20f, 1.0f});
    // White spark tip
    ui->drawRect(fX - 1, fY - 6, 2, 2, {1, 1, 1, 0.9f});
}

void ItemIconRenderer::drawMineral(UIRenderer* ui, float cx, float cy, float size, const Vec4& color, bool gem) {
    float s = size * 0.35f;

    if (gem) {
        // Brilliant-cut faceted jewel
        ui->addQuad4P(cx - s * 0.6f, cy - s * 0.2f,
                      cx + s * 0.6f, cy - s * 0.2f,
                      cx + s * 0.35f, cy - s * 0.6f,
                      cx - s * 0.35f, cy - s * 0.6f, shadeCol(color, 1.25f)); // table/crown

        ui->addTriangle(cx - s * 0.6f, cy - s * 0.2f,
                        cx + s * 0.6f, cy - s * 0.2f,
                        cx, cy + s * 0.7f, shadeCol(color, 0.85f)); // pavilion
        ui->drawLine(cx - s * 0.6f, cy - s * 0.2f, cx + s * 0.6f, cy - s * 0.2f, 1.2f, {1, 1, 1, 0.8f});
    } else {
        // Mineral chunk / coal lump
        ui->addQuad4P(cx - s * 0.6f, cy - s * 0.3f,
                      cx + s * 0.4f, cy - s * 0.5f,
                      cx + s * 0.6f, cy + s * 0.3f,
                      cx - s * 0.4f, cy + s * 0.5f, color);
        ui->drawRectOutline(cx - s * 0.5f, cy - s * 0.4f, s * 0.9f, s * 0.8f, 1.0f, shadeCol(color, 1.3f));
    }
}

void ItemIconRenderer::drawStick(UIRenderer* ui, float cx, float cy, float size) {
    float s = size * 0.38f;
    ui->drawLine(cx - s * 0.65f, cy + s * 0.65f, cx + s * 0.65f, cy - s * 0.65f, 2.5f, {0.55f, 0.40f, 0.22f, 1.0f});
    ui->drawLine(cx + s * 0.10f, cy - s * 0.10f, cx + s * 0.25f, cy - s * 0.35f, 1.5f, {0.48f, 0.34f, 0.18f, 1.0f});
}

void ItemIconRenderer::drawTwine(UIRenderer* ui, float cx, float cy, float size, const Vec4& color) {
    float r = size * 0.32f;
    ui->drawRectOutline(cx - r, cy - r, r * 2.0f, r * 2.0f, 2.2f, color);
    ui->drawLine(cx - r, cy + r, cx + r, cy - r, 1.8f, color);
    ui->drawLine(cx - r * 0.5f, cy + r, cx + r, cy - r * 0.5f, 1.5f, color);
}

void ItemIconRenderer::drawPelt(UIRenderer* ui, float cx, float cy, float size, const Vec4& color) {
    float s = size * 0.36f;
    ui->addQuad4P(cx - s * 0.65f, cy - s * 0.5f,
                  cx + s * 0.65f, cy - s * 0.5f,
                  cx + s * 0.45f, cy + s * 0.6f,
                  cx - s * 0.45f, cy + s * 0.6f, color);
    ui->drawRectOutline(cx - s * 0.65f, cy - s * 0.5f, s * 1.3f, s * 1.1f, 1.0f, shadeCol(color, 0.7f));
}

void ItemIconRenderer::drawBucket(UIRenderer* ui, float cx, float cy, float size) {
    float s = size * 0.36f;
    Vec4 ironCol = {0.78f, 0.82f, 0.88f, 1.0f};

    // Tapered bucket pail
    ui->addQuad4P(cx - s * 0.6f, cy - s * 0.3f,
                  cx + s * 0.6f, cy - s * 0.3f,
                  cx + s * 0.4f, cy + s * 0.6f,
                  cx - s * 0.4f, cy + s * 0.6f, ironCol);
    // Wire handle arch
    ui->drawRectOutline(cx - s * 0.55f, cy - s * 0.7f, s * 1.1f, s * 0.5f, 1.2f, {0.6f, 0.6f, 0.65f, 1.0f});
}

void ItemIconRenderer::drawTechCore(UIRenderer* ui, float cx, float cy, float size, bool ancient) {
    float s = size * 0.38f;
    Vec4 coreCol = ancient ? Vec4(0.85f, 0.35f, 0.95f, 1.0f) : Vec4(0.20f, 0.85f, 0.95f, 1.0f);

    // Outer gyro casing
    ui->drawRect(cx - s * 0.6f, cy - s * 0.6f, s * 1.2f, s * 1.2f, {0.18f, 0.20f, 0.25f, 0.95f});
    ui->drawRectOutline(cx - s * 0.6f, cy - s * 0.6f, s * 1.2f, s * 1.2f, 1.5f, coreCol);

    // Glowing relic center
    ui->drawRect(cx - s * 0.3f, cy - s * 0.3f, s * 0.6f, s * 0.6f, coreCol);
    ui->drawRect(cx - 2, cy - 2, 4, 4, {1, 1, 1, 1});
}

// =========================================================================
// 10. MAIN UNIFIED ITEM DISPATCHER
// =========================================================================
void ItemIconRenderer::drawItem(UIRenderer* ui, float cx, float cy, float size, uint16_t itemId) {
    if (itemId == 0 || !ui) return;

    // 1. Check if it's one of the voxel blocks (1..369)
    if (itemId >= 1 && itemId <= 369) {
        drawIsometricBlock(ui, cx, cy, size, itemId);
        return;
    }

    const ItemDef& def = ItemRegistry::get(itemId);

    if (def.isPlaceableBlock) {
        drawIsometricBlock(ui, cx, cy, size, itemId);
        return;
    }

    // 2. Custom RPG Weapons & Tools
    if (def.category == ItemCategory::Weapon) {
        if (def.toolType == ToolType::Sword) {
            drawSword(ui, cx, cy, size, def.toolTier);
            return;
        } else if (def.toolType == ToolType::Bow) {
            drawBow(ui, cx, cy, size);
            return;
        } else if (itemId == 516) { // Wooden Arrows
            drawArrow(ui, cx, cy, size);
            return;
        }
    } else if (def.category == ItemCategory::Tool) {
        if (def.toolType == ToolType::Pickaxe) {
            drawPickaxe(ui, cx, cy, size, def.toolTier);
            return;
        } else if (def.toolType == ToolType::Axe) {
            drawAxe(ui, cx, cy, size, def.toolTier);
            return;
        } else if (def.toolType == ToolType::Shovel) {
            drawShovel(ui, cx, cy, size, def.toolTier);
            return;
        } else if (itemId == 563) { // Iron Bucket
            drawBucket(ui, cx, cy, size);
            return;
        }
    }

    // 3. Custom RPG Armor
    if (def.category == ItemCategory::Armor) {
        if (def.armorType == ArmorType::Helmet) {
            drawArmorHelmet(ui, cx, cy, size, def.color, def.toolTier);
            return;
        } else if (def.armorType == ArmorType::Chestplate) {
            drawArmorChest(ui, cx, cy, size, def.color, def.toolTier);
            return;
        } else if (def.armorType == ArmorType::Leggings) {
            drawArmorLegs(ui, cx, cy, size, def.color, def.toolTier);
            return;
        } else if (def.armorType == ArmorType::Boots) {
            drawArmorBoots(ui, cx, cy, size, def.color, def.toolTier);
            return;
        }
    }

    // 4. Custom Materials & Consumables by ID
    switch (itemId) {
        // Ingots
        case 501: // Iron Ingot
        case 502: // Copper Ingot
        case 503: // Gold Ingot
            drawIngot(ui, cx, cy, size, def.color);
            return;

        // Potions
        case 520: // Health Potion
            drawPotion(ui, cx, cy, size, {0.95f, 0.18f, 0.22f, 0.95f});
            return;
        case 521: // Mana Elixir
            drawPotion(ui, cx, cy, size, {0.20f, 0.55f, 1.0f, 0.95f});
            return;

        // Food
        case 508: // Raw Meat
            drawFoodMeat(ui, cx, cy, size, false);
            return;
        case 522: // Cooked Feast
            drawFoodMeat(ui, cx, cy, size, true);
            return;
        case 560: // Baked Bread
            drawBread(ui, cx, cy, size);
            return;
        case 561: // Red Apple
            drawApple(ui, cx, cy, size, false);
            return;
        case 562: // Golden Apple
            drawApple(ui, cx, cy, size, true);
            return;

        // Pal Tech & Torches
        case 523: // Pal Sphere
        case 526: // Mega Sphere
        case 527: // Giga Sphere
            drawPalSphere(ui, cx, cy, size, itemId);
            return;
        case 551: // Torch
            drawTorch(ui, cx, cy, size);
            return;

        // Minerals & Gems
        case 552: // Coal
        case 553: // Charcoal
            drawMineral(ui, cx, cy, size, {0.16f, 0.16f, 0.18f, 1.0f}, false);
            return;
        case 554: // Diamond Gem
            drawMineral(ui, cx, cy, size, {0.35f, 0.95f, 1.0f, 1.0f}, true);
            return;
        case 555: // Emerald Gem
            drawMineral(ui, cx, cy, size, {0.15f, 0.95f, 0.45f, 1.0f}, true);
            return;
        case 558: // Flint Shard
            drawMineral(ui, cx, cy, size, {0.38f, 0.38f, 0.40f, 1.0f}, false);
            return;

        // Sticks, Fiber, Pelts
        case 550: // Wooden Stick
            drawStick(ui, cx, cy, size);
            return;
        case 504: // Plant Fiber Rope
        case 556: // Silk String
            drawTwine(ui, cx, cy, size, def.color);
            return;
        case 505: // Animal Pelt
        case 506: // Tanned Leather
            drawPelt(ui, cx, cy, size, def.color);
            return;

        // Tech Relics
        case 507: // Golem Core
            drawTechCore(ui, cx, cy, size, false);
            return;
        case 524: // Ancient Technology
            drawTechCore(ui, cx, cy, size, true);
            return;

        default:
            // Fallback for any other custom material: stylized diamond token with border
            float r = size * 0.32f;
            ui->addQuad4P(cx, cy - r, cx + r, cy, cx, cy + r, cx - r, cy, def.color);
            ui->drawRectOutline(cx - r * 0.7f, cy - r * 0.7f, r * 1.4f, r * 1.4f, 1.0f, {1, 1, 1, 0.8f});
            return;
    }
}

} // namespace Aetheria
