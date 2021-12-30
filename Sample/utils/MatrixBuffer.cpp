#include "MatrixBuffer.h"

#include <stdexcept>

MatrixBuffer::MatrixBuffer()
{
    m_descriptorBinding.binding = 0;
    m_descriptorBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    m_descriptorBinding.descriptorCount = 1;
    m_descriptorBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    m_descriptorBinding.pImmutableSamplers = nullptr; // Optional
}


MatrixBuffer::~MatrixBuffer()
{
}

/* -------------------------- Public methods -------------------------- */

const VkDescriptorSetLayoutBinding& MatrixBuffer::descriptorBinding() const
{
    return m_descriptorBinding;
}
