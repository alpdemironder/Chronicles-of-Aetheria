#version 330 core

in vec2 texCoord;

uniform sampler2D colortex0; // Scene HDR Color
uniform sampler2D colortex1; // Surface Normals (RGB = N * 0.5 + 0.5, A = material flag: 1.0 = water/ice)
uniform sampler2D depthtex0; // Depth Buffer

uniform vec2 sunScreenPos;   // Sun in screen space [0..1]
uniform float sunVisibility; // 1.0 if sun is in front, 0.0 if behind camera
uniform float uTime;
uniform int uVolumetricLight;
uniform int uWaterReflections;
uniform int uSSAO;

out vec4 fragColor;

// High quality interleaved gradient noise for silky-smooth jitter
float interleavedGradientNoise(vec2 coord) {
    return fract(52.9829189 * fract(dot(coord, vec2(0.06711056, 0.00583715))));
}

void main() {
    vec4 baseColor = texture(colortex0, texCoord);
    float depth = texture(depthtex0, texCoord).r;

    // If background sky (depth == 1.0), skip occlusion
    bool isSky = (depth >= 0.9999);

    vec3 finalRGB = baseColor.rgb;

    // =========================================================================
    // 1. ADVANCED SCREEN-SPACE AMBIENT OCCLUSION (SSAO)
    // =========================================================================
    if (uSSAO != 0 && !isSky) {
        float occlusion = 0.0;
        vec2 texelSize = 1.0 / vec2(textureSize(depthtex0, 0));
        float dither = interleavedGradientNoise(gl_FragCoord.xy);
        
        // 12-Sample Spiral Kernel with randomized rotation
        for (int i = 1; i <= 12; ++i) {
            float angle = float(i) * 2.39996 + dither * 6.28318; // Golden angle + rotation
            float radius = (float(i) / 12.0) * 3.5 + 0.5;
            vec2 offset = vec2(cos(angle), sin(angle)) * texelSize * radius;
            float sampleDepth = texture(depthtex0, texCoord + offset).r;
            
            // Bilateral depth falloff (sharp contact shadows in corners, zero halos on edges)
            float diff = depth - sampleDepth;
            if (diff > 0.00015 && diff < 0.035) {
                float weight = 1.0 - (diff / 0.035);
                occlusion += weight;
            }
        }
        float aoFactor = clamp(1.0 - (occlusion / 12.0) * 0.65, 0.38, 1.0);
        finalRGB *= aoFactor;
    }

    // =========================================================================
    // 2. VOLUMETRIC SUN GOD RAYS (24-Step Jittered Light Shafts)
    // =========================================================================
    if (uVolumetricLight != 0 && sunVisibility > 0.05) {
        vec2 deltaUV = (sunScreenPos - texCoord);
        float distToSun = length(deltaUV);
        deltaUV = normalize(deltaUV) * min(distToSun, 0.60) / 24.0;

        float dither = interleavedGradientNoise(gl_FragCoord.xy);
        vec2 curUV = texCoord + deltaUV * dither; // Jitter start to banish banding!
        float rayAccum = 0.0;
        float decay = 1.0;

        for (int step = 0; step < 24; ++step) {
            curUV += deltaUV;
            if (curUV.x < 0.0 || curUV.x > 1.0 || curUV.y < 0.0 || curUV.y > 1.0) break;
            
            float sDepth = texture(depthtex0, curUV).r;
            // Unoccluded sky transmits rich god rays
            if (sDepth >= 0.9997) {
                rayAccum += decay * 0.058;
            }
            decay *= 0.935;
        }

        // Warm golden sunlight tint
        vec3 sunRayColor = vec3(1.15, 0.88, 0.55) * rayAccum * sunVisibility * 0.55;
        finalRGB += sunRayColor;
    }

    // =========================================================================
    // 3. WATER SURFACE SCREEN-SPACE REFLECTIONS (SSR) & CAUSTICS
    // =========================================================================
    if (uWaterReflections != 0 && !isSky) {
        vec4 normalSample = texture(colortex1, texCoord);
        if (normalSample.a > 0.7) { // Water / Ice material flag
            // Dynamic wave normal
            float wave = sin(texCoord.x * 55.0 + uTime * 2.8) * cos(texCoord.y * 55.0 + uTime * 2.2);
            vec2 reflectOffset = vec2(wave * 0.015, -wave * 0.015);
            vec2 reflectUV = clamp(texCoord + reflectOffset + vec2(0.0, 0.04), 0.001, 0.999);

            vec3 reflectedColor = texture(colortex0, reflectUV).rgb;
            float reflMask = clamp((1.0 - depth) * 2.0, 0.15, 0.45);
            finalRGB = mix(finalRGB, reflectedColor, reflMask) + vec3(0.08, 0.18, 0.30) * (0.5 + 0.5 * wave);
        }
    }

    fragColor = vec4(finalRGB, baseColor.a);
}
