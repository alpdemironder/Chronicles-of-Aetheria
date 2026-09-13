#version 330 core

in vec2 texCoord;

uniform sampler2D colortex0; // Scene HDR Color
uniform sampler2D colortex1; // Surface Normals
uniform sampler2D depthtex0; // Depth Buffer

uniform vec2 sunScreenPos;   // Sun in screen space [0..1]
uniform float sunVisibility; // 1.0 if sun is in front, 0.0 if behind camera
uniform float uTime;
uniform int uVolumetricLight;
uniform int uWaterReflections;
uniform int uSSAO;

out vec4 fragColor;

void main() {
    vec4 baseColor = texture(colortex0, texCoord);
    float depth = texture(depthtex0, texCoord).r;

    // If background sky (depth == 1.0), skip occlusion
    bool isSky = (depth >= 0.9999);

    vec3 finalRGB = baseColor.rgb;

    // =========================================================================
    // 1. SCREEN-SPACE AMBIENT OCCLUSION (SSAO)
    // =========================================================================
    if (uSSAO != 0 && !isSky) {
        float occlusion = 0.0;
        vec2 texelSize = 1.0 / vec2(textureSize(depthtex0, 0));
        
        // 8-Sample Spiral Kernel
        for (int i = 1; i <= 8; ++i) {
            float angle = float(i) * 2.39996; // Golden angle
            float radius = float(i) * 1.8;
            vec2 offset = vec2(cos(angle), sin(angle)) * texelSize * radius;
            float sampleDepth = texture(depthtex0, texCoord + offset).r;
            
            // If sample is closer to camera than current fragment, it occludes it
            float diff = depth - sampleDepth;
            if (diff > 0.0002 && diff < 0.04) {
                occlusion += (1.0 - diff / 0.04);
            }
        }
        float aoFactor = clamp(1.0 - (occlusion / 8.0) * 0.55, 0.45, 1.0);
        finalRGB *= aoFactor;
    }

    // =========================================================================
    // 2. VOLUMETRIC SUN GOD RAYS (Light Shafts)
    // =========================================================================
    if (uVolumetricLight != 0 && sunVisibility > 0.05) {
        vec2 deltaUV = (sunScreenPos - texCoord);
        float distToSun = length(deltaUV);
        deltaUV = normalize(deltaUV) * min(distToSun, 0.55) / 16.0;

        vec2 curUV = texCoord;
        float rayAccum = 0.0;
        float decay = 1.0;

        for (int step = 0; step < 16; ++step) {
            curUV += deltaUV;
            if (curUV.x < 0.0 || curUV.x > 1.0 || curUV.y < 0.0 || curUV.y > 1.0) break;
            
            float sDepth = texture(depthtex0, curUV).r;
            // Only unoccluded sky transmits god ray light
            if (sDepth >= 0.9998) {
                rayAccum += decay * 0.08;
            }
            decay *= 0.92;
        }

        // Warm golden sunlight tint
        vec3 sunRayColor = vec3(1.10, 0.90, 0.60) * rayAccum * sunVisibility * 0.45;
        finalRGB += sunRayColor;
    }

    // =========================================================================
    // 3. WATER SURFACE WAVE DISTORTION & CAUSTICS
    // =========================================================================
    if (uWaterReflections != 0 && !isSky) {
        // Subtle water caustics / specular catch
        vec4 normalSample = texture(colortex1, texCoord);
        if (normalSample.a > 0.7) { // Water material flag
            float wave = sin(texCoord.x * 45.0 + uTime * 2.5) * cos(texCoord.y * 45.0 + uTime * 2.0);
            finalRGB += vec3(0.12, 0.22, 0.35) * (0.5 + 0.5 * wave);
        }
    }

    fragColor = vec4(finalRGB, baseColor.a);
}
