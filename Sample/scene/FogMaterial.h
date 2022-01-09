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
        // Should be an array of vec3, but vulkan apparently sucks at indexing arrays of vector3 due to alignment rounded to 16
        // Extension GL_EXT_scalar_block_layout might get it worked out, but why ???????
        glm::vec4 planes[18];
        alignas(16) float fogDensity;
    };

public:
    void createDescriptorBuffer(RenderContext& renderContext, VkBuffer& buffer, VkDeviceMemory& memory) override;
    void updateDescriptorSet(RenderContext& renderContext, VkDescriptorSet descriptorSet, VkBuffer buffer) override;
    void createTextureSampler(RenderContext& renderContext, const ImageView& imageView);
    void cleanUp(RenderContext& renderContext) override;

private:
    ImageView m_noiseTexture3D;
    VkSampler m_textureSampler;
};
