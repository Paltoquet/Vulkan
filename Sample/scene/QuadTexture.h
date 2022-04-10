#pragma once

#include <utils/Quad.h>
#include "SceneObject.h"
#include "TextureMaterial.h"

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <vector>
#include <array>

class QuadTexture : public SceneObject
{
public:
    QuadTexture(Quad& mesh, TextureMaterial& material);
    ~QuadTexture();

public:
    void update(RenderContext& renderContex, Camera& camera, const ViewParams& viewParams, const DescriptorEntry& descriptorEntry) override;

    Mesh* getMesh() override;
    Material* getMaterial() override;

private:
    Quad& m_mesh;
    TextureMaterial& m_material;
};
