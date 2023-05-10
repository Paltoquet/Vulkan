#pragma once

#include "RenderContext.h"
#include "DescriptorTable.h"
#include "Window.h"
#include <scene/RenderScene.h>
#include <scene/ParticleSystem.h>
#include <utils/Camera.h>
#include <ui/ViewParams.h>
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
    void initialize(Window* window, const SwapChainSupportInfos& swapChainSupport, ViewParams& viewParams);
    void createCommandBuffers();
    void updateUniformBuffer(Camera& camera, ViewParams& viewParams, uint32_t imageIndex);
    void updateCommandBuffer(uint32_t imageIndex);
    void fillCommandBuffers(uint32_t imageIndex);
    void drawFrame(Camera& camera, ViewParams& viewParams);
    void resize(int width, int height, const SwapChainSupportInfos& swapChainSupport);
    void cleanUp();

public:
    RenderContext* renderContext();

private:
    void createMainRenderPass();
    void createGraphicInterface(Window* window, ViewParams& viewParams);
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
    std::unique_ptr<ParticleSystem> m_particleSystem;
    // Uniforms
    std::unique_ptr<DescriptorTable> m_descriptorTable;

    // Sync Objects
    uint32_t m_currentFrame;
    std::vector<VkSemaphore> m_imageAvailableSemaphores;
    std::vector<VkSemaphore> m_renderFinishedSemaphores;
};

