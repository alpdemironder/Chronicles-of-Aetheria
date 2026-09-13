#pragma once

#include "../core/Math.hpp"
#include "../render/GLBuffer.hpp"
#include "../render/GLPipeline.hpp"
#include <vector>
#include <string>

namespace Aetheria {

struct UIVertex {
    float x, y;
    float u, v;
    float r, g, b, a;
};

class UIRenderer {
public:
    UIRenderer();
    ~UIRenderer();

    void begin(int screenWidth, int screenHeight);
    void end(GLPipeline* pipeline);

    void drawRect(float x, float y, float w, float h, const Vec4& color);
    void drawRectOutline(float x, float y, float w, float h, float thickness, const Vec4& color);
    void drawProgressBar(float x, float y, float w, float h, float progress, const Vec4& fillColor, const Vec4& bgColor);
    void drawText(const std::string& text, float x, float y, float scale = 2.0f, const Vec4& color = {1, 1, 1, 1});
    void drawTextCentered(const std::string& text, float centerX, float y, float scale = 2.0f, const Vec4& color = {1, 1, 1, 1});

    // Low-level drawing primitives for 3D isometric facets, custom shapes, and icons
    void addTriangle(float x0, float y0, float x1, float y1, float x2, float y2, const Vec4& color);
    inline void drawTriangle(float x0, float y0, float x1, float y1, float x2, float y2, const Vec4& color) {
        addTriangle(x0, y0, x1, y1, x2, y2, color);
    }
    void addQuad4P(float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3, const Vec4& color);
    void drawLine(float x0, float y0, float x1, float y1, float thickness, const Vec4& color);

    // High-Tech & Edgy UI Elements (Chamfered, sleek, non-pixelated)
    void drawEdgyPanel(float x, float y, float w, float h, float cut, const Vec4& bg, const Vec4& border, float borderThick = 1.5f);
    void drawEdgyPanelTopCut(float x, float y, float w, float h, float cut, const Vec4& bg, const Vec4& border, float borderThick = 1.5f);
    void drawEdgyProgressBar(float x, float y, float w, float h, float progress, const Vec4& col1, const Vec4& col2, const Vec4& bg, float cut = 4.0f);
    void drawTechBracket(float x, float y, float w, float h, float armLen, float thickness, const Vec4& color);
    void drawHexBadge(float cx, float cy, float radius, const Vec4& bg, const Vec4& border);
    void drawSlantedBar(float x, float y, float w, float h, float slant, const Vec4& color);

private:
    void addQuad(float x0, float y0, float x1, float y1, const Vec4& color);

    int screenWidth = 1280;
    int screenHeight = 720;

    std::vector<UIVertex> vertices;
    GLBuffer meshBuffer;
    uint32_t vertexCount = 0;
};

} // namespace Aetheria
