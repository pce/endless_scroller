#version 330
out vec4 fragColor;

void main() {
    vec3 skyColor = vec3(0.4, 0.6, 1.0); // Base sky color
    float gradient = gl_FragCoord.y / 720.0; // Gradient based on screen height
    fragColor = vec4(mix(skyColor, vec3(0.1, 0.1, 0.3), gradient), 1.0);
}
