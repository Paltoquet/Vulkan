#pragma once

#include "RenderContext.h"
#include "DescriptorTable.h"
#include "Window.h"
#include <scene/RenderScene.h>
#include <utils/Camera.h>
#include <ui/FogMenu.h>

#include <memory>
#include <vector>
#include <optional>

class Engine
{
public:
    Engine(const VkInstance& vkInstance, const VkSurfaceKHR& surface, const VkPhysicalDevice& device);
    ~Engine();

public:
    void initialize(Window* window, const SwapChainSupportInfos& swapChainSupport);
    void createCommandBuffers();
    void updateUniformBuffer(Camera& camera, uint32_t imageIndex);
    void updateCommandBuffer(uint32_t imageIndex);
    void fillCommandBuffers(uint32_t imageIndex);
    void drawFrame(Camera& camera);
    void resize(int width, int height, const SwapChainSupportInfos& swapChainSupport);
    void cleanUp();

public:
    RenderContext* renderContext();

private:
    void createMainRenderPass();
    void createGraphicInterface(Window* window);
    void createSyncObjects();
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
    // Graphic Interface
    std::unique_ptr<FogMenu> m_graphicInterface;
    // Scene
    std::unique_ptr<RenderScene> m_renderScene;
    // Uniforms
    std::unique_ptr<DescriptorTable> m_descriptorTable;

    // Sync Objects
    uint32_t m_currentFrame;
    std::vector<VkSemaphore> m_imageAvailableSemaphores;
    std::vector<VkSemaphore> m_renderFinishedSemaphores;
};

