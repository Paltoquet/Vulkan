#include "Engine.h"
#include <utils/ShaderLoader.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <array>

/* --------------------------------- Constructors --------------------------------- */

Engine::Engine(const VkInstance& vkInstance, const VkSurfaceKHR& surface, const VkPhysicalDevice& device):
    m_renderContext(nullptr),
    m_renderScene(nullptr),
    m_descriptorTable(nullptr)
{
    m_renderContext = std::make_unique<RenderContext>(vkInstance, surface, device);
}


Engine::~Engine()
{

}

/* --------------------------------- Public methods --------------------------------- */

void Engine::initialize(Window* window, const SwapChainSupportInfos& swapChainSupport, ViewParams& viewParams)
{
    m_swapChainSupportInfo = swapChainSupport;
    m_windowWidth = window->width();
    m_windowHeight = window->height();
    VkExtent2D dimension = { m_windowWidth, m_windowHeight };

    m_renderContext->createLogicalDevice();
    m_renderContext->pickGraphicQueue();
    m_renderContext->pickDepthImageFormat();
    m_renderContext->pickSampleCount();
    m_renderContext->createSwapChain(dimension, swapChainSupport);

    createMainRenderPass();

    m_renderContext->createFrameBuffers(m_mainRenderPass);
    m_renderContext->createCommandPool();

    // Graphic Interface
    createGraphicInterface(window, viewParams);
    // Scene Managements
    m_renderScene = std::make_unique<RenderScene>();
    // Shader Uniforms
    m_descriptorTable = std::make_unique<DescriptorTable>(*m_renderContext);

    // Mesh, Material, Textures & Shaders
    m_renderScene->initialize(*m_renderContext, *m_descriptorTable);

    // Descriptor 
    m_descriptorTable->createDescriptorPool();
    m_descriptorTable->createDescriptorLayouts();
    m_descriptorTable->createDescriptorBuffers();
    m_descriptorTable->createFrameDescriptors();

    // Pipelines
    m_renderScene->createGraphicPipelines(*m_renderContext, m_mainRenderPass, *m_descriptorTable);

    createCommandBuffers();
    createSyncObjects();
}

void Engine::drawFrame(Camera& camera, ViewParams& viewParams)
{
    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(m_renderContext->device(), m_renderContext->swapChain().vkSwapChain(), UINT64_MAX, m_imageAvailableSemaphores[m_currentFrame], VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreateSwapChain();
        return;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    const RenderFrame& currentFrame = m_renderContext->getRenderFrame(imageIndex);
    const VkFence& fence = currentFrame.synchronizationFence();
    vkWaitForFences(m_renderContext->device(), 1, &fence, VK_TRUE, UINT64_MAX);
    vkResetFences(m_renderContext->device(), 1, &fence);

    // --------------------------------- Update UI ---------------------------------
    
    m_graphicInterface->draw(*m_renderContext);

    // --------------------------------- Submit command ---------------------------------

    updateUniformBuffer(camera, viewParams, imageIndex);
    updateCommandBuffer(imageIndex);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = { m_imageAvailableSemaphores[m_currentFrame] };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &m_commandBuffers[imageIndex];

    VkSemaphore signalSemaphores[] = { m_renderFinishedSemaphores[m_currentFrame] };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(m_renderContext->graphicsQueue(), 1, &submitInfo, currentFrame.synchronizationFence()) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    // --------------------------------- Presentation --------------------------------- 
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    VkSwapchainKHR swapChains[] = { m_renderContext->swapChain().vkSwapChain() };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr; // Optional 

    result = vkQueuePresentKHR(m_renderContext->presentQueue(), &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        recreateSwapChain();
    }
    else if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image!");
    }

    m_currentFrame = (m_currentFrame + 1) % m_renderContext->swapChain().size();
}

void Engine::resize(int width, int height, const SwapChainSupportInfos& swapChainSupport)
{
    m_swapChainSupportInfo = swapChainSupport;
    m_windowWidth = width;
    m_windowHeight = height;
    recreateSwapChain();
}

void Engine::recreateSwapChain()
{
    // wait all ressources not in used
    vkDeviceWaitIdle(m_renderContext->device());
    cleanUpSwapchain();

    auto extent = VkExtent2D({ m_windowWidth, m_windowHeight });
    m_renderContext->createSwapChain(extent, m_swapChainSupportInfo);
    // Renderpass
    createMainRenderPass();
    // Pipeline
    m_renderScene->createGraphicPipelines(*m_renderContext, m_mainRenderPass, *m_descriptorTable);
    // FrameBuffers
    m_renderContext->createFrameBuffers(m_mainRenderPass);
    // Command buffers
    createCommandBuffers();
}

