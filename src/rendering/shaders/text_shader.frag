#version 450

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

float pxRange = 2;

float median(float r, float g, float b) {
    return max(min(r, g), min(max(r, g), b));
}

float screenPxRange() {
    vec2 unitRange = vec2(pxRange)/vec2(textureSize(texSampler, 0));
    vec2 screenTexSize = vec2(1.0)/fwidth(fragTexCoord);
    return max(0.5*dot(unitRange, screenTexSize), 1.0);
}

void main() {
    vec3 msd = texture(texSampler, fragTexCoord).rgb;
    float sd = median(msd.r, msd.g, msd.b);
    float screenPxDistance = screenPxRange()*(sd - 0.5);
    float opacity = clamp(screenPxDistance + 0.5, 0.0, 1.0);
    outColor = mix(vec4(0.1, 0.1, 0.1, 0.0), vec4(1.0), opacity);
}