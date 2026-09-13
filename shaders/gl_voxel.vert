#version 330 core

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec4 inColor;
layout(location = 4) in float inTexIndex;

uniform mat4 uMVP;
uniform mat4 uModel;
uniform vec3 uSunDir;
uniform vec3 uCamPos;
uniform int uEnableAO;
uniform float uTime;
uniform int uWavingFoliage;

out vec4 fragColor;
out float fragDist;
out vec2 fragUV;
out float fragTexIndex;
out vec3 fragWorldPos;
out vec3 fragNormal;
out float fragLight;

void main() {
    vec3 displacedPos = inPos;
    if (uWavingFoliage != 0) {
        // Leaves blockIDs: 73, 76, 79... 118 and Saplings 366..369
        bool isLeaves = (inTexIndex >= 72.5 && inTexIndex <= 119.5 && ((int(inTexIndex + 0.1) % 3) == 1)) ||
                        (inTexIndex >= 365.5 && inTexIndex <= 369.5);
        if (isLeaves) {
            vec3 blockCenter = floor(inPos);
            float wave = sin(uTime * 2.2 + blockCenter.x * 0.45 + blockCenter.z * 0.45) * 0.04
                       + cos(uTime * 1.6 + blockCenter.y * 0.6) * 0.02;
            displacedPos.x += wave;
            displacedPos.z += wave * 0.7;
            displacedPos.y += sin(uTime * 2.8 + blockCenter.x + blockCenter.z) * 0.015;
        }
    }

    // Fluid Surface Dynamics (Water 17, Molten Lava 32)
    bool isWater = (abs(inTexIndex - 17.0) < 0.2);
    bool isLava = (abs(inTexIndex - 32.0) < 0.2);
    if (isWater) {
        if (inNormal.y > 0.5) {
            float ripple = sin(uTime * 2.8 + inPos.x * 1.5 + inPos.z * 1.5) * 0.035
                         + cos(uTime * 2.1 + inPos.x * 1.2 - inPos.z * 1.0) * 0.025;
            displacedPos.y += ripple - 0.05; // natural liquid level depression
        }
    } else if (isLava) {
        if (inNormal.y > 0.5) {
            float slosh = sin(uTime * 1.3 + inPos.x * 0.7 + inPos.z * 0.7) * 0.025;
            displacedPos.y += slosh - 0.04;
        }
    }

    vec4 worldPos = uModel * vec4(displacedPos, 1.0);
    gl_Position = uMVP * vec4(displacedPos, 1.0);

    vec3 norm = normalize((uModel * vec4(inNormal, 0.0)).xyz);
    vec3 sunDir = normalize(uSunDir);
    vec3 moonDir = -sunDir;

    // Sun directional contribution
    float sunDiff = max(dot(norm, sunDir), 0.0);
    float sunElev = clamp(sunDir.y * 1.5 + 0.1, 0.0, 1.0);
    float sunContrib = sunDiff * sunElev;

    // Moon directional contribution (soft cool nocturnal glow)
    float moonDiff = max(dot(norm, moonDir), 0.0);
    float moonElev = clamp(moonDir.y * 1.5 + 0.1, 0.0, 1.0);
    float moonContrib = moonDiff * moonElev * 0.28;

    // Dynamic ambient lighting modulated between night (0.22) and high noon (0.52)
    float ambient = mix(0.22, 0.52, sunElev);
    float directLight = sunContrib * (1.0 - ambient) + moonContrib;
    float sunLight = clamp(ambient + directLight, 0.20, 1.0);

    // inColor.a contains ambient occlusion term (0.5 to 1.0)
    float ao = (uEnableAO != 0) ? inColor.a : 1.0;

    // inColor.r contains the face directional lighting multiplier (0.55 to 1.0)
    float faceDir = inColor.r;

    // Blend face direction and dynamic sun light smoothly without squaring shadows
    float combinedLight = mix(faceDir, sunLight, 0.55);

    fragColor = vec4(1.0, 1.0, 1.0, inColor.a);
    fragLight = combinedLight * ao;
    fragDist = length(worldPos.xyz - uCamPos);
    fragUV = inUV;
    fragTexIndex = inTexIndex;
    fragWorldPos = worldPos.xyz;
    fragNormal = norm;
}
