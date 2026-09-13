#include "GLPipeline.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

namespace Aetheria {

GLPipeline::GLPipeline() {}

GLPipeline::~GLPipeline() {
    if (voxelProgram) glDeleteProgram(voxelProgram);
    if (hologramProgram) glDeleteProgram(hologramProgram);
    if (uiProgram) glDeleteProgram(uiProgram);
}

GLuint GLPipeline::compileShader(GLenum type, const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Failed to open shader file: " << filepath << std::endl;
        return 0;
    }
    std::stringstream ss;
    ss << file.rdbuf();
    std::string sourceStr = ss.str();
    const char* src = sourceStr.c_str();

    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    GLint success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLint logLen = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLen);
        std::vector<char> log(logLen);
        glGetShaderInfoLog(shader, logLen, nullptr, log.data());
        std::cerr << "Shader compile error in " << filepath << ":\n" << log.data() << std::endl;
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

GLuint GLPipeline::linkProgram(GLuint vert, GLuint frag) {
    if (!vert || !frag) return 0;
    GLuint prog = glCreateProgram();
    glAttachShader(prog, vert);
    glAttachShader(prog, frag);
    glLinkProgram(prog);

    GLint success = 0;
    glGetProgramiv(prog, GL_LINK_STATUS, &success);
    if (!success) {
        GLint logLen = 0;
        glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &logLen);
        std::vector<char> log(logLen);
        glGetProgramInfoLog(prog, logLen, nullptr, log.data());
        std::cerr << "Program link error:\n" << log.data() << std::endl;
        glDeleteProgram(prog);
        return 0;
    }

    glDeleteShader(vert);
    glDeleteShader(frag);
    return prog;
}

bool GLPipeline::init() {
    // 1. Voxel Program
    GLuint vVert = compileShader(GL_VERTEX_SHADER, "shaders/gl_voxel.vert");
    GLuint vFrag = compileShader(GL_FRAGMENT_SHADER, "shaders/gl_voxel.frag");
    voxelProgram = linkProgram(vVert, vFrag);
    if (!voxelProgram) return false;

    locVoxelMVP = glGetUniformLocation(voxelProgram, "uMVP");
    locVoxelModel = glGetUniformLocation(voxelProgram, "uModel");
    locVoxelSunDir = glGetUniformLocation(voxelProgram, "uSunDir");
    locVoxelCamPos = glGetUniformLocation(voxelProgram, "uCamPos");
    locVoxelEnableAO = glGetUniformLocation(voxelProgram, "uEnableAO");
    locVoxelFogColor = glGetUniformLocation(voxelProgram, "uFogColor");
    locVoxelFogStart = glGetUniformLocation(voxelProgram, "uFogStart");
    locVoxelFogEnd = glGetUniformLocation(voxelProgram, "uFogEnd");
    locVoxelEnableFog = glGetUniformLocation(voxelProgram, "uEnableFog");
    locVoxelTexArray = glGetUniformLocation(voxelProgram, "uTextureArray");
    locVoxelTime = glGetUniformLocation(voxelProgram, "uTime");
    locVoxelWavingFoliage = glGetUniformLocation(voxelProgram, "uWavingFoliage");

    // 2. Hologram Program
    GLuint hVert = compileShader(GL_VERTEX_SHADER, "shaders/gl_hologram.vert");
    GLuint hFrag = compileShader(GL_FRAGMENT_SHADER, "shaders/gl_hologram.frag");
    hologramProgram = linkProgram(hVert, hFrag);
    if (!hologramProgram) return false;

    locHoloMVP = glGetUniformLocation(hologramProgram, "uMVP");
    locHoloModel = glGetUniformLocation(hologramProgram, "uModel");
    locHoloCamPos = glGetUniformLocation(hologramProgram, "uCamPos");
    locHoloColor = glGetUniformLocation(hologramProgram, "uHoloColor");
    locHoloTime = glGetUniformLocation(hologramProgram, "uTime");

    // 3. UI Program
    GLuint uVert = compileShader(GL_VERTEX_SHADER, "shaders/gl_ui.vert");
    GLuint uFrag = compileShader(GL_FRAGMENT_SHADER, "shaders/gl_ui.frag");
    uiProgram = linkProgram(uVert, uFrag);
    if (!uiProgram) return false;

    locUIOrtho = glGetUniformLocation(uiProgram, "uOrtho");

    std::cout << "All OpenGL shaders compiled and programs linked successfully!" << std::endl;
    return true;
}

