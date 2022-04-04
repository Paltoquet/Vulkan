#pragma once

#include "SceneObject.h"

#include <core/DescriptorTable.h>
#include <ui/ViewParams.h>
#include <utils/Camera.h>
#include <utils/TextureLoader.h>
#include <utils/Material.h>
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <vector>
#include <array>

#include "CubicFog.h"

class RenderScene
{
public:
    RenderScene();
    ~RenderScene();

public:
    void initialize(RenderContext& renderContext, DescriptorTable& descriptorTable, ViewParams& viewParams);
    void createGraphicPipelines(RenderContext& renderContext, VkRenderPass renderPass, DescriptorTable& descriptorTable);
    void destroyGraphicPipelines(RenderContext& renderContext);
    void updateUniforms(RenderContext& renderContext, Camera& camera, ViewParams& viewParams, DescriptorTable& descriptorTable, FrameDescriptor& currentDescriptor, DescriptorEntry& golbalDescriptor);
    void fillCommandBuffer(RenderContext& renderContext, VkCommandBuffer cmdBuffer, FrameDescriptor frameDescriptor, VkDescriptorSet globalDescriptor);
    void cleanUp(RenderContext& renderContext);

private:
    std::unique_ptr<TextureLoader> m_textureLoader;
    std::vector<std::unique_ptr<Mesh>> m_meshes;
    std::vector<std::unique_ptr<Material>> m_materials;
    std::vector<std::unique_ptr<SceneObject>> m_sceneObjects;

    ImageView m_cloudTexture;
    ImageView m_noiseTexture;
};
