#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 fragTexCoord;
layout(location = 2) out vec3 cameraDirection;

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(binding = 3) uniform CloudData {
    vec3 worldCamera;
    vec3[24] planes;
} cloud;

void main() {
	//[-0.5, 0.5] -> [-1; 1]  -> [0; 2] -> [0; 1]
	vec3 texturePos = (inPosition * 2.0 + 1.0) / 2.0;
    fragColor = inColor;
    fragTexCoord = vec3(texturePos.x, texturePos.z, texturePos.y);
    cameraDirection = inPosition - cloud.worldCamera;
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);
}