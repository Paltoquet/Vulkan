#include "RenderContext.h"

#include "VkInitializer.h"

#include <set>
#include <array>

#ifdef AHAH
const bool RenderContext::enableValidationLayers = false;
#else
const bool RenderContext::enableValidationLayers = true;
#endif

const std::vector<const char*> RenderContext::requiredExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

const std::vector<const char*> RenderContext::validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

/* --------------------------------- Constructors --------------------------------- */

RenderContext::RenderContext(const VkInstance& vkInstance, const VkSurfaceKHR& surface, const VkPhysicalDevice& device):
    m_vkInstance(vkInstance),
    m_surface(surface),
    m_physicalDevice(device),
    m_swapChain(nullptr),
    m_depthImageFormat(VK_FORMAT_UNDEFINED)
{

}


RenderContext::~RenderContext()
{
}

/* --------------------------------- Public methods --------------------------------- */

void RenderContext::cleanUpDevice()
{
    vkDestroyCommandPool(m_device, m_commandPool, nullptr);
    vkDestroyDevice(m_device, nullptr);
}

void RenderContext::cleanUpFrameBuffers()
{
    int frameCount = static_cast<int>(m_frames.size());
    for (int i = 0; i < frameCount; i++) {
        vkDestroyImageView(m_device, m_offscrenColorAttachments[i].imageView, nullptr);
        vkDestroyImage(m_device, m_offscrenColorAttachments[i].image, nullptr);
        vkFreeMemory(m_device, m_offscrenColorAttachments[i].imageMemory, nullptr);

        vkDestroyImageView(m_device, m_resolveColorAttachments[i].imageView, nullptr);
        vkDestroyImage(m_device, m_resolveColorAttachments[i].image, nullptr);
        vkFreeMemory(m_device, m_resolveColorAttachments[i].imageMemory, nullptr);

        vkDestroyImageView(m_device, m_offscreenDepthAttachments[i].imageView, nullptr);
        vkDestroyImage(m_device, m_offscreenDepthAttachments[i].image, nullptr);
        vkFreeMemory(m_device, m_offscreenDepthAttachments[i].imageMemory, nullptr);

        vkDestroyImageView(m_device, m_blurDepthAttachments[i].imageView, nullptr);
        vkDestroyImage(m_device, m_blurDepthAttachments[i].image, nullptr);
        vkFreeMemory(m_device, m_blurDepthAttachments[i].imageMemory, nullptr);

        vkDestroyFramebuffer(m_device, m_offscreenFrameBuffers[i], nullptr);
        vkDestroyFramebuffer(m_device, m_blurFrameBuffers[i], nullptr);
    }
}

void RenderContext::cleanUpSwapChain()
{
    m_frames.clear();
    m_swapChain.reset();
}

void RenderContext::createLogicalDevice()
{
    // Create a queue for each family
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = { m_graphicQueueIndex, m_presentQueueIndex };
    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.samplerAnisotropy = VK_TRUE;
    deviceFeatures.sampleRateShading = VK_TRUE; // enable sample shading feature for the device
    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
    createInfo.ppEnabledExtensionNames = requiredExtensions.data();

    if (enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    }
    else {
        createInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device) != VK_SUCCESS) {
        throw std::runtime_error("failed to create logical device!");
    }

    vkGetDeviceQueue(m_device, m_graphicQueueIndex, 0, &m_graphicsQueue);
    vkGetDeviceQueue(m_device, m_presentQueueIndex, 0, &m_presentQueue);
}

