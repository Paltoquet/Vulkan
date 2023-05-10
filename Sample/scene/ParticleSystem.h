#pragma once

#include <core/DescriptorTable.h>
#include <ui/ViewParams.h>
#include <utils/Camera.h>
#include <utils/TextureLoader.h>
#include <utils/Material.h>
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <vector>
#include <array>

#include <utils/Quad.h>
#include "ParticleMaterial.h"

struct ParticleData {
    ParticleData(const glm::vec4& c, const glm::vec2& p) :
        color(c), pos(glm::vec4(p.x, p.y, 0.0, 1.0)) {}

    glm::vec4 color;
    glm::vec4 pos;  // Particle position
};

class ParticleSystem
{
public:
    ParticleSystem();
    ~ParticleSystem();

public:
    void initialize(RenderContext& renderContext, DescriptorTable& descriptorTable, ViewParams& viewParams);
    void createGraphicPipelines(RenderContext& renderContext, VkRenderPass renderPass, DescriptorTable& descriptorTable);
    void destroyGraphicPipelines(RenderContext& renderContext);
    void updateUniforms(RenderContext& renderContext, Camera& camera, ViewParams& viewParams, DescriptorTable& descriptorTable, FrameDescriptor& currentDescriptor, DescriptorEntry& golbalDescriptor);
    void fillCommandBuffer(RenderContext& renderContext, VkCommandBuffer cmdBuffer, DescriptorTable& descriptorTable, FrameDescriptor frameDescriptor, VkDescriptorSet globalDescriptor);
    void cleanUp(RenderContext& renderContext);

private:
    uint32_t m_nbRow;
    uint32_t m_nbCol;
    std::vector<ParticleData> m_particleDatas;
    std::unique_ptr<TextureLoader> m_textureLoader;
    std::unique_ptr<Quad> m_quad;
    std::unique_ptr<ParticleMaterial> m_particleMaterial;

    VkBuffer m_particleBuffer;
    VkDeviceMemory m_particleBufferMemory;
};
