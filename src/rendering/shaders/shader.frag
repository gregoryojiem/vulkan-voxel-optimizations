#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 bary;

layout(location = 0) out vec4 outColor;

const float epsilon = 0.01f;

void main() {
    // set edges to black (wireframe)
    if (abs(bary.x) < epsilon || abs(bary.y) < epsilon || abs(bary.z) < epsilon) {
        outColor = vec4(fragColor, 1.0);
    } else {
        outColor = vec4(fragColor, 1.0);
    }
}