#include "BlurMaterial.h"
#include <iostream>
#include <glm/gtx/string_cast.hpp>

BlurMaterial::BlurMaterial(VkDevice device, VkShaderModule vertexShader, VkShaderModule fragmentShader):
    Material(device, vertexShader, fragmentShader)
{
    /* ------------------ Descriptor Binding ------------------ */

    VkDescriptorSetLayoutBinding textureDataBinding;
    textureDataBinding.binding = 1;
    textureDataBinding.descriptorCount = 1;
    textureDataBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    textureDataBinding.pImmutableSamplers = nullptr;
    textureDataBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    m_descriptorBindings.push_back(textureDataBinding);
}

BlurMaterial::~BlurMaterial()
{

}

void BlurMaterial::createDescriptorBuffer(RenderContext& renderContext, VkBuffer& buffer, VkDeviceMemory& memory)
{
    //VkDeviceSize fogBufferSize = sizeof(CloudData);
    //VkMemoryPropertyFlags memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    //renderContext.createBuffer(fogBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, memoryPropertyFlags, buffer, memory);
}

void BlurMaterial::createTextureSampler(RenderContext& renderContext)
{
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(renderContext.physicalDevice(), &properties);

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = VK_LOD_CLAMP_NONE;

    if (vkCreateSampler(renderContext.device(), &samplerInfo, nullptr, &m_textureSampler) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture sampler!");
    }
}

void BlurMaterial::updateDescriptorSet(RenderContext& renderContext, VkDescriptorSet descriptorSet, VkBuffer buffer, size_t frameIndex)
{
    std::vector<VkWriteDescriptorSet> descriptorWrites;

    VkImageView inputTexture = renderContext.getOffScreenRenderTexture(frameIndex).imageView;

    VkDescriptorImageInfo imageInfo;
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    //imageInfo.imageView = m_textureImageView.view();
    imageInfo.imageView = inputTexture;
    imageInfo.sampler = m_textureSampler;

    VkWriteDescriptorSet inputSampler;
    inputSampler.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    inputSampler.dstSet = descriptorSet;
    inputSampler.dstBinding = 1;
    inputSampler.dstArrayElement = 0;
    inputSampler.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    inputSampler.descriptorCount = 1;
    inputSampler.pImageInfo = &imageInfo;
    inputSampler.pNext = nullptr;

    descriptorWrites.push_back(inputSampler);
    vkUpdateDescriptorSets(renderContext.device(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
}

void BlurMaterial::cleanUp(RenderContext& renderContext)
{
    Material::cleanUp(renderContext);
    vkDestroySampler(renderContext.device(), m_textureSampler, nullptr);
}