void Engine::cleanUp()
{
    cleanUpSwapchain();

    m_graphicInterface->cleanUp(*m_renderContext);
    m_renderScene->cleanUp(*m_renderContext);
    m_descriptorTable->cleanUp();

    for (size_t i = 0; i < m_imageAvailableSemaphores.size(); i++) {
        vkDestroySemaphore(m_renderContext->device(), m_renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(m_renderContext->device(), m_imageAvailableSemaphores[i], nullptr);
    }
    m_renderContext->cleanUpDevice();
}

void Engine::cleanUpSwapchain()
{
    m_renderContext->cleanUpFrameBuffers();

    vkFreeCommandBuffers(m_renderContext->device(), m_renderContext->commandPool(), static_cast<uint32_t>(m_commandBuffers.size()), m_commandBuffers.data());
    m_renderScene->destroyGraphicPipelines(*m_renderContext);
    vkDestroyRenderPass(m_renderContext->device(), m_mainRenderPass, nullptr);

    m_renderContext->cleanUpSwapChain();
}

RenderContext* Engine::renderContext()
{
    return m_renderContext.get();
}

/* --------------------------------- Private methods --------------------------------- */

void Engine::updateUniformBuffer(Camera& camera, ViewParams& viewParams, uint32_t imageIndex)
{
    auto& frameDescriptors = m_descriptorTable->getFrameDescriptor(imageIndex);
    auto& globalDescritpor = m_descriptorTable->getGlobalDescriptor(imageIndex);
    m_renderScene->updateUniforms(*m_renderContext, camera, viewParams, frameDescriptors, globalDescritpor);
}

void Engine::createMainRenderPass()
{
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = m_renderContext->swapChain().currentImageFormat();
    colorAttachment.samples = m_renderContext->multiSamplingSamples();
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = m_renderContext->depthImageFormat();
    depthAttachment.samples = m_renderContext->multiSamplingSamples();
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription colorAttachmentResolve{};
    colorAttachmentResolve.format = m_renderContext->swapChain().currentImageFormat();
    colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorAttachmentResolveRef{};
    colorAttachmentResolveRef.attachment = 2;
    colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;
    subpass.pResolveAttachments = &colorAttachmentResolveRef;

    /* Start the RenderPass when the piepeline is ready to write to the color attachment*/
    VkSubpassDependency dependency{};
    // Implicit before and after subpass
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    // the current subpass
    dependency.dstSubpass = 0;
    // which state to wait
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    // which state to wait
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    // when the transition start
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    std::array<VkAttachmentDescription, 3> attachments = { colorAttachment, depthAttachment, colorAttachmentResolve };
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());;
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(m_renderContext->device(), &renderPassInfo, nullptr, &m_mainRenderPass) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
    }
}

void Engine::createGraphicInterface(Window* window, ViewParams& viewParams)
{
    m_graphicInterface = std::make_unique<FogMenu>(viewParams);
    m_graphicInterface->initialize(window, *m_renderContext, m_mainRenderPass);
}

void Engine::createCommandBuffers()
{
    uint32_t swapChainImageSize = m_renderContext->swapChain().size();
    m_commandBuffers.resize(swapChainImageSize);
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_renderContext->commandPool();
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t)m_commandBuffers.size();

    if (vkAllocateCommandBuffers(m_renderContext->device(), &allocInfo, m_commandBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }

    for (size_t i = 0; i < m_commandBuffers.size(); i++) {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = 0; // Optional
        beginInfo.pInheritanceInfo = nullptr; // Optional

        if (vkBeginCommandBuffer(m_commandBuffers[i], &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

        // Empty buffers

        if (vkEndCommandBuffer(m_commandBuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }
    }
}

void Engine::updateCommandBuffer(uint32_t imageIndex)
{
    vkResetCommandBuffer(m_commandBuffers[imageIndex], 0);
    //vkResetCommandBuffer(m_commandBuffers[imageIndex], VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0; // Optional
    beginInfo.pInheritanceInfo = nullptr; // Optional

    if (vkBeginCommandBuffer(m_commandBuffers[imageIndex], &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    fillCommandBuffers(imageIndex);

    if (vkEndCommandBuffer(m_commandBuffers[imageIndex]) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }
}

void Engine::fillCommandBuffers(uint32_t imageIndex)
{
    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
    clearValues[1].depthStencil = { 1.0f, 0 };

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_mainRenderPass;
    renderPassInfo.framebuffer = m_renderContext->frameBuffers()[imageIndex];
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = m_renderContext->dimension();
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(m_commandBuffers[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    auto& frameDescriptor = m_descriptorTable->getFrameDescriptor(imageIndex);
    auto& globalDescriptor = m_descriptorTable->getGlobalDescriptor(imageIndex);

    // Fill Scene command buffer
    m_renderScene->fillCommandBuffer(*m_renderContext, m_commandBuffers[imageIndex], frameDescriptor, globalDescriptor.descriptorSet);
    m_graphicInterface->fillCommandBuffer(m_commandBuffers[imageIndex]);

    vkCmdEndRenderPass(m_commandBuffers[imageIndex]);
}

void Engine::createSyncObjects()
{
    m_currentFrame = 0;
    m_imageAvailableSemaphores.resize(m_renderContext->swapChain().size());
    m_renderFinishedSemaphores.resize(m_renderContext->swapChain().size());

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < m_renderContext->swapChain().size(); i++) {
        if (vkCreateSemaphore(m_renderContext->device(), &semaphoreInfo, nullptr, &m_imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(m_renderContext->device(), &semaphoreInfo, nullptr, &m_renderFinishedSemaphores[i]) != VK_SUCCESS ) {

            throw std::runtime_error("failed to create semaphores for a frame!");
        }
    }
}