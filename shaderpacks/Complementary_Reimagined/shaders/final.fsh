#version 330 core

in vec2 texCoord;

uniform sampler2D colortex0; // Output from composite pass
uniform sampler2D depthtex0; // Scene depth buffer for motion reconstruction

uniform mat4 uInvViewProj;   // Inverse View-Projection of current frame
uniform mat4 uPrevViewProj;  // View-Projection of previous frame
uniform int uMotionBlur;      // Motion blur toggle (0 = disabled, 1 = enabled)
uniform float uMotionBlurStrength; // Motion blur intensity multiplier

uniform int uBloom;
uniform int uToneMapping;
uniform float uTime;

out vec4 fragColor;

// ACES Filmic Tone Mapping curve (used by Unreal Engine & standard Minecraft shaderpacks)
vec3 ACESFilm(vec3 x) {
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

void main() {
    // Subtle optical lens chromatic aberration towards screen periphery
    vec2 caOffset = (texCoord - 0.5) * 0.0014;
    float rChannel = texture(colortex0, clamp(texCoord + caOffset, 0.001, 0.999)).r;
    float gChannel = texture(colortex0, texCoord).g;
    float bChannel = texture(colortex0, clamp(texCoord - caOffset, 0.001, 0.999)).b;
    vec3 color = vec3(rChannel, gChannel, bChannel);

    // =========================================================================
    // 0. CINEMATIC CAMERA & VELOCITY MOTION BLUR
    // =========================================================================
    if (uMotionBlur != 0 && uMotionBlurStrength > 0.001) {
        float depthVal = texture(depthtex0, texCoord).r;
        // Clamp depth slightly below 1.0 to allow distant terrain and sky to blur smoothly with camera rotation
        float d = min(depthVal, 0.9996);
        vec4 clipPos = vec4(texCoord * 2.0 - 1.0, d * 2.0 - 1.0, 1.0);
        vec4 worldPos = uInvViewProj * clipPos;
        worldPos /= worldPos.w;

        vec4 prevClip = uPrevViewProj * worldPos;
        prevClip /= prevClip.w;

        // Screen-space velocity vector from previous position to current
        vec2 velocity = (clipPos.xy - prevClip.xy) * 0.5 * uMotionBlurStrength;

        // Avoid extreme streak artifacts during fast turns, respawns, or teleportation
        float maxVelocity = 0.055;
        float speed = length(velocity);
        if (speed > maxVelocity) {
            velocity = (velocity / speed) * maxVelocity;
        }

        // Only apply blur if camera/world actually moved
        if (speed > 0.0006) {
            const int SAMPLES = 8;
            vec3 blurAccum = vec3(0.0);
            float totalWeight = 0.0;

            for (int i = 0; i < SAMPLES; ++i) {
                float t = (float(i) / float(SAMPLES - 1)) - 0.5; // [-0.5, 0.5] centered
                vec2 sampleCoord = clamp(texCoord + velocity * t, vec2(0.001), vec2(0.999));
                float w = 1.0 - abs(t) * 0.55;
                blurAccum += texture(colortex0, sampleCoord).rgb * w;
                totalWeight += w;
            }
            color = blurAccum / totalWeight;
        }
    }

    // =========================================================================
    // 1. DYNAMIC BLOOM & GLOW ACCUMULATION
    // =========================================================================
    if (uBloom != 0) {
        vec3 bloomAccum = vec3(0.0);
        vec2 texelSize = 1.0 / vec2(textureSize(colortex0, 0));
        
        // 9-Tap Cross Blur Kernel
        float offsets[4] = float[](1.5, 3.0, 5.0, 7.5);
        float weights[4] = float[](0.28, 0.20, 0.14, 0.08);

        for (int i = 0; i < 4; ++i) {
            vec2 offX = vec2(offsets[i] * texelSize.x, 0.0);
            vec2 offY = vec2(0.0, offsets[i] * texelSize.y);

            vec3 s0 = texture(colortex0, texCoord + offX).rgb;
            vec3 s1 = texture(colortex0, texCoord - offX).rgb;
            vec3 s2 = texture(colortex0, texCoord + offY).rgb;
            vec3 s3 = texture(colortex0, texCoord - offY).rgb;

            // Threshold bright pixels
            bloomAccum += max(s0 - vec3(0.72), vec3(0.0)) * weights[i];
            bloomAccum += max(s1 - vec3(0.72), vec3(0.0)) * weights[i];
            bloomAccum += max(s2 - vec3(0.72), vec3(0.0)) * weights[i];
            bloomAccum += max(s3 - vec3(0.72), vec3(0.0)) * weights[i];
        }

        color += bloomAccum * 0.45;
    }

    // =========================================================================
    // 2. HDR TONE MAPPING & COLOR GRADING
    // =========================================================================
    if (uToneMapping == 0) {
        // ACES Filmic HDR
        color = ACESFilm(color * 1.15);
    } else if (uToneMapping == 1) {
        // Reinhard
        color = color / (color + vec3(1.0));
    } else if (uToneMapping == 2) {
        // Vibrant Fantasy Saturation
        float luma = dot(color, vec3(0.299, 0.587, 0.114));
        color = mix(vec3(luma), color, 1.30);
        color = pow(clamp(color, 0.0, 1.0), vec3(1.0 / 2.1));
    }
    // uToneMapping == 3 is Vanilla passthrough

    // =========================================================================
    // 3. CINEMATIC VIGNETTE
    // =========================================================================
    vec2 vUV = (texCoord - 0.5) * 2.0;
    float vDist = dot(vUV, vUV);
    float vignette = clamp(1.0 - vDist * 0.18, 0.0, 1.0);
    color *= vignette;

    fragColor = vec4(color, 1.0);
}
