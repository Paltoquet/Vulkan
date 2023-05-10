#include "ParticleSystem.h"

#include "QuadTexture.h"

#include <iostream>
#include <cmath> 
#include <random> 
#include <glm/gtx/string_cast.hpp>
#include <utils/MatrixBuffer.h>
#include <utils/ShaderLoader.h>
#include <utils/Quad.h>

ParticleSystem::ParticleSystem() :
    m_nbRow(20),
    m_nbCol(40),
    m_textureLoader(nullptr)
{

}

ParticleSystem::~ParticleSystem()
{

}

/* -------------------------- Public methods -------------------------- */
void ParticleSystem::initialize(RenderContext& renderContext, DescriptorTable& descriptorTable, ViewParams& viewParams)
{
    m_textureLoader = std::make_unique<TextureLoader>(&renderContext);

    /* -------------- Partcle datas -------------- */

    std::mt19937 gen(42);
    std::uniform_real_distribution<float> distrFloat;
    auto randFloat = std::bind(distrFloat, gen);

    m_particleDatas.reserve(m_nbCol * m_nbRow);
    float colOffset = 1.0f / m_nbCol;
    float rowOffset = 1.0f / m_nbRow;
    float quadWidth = colOffset / 2.0f;
    float quadHeiht = rowOffset / 2.0f;
    for (uint32_t col = 0; col < m_nbCol; col++) {
        glm::vec4 colColor = glm::vec4(randFloat(), randFloat(), randFloat(), 1.0f);
        for (uint32_t row = 0; row < m_nbRow; row++) {
            glm::vec2 particlePos = glm::vec2(float(col) * colOffset, float(row) * rowOffset) * glm::vec2(2.0) - glm::vec2(1.0);
            m_particleDatas.emplace_back(colColor, particlePos);
        }
    }

    VkDeviceSize storageBufferSize = m_particleDatas.size() * sizeof(ParticleData);
    VkBufferUsageFlags usageFlags = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    VkMemoryPropertyFlags memoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    renderContext.createBuffer(storageBufferSize, usageFlags, memoryPropertyFlags, m_particleBuffer, m_particleBufferMemory);

    void* particlePtr;
    vkMapMemory(renderContext.device(), m_particleBufferMemory, 0, storageBufferSize, 0, &particlePtr);
    memcpy(particlePtr, m_particleDatas.data(), storageBufferSize);
    vkUnmapMemory(renderContext.device(), m_particleBufferMemory);

    /* -------------- Init Meshes -------------- */
    m_quad = std::make_unique<Quad>();
    m_quad->createBuffers(renderContext);

    /* -------------- Textures -------------- */
    VkExtent2D dimension = VkExtent2D({ 512, 512 });
    VkExtent3D dimension3D = VkExtent3D({ 32, 32, 32 });

    //m_textures.push_back(m_textureLoader->loadTexture("ressources/textures/viking_room.png", VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT));

    /* -------------- Init Materials -------------- */
    VkShaderModule particleVertexShader = ShaderLoader::loadShader("shaders/particle_vert.spv", renderContext.device());
    VkShaderModule partcileFragmentShader = ShaderLoader::loadShader("shaders/particle_frag.spv", renderContext.device());
    VkDeviceSize particleBufferSize = m_nbCol * m_nbRow * sizeof(ParticleData);
    m_particleMaterial = std::make_unique<ParticleMaterial>(renderContext.device(), particleVertexShader, partcileFragmentShader, m_particleBuffer, particleBufferSize);
    //fogMaterialPtr->createTextureSampler(renderContext, m_cloudTexture);

    descriptorTable.addMaterial(m_particleMaterial.get());
}

void ParticleSystem::createGraphicPipelines(RenderContext& renderContext, VkRenderPass renderPass, DescriptorTable& descriptorTable)
{
    auto globalDescriptorLayout = descriptorTable.worldDescriptorLayout();
    m_particleMaterial->createPipeline(renderContext, renderPass, m_quad->getBindingDescription(), m_quad->getAttributeDescriptions(), globalDescriptorLayout);
}

void ParticleSystem::destroyGraphicPipelines(RenderContext& renderContext)
{
    m_particleMaterial->destroyPipeline(renderContext);
}

void ParticleSystem::updateUniforms(RenderContext& renderContext, Camera& camera, ViewParams& viewParams, DescriptorTable& descriptorTable, FrameDescriptor& currentDescriptor, DescriptorEntry& golbalDescriptor)
{

    // ------------------- Matrices


    // ------------------- Textures

    // ------------------ SceneObjects

    /*for (auto& sceneObject : m_sceneObjects) {
        auto* material = sceneObject->getMaterial();
        auto& descriptorEntry = currentDescriptor.getDescriptorEntry(material->materialId());
        sceneObject->update(renderContext, camera, viewParams, descriptorEntry);
    }*/
}

void ParticleSystem::fillCommandBuffer(RenderContext& renderContext, VkCommandBuffer cmdBuffer, DescriptorTable& descriptorTable, FrameDescriptor frameDescriptor, VkDescriptorSet globalDescriptor)
{
    vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_particleMaterial->pipeline());

    VkBuffer vertexBuffers[] = { m_quad->vertexBuffer() };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(cmdBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(cmdBuffer, m_quad->indexBuffer(), 0, VK_INDEX_TYPE_UINT32);

    // Global Matrix Buffer
    vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_particleMaterial->pipelineLayout(), 0, 1, &globalDescriptor, 0, nullptr);
    // Material Descriptor
    VkDescriptorSet materialDescriptor = frameDescriptor.getDescriptorEntry(m_particleMaterial->materialId()).descriptorSet;
    vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_particleMaterial->pipelineLayout(), 1, 1, &materialDescriptor, 0, nullptr);
    // Particle Descriptor
    VkDescriptorSet particleDescriptor = descriptorTable.getMaterialRessourceDescriptor(m_particleMaterial->materialId());
    vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_particleMaterial->pipelineLayout(), 2, 1, &particleDescriptor, 0, nullptr);

    //we can now draw
    uint32_t nbParticles = m_nbCol * m_nbRow;
    vkCmdDrawIndexed(cmdBuffer, static_cast<uint32_t>(m_quad->indices().size()), nbParticles, 0, 0, 0);
}

void ParticleSystem::cleanUp(RenderContext& renderContext)
{
    //m_noiseTexture.cleanUp(renderContext.device());

    vkDestroyBuffer(renderContext.device(), m_particleBuffer, nullptr);
    vkFreeMemory(renderContext.device(), m_particleBufferMemory, nullptr);

    m_quad->cleanUp(renderContext);
    m_particleMaterial->cleanUp(renderContext);
}
