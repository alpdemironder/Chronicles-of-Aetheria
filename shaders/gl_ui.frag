#version 330 core

in vec4 fragColor;
in vec2 fragUV;

out vec4 finalColor;

void main() {
    finalColor = fragColor;
}
