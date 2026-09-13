#define NOMINMAX
#include "IrisShaderManager.hpp"
#include <windows.h>
#include <shellapi.h>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

namespace Aetheria {

namespace fs = std::filesystem;

IrisShaderManager::IrisShaderManager() {
}

IrisShaderManager::~IrisShaderManager() {
    cleanup();
}

bool IrisShaderManager::init(int width, int height) {
    viewportWidth = width > 0 ? width : 1280;
    viewportHeight = height > 0 ? height : 720;

    initQuad();
    scanShaderpacks();
    loadConfig();

    if (!initFBOs(viewportWidth, viewportHeight)) {
        std::cerr << "[Iris] Failed to initialize Iris Framebuffers!" << std::endl;
        return false;
    }

    if (activePackIndex > 0 && activePackIndex < (int)shaderpacks.size()) {
        loadShadersForPack(shaderpacks[activePackIndex].path);
    }

    std::cout << "[Iris] Iris Shader System initialized successfully (" << shaderpacks.size() << " packs detected)." << std::endl;
    return true;
}

void IrisShaderManager::resize(int width, int height) {
    if (width <= 0 || height <= 0) return;
    if (width == viewportWidth && height == viewportHeight) return;

    viewportWidth = width;
    viewportHeight = height;
    hasPrevViewProj = false;
    initFBOs(viewportWidth, viewportHeight);
}

void IrisShaderManager::cleanup() {
    destroyFBOs();
    hasPrevViewProj = false;

    if (quadVAO) {
        glDeleteVertexArrays(1, &quadVAO);
        quadVAO = 0;
    }
    if (quadVBO) {
        glDeleteBuffers(1, &quadVBO);
        quadVBO = 0;
    }
    if (compositeProgram) {
        glDeleteProgram(compositeProgram);
        compositeProgram = 0;
    }
    if (finalProgram) {
        glDeleteProgram(finalProgram);
        finalProgram = 0;
    }
}

void IrisShaderManager::initQuad() {
    if (quadVAO != 0) return;

    // 2 Triangles covering NDC [-1, 1], UV [0, 1]
    float quadVertices[] = {
        // x,     y,     u,    v
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f,

        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f,
        -1.0f,  1.0f,  0.0f, 1.0f
    };

    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);

    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    // Position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

    // TexCoord
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void IrisShaderManager::destroyFBOs() {
    if (gBufferFBO) {
        glDeleteFramebuffers(1, &gBufferFBO);
        gBufferFBO = 0;
    }
    if (colortex0) {
        glDeleteTextures(1, &colortex0);
        colortex0 = 0;
    }
    if (colortex1) {
        glDeleteTextures(1, &colortex1);
        colortex1 = 0;
    }
    if (depthtex0) {
        glDeleteTextures(1, &depthtex0);
        depthtex0 = 0;
    }

    if (compositeFBO) {
        glDeleteFramebuffers(1, &compositeFBO);
        compositeFBO = 0;
    }
    if (compositeTex0) {
        glDeleteTextures(1, &compositeTex0);
        compositeTex0 = 0;
    }
}

bool IrisShaderManager::initFBOs(int width, int height) {
    destroyFBOs();

    // 1. Create G-Buffer FBO
    glGenFramebuffers(1, &gBufferFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, gBufferFBO);

    // colortex0 (HDR Color, RGBA16F)
    glGenTextures(1, &colortex0);
    glBindTexture(GL_TEXTURE_2D, colortex0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colortex0, 0);

    // colortex1 (Normals + Material flags, RGBA8)
    glGenTextures(1, &colortex1);
    glBindTexture(GL_TEXTURE_2D, colortex1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, colortex1, 0);

    // depthtex0 (Depth24 + Stencil8)
    glGenTextures(1, &depthtex0);
    glBindTexture(GL_TEXTURE_2D, depthtex0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depthtex0, 0);

    GLenum gDrawBuffers[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    glDrawBuffers(2, gDrawBuffers);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "[Iris] G-Buffer Framebuffer not complete!" << std::endl;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return false;
    }

    // 2. Create Composite FBO
    glGenFramebuffers(1, &compositeFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, compositeFBO);

    // compositeTex0 (RGBA16F)
    glGenTextures(1, &compositeTex0);
    glBindTexture(GL_TEXTURE_2D, compositeTex0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, compositeTex0, 0);

    GLenum compDrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, compDrawBuffers);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "[Iris] Composite Framebuffer not complete!" << std::endl;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return false;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return true;
}

