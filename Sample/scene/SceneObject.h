#pragma once

#include <vulkan/vulkan.h>
#include <utils/Camera.h>
#include <utils/SkinMesh.h>
#include <utils/Material.h>
#include <core/DescriptorTable.h>
#include <ui/ViewParams.h>

class SceneObject
{
public:
    SceneObject();
    virtual ~SceneObject() = default;

public:
    virtual void update(RenderContext& renderContext, Camera& camera, const ViewParams& viewParams, const DescriptorEntry& descriptorEntry) = 0;
    virtual Mesh* getMesh() = 0;
    virtual Material* getMaterial() = 0;

    glm::mat4 transform() const;

protected:
    glm::mat4 m_transform;
};
