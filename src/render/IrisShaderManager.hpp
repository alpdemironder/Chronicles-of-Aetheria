#pragma once

#include "GLHeaders.hpp"
#include "../core/Math.hpp"
#include <string>
#include <vector>

namespace Aetheria {

struct ShaderpackInfo {
    std::string name;
    std::string displayName;
    std::string path;
    bool isValid = false;
};

class IrisShaderManager {
public:
    IrisShaderManager();
    ~IrisShaderManager();

    bool init(int width, int height);
    void resize(int width, int height);
    void cleanup();

    // Shaderpack discovery & management
    void scanShaderpacks();
    const std::vector<ShaderpackInfo>& getShaderpacks() const { return shaderpacks; }
    int getActivePackIndex() const { return activePackIndex; }
    std::string getActivePackName() const;
    bool setActivePack(int index);
    bool setActivePack(const std::string& packName);
    bool reloadCurrentPack();

    // Configuration options
    bool isShadersEnabled() const { return activePackIndex > 0; }
    bool getVolumetricLight() const { return optVolumetricLight; }
    void setVolumetricLight(bool v) { optVolumetricLight = v; }

    bool getWaterReflections() const { return optWaterReflections; }
    void setWaterReflections(bool v) { optWaterReflections = v; }

    bool getSSAO() const { return optSSAO; }
    void setSSAO(bool v) { optSSAO = v; }

    bool getBloom() const { return optBloom; }
    void setBloom(bool v) { optBloom = v; }

    bool getWavingFoliage() const { return optWavingFoliage; }
    void setWavingFoliage(bool v) { optWavingFoliage = v; }

    bool getMotionBlur() const { return optMotionBlur; }
    void setMotionBlur(bool v) { optMotionBlur = v; }

    float getMotionBlurStrength() const { return optMotionBlurStrength; }
    void setMotionBlurStrength(float s) { optMotionBlurStrength = std::clamp(s, 0.0f, 2.0f); }

    int getToneMapping() const { return optToneMapping; }
    void setToneMapping(int tm) { optToneMapping = tm; }

    const std::string& getProfile() const { return currentProfile; }
    void applyProfile(const std::string& profile);

    void loadConfig();
    void saveConfig();
    void openShaderpacksFolder();

    // Pipeline rendering passes
    void beginScene();
    void endScene();
    void renderCompositeAndFinal(const Vec3& sunWorldDir, const Mat4& viewProj, float time);

    GLuint getColortex0() const { return colortex0; }
    GLuint getColortex1() const { return colortex1; }
    GLuint getDepthtex0() const { return depthtex0; }

private:
    bool initFBOs(int width, int height);
    void destroyFBOs();
    void initQuad();
    bool loadShadersForPack(const std::string& packPath);
    GLuint compileShader(GLenum type, const std::string& filepath);
    GLuint linkProgram(GLuint vert, GLuint frag);

    int viewportWidth = 1280;
    int viewportHeight = 720;

    // Shaderpack collection
    std::vector<ShaderpackInfo> shaderpacks;
    int activePackIndex = 0; // 0 = OFF (Vanilla)

    // Option state
    bool optVolumetricLight = true;
    bool optWaterReflections = true;
    bool optSSAO = true;
    bool optBloom = true;
    bool optWavingFoliage = true;
    bool optMotionBlur = true;
    float optMotionBlurStrength = 0.60f;
    int optToneMapping = 0; // 0: ACES Filmic, 1: Reinhard, 2: Vibrant Fantasy, 3: Vanilla
    std::string currentProfile = "High";

    // G-Buffer FBO & Textures
    GLuint gBufferFBO = 0;
    GLuint colortex0 = 0; // HDR Color
    GLuint colortex1 = 0; // Normals & material flags
    GLuint depthtex0 = 0; // Depth/stencil

    // Composite FBO & Texture
    GLuint compositeFBO = 0;
    GLuint compositeTex0 = 0;

    // Fullscreen quad VAO/VBO
    GLuint quadVAO = 0;
    GLuint quadVBO = 0;

    // Temporal motion tracking
    Mat4 prevViewProj;
    bool hasPrevViewProj = false;

    // Shader programs & uniform locations
    GLuint compositeProgram = 0;
    GLint locCompColortex0 = -1;
    GLint locCompColortex1 = -1;
    GLint locCompDepthtex0 = -1;
    GLint locCompSunScreenPos = -1;
    GLint locCompSunVisibility = -1;
    GLint locCompTime = -1;
    GLint locCompVolumetric = -1;
    GLint locCompWaterReflect = -1;
    GLint locCompSSAO = -1;

    GLuint finalProgram = 0;
    GLint locFinalColortex0 = -1;
    GLint locFinalDepthtex0 = -1;
    GLint locFinalInvViewProj = -1;
    GLint locFinalPrevViewProj = -1;
    GLint locFinalMotionBlur = -1;
    GLint locFinalMotionBlurStrength = -1;
    GLint locFinalBloom = -1;
    GLint locFinalToneMapping = -1;
    GLint locFinalTime = -1;
};

} // namespace Aetheria
