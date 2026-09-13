#version 330 core

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec4 inColor;

uniform mat4 uMVP;
uniform mat4 uModel;
uniform vec3 uCamPos;

out vec3 fragNormal;
out vec3 fragWorldPos;
out vec3 fragViewDir;

void main() {
    vec4 worldPos = uModel * vec4(inPos, 1.0);
    gl_Position = uMVP * vec4(inPos, 1.0);

    fragWorldPos = worldPos.xyz;
    fragNormal = normalize((uModel * vec4(inNormal, 0.0)).xyz);
    fragViewDir = normalize(uCamPos - worldPos.xyz);
}
