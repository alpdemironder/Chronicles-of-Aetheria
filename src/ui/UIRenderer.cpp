#include "UIRenderer.hpp"
#include <algorithm>
#include <cstring>

namespace Aetheria {

// Compact 5x7 Font bitmaps for ASCII 32 to 126
static const uint8_t FONT_5X7[][5] = {
    {0x00, 0x00, 0x00, 0x00, 0x00}, // ' '
    {0x00, 0x00, 0x5F, 0x00, 0x00}, // '!'
    {0x00, 0x07, 0x00, 0x07, 0x00}, // '"'
    {0x14, 0x7F, 0x14, 0x7F, 0x14}, // '#'
    {0x24, 0x2A, 0x7F, 0x2A, 0x12}, // '$'
    {0x23, 0x13, 0x08, 0x64, 0x62}, // '%'
    {0x36, 0x49, 0x55, 0x22, 0x50}, // '&'
    {0x00, 0x05, 0x03, 0x00, 0x00}, // '''
    {0x00, 0x1C, 0x22, 0x41, 0x00}, // '('
    {0x00, 0x41, 0x22, 0x1C, 0x00}, // ')'
    {0x08, 0x2A, 0x1C, 0x2A, 0x08}, // '*'
    {0x08, 0x08, 0x3E, 0x08, 0x08}, // '+'
    {0x00, 0x50, 0x30, 0x00, 0x00}, // ','
    {0x08, 0x08, 0x08, 0x08, 0x08}, // '-'
    {0x00, 0x60, 0x60, 0x00, 0x00}, // '.'
    {0x20, 0x10, 0x08, 0x04, 0x02}, // '/'
    {0x3E, 0x51, 0x49, 0x45, 0x3E}, // '0'
    {0x00, 0x42, 0x7F, 0x40, 0x00}, // '1'
    {0x42, 0x61, 0x51, 0x49, 0x46}, // '2'
    {0x21, 0x41, 0x45, 0x4B, 0x31}, // '3'
    {0x18, 0x14, 0x12, 0x7F, 0x10}, // '4'
    {0x27, 0x45, 0x45, 0x45, 0x39}, // '5'
    {0x3C, 0x4A, 0x49, 0x49, 0x30}, // '6'
    {0x01, 0x71, 0x09, 0x05, 0x03}, // '7'
    {0x36, 0x49, 0x49, 0x49, 0x36}, // '8'
    {0x06, 0x49, 0x49, 0x29, 0x1E}, // '9'
    {0x00, 0x36, 0x36, 0x00, 0x00}, // ':'
    {0x00, 0x56, 0x36, 0x00, 0x00}, // ';'
    {0x00, 0x08, 0x14, 0x22, 0x41}, // '<'
    {0x14, 0x14, 0x14, 0x14, 0x14}, // '='
    {0x41, 0x22, 0x14, 0x08, 0x00}, // '>'
    {0x02, 0x01, 0x51, 0x09, 0x06}, // '?'
    {0x32, 0x49, 0x79, 0x41, 0x3E}, // '@'
    {0x7E, 0x11, 0x11, 0x11, 0x7E}, // 'A'
    {0x7F, 0x49, 0x49, 0x49, 0x36}, // 'B'
    {0x3E, 0x41, 0x41, 0x41, 0x22}, // 'C'
    {0x7F, 0x41, 0x41, 0x22, 0x1C}, // 'D'
    {0x7F, 0x49, 0x49, 0x49, 0x41}, // 'E'
    {0x7F, 0x09, 0x09, 0x01, 0x01}, // 'F'
    {0x3E, 0x41, 0x41, 0x51, 0x32}, // 'G'
    {0x7F, 0x08, 0x08, 0x08, 0x7F}, // 'H'
    {0x00, 0x41, 0x7F, 0x41, 0x00}, // 'I'
    {0x20, 0x40, 0x41, 0x3F, 0x01}, // 'J'
    {0x7F, 0x08, 0x14, 0x22, 0x41}, // 'K'
    {0x7F, 0x40, 0x40, 0x40, 0x40}, // 'L'
    {0x7F, 0x02, 0x04, 0x02, 0x7F}, // 'M'
    {0x7F, 0x04, 0x08, 0x10, 0x7F}, // 'N'
    {0x3E, 0x41, 0x41, 0x41, 0x3E}, // 'O'
    {0x7F, 0x09, 0x09, 0x09, 0x06}, // 'P'
    {0x3E, 0x41, 0x51, 0x21, 0x5E}, // 'Q'
    {0x7F, 0x09, 0x19, 0x29, 0x46}, // 'R'
    {0x46, 0x49, 0x49, 0x49, 0x31}, // 'S'
    {0x01, 0x01, 0x7F, 0x01, 0x01}, // 'T'
    {0x3F, 0x40, 0x40, 0x40, 0x3F}, // 'U'
    {0x1F, 0x20, 0x40, 0x20, 0x1F}, // 'V'
    {0x7F, 0x20, 0x18, 0x20, 0x7F}, // 'W'
    {0x63, 0x14, 0x08, 0x14, 0x63}, // 'X'
    {0x03, 0x04, 0x78, 0x04, 0x03}, // 'Y'
    {0x61, 0x51, 0x49, 0x45, 0x43}, // 'Z'
    {0x00, 0x7F, 0x41, 0x41, 0x00}, // '['
    {0x02, 0x04, 0x08, 0x10, 0x20}, // '\'
    {0x00, 0x41, 0x41, 0x7F, 0x00}, // ']'
    {0x04, 0x02, 0x01, 0x02, 0x04}, // '^'
    {0x40, 0x40, 0x40, 0x40, 0x40}, // '_'
    {0x00, 0x01, 0x02, 0x04, 0x00}, // '`'
    {0x20, 0x54, 0x54, 0x54, 0x78}, // 'a'
    {0x7F, 0x48, 0x44, 0x44, 0x38}, // 'b'
    {0x38, 0x44, 0x44, 0x44, 0x20}, // 'c'
    {0x38, 0x44, 0x44, 0x48, 0x7F}, // 'd'
    {0x38, 0x54, 0x54, 0x54, 0x18}, // 'e'
    {0x08, 0x7E, 0x09, 0x01, 0x02}, // 'f'
    {0x08, 0x14, 0x54, 0x54, 0x3C}, // 'g'
    {0x7F, 0x08, 0x04, 0x04, 0x78}, // 'h'
    {0x00, 0x44, 0x7D, 0x40, 0x00}, // 'i'
    {0x20, 0x40, 0x44, 0x3D, 0x00}, // 'j'
    {0x00, 0x7F, 0x10, 0x28, 0x44}, // 'k'
    {0x00, 0x41, 0x7F, 0x40, 0x00}, // 'l'
    {0x7C, 0x04, 0x18, 0x04, 0x78}, // 'm'
    {0x7C, 0x08, 0x04, 0x04, 0x78}, // 'n'
    {0x38, 0x44, 0x44, 0x44, 0x38}, // 'o'
    {0x7C, 0x14, 0x14, 0x14, 0x08}, // 'p'
    {0x08, 0x14, 0x14, 0x18, 0x7C}, // 'q'
    {0x7C, 0x08, 0x04, 0x04, 0x08}, // 'r'
    {0x48, 0x54, 0x54, 0x54, 0x20}, // 's'
    {0x04, 0x3F, 0x44, 0x40, 0x20}, // 't'
    {0x3C, 0x40, 0x40, 0x20, 0x7C}, // 'u'
    {0x1C, 0x20, 0x40, 0x20, 0x1C}, // 'v'
    {0x3C, 0x40, 0x30, 0x40, 0x3C}, // 'w'
    {0x44, 0x28, 0x10, 0x28, 0x44}, // 'x'
    {0x0C, 0x50, 0x50, 0x50, 0x3C}, // 'y'
    {0x44, 0x64, 0x54, 0x4C, 0x44}, // 'z'
    {0x00, 0x08, 0x36, 0x41, 0x00}, // '{'
    {0x00, 0x00, 0x7F, 0x00, 0x00}, // '|'
    {0x00, 0x41, 0x36, 0x08, 0x00}, // '}'
    {0x08, 0x08, 0x2A, 0x1C, 0x08}  // '~'
};

UIRenderer::UIRenderer() {
}

UIRenderer::~UIRenderer() {
}

void UIRenderer::begin(int width, int height) {
    screenWidth = width;
    screenHeight = height;
    vertices.clear();
}

void UIRenderer::addQuad(float x0, float y0, float x1, float y1, const Vec4& color) {
    // Counter-Clockwise winding in OpenGL NDC space (where Y is up):
    // Triangle 1: Top-Left (x0, y0) -> Bottom-Left (x0, y1) -> Bottom-Right (x1, y1)
    vertices.push_back({x0, y0, 0.0f, 0.0f, color.x, color.y, color.z, color.w});
    vertices.push_back({x0, y1, 0.0f, 1.0f, color.x, color.y, color.z, color.w});
    vertices.push_back({x1, y1, 1.0f, 1.0f, color.x, color.y, color.z, color.w});

    // Triangle 2: Top-Left (x0, y0) -> Bottom-Right (x1, y1) -> Top-Right (x1, y0)
    vertices.push_back({x0, y0, 0.0f, 0.0f, color.x, color.y, color.z, color.w});
    vertices.push_back({x1, y1, 1.0f, 1.0f, color.x, color.y, color.z, color.w});
    vertices.push_back({x1, y0, 1.0f, 0.0f, color.x, color.y, color.z, color.w});
}

void UIRenderer::addTriangle(float x0, float y0, float x1, float y1, float x2, float y2, const Vec4& color) {
    // In screen space, Y points downwards. In OpenGL NDC, Y points upwards.
    // Screen cross: (x1 - x0)*(y2 - y0) - (y1 - y0)*(x2 - x0).
    // If cross > 0, vertices are clockwise in screen space -> counter-clockwise in NDC!
    float cross = (x1 - x0) * (y2 - y0) - (y1 - y0) * (x2 - x0);
    if (cross < 0.0f) {
        // Swap vertices 1 and 2 to guarantee CCW in NDC
        vertices.push_back({x0, y0, 0.0f, 0.0f, color.x, color.y, color.z, color.w});
        vertices.push_back({x2, y2, 1.0f, 1.0f, color.x, color.y, color.z, color.w});
        vertices.push_back({x1, y1, 1.0f, 0.0f, color.x, color.y, color.z, color.w});
    } else {
        vertices.push_back({x0, y0, 0.0f, 0.0f, color.x, color.y, color.z, color.w});
        vertices.push_back({x1, y1, 1.0f, 0.0f, color.x, color.y, color.z, color.w});
        vertices.push_back({x2, y2, 1.0f, 1.0f, color.x, color.y, color.z, color.w});
    }
}

void UIRenderer::addQuad4P(float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3, const Vec4& color) {
    addTriangle(x0, y0, x1, y1, x2, y2, color);
    addTriangle(x0, y0, x2, y2, x3, y3, color);
}

void UIRenderer::drawLine(float x0, float y0, float x1, float y1, float thickness, const Vec4& color) {
    float dx = x1 - x0;
    float dy = y1 - y0;
    float len = std::sqrt(dx * dx + dy * dy);
    if (len < 0.001f) return;

    float nx = -dy / len * (thickness * 0.5f);
    float ny =  dx / len * (thickness * 0.5f);

    addQuad4P(x0 + nx, y0 + ny,
              x1 + nx, y1 + ny,
              x1 - nx, y1 - ny,
              x0 - nx, y0 - ny,
              color);
}

void UIRenderer::drawRect(float x, float y, float w, float h, const Vec4& color) {
    addQuad(x, y, x + w, y + h, color);
}

void UIRenderer::drawRectOutline(float x, float y, float w, float h, float thickness, const Vec4& color) {
    drawRect(x, y, w, thickness, color);                 // Top
    drawRect(x, y + h - thickness, w, thickness, color); // Bottom
    drawRect(x, y, thickness, h, color);                 // Left
    drawRect(x + w - thickness, y, thickness, h, color); // Right
}

void UIRenderer::drawProgressBar(float x, float y, float w, float h, float progress, const Vec4& fillColor, const Vec4& bgColor) {
    drawEdgyProgressBar(x, y, w, h, progress, fillColor, fillColor * 0.85f, bgColor, 3.0f);
}

void UIRenderer::drawEdgyPanel(float x, float y, float w, float h, float cut, const Vec4& bg, const Vec4& border, float borderThick) {
    float c = std::min(cut, std::min(w * 0.42f, h * 0.42f));
    float cx = x + w * 0.5f;
    float cy = y + h * 0.5f;

    // 8 perimeter vertices
    float px[8] = {x + c, x + w - c, x + w, x + w, x + w - c, x + c, x, x};
    float py[8] = {y, y, y + c, y + h - c, y + h, y + h, y + h - c, y + c};

    // Solid Background Fill
    if (bg.w > 0.001f) {
        for (int i = 0; i < 8; ++i) {
            int next = (i + 1) % 8;
            addTriangle(cx, cy, px[i], py[i], px[next], py[next], bg);
        }
    }

    // High-Tech Sleek Border
    if (border.w > 0.001f && borderThick > 0.0f) {
        for (int i = 0; i < 8; ++i) {
            int next = (i + 1) % 8;
            drawLine(px[i], py[i], px[next], py[next], borderThick, border);
        }
    }
}

void UIRenderer::drawEdgyPanelTopCut(float x, float y, float w, float h, float cut, const Vec4& bg, const Vec4& border, float borderThick) {
    float c = std::min(cut, std::min(w * 0.45f, h * 0.45f));
    float cx = x + w * 0.5f;
    float cy = y + h * 0.5f;

    // 6 vertices: top-left cut, bottom-right cut
    float px[6] = {x + c, x + w, x + w, x + w - c, x, x};
    float py[6] = {y, y, y + h - c, y + h, y + h, y + c};

    if (bg.w > 0.001f) {
        for (int i = 0; i < 6; ++i) {
            int next = (i + 1) % 6;
            addTriangle(cx, cy, px[i], py[i], px[next], py[next], bg);
        }
    }

    if (border.w > 0.001f && borderThick > 0.0f) {
        for (int i = 0; i < 6; ++i) {
            int next = (i + 1) % 6;
            drawLine(px[i], py[i], px[next], py[next], borderThick, border);
        }
    }
}

void UIRenderer::drawEdgyProgressBar(float x, float y, float w, float h, float progress, const Vec4& col1, const Vec4& col2, const Vec4& bg, float cut) {
    // 1. Outer Chassis
    drawEdgyPanel(x, y, w, h, cut, bg, {bg.x * 1.8f, bg.y * 1.8f, bg.z * 2.0f, 0.9f}, 1.2f);

    float clampedP = std::clamp(progress, 0.0f, 1.0f);
    if (clampedP < 0.005f) return;

    float pad = 2.0f;
    float bw = (w - pad * 2.0f) * clampedP;
    float bh = h - pad * 2.0f;
    float bx = x + pad;
    float by = y + pad;

    // 2. Multi-segment gradient fill
    const int SLICES = 8;
    float sliceW = bw / static_cast<float>(SLICES);
    for (int s = 0; s < SLICES; ++s) {
        float t0 = static_cast<float>(s) / static_cast<float>(SLICES);
        float t1 = static_cast<float>(s + 1) / static_cast<float>(SLICES);
        Vec4 sc = col1 * (1.0f - t0) + col2 * t0;
        addQuad(bx + s * sliceW, by, bx + (s + 1) * sliceW, by + bh, sc);
    }

    // 3. Leading edge active energy highlight
    addQuad(bx + bw - 2.5f, by, bx + bw, by + bh, {1.0f, 1.0f, 1.0f, 0.95f});

    // 4. Subtle segmented divider ticks every 25%
    for (int k = 1; k < 4; ++k) {
        float tickX = x + pad + (w - pad * 2.0f) * (k * 0.25f);
        if (tickX < bx + bw) {
            drawLine(tickX, by, tickX, by + bh, 1.0f, {0.0f, 0.0f, 0.0f, 0.45f});
        }
    }
}

void UIRenderer::drawTechBracket(float x, float y, float w, float h, float armLen, float thickness, const Vec4& color) {
    float al = std::min(armLen, std::min(w * 0.4f, h * 0.4f));
    // Top-Left
    drawLine(x, y, x + al, y, thickness, color);
    drawLine(x, y, x, y + al, thickness, color);
    // Top-Right
    drawLine(x + w, y, x + w - al, y, thickness, color);
    drawLine(x + w, y, x + w, y + al, thickness, color);
    // Bottom-Left
    drawLine(x, y + h, x + al, y + h, thickness, color);
    drawLine(x, y + h, x, y + h - al, thickness, color);
    // Bottom-Right
    drawLine(x + w, y + h, x + w - al, y + h, thickness, color);
    drawLine(x + w, y + h, x + w, y + h - al, thickness, color);
}

void UIRenderer::drawHexBadge(float cx, float cy, float radius, const Vec4& bg, const Vec4& border) {
    float px[6], py[6];
    for (int i = 0; i < 6; ++i) {
        float ang = (static_cast<float>(i) * 60.0f - 30.0f) * 0.0174532925f;
        px[i] = cx + std::cos(ang) * radius;
        py[i] = cy + std::sin(ang) * radius;
    }

    if (bg.w > 0.001f) {
        for (int i = 0; i < 6; ++i) {
            int next = (i + 1) % 6;
            addTriangle(cx, cy, px[i], py[i], px[next], py[next], bg);
        }
    }
    if (border.w > 0.001f) {
        for (int i = 0; i < 6; ++i) {
            int next = (i + 1) % 6;
            drawLine(px[i], py[i], px[next], py[next], 1.5f, border);
        }
    }
}

void UIRenderer::drawSlantedBar(float x, float y, float w, float h, float slant, const Vec4& color) {
    addQuad4P(x + slant, y, x + w + slant, y, x + w, y + h, x, y + h, color);
}

void UIRenderer::drawText(const std::string& text, float startX, float startY, float scale, const Vec4& color) {
    float curX = startX;
    float curY = startY;

    for (char c : text) {
        if (c == '\n') {
            curY += 10.0f * scale;
            curX = startX;
            continue;
        }

        if (c < 32 || c > 126) c = '?';
        int charIdx = c - 32;

        // Smooth Typographic Stroke Rendering (No blocky disconnected squares)
        // 1. Scan and render continuous vertical stroke spans
        for (int col = 0; col < 5; ++col) {
            uint8_t line = FONT_5X7[charIdx][col];
            int r = 0;
            while (r < 7) {
                if (line & (1 << r)) {
                    int rStart = r;
                    while (r < 7 && (line & (1 << r))) {
                        r++;
                    }
                    int rEnd = r;
                    float x0 = curX + col * scale;
                    float y0 = curY + rStart * scale;
                    float x1 = x0 + scale;
                    float y1 = curY + rEnd * scale;
                    addQuad(x0, y0, x1, y1, color);
                } else {
                    r++;
                }
            }
        }

        // 2. Connect continuous horizontal crossbar spans (smooth junctions for E, H, T, A, F, -, etc.)
        for (int row = 0; row < 7; ++row) {
            int c = 0;
            while (c < 5) {
                if (FONT_5X7[charIdx][c] & (1 << row)) {
                    int cStart = c;
                    while (c < 5 && (FONT_5X7[charIdx][c] & (1 << row))) {
                        c++;
                    }
                    int cEnd = c;
                    if (cEnd - cStart > 1) {
                        float x0 = curX + cStart * scale;
                        float y0 = curY + row * scale;
                        float x1 = curX + cEnd * scale;
                        float y1 = y0 + scale;
                        addQuad(x0, y0, x1, y1, color);
                    }
                } else {
                    c++;
                }
            }
        }

        curX += 6.0f * scale;
    }
}

void UIRenderer::drawTextCentered(const std::string& text, float centerX, float y, float scale, const Vec4& color) {
    float textWidth = static_cast<float>(text.length()) * 6.0f * scale;
    drawText(text, centerX - textWidth * 0.5f, y, scale, color);
}

void UIRenderer::end(GLPipeline* pipeline) {
    vertexCount = static_cast<uint32_t>(vertices.size());
    if (vertexCount == 0 || !pipeline) return;

    meshBuffer.uploadUIData(vertices.data(), vertices.size() * sizeof(UIVertex), vertexCount);

    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glDisable(GL_CULL_FACE);

    pipeline->useUI();
    Mat4 ortho = Mat4::orthoGL(0.0f, static_cast<float>(screenWidth), static_cast<float>(screenHeight), 0.0f, -1.0f, 1.0f);
    pipeline->setUIOrtho(ortho);
    meshBuffer.draw();

    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
}

} // namespace Aetheria
