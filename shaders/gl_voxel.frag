#version 330 core

in vec4 fragColor;
in float fragDist;
in vec2 fragUV;
in float fragTexIndex;
in vec3 fragWorldPos;
in vec3 fragNormal;
in float fragLight;

uniform sampler2DArray uTextureArray;
uniform vec4 uFogColor;
uniform float uFogStart;
uniform float uFogEnd;
uniform int uEnableFog;
uniform vec3 uCamPos;
uniform vec3 uSunDir;
uniform float uTime;

layout(location = 0) out vec4 finalColor;
layout(location = 1) out vec4 finalNormal;

// Analytic sub-texel anti-aliasing for 16x16 pixel-art textures:
// Preserves crisp pixel art up-close while smoothing texel boundaries over 1 screen pixel!
vec2 smoothTexelCoord(vec2 uv, vec2 res) {
    vec2 pixel = uv * res;
    vec2 seam = floor(pixel + 0.5);
    vec2 duv = max(fwidth(pixel), vec2(0.0001));
    return (seam + clamp((pixel - seam) / duv, -0.5, 0.5)) / res;
}

void main() {
    // 1. Sample Texture from Texture Array with anti-aliased sub-texel filtering & anisotropic mipmapping
    vec4 texCol = vec4(1.0);
    if (fragTexIndex > 0.0) {
        vec2 clampedUV = clamp(fragUV, 0.0005, 0.9995);
        vec2 dUVdx = dFdx(clampedUV);
        vec2 dUVdy = dFdy(clampedUV);
        vec2 filteredUV = clamp(smoothTexelCoord(clampedUV, vec2(16.0)), 0.0005, 0.9995);
        texCol = textureGrad(uTextureArray, vec3(filteredUV, fragTexIndex), dUVdx, dUVdy);
    }

    // Cutout alpha test (foliage cutouts, sapling transparent backgrounds)
    if (texCol.a < 0.1) {
        discard;
    }

    // Two-sided normal calculation so back-facing cutout foliage is illuminated
    vec3 N = normalize(fragNormal);
    if (!gl_FrontFacing) {
        N = -N;
    }

    // Camera view vector
    vec3 V = normalize(uCamPos - fragWorldPos);

    // Dynamic Atmospheric Lighting Vectors & Solar Color
    vec3 sunDir = normalize(uSunDir);
    float sunElev = clamp(sunDir.y * 1.5 + 0.1, 0.0, 1.0);
    float sunsetFactor = clamp(1.0 - abs(sunDir.y) * 3.5, 0.0, 1.0);
    float nightFactor = clamp(-sunDir.y * 2.0, 0.0, 1.0);

    // Warm golden amber at sunrise/sunset vs radiant noon
    vec3 noonSunColor = vec3(1.0, 0.97, 0.91);
    vec3 sunsetSunColor = vec3(1.0, 0.58, 0.24);
    vec3 sunColor = mix(noonSunColor, sunsetSunColor, sunsetFactor);

    // Sky ambient color
    vec3 dayAmbient = vec3(0.58, 0.70, 0.88);
    vec3 sunsetAmbient = vec3(0.52, 0.35, 0.48);
    vec3 nightAmbient = vec3(0.08, 0.12, 0.22);
    vec3 ambientColor = mix(mix(dayAmbient, sunsetAmbient, sunsetFactor), nightAmbient, nightFactor);

    // Modulate base surface lighting by dynamic celestial colors
    vec3 atmosphericLight = mix(ambientColor, sunColor, clamp(fragLight, 0.0, 1.0));

    // Check if surface is a Creature / Mob (Layers 390 to 399)
    bool isMob = (fragTexIndex >= 389.5 && fragTexIndex <= 405.0);

    vec3 surfaceColor;
    if (isMob) {
        // =====================================================================
        // SMOOTH & EDGY MOB SHADING (No blocky pixel bevels!)
        // =====================================================================
        // Dynamic Fresnel Rim Light (accentuates sharp contours and silhouette)
        float NdotV = max(dot(N, V), 0.0);
        float rimFactor = pow(1.0 - NdotV, 2.6);
        vec3 rimColor = mix(texCol.rgb, vec3(0.2, 0.85, 1.0), 0.35) * rimFactor * 0.75;

        // Blinn-Phong Specular Highlight (for sleek chitin, armor, and fangs)
        vec3 H = normalize(sunDir + V);
        float NdotH = max(dot(N, H), 0.0);
        float spec = pow(NdotH, 28.0) * 0.45;

        // Emissive check (eyes, runic cores, damage flashes)
        bool isEmissive = (fragTexIndex >= 394.5 && fragTexIndex <= 396.5) ||
                          (fragColor.r > 1.3 || fragColor.g > 1.3 || fragColor.b > 1.3);

        vec3 litMob = texCol.rgb * fragColor.rgb * (isEmissive ? 1.4 : fragLight) * (isEmissive ? vec3(1.0) : atmosphericLight);
        surfaceColor = litMob + rimColor + vec3(spec);
    } else {
        // 2. Authentic block shading (with self-illuminating Molten Lava)
        bool isLava = (abs(fragTexIndex - 32.0) < 0.2);
        if (isLava) {
            surfaceColor = texCol.rgb * 1.35;
        } else {
            surfaceColor = texCol.rgb * fragColor.rgb * atmosphericLight;

            // PBR Specular & Fresnel on Reflective Blocks
            bool isWater = (abs(fragTexIndex - 17.0) < 0.2);
            bool isIce = (fragTexIndex >= 17.5 && fragTexIndex <= 20.5);
            bool isGlass = (abs(fragTexIndex - 342.0) < 0.2 || abs(fragTexIndex - 343.0) < 0.2);
            bool isCrystal = (fragTexIndex >= 161.0 && fragTexIndex <= 205.5);

            if (isWater) {
                // Animated micro-wave surface normal perturbation
                vec3 waveN = N;
                float w1 = sin(fragWorldPos.x * 4.0 + uTime * 3.0 + fragWorldPos.z * 2.0) * 0.08;
                float w2 = cos(fragWorldPos.z * 4.0 + uTime * 2.5 - fragWorldPos.x * 2.0) * 0.08;
                waveN.x += w1;
                waveN.z += w2;
                waveN = normalize(waveN);

                vec3 waveH = normalize(sunDir + V);
                float waveNdotH = max(dot(waveN, waveH), 0.0);
                float waterSpec = pow(waveNdotH, 64.0) * 0.95 * sunElev;
                float fresnel = pow(1.0 - max(dot(waveN, V), 0.0), 3.5);

                surfaceColor = mix(surfaceColor, ambientColor * 1.4, fresnel * 0.45) + sunColor * waterSpec;
            } else if (isIce || isGlass) {
                vec3 H = normalize(sunDir + V);
                float NdotH = max(dot(N, H), 0.0);
                float spec = pow(NdotH, 48.0) * 0.65 * sunElev;
                float fresnel = pow(1.0 - max(dot(N, V), 0.0), 4.0);
                surfaceColor = mix(surfaceColor, ambientColor * 1.25, fresnel * 0.35) + sunColor * spec;
            } else if (isCrystal) {
                vec3 H = normalize(sunDir + V);
                float NdotH = max(dot(N, H), 0.0);
                float spec = pow(NdotH, 32.0) * 0.85 * sunElev;
                surfaceColor += sunColor * spec;
            }

            // Subsurface Foliage Scattering (Backlit Leaves Translucency)
            bool isLeaves = (fragTexIndex >= 72.5 && fragTexIndex <= 119.5 && ((int(fragTexIndex + 0.1) % 3) == 1)) ||
                            (fragTexIndex >= 365.5 && fragTexIndex <= 369.5);
            if (isLeaves) {
                float sunBacklight = max(dot(-V, sunDir), 0.0);
                float sss = pow(sunBacklight, 3.0) * 0.45 * sunElev;
                surfaceColor += vec3(0.40, 0.88, 0.22) * sss * texCol.rgb;
            }
        }
    }

    // 4. Atmospheric Distance Fog with Mie Solar Forward-Scattering Halo
    if (uEnableFog != 0) {
        float fogFactor = clamp((fragDist - uFogStart) / (uFogEnd - uFogStart), 0.0, 1.0);
        float sunGaze = max(dot(-V, sunDir), 0.0);
        vec3 atmosphericFog = mix(uFogColor.rgb, sunColor * 1.30, pow(sunGaze, 6.0) * 0.48 * (1.0 - nightFactor));
        finalColor = vec4(mix(surfaceColor, atmosphericFog, fogFactor), texCol.a);
    } else {
        finalColor = vec4(surfaceColor, texCol.a);
    }

    // Material and reflection flag in finalNormal.a (Ice / Water blocks 17-19)
    float matFlag = (fragTexIndex >= 16.5 && fragTexIndex <= 19.5) ? 1.0 : 0.0;
    finalNormal = vec4(N * 0.5 + 0.5, matFlag);
}
