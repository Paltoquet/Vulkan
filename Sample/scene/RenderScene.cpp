#include "RenderScene.h"

#include "CubicFog.h"
#include "QuadTexture.h"

#include <iostream>
#include <glm/gtx/string_cast.hpp>
#include <utils/MatrixBuffer.h>
#include <utils/ShaderLoader.h>
#include <utils/Quad.h>

RenderScene::RenderScene():
    m_textureLoader(nullptr)
{
    
}

RenderScene::~RenderScene()
{

}

/* -------------------------- Public methods -------------------------- */
void RenderScene::initialize(RenderContext& renderContext, DescriptorTable& descriptorTable)
{
    m_textureLoader = std::make_unique<TextureLoader>(&renderContext);
    /* -------------- Init Meshes -------------- */
    auto cube = std::make_unique<Cube>();
    cube->createBuffers(renderContext);
    Cube* cubePtr = cube.get();

    auto quad = std::make_unique<Quad>();
    quad->createBuffers(renderContext);
    Quad* quadPtr = quad.get();

    m_meshes.push_back(std::move(cube));
    m_meshes.push_back(std::move(quad));

    /* -------------- Textures -------------- */
    VkExtent2D dimension = VkExtent2D({ 512, 512 });
    VkExtent3D dimension3D = VkExtent3D({ 48, 48, 64 });

    ImageView noiseTexture3D = m_textureLoader->load3DNoiseTexture(dimension3D, VK_IMAGE_ASPECT_COLOR_BIT);
    ImageView worleyNoiseTexture = m_textureLoader->loadWorleyNoiseTexture(dimension, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
    m_textures.push_back(noiseTexture3D);
    m_textures.push_back(worleyNoiseTexture);
    m_textures.push_back(m_textureLoader->loadTexture("ressources/textures/viking_room.png", VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT));


    /* -------------- Init Materials -------------- */
    VkShaderModule fogVertexTextureShader = ShaderLoader::loadShader("shaders/cloud_vert.spv", renderContext.device());
    VkShaderModule fogFragmentTextureShader = ShaderLoader::loadShader("shaders/cloud_frag.spv", renderContext.device());
    auto fogMaterial = std::make_unique<FogMaterial>(renderContext.device(), fogVertexTextureShader, fogFragmentTextureShader);
    FogMaterial* fogMaterialPtr = fogMaterial.get();
    fogMaterialPtr->createTextureSampler(renderContext, noiseTexture3D);

    VkShaderModule textureVertexTextureShader = ShaderLoader::loadShader("shaders/texture_vert.spv", renderContext.device());
    VkShaderModule textureFragmentTextureShader = ShaderLoader::loadShader("shaders/texture_frag.spv", renderContext.device());
    auto quadMaterial = std::make_unique<TextureMaterial>(renderContext.device(), textureVertexTextureShader, textureFragmentTextureShader);
    TextureMaterial* quadMaterialPtr = quadMaterial.get();
    quadMaterialPtr->createTextureSampler(renderContext, worleyNoiseTexture);

    m_materials.push_back(std::move(fogMaterial));
    m_materials.push_back(std::move(quadMaterial));
    descriptorTable.addMaterial(fogMaterialPtr);
    descriptorTable.addMaterial(quadMaterialPtr);

    /* -------------- Init SceneObjects -------------- */
    auto fogObject = std::make_unique<CubicFog>(*cubePtr, *fogMaterialPtr);
    auto quadObject = std::make_unique<QuadTexture>(*quadPtr, *quadMaterialPtr);
    //m_sceneObjects.push_back(std::move(fogObject));
    m_sceneObjects.push_back(std::move(quadObject));
}

void RenderScene::createGraphicPipelines(RenderContext& renderContext, VkRenderPass renderPass, DescriptorTable& descriptorTable)
{
    auto globalDescriptorLayout = descriptorTable.globalDescriptorLayout();
    for (auto& sceneObject : m_sceneObjects) {
        auto* material = sceneObject->getMaterial();
        auto* mesh = sceneObject->getMesh();
        material->createPipeline(renderContext, renderPass, mesh->getBindingDescription(), mesh->getAttributeDescriptions(), globalDescriptorLayout);
    }
}

void RenderScene::destroyGraphicPipelines(RenderContext& renderContext)
{
    for (auto& sceneObject : m_sceneObjects) {
        auto* material = sceneObject->getMaterial();
        material->destroyPipeline(renderContext);
    }
}

void RenderScene::updateUniforms(RenderContext& renderContext, Camera& camera, FrameDescriptor& currentDescriptor, DescriptorEntry& golbalDescriptor)
{
    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    MatrixBuffer matrixBuffer;
    matrixBuffer.buffer.model = camera.arcBallModel() * glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, 1.8f));
    matrixBuffer.buffer.view = camera.viewMatrix();
    matrixBuffer.buffer.proj = camera.projectionMatrix();
    matrixBuffer.buffer.proj[1][1] *= -1;
    matrixBuffer.buffer.time = time;

    void* matrixData;
    vkMapMemory(renderContext.device(), golbalDescriptor.memory, 0, sizeof(MatrixBuffer::BufferData), 0, &matrixData);
    memcpy(matrixData, &matrixBuffer.buffer, sizeof(MatrixBuffer::BufferData));
    vkUnmapMemory(renderContext.device(), golbalDescriptor.memory);

    for (auto& sceneObject : m_sceneObjects) {
        auto* material = sceneObject->getMaterial();
        auto& descriptorEntry = currentDescriptor.getDescriptorEntry(material->materialId());
        sceneObject->update(renderContext, camera, descriptorEntry);
    }
}

