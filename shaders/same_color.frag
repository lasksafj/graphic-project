#version 330
// Make everything green.
layout (location=0) out vec4 FragColor;

uniform vec4 color;

void main() {
    FragColor = color;
}