void IrisShaderManager::scanShaderpacks() {
    shaderpacks.clear();

    // Index 0 is always OFF (Vanilla)
    shaderpacks.push_back({ "OFF", "OFF (Vanilla Engine)", "", true });

    try {
        if (!fs::exists("shaderpacks")) {
            fs::create_directories("shaderpacks");
        }

        for (const auto& entry : fs::directory_iterator("shaderpacks")) {
            if (entry.is_directory()) {
                std::string folderName = entry.path().filename().string();
                fs::path shaderDir = entry.path() / "shaders";
                if (fs::exists(shaderDir) && fs::is_directory(shaderDir)) {
                    std::string displayName = folderName;
                    std::replace(displayName.begin(), displayName.end(), '_', ' ');
                    shaderpacks.push_back({ folderName, displayName, entry.path().string(), true });
                }
            }
        }
    } catch (const std::exception& ex) {
        std::cerr << "[Iris] Error scanning shaderpacks: " << ex.what() << std::endl;
    }
}

std::string IrisShaderManager::getActivePackName() const {
    if (activePackIndex >= 0 && activePackIndex < (int)shaderpacks.size()) {
        return shaderpacks[activePackIndex].name;
    }
    return "OFF";
}

bool IrisShaderManager::setActivePack(int index) {
    if (index < 0 || index >= (int)shaderpacks.size()) return false;
    activePackIndex = index;

    if (activePackIndex == 0) {
        saveConfig();
        return true;
    }

    bool ok = loadShadersForPack(shaderpacks[activePackIndex].path);
    saveConfig();
    return ok;
}

bool IrisShaderManager::setActivePack(const std::string& packName) {
    for (size_t i = 0; i < shaderpacks.size(); ++i) {
        if (shaderpacks[i].name == packName) {
            return setActivePack((int)i);
        }
    }
    return false;
}

bool IrisShaderManager::reloadCurrentPack() {
    scanShaderpacks();
    if (activePackIndex > 0 && activePackIndex < (int)shaderpacks.size()) {
        return loadShadersForPack(shaderpacks[activePackIndex].path);
    }
    return true;
}

void IrisShaderManager::applyProfile(const std::string& profile) {
    currentProfile = profile;
    if (profile == "Low") {
        optVolumetricLight = false;
        optWaterReflections = false;
        optSSAO = false;
        optBloom = false;
        optWavingFoliage = false;
        optMotionBlur = false;
        optMotionBlurStrength = 0.0f;
        optToneMapping = 1;
    } else if (profile == "Medium") {
        optVolumetricLight = true;
        optWaterReflections = false;
        optSSAO = false;
        optBloom = true;
        optWavingFoliage = false;
        optMotionBlur = true;
        optMotionBlurStrength = 0.40f;
        optToneMapping = 0;
    } else if (profile == "High") {
        optVolumetricLight = true;
        optWaterReflections = true;
        optSSAO = true;
        optBloom = true;
        optWavingFoliage = true;
        optMotionBlur = true;
        optMotionBlurStrength = 0.60f;
        optToneMapping = 0;
    } else if (profile == "Ultra") {
        optVolumetricLight = true;
        optWaterReflections = true;
        optSSAO = true;
        optBloom = true;
        optWavingFoliage = true;
        optMotionBlur = true;
        optMotionBlurStrength = 0.75f;
        optToneMapping = 0;
    } else if (profile == "Extreme") {
        optVolumetricLight = true;
        optWaterReflections = true;
        optSSAO = true;
        optBloom = true;
        optWavingFoliage = true;
        optMotionBlur = true;
        optMotionBlurStrength = 0.90f;
        optToneMapping = 2;
    }
    saveConfig();
}

