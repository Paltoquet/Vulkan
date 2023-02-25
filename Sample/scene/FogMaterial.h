#pragma once

#include <utils/Material.h>
#include <utils/ImageView.h>
#include <glm/glm.hpp>

class FogMaterial : public Material
{
public:
    FogMaterial(VkDevice device, VkShaderModule vertexShader, VkShaderModule fragmentShader);
    ~FogMaterial();

public:
    struct CloudData {
        glm::vec4 worldCamera;
        glm::vec4 lightPosition;
        glm::vec4 bboxMin;
        glm::vec4 bboxMax;
        glm::vec4 lightColor;
        glm::vec4 lightAbsorption;
        glm::vec4 densityTreshold;
        glm::vec4 phaseParams;
        glm::vec4 fogSpeed;
        alignas(16) float fogDensity;
    };

public:
    void createDescriptorBuffer(RenderContext& renderContext, VkBuffer& buffer, VkDeviceMemory& memory) override;
    void updateDescriptorSet(RenderContext& renderContext, VkDescriptorSet descriptorSet, VkBuffer buffer, size_t frameIndex) override;
    void createTextureSampler(RenderContext& renderContext, const ImageView& imageView);
    void cleanUp(RenderContext& renderContext) override;

private:
    ImageView m_noiseTexture3D;
    VkSampler m_textureSampler;
};
