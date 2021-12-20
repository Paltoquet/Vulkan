#pragma once

#include "RenderContext.h"
#include <utils/Camera.h>
#include <utils/TextureLoader.h>
#include "utils/NoiseTextureLoader.h"
#include <utils/MatrixBuffer.h>
#include <utils/SkinMesh.h>
#include <utils/Cube.h>
#include <utils/Quad.h>

#include <memory>
#include <vector>
#include <optional>

class Engine
{
public:
    Engine(const VkSurfaceKHR& surface, const VkPhysicalDevice& device);
    ~Engine();

public:
    void initialize(const VkExtent2D& dimension, const SwapChainSupportInfos& swapChainSupport);
    void drawFrame(const Camera& camera);
    void resize(int width, int height, const SwapChainSupportInfos& swapChainSupport);
    void cleanUp();

public:
    RenderContext* renderContext();

private:
    void createMainRenderPass();
    void createDescriptorLayout();
    void loadModels();
    void loadTextures();
    void loadShaders();
    void createGraphicsPipeline();
    void createTextureSampler();
    void createMeshBuffers();
    void createUniformBuffer();
    void createDescriptorPool();
    void createDescriptorSet();
    void createCommandBuffers();
    void createSyncObjects();

    void updateUniformBuffer(const Camera& camera, uint32_t currentImage);

    void recreateSwapChain();
    void cleanUpSwapchain();

private:
    // Pipeline
    std::unique_ptr<RenderContext> m_renderContext;
    std::unique_ptr<TextureLoader> m_textureLoader;
    std::unique_ptr<NoiseTextureLoader> m_noiseTextureLoader;
    VkRenderPass m_mainRenderPass;
    VkPipelineLayout m_mainPipelineLayout;
    VkPipeline m_mainGraphicsPipeline;

    // Window
    SwapChainSupportInfos m_swapChainSupportInfo;
    uint32_t m_windowWidth;
    uint32_t m_windowHeight;

    // Uniforms
    VkDescriptorSetLayout m_sceneDescriptorLayout;
    std::vector<VkBuffer> m_uniformBuffers;
    std::vector<VkDeviceMemory> m_uniformBuffersMemory;
    VkDescriptorPool m_descriptorPool;
    std::vector<VkDescriptorSet> m_descriptorSets;

    // Draw commands
    std::vector<VkCommandBuffer> m_commandBuffers;

    // Models data
    SkinMesh m_mesh;
    Cube m_cube;
    ImageView m_textureImageView;
    ImageView m_noiseTexture3D;
    VkSampler m_textureSampler;
    MatrixBuffer m_matrixBuffer;
    VkShaderModule m_vertTextureShader;
    VkShaderModule m_fragTextureShader;

    uint32_t m_currentFrame;
    std::vector<VkSemaphore> m_imageAvailableSemaphores;
    std::vector<VkSemaphore> m_renderFinishedSemaphores;
};

