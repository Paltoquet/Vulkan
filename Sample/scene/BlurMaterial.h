#pragma once

#include <utils/Material.h>
#include <utils/ImageView.h>
#include <glm/glm.hpp>

class BlurMaterial : public Material
{
public:
    BlurMaterial(VkDevice device, VkShaderModule vertexShader, VkShaderModule fragmentShader);
    ~BlurMaterial();

public:
    void createDescriptorBuffer(RenderContext& renderContext, VkBuffer& buffer, VkDeviceMemory& memory) override;
    void updateDescriptorSet(RenderContext& renderContext, VkDescriptorSet descriptorSet, VkBuffer buffer, size_t frameIndex) override;
    void createTextureSampler(RenderContext& renderContext);
    void cleanUp(RenderContext& renderContext) override;

private:
    VkSampler m_textureSampler;
};
