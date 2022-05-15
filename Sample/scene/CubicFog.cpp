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
    glm::vec3 bboxMin = mesh.bboxMin();
    glm::vec3 bboxMax = mesh.bboxMax();

    m_shaderData.lightPosition = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
    m_shaderData.bboxMin = glm::vec4(bboxMin.x, bboxMin.y, bboxMin.z, 0.0f);
    m_shaderData.bboxMax = glm::vec4(bboxMax.x, bboxMax.y, bboxMax.z, 0.0f);
    m_shaderData.fogDensity = 0.6f;
    m_shaderData.lightAbsorption = glm::vec4(0.6f);
    m_shaderData.densityTreshold = glm::vec4(0.6f);
    m_shaderData.phaseParams = glm::vec4(0.6f, 0.6f, 0.5f, 0.5f);
    m_shaderData.fogSpeed = glm::vec4(1.0f);
}

CubicFog::~CubicFog()
{

}

/* -------------------------- Public methods -------------------------- */

void CubicFog::update(RenderContext& renderContext, Camera& camera, const ViewParams& viewParams, const DescriptorEntry& descriptorEntry)
{
    void* fogData;
    glm::mat3 rot = camera.arcBallModel();
    rot = glm::inverse(rot);
    auto worldEye = rot * camera.eye();
    auto boxScale = viewParams.fogScale();
    const glm::vec3& lightPosition = viewParams.lightPosition();
    glm::vec3 bboxMin = m_mesh.bboxMin();
    glm::vec3 bboxMax = m_mesh.bboxMax();
    bboxMin.z *= boxScale;
    bboxMax.z *= boxScale;

    m_shaderData.lightPosition = glm::vec4(lightPosition.x, lightPosition.y, lightPosition.z, 0.0f);
    m_shaderData.bboxMin = glm::vec4(bboxMin.x, bboxMin.y, bboxMin.z, 0.0f);
    m_shaderData.bboxMax = glm::vec4(bboxMax.x, bboxMax.y, bboxMax.z, 0.0f);
    m_shaderData.worldCamera = glm::vec4(worldEye, 1.0);
    m_shaderData.fogSpeed = glm::vec4(viewParams.speed());
    m_shaderData.lightColor = viewParams.lightColor();
    m_shaderData.lightAbsorption = glm::vec4(viewParams.lightAbsorption());
    m_shaderData.densityTreshold = glm::vec4(viewParams.densityTreshold());
    m_shaderData.phaseParams = glm::vec4(viewParams.inScatering(), viewParams.outScatering(), viewParams.phaseFactor(), viewParams.phaseOffset());

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

FogMaterial::CloudData* CubicFog::shaderData()
{
    return &m_shaderData;
}