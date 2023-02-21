#pragma once

#include <utils/Quad.h>
#include "SceneObject.h"
#include "BlurMaterial.h"

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <vector>
#include <array>

class ScreenBlur : public SceneObject
{
public:
    ScreenBlur(Quad& mesh, BlurMaterial& material);
    ~ScreenBlur();

public:
    void update(RenderContext& renderContex, Camera& camera, const ViewParams& viewParams, const DescriptorEntry& descriptorEntry) override;

    Mesh* getMesh() override;
    Material* getMaterial() override;

private:
    Quad& m_mesh;
    BlurMaterial& m_material;
};
