#pragma once

#include "GLHeaders.hpp"
#include "../core/Math.hpp"
#include <string>

namespace Aetheria {

class GLPipeline {
public:
    GLPipeline();
    ~GLPipeline();

    bool init();

    void useVoxel();
    void setVoxelUniforms(const Mat4& mvp, const Mat4& model, const Vec3& sunDir, const Vec3& camPos,
                          bool enableAO, const Vec4& fogColor, float fogStart, float fogEnd, bool enableFog,
                          float time = 0.0f, bool wavingFoliage = false);

    void useHologram();
    void setHologramUniforms(const Mat4& mvp, const Mat4& model, const Vec3& camPos,
                             const Vec4& holoColor, float time);

    void useUI();
    void setUIOrtho(const Mat4& ortho);

private:
    GLuint compileShader(GLenum type, const std::string& filepath);
    GLuint linkProgram(GLuint vert, GLuint frag);

    GLuint voxelProgram = 0;
    GLint locVoxelMVP = -1;
    GLint locVoxelModel = -1;
    GLint locVoxelSunDir = -1;
    GLint locVoxelCamPos = -1;
    GLint locVoxelEnableAO = -1;
    GLint locVoxelFogColor = -1;
    GLint locVoxelFogStart = -1;
    GLint locVoxelFogEnd = -1;
    GLint locVoxelEnableFog = -1;
    GLint locVoxelTexArray = -1;
    GLint locVoxelTime = -1;
    GLint locVoxelWavingFoliage = -1;

    GLuint hologramProgram = 0;
    GLint locHoloMVP = -1;
    GLint locHoloModel = -1;
    GLint locHoloCamPos = -1;
    GLint locHoloColor = -1;
    GLint locHoloTime = -1;

    GLuint uiProgram = 0;
    GLint locUIOrtho = -1;
};

} // namespace Aetheria
