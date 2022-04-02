#include "CubicFog.h"
#include <iostream>
#include <glm/gtx/string_cast.hpp>

CubicFog::CubicFog(Cube& mesh, FogMaterial& material) :
    SceneObject(),
    m_mesh(mesh),
    m_material(material)
{
    auto& vertices = m_mesh.vertices();
    auto planeIndex = 0;
    int nbIndices = 18;
    for (auto i = 0; planeIndex < nbIndices; i += 4) {
        m_shaderData.planes[planeIndex] = glm::vec4(vertices.at(i + 1).pos, 1.0f);
        m_shaderData.planes[planeIndex + 1] = glm::vec4(vertices.at(i).pos, 1.0f);
        m_shaderData.planes[planeIndex + 2] = glm::vec4(vertices.at(i + 2).pos, 1.0f);
        planeIndex += 3;
    }
    m_shaderData.fogDensity = 0.6f;
}

CubicFog::~CubicFog()
{

}

/* -------------------------- Public methods -------------------------- */

void CubicFog::update(RenderContext& renderContext, Camera& camera, const DescriptorEntry& descriptorEntry)
{
    void* fogData;
    glm::mat3 rot = camera.arcBallModel();
    rot = glm::inverse(rot);
    auto worldEye = rot * camera.eye();
    m_shaderData.worldCamera = glm::vec4(worldEye, 1.0);

    vkMapMemory(renderContext.device(), descriptorEntry.memory, 0, sizeof(FogMaterial::CloudData), 0, &fogData);
    memcpy(fogData, &m_shaderData, sizeof(FogMaterial::CloudData));
    vkUnmapMemory(renderContext.device(), descriptorEntry.memory);
}

void CubicFog::setFogDensity(float opacity)
{
    m_shaderData.fogDensity = glm::max(opacity, 0.05f);
}

float CubicFog::fogDensity() const
{
    return m_shaderData.fogDensity;
}

Mesh* CubicFog::getMesh()
{
    return &m_mesh;
}

Material* CubicFog::getMaterial()
{
    return &m_material;
}

FogMaterial* CubicFog::getFogMaterial()
{
    return &m_material;
}

FogMaterial::CloudData* CubicFog::shaderData()
{
    return &m_shaderData;
}