#pragma once

#include "RenderContext.h"
#include "DescriptorTable.h"
#include <scene/RenderScene.h>
#include <utils/Camera.h>

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
    void drawFrame(Camera& camera);
    void resize(int width, int height, const SwapChainSupportInfos& swapChainSupport);
    void cleanUp();

public:
    RenderContext* renderContext();

private:
    void createMainRenderPass();
    void createCommandBuffers();
    void createSyncObjects();

    void updateUniformBuffer(Camera& camera, uint32_t currentImage);

    void recreateSwapChain();
    void cleanUpSwapchain();

private:
    // Window
    SwapChainSupportInfos m_swapChainSupportInfo;
    uint32_t m_windowWidth;
    uint32_t m_windowHeight;
    // RenderPass
    std::unique_ptr<RenderContext> m_renderContext;
    VkRenderPass m_mainRenderPass;
    // Draw commands
    std::vector<VkCommandBuffer> m_commandBuffers;
    // Scene
    std::unique_ptr<RenderScene> m_renderScene;
    // Uniforms
    std::unique_ptr<DescriptorTable> m_descriptorTable;

    // Sync Objects
    uint32_t m_currentFrame;
    std::vector<VkSemaphore> m_imageAvailableSemaphores;
    std::vector<VkSemaphore> m_renderFinishedSemaphores;
};

