#version 450

layout(location = 0) in vec3 fragNormal;
layout(location = 1) in vec3 fragWorldPos;

layout(location = 0) out vec4 outColor;

layout(push_constant) uniform PushConstants {
    mat4 viewProj;
    mat4 model;
    vec4 holoColor;
} push;

void main() {
    float time = push.holoColor.a;
    float scanline = sin(fragWorldPos.y * 14.0 - time * 6.0) * 0.5 + 0.5;
    float pulse = sin(time * 3.0) * 0.15 + 0.85;

    vec3 N = normalize(fragNormal);
    float edge = 1.0 - abs(N.z);

    vec3 col = push.holoColor.rgb * (0.6 + scanline * 0.4) * pulse;
    float alpha = 0.4 + edge * 0.35 + scanline * 0.1;

    outColor = vec4(col, clamp(alpha, 0.25, 0.9));
}
