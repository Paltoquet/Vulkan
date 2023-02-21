#include "BlurMaterial.h"
#include <iostream>
#include <glm/gtx/string_cast.hpp>

BlurMaterial::BlurMaterial(VkDevice device, VkShaderModule vertexShader, VkShaderModule fragmentShader):
    Material(device, vertexShader, fragmentShader, 1)
{
    /* ------------------ Descriptor Binding ------------------ */

    VkDescriptorSetLayoutBinding textureDataBinding;
    textureDataBinding.binding = 1;
    textureDataBinding.descriptorCount = 1;
    // From input attachment
    textureDataBinding.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
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

void BlurMaterial::updateDescriptorSet(RenderContext& renderContext, VkDescriptorSet descriptorSet, VkBuffer buffer)
{
    std::vector<VkWriteDescriptorSet> descriptorWrites;

    VkImageView frameBufferView = renderContext.getBlurColorAttachment().imageView;

    VkDescriptorImageInfo imageInfo;
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    //imageInfo.imageView = m_textureImageView.view();
    imageInfo.imageView = renderContext.getBlurColorAttachment().imageView;
    //Note that we don’t pass a sampler, as input attachments are just pixel local loads and as such aren’t sampled in any way. 
    // By reading them you read that exact same value that was previously written at that position.
    imageInfo.sampler = VK_NULL_HANDLE;

    VkWriteDescriptorSet inputSampler;
    inputSampler.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    inputSampler.dstSet = descriptorSet;
    inputSampler.dstBinding = 1;
    inputSampler.dstArrayElement = 0;
    inputSampler.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    inputSampler.descriptorCount = 1;
    inputSampler.pImageInfo = &imageInfo;
    inputSampler.pNext = nullptr;

    descriptorWrites.push_back(inputSampler);
    vkUpdateDescriptorSets(renderContext.device(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
}

void BlurMaterial::cleanUp(RenderContext& renderContext)
{
    Material::cleanUp(renderContext);
}
