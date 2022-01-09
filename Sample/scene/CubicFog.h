#pragma once

#include <utils/Cube.h>
#include "SceneObject.h"
#include "FogMaterial.h"

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <vector>
#include <array>

class CubicFog : public SceneObject
{
public:
    CubicFog(Cube& mesh, FogMaterial& material);
    ~CubicFog();

public:
    void update(RenderContext& renderContex, Camera& camera, const DescriptorEntry& descriptorEntry) override;

    Mesh* getMesh() override;
    Material* getMaterial() override;
    FogMaterial::CloudData* shaderData();
    void setFogDensity(float density);
    float fogDensity() const;

private:
    Cube& m_mesh;
    FogMaterial& m_material;
    FogMaterial::CloudData m_shaderData;
};