void IrisShaderManager::loadConfig() {
    std::ifstream file("shaderpacks/iris.properties");
    if (!file.is_open()) return;

    std::string line;
    std::string targetPack = "Complementary_Reimagined";
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;
        size_t eq = line.find('=');
        if (eq == std::string::npos) continue;

        std::string key = line.substr(0, eq);
        std::string val = line.substr(eq + 1);

        if (key == "shaderPack") targetPack = val;
        else if (key == "profile") currentProfile = val;
        else if (key == "volumetricLight") optVolumetricLight = (val == "true");
        else if (key == "waterReflections") optWaterReflections = (val == "true");
        else if (key == "ssAO" || key == "ssao") optSSAO = (val == "true");
        else if (key == "bloom") optBloom = (val == "true");
        else if (key == "wavingFoliage") optWavingFoliage = (val == "true");
        else if (key == "motionBlur") optMotionBlur = (val == "true");
        else if (key == "motionBlurStrength") {
            try { optMotionBlurStrength = std::stof(val); } catch (...) {}
        }
        else if (key == "toneMapping") optToneMapping = std::stoi(val);
    }

    activePackIndex = 0;
    for (size_t i = 0; i < shaderpacks.size(); ++i) {
        if (shaderpacks[i].name == targetPack) {
            activePackIndex = (int)i;
            break;
        }
    }
}

void IrisShaderManager::saveConfig() {
    std::ofstream file("shaderpacks/iris.properties");
    if (!file.is_open()) return;

    file << "# Iris Minecraft Shaders Configuration\n";
    file << "shaderPack=" << getActivePackName() << "\n";
    file << "profile=" << currentProfile << "\n";
    file << "volumetricLight=" << (optVolumetricLight ? "true" : "false") << "\n";
    file << "waterReflections=" << (optWaterReflections ? "true" : "false") << "\n";
    file << "ssao=" << (optSSAO ? "true" : "false") << "\n";
    file << "bloom=" << (optBloom ? "true" : "false") << "\n";
    file << "wavingFoliage=" << (optWavingFoliage ? "true" : "false") << "\n";
    file << "motionBlur=" << (optMotionBlur ? "true" : "false") << "\n";
    file << "motionBlurStrength=" << optMotionBlurStrength << "\n";
    file << "toneMapping=" << optToneMapping << "\n";
}

void IrisShaderManager::openShaderpacksFolder() {
    ShellExecuteA(NULL, "open", "shaderpacks", NULL, NULL, SW_SHOWNORMAL);
}

GLuint IrisShaderManager::compileShader(GLenum type, const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "[Iris] Could not open shader file: " << filepath << std::endl;
        return 0;
    }

    std::stringstream ss;
    ss << file.rdbuf();
    std::string source = ss.str();
    const char* srcPtr = source.c_str();

    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &srcPtr, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[1024];
        glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
        std::cerr << "[Iris] Shader compile error in " << filepath << ":\n" << infoLog << std::endl;
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

GLuint IrisShaderManager::linkProgram(GLuint vert, GLuint frag) {
    if (!vert || !frag) return 0;

    GLuint program = glCreateProgram();
    glAttachShader(program, vert);
    glAttachShader(program, frag);
    glLinkProgram(program);

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[1024];
        glGetProgramInfoLog(program, 1024, nullptr, infoLog);
        std::cerr << "[Iris] Program link error:\n" << infoLog << std::endl;
        glDeleteProgram(program);
        return 0;
    }

    glDeleteShader(vert);
    glDeleteShader(frag);
    return program;
}

