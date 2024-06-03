#version 450

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

void main() {
    float sdfValue = texture(texSampler, fragTexCoord).r;

    // Check if the fragment is inside the glyph
    if (sdfValue > 0.5) {
        // Set the output color to the fragment color
        outColor = vec4(1.0);
    } else {
        // Set the output color to transparent
        outColor = vec4(0.0);
    }
}