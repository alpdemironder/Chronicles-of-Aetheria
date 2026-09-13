#version 330 core

layout(location = 0) in vec2 inPos;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec4 inColor;

uniform mat4 uOrtho;

out vec4 fragColor;
out vec2 fragUV;

void main() {
    gl_Position = uOrtho * vec4(inPos, 0.0, 1.0);
    fragColor = inColor;
    fragUV = inUV;
}
