#version 150
in float brightness;

out vec4 FragColor;

void main() {
    FragColor = vec4(brightness, brightness, brightness, 1.0);
}