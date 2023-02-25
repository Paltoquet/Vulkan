#pragma once

#include "SwapChain.h"
#include "RenderFrame.h"

#include <vulkan/vulkan.h>
#include <vector>
#include <memory>

struct FrameBufferAttachment {
    VkImage image;
    VkDeviceMemory imageMemory;
    VkImageView imageView;
};

class RenderContext
{
public:
    RenderContext(const VkInstance& vkInstance, const VkSurfaceKHR& surface, const VkPhysicalDevice& device);
    ~RenderContext();

public:
    void createLogicalDevice();
    void createSwapChain(const VkExtent2D& dimension, const SwapChainSupportInfos& availableDetails);
    void createOffScreenFrameBuffer(const VkRenderPass& renderPass);
    void createBlurFrameBuffer(const VkRenderPass& renderPass);
    void createCommandPool();
    void pickGraphicQueue();
    void pickDepthImageFormat();
    void pickSampleCount();

    void cleanUpDevice();
    void cleanUpFrameBuffers();
    void cleanUpSwapChain();

    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) const;
    void copyBuffer(VkBuffer sourceBuffer, VkBuffer destinationBuffer, VkDeviceSize bufferSize) const;
    VkCommandBuffer beginSingleTimeCommands() const;
    void endSingleTimeCommands(VkCommandBuffer commandBuffer) const;

    const SwapChain& swapChain() const;
    const std::vector<VkFramebuffer>& offScreenFrameBuffers() const;
    const std::vector<VkFramebuffer>& blurFrameBuffers() const;
    VkFormat depthImageFormat() const;
    VkSampleCountFlagBits multiSamplingSamples() const;
    const RenderFrame& getRenderFrame(uint32_t index) const;
    const FrameBufferAttachment& getOffScreenRenderTexture(uint32_t index);

    const VkInstance& vkInstance() const;
    const VkSurfaceKHR& surface() const;
    const VkPhysicalDevice& physicalDevice() const;
    const VkDevice& device() const;
    const VkCommandPool& commandPool() const;
    const VkQueue& graphicsQueue() const;
    const VkQueue& presentQueue() const;
    uint32_t graphicQueueIndex() const;
    uint32_t presentQueueIndex() const;

    const VkExtent2D& dimension() const;
    int width() const;
    int height() const;

public:
    static const bool enableValidationLayers;
    static const std::vector<const char*> requiredExtensions;
    static const std::vector<const char*> validationLayers;

private:
    const VkInstance& m_vkInstance;
    const VkSurfaceKHR& m_surface;
    const VkPhysicalDevice& m_physicalDevice;
    VkDevice m_device;
    VkCommandPool m_commandPool;
    std::unique_ptr<SwapChain> m_swapChain;
    std::vector<std::unique_ptr<RenderFrame>> m_frames;
    std::vector<VkFramebuffer> m_offscreenFrameBuffers;
    std::vector<VkFramebuffer> m_blurFrameBuffers;

    std::vector<FrameBufferAttachment> m_offscrenColorAttachments;
    std::vector<FrameBufferAttachment> m_resolveColorAttachments;
    std::vector<FrameBufferAttachment> m_offscreenDepthAttachments;
    std::vector<FrameBufferAttachment> m_blurDepthAttachments;

    VkQueue m_graphicsQueue;
    VkQueue m_presentQueue;
    uint32_t m_graphicQueueIndex;
    uint32_t m_presentQueueIndex;
    VkFormat m_depthImageFormat;
    VkSampleCountFlagBits m_MSAASamples;
};

