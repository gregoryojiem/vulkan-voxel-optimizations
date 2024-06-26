#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in uint packedColorInfo;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 bary;

void main() {
    float r = (packedColorInfo & 255) / 255.0;
    float g = ((packedColorInfo >> 8) & 255) / 255.0;
    float b = ((packedColorInfo >> 16) & 255) / 255.0;
    fragColor = vec3(r, g, b);

    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);
    bary = vec3(gl_VertexIndex % 3 == 0, gl_VertexIndex % 3 == 1, gl_VertexIndex % 3 == 2);
}