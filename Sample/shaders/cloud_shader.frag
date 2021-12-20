#version 450

layout(binding = 1) uniform sampler3D texSampler3D;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragTexCoord;

layout(location = 0) out vec4 outColor;

void main() {
    float noiseValue = texture(texSampler3D, fragTexCoord).r;
    outColor = vec4(noiseValue, noiseValue, noiseValue, 1.0);
    //outColor = vec4(fragTexCoord.z, fragTexCoord.z, fragTexCoord.z, 1.0);
    //outColor = vec4(fragTexCoord.x, fragTexCoord.y, 0.0, 1.0);
}