bool IrisShaderManager::loadShadersForPack(const std::string& packPath) {
    if (compositeProgram) {
        glDeleteProgram(compositeProgram);
        compositeProgram = 0;
    }
    if (finalProgram) {
        glDeleteProgram(finalProgram);
        finalProgram = 0;
    }

    std::string shadersDir = packPath + "/shaders/";
    std::string compVsh = shadersDir + "composite.vsh";
    std::string compFsh = shadersDir + "composite.fsh";
    std::string finVsh = shadersDir + "final.vsh";
    std::string finFsh = shadersDir + "final.fsh";

    // Compile composite pass
    GLuint cVert = compileShader(GL_VERTEX_SHADER, compVsh);
    GLuint cFrag = compileShader(GL_FRAGMENT_SHADER, compFsh);
    compositeProgram = linkProgram(cVert, cFrag);

    if (compositeProgram) {
        locCompColortex0 = glGetUniformLocation(compositeProgram, "colortex0");
        locCompColortex1 = glGetUniformLocation(compositeProgram, "colortex1");
        locCompDepthtex0 = glGetUniformLocation(compositeProgram, "depthtex0");
        locCompSunScreenPos = glGetUniformLocation(compositeProgram, "sunScreenPos");
        locCompSunVisibility = glGetUniformLocation(compositeProgram, "sunVisibility");
        locCompTime = glGetUniformLocation(compositeProgram, "uTime");
        locCompVolumetric = glGetUniformLocation(compositeProgram, "uVolumetricLight");
        locCompWaterReflect = glGetUniformLocation(compositeProgram, "uWaterReflections");
        locCompSSAO = glGetUniformLocation(compositeProgram, "uSSAO");
    } else {
        std::cerr << "[Iris] Failed to link composite program for " << packPath << std::endl;
        return false;
    }

    // Compile final pass
    GLuint fVert = compileShader(GL_VERTEX_SHADER, finVsh);
    GLuint fFrag = compileShader(GL_FRAGMENT_SHADER, finFsh);
    finalProgram = linkProgram(fVert, fFrag);
    hasPrevViewProj = false;

    if (finalProgram) {
        locFinalColortex0 = glGetUniformLocation(finalProgram, "colortex0");
        locFinalDepthtex0 = glGetUniformLocation(finalProgram, "depthtex0");
        locFinalInvViewProj = glGetUniformLocation(finalProgram, "uInvViewProj");
        locFinalPrevViewProj = glGetUniformLocation(finalProgram, "uPrevViewProj");
        locFinalMotionBlur = glGetUniformLocation(finalProgram, "uMotionBlur");
        locFinalMotionBlurStrength = glGetUniformLocation(finalProgram, "uMotionBlurStrength");
        locFinalBloom = glGetUniformLocation(finalProgram, "uBloom");
        locFinalToneMapping = glGetUniformLocation(finalProgram, "uToneMapping");
        locFinalTime = glGetUniformLocation(finalProgram, "uTime");
    } else {
        std::cerr << "[Iris] Failed to link final program for " << packPath << std::endl;
        return false;
    }

    std::cout << "[Iris] Successfully loaded and linked shaderpack: " << packPath << std::endl;
    return true;
}

void IrisShaderManager::beginScene() {
    if (!isShadersEnabled() || !gBufferFBO) {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, viewportWidth, viewportHeight);
        return;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, gBufferFBO);
    glViewport(0, 0, viewportWidth, viewportHeight);
}

