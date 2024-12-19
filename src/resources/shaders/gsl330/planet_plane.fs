#version 330
in vec2 fragTexCoord;
out vec4 fragColor;

void main() {
    float dist = length(fragTexCoord - vec2(0.5));
    float glow = smoothstep(0.3, 0.4, dist); // Creates a glowing edge
    fragColor = mix(vec4(0.1, 0.1, 0.2, 1.0), vec4(0.6, 0.6, 0.8, 1.0), glow); // Lunar-like coloring
}
