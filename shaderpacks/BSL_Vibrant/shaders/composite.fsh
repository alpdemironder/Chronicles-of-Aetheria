#version 330 core

in vec2 texCoord;

uniform sampler2D colortex0; // Scene HDR Color
uniform sampler2D colortex1; // Surface Normals
uniform sampler2D depthtex0; // Depth Buffer

uniform vec2 sunScreenPos;   // Sun in screen space [0..1]
uniform float sunVisibility; // 1.0 if sun is visible, 0.0 if occluded
uniform float uTime;
uniform int uVolumetricLight;
uniform int uWaterReflections;
uniform int uSSAO;

out vec4 fragColor;

void main() {
    vec4 baseColor = texture(colortex0, texCoord);
    float depth = texture(depthtex0, texCoord).r;

    bool isSky = (depth >= 0.9999);
    vec3 finalRGB = baseColor.rgb;

    // =========================================================================
    // 1. BSL SCREEN-SPACE AMBIENT OCCLUSION (SSAO)
    // =========================================================================
    if (uSSAO != 0 && !isSky) {
        float occlusion = 0.0;
        vec2 texelSize = 1.0 / vec2(textureSize(depthtex0, 0));
        
        // 12-sample circular distribution
        for (int i = 1; i <= 12; ++i) {
            float angle = float(i) * 2.399963; // Golden angle
            float radius = float(i) * 1.5;
            vec2 offset = vec2(cos(angle), sin(angle)) * texelSize * radius;
            float sampleDepth = texture(depthtex0, texCoord + offset).r;
            
            float diff = depth - sampleDepth;
            if (diff > 0.00015 && diff < 0.035) {
                occlusion += (1.0 - diff / 0.035);
            }
        }
        float aoFactor = clamp(1.0 - (occlusion / 12.0) * 0.65, 0.35, 1.0);
        // Cool blue-tinted shadows characteristic of BSL
        finalRGB *= mix(vec3(0.85, 0.90, 1.05) * aoFactor, vec3(1.0), aoFactor);
    }

    // =========================================================================
    // 2. BSL VIBRANT VOLUMETRIC GOD RAYS (Warm Golden Sun Beams)
    // =========================================================================
    if (uVolumetricLight != 0 && sunVisibility > 0.05) {
        vec2 deltaUV = (sunScreenPos - texCoord);
        float distToSun = length(deltaUV);
        deltaUV = normalize(deltaUV) * min(distToSun, 0.65) / 20.0;

        vec2 curUV = texCoord;
        float rayAccum = 0.0;
        float decay = 1.0;

        for (int step = 0; step < 20; ++step) {
            curUV += deltaUV;
            if (curUV.x < 0.0 || curUV.x > 1.0 || curUV.y < 0.0 || curUV.y > 1.0) break;
            
            float sDepth = texture(depthtex0, curUV).r;
            if (sDepth >= 0.9998) {
                rayAccum += decay * 0.07;
            }
            decay *= 0.935;
        }

        // BSL vibrant amber-gold light shaft coloration
        vec3 bslSunColor = vec3(1.25, 0.95, 0.55) * rayAccum * sunVisibility * 0.52;
        finalRGB += bslSunColor;
    }

    // =========================================================================
    // 3. BSL WATER REFLECTIONS & FRESNEL CAUSTICS
    // =========================================================================
    if (uWaterReflections != 0 && !isSky) {
        vec4 normalSample = texture(colortex1, texCoord);
        if (normalSample.a > 0.7) { // Water flagged
            float wave1 = sin(texCoord.x * 60.0 + uTime * 3.0) * cos(texCoord.y * 60.0 + uTime * 2.2);
            float wave2 = cos(texCoord.x * 35.0 - uTime * 1.8) * sin(texCoord.y * 35.0 + uTime * 1.5);
            float waves = (wave1 + wave2) * 0.5;

            // BSL turquoise-cyan clear water tint
            vec3 waterTint = vec3(0.08, 0.30, 0.45);
            finalRGB = mix(finalRGB, waterTint, 0.35) + vec3(0.20, 0.35, 0.40) * (0.5 + 0.5 * waves);
        }
    }

    fragColor = vec4(finalRGB, baseColor.a);
}
