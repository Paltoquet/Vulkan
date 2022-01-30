#pragma once

#include <utils/Material.h>
#include <utils/ImageView.h>
#include <glm/glm.hpp>

class TextureMaterial : public Material
{
public:
    TextureMaterial(VkDevice device, VkShaderModule vertexShader, VkShaderModule fragmentShader);
    ~TextureMaterial();

public:
    void createDescriptorBuffer(RenderContext& renderContext, VkBuffer& buffer, VkDeviceMemory& memory) override;
    void updateDescriptorSet(RenderContext& renderContext, VkDescriptorSet descriptorSet, VkBuffer buffer) override;
    void createTextureSampler(RenderContext& renderContext, const ImageView& imageView);
    void cleanUp(RenderContext& renderContext) override;

private:
    ImageView m_noiseTexture2D;
    VkSampler m_textureSampler;
};
