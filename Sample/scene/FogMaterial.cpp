#include "FogMaterial.h"
#include <iostream>
#include <glm/gtx/string_cast.hpp>

FogMaterial::FogMaterial(VkDevice device, VkShaderModule vertexShader, VkShaderModule fragmentShader):
    Material(device, vertexShader, fragmentShader)
{
    /* ------------------ Descriptor Binding ------------------ */

    VkDescriptorSetLayoutBinding textureDataBinding;
    textureDataBinding.binding = 1;
    textureDataBinding.descriptorCount = 1;
    textureDataBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    textureDataBinding.pImmutableSamplers = nullptr;
    textureDataBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutBinding fogDataBinding;
    fogDataBinding.binding = 3;
    fogDataBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    fogDataBinding.descriptorCount = 1;
    fogDataBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    fogDataBinding.pImmutableSamplers = nullptr; // Optional

    m_descriptorBindings.push_back(textureDataBinding);
    m_descriptorBindings.push_back(fogDataBinding);
}

FogMaterial::~FogMaterial()
{

}

void FogMaterial::createDescriptorBuffer(RenderContext& renderContext, VkBuffer& buffer, VkDeviceMemory& memory)
{
    VkDeviceSize fogBufferSize = sizeof(CloudData);
    VkMemoryPropertyFlags memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    renderContext.createBuffer(fogBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, memoryPropertyFlags, buffer, memory);
}

void FogMaterial::createTextureSampler(RenderContext& renderContext, const ImageView& imageView)
{
    m_noiseTexture3D = imageView;

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
    samplerInfo.maxLod = static_cast<float>(m_noiseTexture3D.mipLevels());

    if (vkCreateSampler(renderContext.device(), &samplerInfo, nullptr, &m_textureSampler) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture sampler!");
    }
}

void FogMaterial::updateDescriptorSet(RenderContext& renderContext, VkDescriptorSet descriptorSet, VkBuffer buffer, size_t frameIndex)
{
    std::vector<VkWriteDescriptorSet> descriptorWrites;

    VkDescriptorBufferInfo fogInfo;
    fogInfo.buffer = buffer;
    fogInfo.offset = 0;
    fogInfo.range = sizeof(CloudData);

    VkDescriptorImageInfo imageInfo;
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    //imageInfo.imageView = m_textureImageView.view();
    imageInfo.imageView = m_noiseTexture3D.view();
    imageInfo.sampler = m_textureSampler;

    VkWriteDescriptorSet imageSampler;
    imageSampler.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    imageSampler.dstSet = descriptorSet;
    imageSampler.dstBinding = 1;
    imageSampler.dstArrayElement = 0;
    imageSampler.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    imageSampler.descriptorCount = 1;
    imageSampler.pImageInfo = &imageInfo;
    imageSampler.pNext = nullptr;

    VkWriteDescriptorSet cloudBuffer;
    cloudBuffer.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    cloudBuffer.dstSet = descriptorSet;
    cloudBuffer.dstBinding = 3;
    cloudBuffer.dstArrayElement = 0;
    cloudBuffer.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    cloudBuffer.descriptorCount = 1;
    cloudBuffer.pBufferInfo = &fogInfo;
    cloudBuffer.pNext = nullptr;

    descriptorWrites.push_back(imageSampler);
    descriptorWrites.push_back(cloudBuffer);

    vkUpdateDescriptorSets(renderContext.device(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
}

void FogMaterial::cleanUp(RenderContext& renderContext)
{
    Material::cleanUp(renderContext);
    vkDestroySampler(renderContext.device(), m_textureSampler, nullptr);
}
