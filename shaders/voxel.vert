#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec4 inColor;

layout(location = 0) out vec2 fragTexCoord;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec4 fragColor;
layout(location = 3) out vec3 fragWorldPos;

layout(push_constant) uniform PushConstants {
    mat4 viewProj;
    vec4 sunDir_Time;
    vec4 fogColor_Density;
    vec4 camPos;
} push;

void main() {
    fragTexCoord = inTexCoord;
    fragNormal = inNormal;
    fragColor = inColor;
    fragWorldPos = inPosition;
    gl_Position = push.viewProj * vec4(inPosition, 1.0);
}