void GLPipeline::useVoxel() {
    if (voxelProgram) {
        glUseProgram(voxelProgram);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        glDisable(GL_CULL_FACE); // Two-sided rendering for cutout leaves, saplings & structures
        if (locVoxelTexArray != -1) {
            glUniform1i(locVoxelTexArray, 0);
        }
    }
}

void GLPipeline::setVoxelUniforms(const Mat4& mvp, const Mat4& model, const Vec3& sunDir, const Vec3& camPos,
                                  bool enableAO, const Vec4& fogColor, float fogStart, float fogEnd, bool enableFog,
                                  float time, bool wavingFoliage) {
    if (!voxelProgram) return;
    if (locVoxelMVP != -1) glUniformMatrix4fv(locVoxelMVP, 1, GL_FALSE, mvp.m);
    if (locVoxelModel != -1) glUniformMatrix4fv(locVoxelModel, 1, GL_FALSE, model.m);
    if (locVoxelSunDir != -1) glUniform3f(locVoxelSunDir, sunDir.x, sunDir.y, sunDir.z);
    if (locVoxelCamPos != -1) glUniform3f(locVoxelCamPos, camPos.x, camPos.y, camPos.z);
    if (locVoxelEnableAO != -1) glUniform1i(locVoxelEnableAO, enableAO ? 1 : 0);
    if (locVoxelFogColor != -1) glUniform4f(locVoxelFogColor, fogColor.x, fogColor.y, fogColor.z, fogColor.w);
    if (locVoxelFogStart != -1) glUniform1f(locVoxelFogStart, fogStart);
    if (locVoxelFogEnd != -1) glUniform1f(locVoxelFogEnd, fogEnd);
    if (locVoxelEnableFog != -1) glUniform1i(locVoxelEnableFog, enableFog ? 1 : 0);
    if (locVoxelTime != -1) glUniform1f(locVoxelTime, time);
    if (locVoxelWavingFoliage != -1) glUniform1i(locVoxelWavingFoliage, wavingFoliage ? 1 : 0);
}

void GLPipeline::useHologram() {
    if (hologramProgram) {
        glUseProgram(hologramProgram);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
}

void GLPipeline::setHologramUniforms(const Mat4& mvp, const Mat4& model, const Vec3& camPos,
                                     const Vec4& holoColor, float time) {
    if (!hologramProgram) return;
    if (locHoloMVP != -1) glUniformMatrix4fv(locHoloMVP, 1, GL_FALSE, mvp.m);
    if (locHoloModel != -1) glUniformMatrix4fv(locHoloModel, 1, GL_FALSE, model.m);
    if (locHoloCamPos != -1) glUniform3f(locHoloCamPos, camPos.x, camPos.y, camPos.z);
    if (locHoloColor != -1) glUniform4f(locHoloColor, holoColor.x, holoColor.y, holoColor.z, holoColor.w);
    if (locHoloTime != -1) glUniform1f(locHoloTime, time);
}

void GLPipeline::useUI() {
    if (uiProgram) {
        glUseProgram(uiProgram);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_CULL_FACE);
        glDisable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);
    }
}

void GLPipeline::setUIOrtho(const Mat4& ortho) {
    if (!uiProgram) return;
    if (locUIOrtho != -1) glUniformMatrix4fv(locUIOrtho, 1, GL_FALSE, ortho.m);
}

} // namespace Aetheria