void RenderScene::fillCommandBuffer(RenderContext& renderContext, VkCommandBuffer cmdBuffer, FrameDescriptor frameDescriptor, VkDescriptorSet globalDescriptor)
{
    Mesh* lastMesh = nullptr;
    Material* currentMaterial = nullptr;

    for (auto& sceneObject : m_sceneObjects)
    {
        //only bind the pipeline if it doesn't match with the already bound one
        Material* material = sceneObject->getMaterial();
        if (material != currentMaterial) {
            currentMaterial = material;
            vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, currentMaterial->pipeline());
        }

        //only bind the mesh if it's a different one from last bind
        if (sceneObject->getMesh() != lastMesh) {
            Mesh* currentMesh = sceneObject->getMesh();
            VkBuffer vertexBuffers[] = { currentMesh->vertexBuffer() };
            VkDeviceSize offsets[] = { 0 };
            vkCmdBindVertexBuffers(cmdBuffer, 0, 1, vertexBuffers, offsets);
            vkCmdBindIndexBuffer(cmdBuffer, currentMesh->indexBuffer(), 0, VK_INDEX_TYPE_UINT32);
            lastMesh = currentMesh;
        }

        // Global Matrix Buffer
        vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, sceneObject->getMaterial()->pipelineLayout(), 0, 1, &globalDescriptor, 0, nullptr);
        // Material Descriptor
        VkDescriptorSet materialDescriptor = frameDescriptor.getDescriptorEntry(currentMaterial->materialId()).descriptorSet;
        vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, sceneObject->getMaterial()->pipelineLayout(), 1, 1, &materialDescriptor, 0, nullptr);

        //we can now draw
        vkCmdDrawIndexed(cmdBuffer, static_cast<uint32_t>(sceneObject->getMesh()->indices().size()), 1, 0, 0, 0);
    }
}

void RenderScene::cleanUp(RenderContext& renderContext)
{
    for (auto& texture : m_textures) {
        texture.cleanUp(renderContext.device());
    }

    for (auto& mesh : m_meshes) {
        mesh->cleanUp(renderContext);
    }

    for (auto& material : m_materials) {
        material->cleanUp(renderContext);
    }

    m_sceneObjects.clear();
}
