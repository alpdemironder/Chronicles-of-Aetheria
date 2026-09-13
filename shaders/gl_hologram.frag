#version 330 core

in vec3 fragNormal;
in vec3 fragWorldPos;
in vec3 fragViewDir;

uniform vec4 uHoloColor;
uniform float uTime;

out vec4 finalColor;

void main() {
    float fresnel = 1.0 - max(dot(fragNormal, fragViewDir), 0.0);
    fresnel = pow(fresnel, 2.0);

    float scanline = sin(fragWorldPos.y * 24.0 + uTime * 7.0) * 0.15;
    float pulse = 0.8 + 0.2 * sin(uTime * 4.0);

    float alpha = clamp((0.45 + fresnel * 0.45 + scanline) * pulse, 0.2, 0.9);
    finalColor = vec4(uHoloColor.rgb, alpha);
}
