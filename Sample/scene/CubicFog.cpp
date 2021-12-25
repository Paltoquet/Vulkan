#include "CubicFog.h"

CubicFog::CubicFog()
{
    m_descriptorBinding.binding = 3;
    m_descriptorBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    m_descriptorBinding.descriptorCount = 1;
    m_descriptorBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    m_descriptorBinding.pImmutableSamplers = nullptr; // Optional

    m_shaderData.worldCamera = glm::vec4(0.0f, 0.0f, -1.0f, 1.0f);
    m_shaderData.planes;
}

CubicFog::~CubicFog()
{

}

/* -------------------------- Public methods -------------------------- */

Cube& CubicFog::mesh()
{
    return m_mesh;
}

const VkDescriptorSetLayoutBinding& CubicFog::descriptorBinding() const
{
    return m_descriptorBinding;
}

CubicFog::CloudData& CubicFog::shaderData()
{
    return m_shaderData;
}