void RenderContext::createSwapChain(const VkExtent2D& dimension, const SwapChainSupportInfos& availableDetails)
{
    m_swapChain = std::make_unique<SwapChain>(m_device, m_surface, dimension, availableDetails);
    m_swapChain->create();

    auto& swapChainImages = m_swapChain->images();
    auto imageFormat = m_swapChain->currentImageFormat();
    m_frames.clear();
    for (size_t i = 0; i < swapChainImages.size(); i++) {
        std::unique_ptr<RenderFrame> frame = std::make_unique<RenderFrame>(m_device, swapChainImages[i], imageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
        m_frames.push_back(std::move(frame));
    }
}

void RenderContext::createOffScreenFrameBuffer(const VkRenderPass& renderPass)
{
    int frameCount = static_cast<int>(m_frames.size());
    m_offscreenFrameBuffers.resize(frameCount);
    m_offscrenColorAttachments.resize(frameCount);
    m_offscreenDepthAttachments.resize(frameCount);
    m_resolveColorAttachments.resize(frameCount);

    for (int i = 0; i < frameCount; i++) {
        // Color Attachment
        vk_initializer::createImage(m_device, m_physicalDevice, this->width(), this->height(), 1, m_MSAASamples,
            m_swapChain->currentImageFormat(), VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_offscrenColorAttachments[i].image, m_offscrenColorAttachments[i].imageMemory);

        m_offscrenColorAttachments[i].imageView = vk_initializer::createImageView(m_device, m_offscrenColorAttachments[i].image, m_swapChain->currentImageFormat(), VK_IMAGE_ASPECT_COLOR_BIT, 1);

        // Depth Attachment
        vk_initializer::createImage(m_device, m_physicalDevice, this->width(), this->height(), 1, m_MSAASamples,
            m_depthImageFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            m_offscreenDepthAttachments[i].image, m_offscreenDepthAttachments[i].imageMemory);

        m_offscreenDepthAttachments[i].imageView = vk_initializer::createImageView(m_device, m_offscreenDepthAttachments[i].image, m_depthImageFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);

        // Resolve Attachment
        vk_initializer::createImage(m_device, m_physicalDevice, this->width(), this->height(), 1, VK_SAMPLE_COUNT_1_BIT,
            m_swapChain->currentImageFormat(), VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT ,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_resolveColorAttachments[i].image, m_resolveColorAttachments[i].imageMemory);

        m_resolveColorAttachments[i].imageView = vk_initializer::createImageView(m_device, m_resolveColorAttachments[i].image, m_swapChain->currentImageFormat(), VK_IMAGE_ASPECT_COLOR_BIT, 1);

        // FrameBuffers
        std::array<VkImageView, 3> attachments = {
            m_offscrenColorAttachments[i].imageView,
            m_offscreenDepthAttachments[i].imageView,
            m_resolveColorAttachments[i].imageView,
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = this->width();
        framebufferInfo.height = this->height();
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(m_device, &framebufferInfo, nullptr, &m_offscreenFrameBuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }
}

void RenderContext::createBlurFrameBuffer(const VkRenderPass& renderPass)
{
    int frameCount = static_cast<int>(m_frames.size());
    m_blurFrameBuffers.resize(frameCount);
    m_blurDepthAttachments.resize(frameCount);

    for (int i = 0; i < frameCount; i++) {

        // Depth Attachment
        vk_initializer::createImage(m_device, m_physicalDevice, this->width(), this->height(), 1, VK_SAMPLE_COUNT_1_BIT,
            m_depthImageFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            m_blurDepthAttachments[i].image, m_blurDepthAttachments[i].imageMemory);

        m_blurDepthAttachments[i].imageView = vk_initializer::createImageView(m_device, m_blurDepthAttachments[i].image, m_depthImageFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);


        // FrameBuffers
        std::array<VkImageView, 2> attachments = {
            m_frames[i]->getImageView(),
            m_blurDepthAttachments[i].imageView,
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = this->width();
        framebufferInfo.height = this->height();
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(m_device, &framebufferInfo, nullptr, &m_blurFrameBuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }
}

void RenderContext::createCommandPool()
{
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = m_graphicQueueIndex;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    //poolInfo.flags = 0; // Optional

    if (vkCreateCommandPool(m_device, &poolInfo, nullptr, &m_commandPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create command pool!");
    }
}

void RenderContext::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) const
{
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(m_device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(m_device, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = vk_initializer::findMemoryType(m_physicalDevice, memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(m_device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate buffer memory!");
    }

    vkBindBufferMemory(m_device, buffer, bufferMemory, 0);
}

void RenderContext::copyBuffer(VkBuffer sourceBuffer, VkBuffer destinationBuffer, VkDeviceSize size) const
{
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = 0; // Optional
    copyRegion.dstOffset = 0; // Optional
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, sourceBuffer, destinationBuffer, 1, &copyRegion);

    endSingleTimeCommands(commandBuffer);
}

VkCommandBuffer RenderContext::beginSingleTimeCommands() const
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = m_commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(m_device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void RenderContext::endSingleTimeCommands(VkCommandBuffer commandBuffer) const
{
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(m_graphicsQueue);

    vkFreeCommandBuffers(m_device, m_commandPool, 1, &commandBuffer);
}

void RenderContext::pickGraphicQueue()
{
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, queueFamilies.data());

    for (auto index = 0; index < queueFamilies.size(); index++) {
        VkQueueFamilyProperties& queueFamily = queueFamilies[index];
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(m_physicalDevice, index, m_surface, &presentSupport);
            // GraphicFamily and Window presenting should be the same
            if (presentSupport) {
                m_graphicQueueIndex = index;
                m_presentQueueIndex = index;
            }
        }
    }
}

void RenderContext::pickDepthImageFormat()
{
    std::vector<VkFormat> candidates = { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT };
    VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
    VkFormatFeatureFlags features = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;

    for (VkFormat format : candidates) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(m_physicalDevice, format, &props);

        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
            m_depthImageFormat = format;
        } else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
            m_depthImageFormat = format;
        }
    }

    if (m_depthImageFormat == VK_FORMAT_UNDEFINED) {
        throw std::runtime_error("failed to find a depth format!");
    }
}

void RenderContext::pickSampleCount() {
    VkPhysicalDeviceProperties physicalDeviceProperties;
    vkGetPhysicalDeviceProperties(m_physicalDevice, &physicalDeviceProperties);

    VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;
    VkSampleCountFlagBits nbSamples = VK_SAMPLE_COUNT_1_BIT;
    if (counts & VK_SAMPLE_COUNT_64_BIT) { nbSamples = VK_SAMPLE_COUNT_64_BIT; }
    if (counts & VK_SAMPLE_COUNT_32_BIT) { nbSamples = VK_SAMPLE_COUNT_32_BIT; }
    if (counts & VK_SAMPLE_COUNT_16_BIT) { nbSamples = VK_SAMPLE_COUNT_16_BIT; }
    if (counts & VK_SAMPLE_COUNT_8_BIT) { nbSamples = VK_SAMPLE_COUNT_8_BIT; }
    if (counts & VK_SAMPLE_COUNT_4_BIT) { nbSamples = VK_SAMPLE_COUNT_4_BIT; }
    if (counts & VK_SAMPLE_COUNT_2_BIT) { nbSamples = VK_SAMPLE_COUNT_2_BIT; }

    m_MSAASamples = nbSamples;
}

const VkExtent2D& RenderContext::dimension() const
{
    return m_swapChain->dimension();
}

int RenderContext::width() const
{
    return m_swapChain->dimension().width;
}

int RenderContext::height() const
{
    return m_swapChain->dimension().height;
}

const SwapChain& RenderContext::swapChain() const
{
    return *m_swapChain;
}

const std::vector<VkFramebuffer>& RenderContext::offScreenFrameBuffers() const
{
    return m_offscreenFrameBuffers;
}

const std::vector<VkFramebuffer>& RenderContext::blurFrameBuffers() const
{
    return m_blurFrameBuffers;
}

VkFormat RenderContext::depthImageFormat() const
{
    return m_depthImageFormat;
}

VkSampleCountFlagBits RenderContext::multiSamplingSamples() const
{
    return m_MSAASamples;
}

const RenderFrame& RenderContext::getRenderFrame(uint32_t index) const
{
    return *m_frames.at(index);
}

const FrameBufferAttachment& RenderContext::getOffScreenRenderTexture(uint32_t index)
{
    return m_resolveColorAttachments[index];
}

const VkInstance& RenderContext::vkInstance() const
{
    return m_vkInstance;
}

const VkSurfaceKHR& RenderContext::surface() const
{
    return m_surface;
}

const VkPhysicalDevice& RenderContext::physicalDevice() const
{
    return m_physicalDevice;
}

const VkDevice& RenderContext::device() const
{
    return m_device;
}

const VkCommandPool& RenderContext::commandPool() const
{
    return m_commandPool;
}

const VkQueue& RenderContext::graphicsQueue() const
{
    return m_graphicsQueue;
}

const VkQueue& RenderContext::presentQueue() const
{
    return m_presentQueue;
}

uint32_t RenderContext::graphicQueueIndex() const
{
    return m_graphicQueueIndex;
}

uint32_t RenderContext::presentQueueIndex() const
{
    return m_presentQueueIndex;
}