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

layout(location = 0) out vec4 finalColor;
layout(location = 1) out vec4 finalNormal;

void main() {
    // 1. Sample Texture from Texture Array with edge clamp
    vec4 texCol = vec4(1.0);
    if (fragTexIndex > 0.0) {
        vec2 clampedUV = clamp(fragUV, 0.0005, 0.9995);
        texCol = texture(uTextureArray, vec3(clampedUV, fragTexIndex));
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

    // Check if surface is a Creature / Mob (Layers 390 to 399)
    bool isMob = (fragTexIndex >= 389.5 && fragTexIndex <= 405.0);

    vec3 surfaceColor;
    if (isMob) {
        // =====================================================================
        // SMOOTH & EDGY MOB SHADING (No blocky pixel bevels!)
        // =====================================================================
        vec3 V = normalize(uCamPos - fragWorldPos);

        // Dynamic Fresnel Rim Light (accentuates sharp contours and silhouette)
        float NdotV = max(dot(N, V), 0.0);
        float rimFactor = pow(1.0 - NdotV, 2.6);
        vec3 rimColor = mix(texCol.rgb, vec3(0.2, 0.85, 1.0), 0.35) * rimFactor * 0.75;

        // Blinn-Phong Specular Highlight (for sleek chitin, armor, and fangs)
        vec3 L = normalize(vec3(0.5, 0.9, 0.3));
        vec3 H = normalize(L + V);
        float NdotH = max(dot(N, H), 0.0);
        float spec = pow(NdotH, 28.0) * 0.45;

        // Emissive check (eyes, runic cores, damage flashes)
        bool isEmissive = (fragTexIndex >= 394.5 && fragTexIndex <= 396.5) ||
                          (fragColor.r > 1.3 || fragColor.g > 1.3 || fragColor.b > 1.3);

        vec3 litMob = texCol.rgb * fragColor.rgb * (isEmissive ? 1.4 : fragLight);
        surfaceColor = litMob + rimColor + vec3(spec);
    } else {
        // 2. Authentic block shading (with self-illuminating Molten Lava)
        bool isLava = (abs(fragTexIndex - 32.0) < 0.2);
        if (isLava) {
            surfaceColor = texCol.rgb * 1.35;
        } else {
            surfaceColor = texCol.rgb * fragColor.rgb * fragLight;
        }
    }

    // 4. Atmospheric Distance Fog
    if (uEnableFog != 0) {
        float fogFactor = clamp((fragDist - uFogStart) / (uFogEnd - uFogStart), 0.0, 1.0);
        finalColor = vec4(mix(surfaceColor, uFogColor.rgb, fogFactor), texCol.a);
    } else {
        finalColor = vec4(surfaceColor, texCol.a);
    }

    // Material and reflection flag in finalNormal.a (Ice / Water blocks 17-19)
    float matFlag = (fragTexIndex >= 16.5 && fragTexIndex <= 19.5) ? 1.0 : 0.0;
    finalNormal = vec4(N * 0.5 + 0.5, matFlag);
}
