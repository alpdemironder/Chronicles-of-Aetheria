#version 450

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec4 fragColor;
layout(location = 3) in vec3 fragWorldPos;

layout(location = 0) out vec4 outColor;

layout(push_constant) uniform PushConstants {
    mat4 viewProj;
    vec4 sunDir_Time;
    vec4 fogColor_Density;
    vec4 camPos;
} push;

void main() {
    vec3 N = normalize(fragNormal);
    vec3 L = normalize(push.sunDir_Time.xyz);
    float diff = max(dot(N, L), 0.0);
    float ambient = 0.38;
    float light = ambient + diff * 0.62;

    // Grid / block outline subtle shading
    vec3 baseCol = fragColor.rgb * light;

    // Distance fog towards biome sky/fog color
    float dist = length(fragWorldPos - push.camPos.xyz);
    float fogFactor = clamp((dist - 35.0) / (110.0 - 35.0), 0.0, 1.0);
    vec3 finalCol = mix(baseCol, push.fogColor_Density.rgb, fogFactor);

    outColor = vec4(finalCol, fragColor.a);
}
