#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec4 inColor;

layout(location = 0) out vec3 fragNormal;
layout(location = 1) out vec3 fragWorldPos;

layout(push_constant) uniform PushConstants {
    mat4 viewProj;
    mat4 model;
    vec4 holoColor;
} push;

void main() {
    vec4 worldPos = push.model * vec4(inPosition, 1.0);
    fragWorldPos = worldPos.xyz;
    fragNormal = mat3(push.model) * inNormal;
    gl_Position = push.viewProj * worldPos;
}