void IrisShaderManager::endScene() {
    if (!isShadersEnabled()) return;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void IrisShaderManager::renderCompositeAndFinal(const Vec3& sunWorldDir, const Mat4& viewProj, float time) {
    if (!isShadersEnabled() || !compositeProgram || !finalProgram) return;

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    // Calculate Sun screen-space projection
    Vec4 sunClip = viewProj * Vec4(sunWorldDir.x * 1000.0f, sunWorldDir.y * 1000.0f, sunWorldDir.z * 1000.0f, 1.0f);
    Vec2 sunScreen(0.5f, 0.5f);
    float sunVis = 0.0f;

    if (sunClip.w > 0.001f) {
        Vec2 ndc(sunClip.x / sunClip.w, sunClip.y / sunClip.w);
        sunScreen.x = (ndc.x + 1.0f) * 0.5f;
        sunScreen.y = (ndc.y + 1.0f) * 0.5f;

        if (sunScreen.x >= -0.2f && sunScreen.x <= 1.2f && sunScreen.y >= -0.2f && sunScreen.y <= 1.2f) {
            sunVis = 1.0f;
        }
    }

    // =========================================================================
    // PASS 1: COMPOSITE (God Rays, SSAO, SSR) -> compositeFBO
    // =========================================================================
    glBindFramebuffer(GL_FRAMEBUFFER, compositeFBO);
    glViewport(0, 0, viewportWidth, viewportHeight);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(compositeProgram);

    // Bind textures: unit 0 = colortex0, unit 1 = colortex1, unit 2 = depthtex0
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, colortex0);
    if (locCompColortex0 != -1) glUniform1i(locCompColortex0, 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, colortex1);
    if (locCompColortex1 != -1) glUniform1i(locCompColortex1, 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, depthtex0);
    if (locCompDepthtex0 != -1) glUniform1i(locCompDepthtex0, 2);

    if (locCompSunScreenPos != -1) glUniform2f(locCompSunScreenPos, sunScreen.x, sunScreen.y);
    if (locCompSunVisibility != -1) glUniform1f(locCompSunVisibility, sunVis);
    if (locCompTime != -1) glUniform1f(locCompTime, time);
    if (locCompVolumetric != -1) glUniform1i(locCompVolumetric, optVolumetricLight ? 1 : 0);
    if (locCompWaterReflect != -1) glUniform1i(locCompWaterReflect, optWaterReflections ? 1 : 0);
    if (locCompSSAO != -1) glUniform1i(locCompSSAO, optSSAO ? 1 : 0);

    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // =========================================================================
    // PASS 2: FINAL (ACES Tone Mapping, Bloom, Motion Blur, Vignette) -> default screen FBO 0
    // =========================================================================
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, viewportWidth, viewportHeight);

    glUseProgram(finalProgram);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, compositeTex0);
    if (locFinalColortex0 != -1) glUniform1i(locFinalColortex0, 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, depthtex0);
    if (locFinalDepthtex0 != -1) glUniform1i(locFinalDepthtex0, 1);

    if (!hasPrevViewProj) {
        prevViewProj = viewProj;
        hasPrevViewProj = true;
    }

    Mat4 invVP = viewProj.inverse();
    if (locFinalInvViewProj != -1) glUniformMatrix4fv(locFinalInvViewProj, 1, GL_FALSE, invVP.m);
    if (locFinalPrevViewProj != -1) glUniformMatrix4fv(locFinalPrevViewProj, 1, GL_FALSE, prevViewProj.m);
    if (locFinalMotionBlur != -1) glUniform1i(locFinalMotionBlur, optMotionBlur ? 1 : 0);
    if (locFinalMotionBlurStrength != -1) glUniform1f(locFinalMotionBlurStrength, optMotionBlurStrength);

    if (locFinalBloom != -1) glUniform1i(locFinalBloom, optBloom ? 1 : 0);
    if (locFinalToneMapping != -1) glUniform1i(locFinalToneMapping, optToneMapping);
    if (locFinalTime != -1) glUniform1f(locFinalTime, time);

    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    glActiveTexture(GL_TEXTURE0);

    // Save previous frame's View-Projection matrix for next frame
    prevViewProj = viewProj;
}

} // namespace Aetheria
