#include "CubicFog.h"

CubicFog::CubicFog()
{
    m_descriptorBinding.binding = 3;
    m_descriptorBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    m_descriptorBinding.descriptorCount = 1;
    m_descriptorBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    m_descriptorBinding.pImmutableSamplers = nullptr; // Optional

    auto& vertices = m_mesh.vertices();
    auto planeIndex = 0;
    int nbIndices = 18;
    for (auto i = 0; planeIndex < nbIndices; i += 4) {
        m_shaderData.planes[planeIndex] = glm::vec4(vertices.at(i + 1).pos, 1.0f);
        m_shaderData.planes[planeIndex + 1] = glm::vec4(vertices.at(i).pos, 1.0f);
        m_shaderData.planes[planeIndex + 2] = glm::vec4(vertices.at(i + 2).pos, 1.0f);
        planeIndex += 3;
    }
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