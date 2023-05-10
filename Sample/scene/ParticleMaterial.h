#pragma once

#include <utils/Material.h>
#include <utils/ImageView.h>
#include <glm/glm.hpp>

class ParticleMaterial : public Material
{
public:
    ParticleMaterial(VkDevice device, VkShaderModule vertexShader, VkShaderModule fragmentShader, VkBuffer particleBuffer, VkDeviceSize bufferSize);
    ~ParticleMaterial();

public:

    void createFrameDescriptorBuffer(RenderContext& renderContext, VkBuffer& buffer, VkDeviceMemory& memory) override;
    void updateFrameDescriptorSet(RenderContext& renderContext, VkDescriptorSet descriptorSet, VkBuffer buffer) override;
    void updateRessourceDescripotSet(RenderContext& renderContext, VkDescriptorSet descriptorSet) override;
    void createTextureSampler(RenderContext& renderContext, const ImageView& imageView);
    void cleanUp(RenderContext& renderContext) override;

private:
    VkBuffer m_particleBuffer;
    VkDeviceSize m_bufferSize;
};
