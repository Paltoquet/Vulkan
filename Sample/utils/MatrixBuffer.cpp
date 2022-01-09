#include "MatrixBuffer.h"

#include <stdexcept>

MatrixBuffer::MatrixBuffer()
{

}


MatrixBuffer::~MatrixBuffer()
{
}

/* -------------------------- Public methods -------------------------- */

VkDescriptorSetLayoutBinding MatrixBuffer::descriptorBinding()
{
    VkDescriptorSetLayoutBinding descriptorBinding;
    descriptorBinding.binding = 0;
    descriptorBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorBinding.descriptorCount = 1;
    descriptorBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    descriptorBinding.pImmutableSamplers = nullptr; // Optional
    return descriptorBinding;
}
