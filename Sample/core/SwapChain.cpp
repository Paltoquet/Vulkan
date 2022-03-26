#include "SwapChain.h"

#include <cstdint>
#include <algorithm>

#include <iostream>

const VkFormat SwapChain::imageFormat = VK_FORMAT_B8G8R8A8_SRGB;
const VkColorSpaceKHR SwapChain::colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

/* --------------------------------- Constructors --------------------------------- */

SwapChain::SwapChain(const VkDevice& device, const VkSurfaceKHR& surface, const VkExtent2D& dimension, const SwapChainSupportInfos& availableDetails):
    m_device(device),
    m_surface(surface),
    m_dimension(dimension)
{
    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(availableDetails.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(availableDetails.presentModes);
    m_dimension = chooseSwapExtent(availableDetails.capabilities);

    uint32_t imageCount = availableDetails.capabilities.minImageCount + 1;
    if (availableDetails.capabilities.maxImageCount > 0 && imageCount > availableDetails.capabilities.maxImageCount) {
        imageCount = availableDetails.capabilities.maxImageCount;
    }

    m_properties.extent = m_dimension;
    m_properties.imageCount = imageCount;
    m_properties.surfaceFormat = surfaceFormat;
    // could be more than one for stereoscopic application
    m_properties.arrayLayers = 1;
    m_properties.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    // specifies the transformation applied to the image ex: rotation on a certain axis
    m_properties.preTransform = availableDetails.capabilities.currentTransform;
    // specifies if the alpha channel should be used for blending with other windows
    m_properties.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    m_properties.presentMode = presentMode;
}

SwapChain::~SwapChain()
{
    vkDestroySwapchainKHR(m_device, m_swapChain, nullptr);
}

/* --------------------------------- Public Methods  --------------------------------- */

void SwapChain::create()
{
    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface          = m_surface;
    createInfo.minImageCount    = m_properties.imageCount;
    createInfo.imageFormat      = m_properties.surfaceFormat.format;
    createInfo.imageColorSpace  = m_properties.surfaceFormat.colorSpace;
    createInfo.imageExtent      = m_properties.extent;
    createInfo.imageArrayLayers = m_properties.arrayLayers;
    createInfo.imageUsage       = m_properties.imageUsage;

    // Images can be used across multiple queue families without explicit ownership transfers.
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.queueFamilyIndexCount = 0; // Optional
    createInfo.pQueueFamilyIndices = nullptr; // Optional

    createInfo.preTransform = m_properties.preTransform;
    createInfo.compositeAlpha = m_properties.compositeAlpha;
    createInfo.presentMode = m_properties.presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(m_device, &createInfo, nullptr, &m_swapChain) != VK_SUCCESS) {
        throw std::runtime_error("failed to create swap chain!");
    }

    vkGetSwapchainImagesKHR(m_device, m_swapChain, &m_properties.imageCount, nullptr);
    m_swapChainImages.resize(m_properties.imageCount);
    vkGetSwapchainImagesKHR(m_device, m_swapChain, &m_properties.imageCount, m_swapChainImages.data());
}

const std::vector<VkImage>& SwapChain::images() const
{
    return m_swapChainImages;
}

uint32_t SwapChain::size() const
{
    return static_cast<uint32_t>(m_swapChainImages.size());
}

const VkExtent2D& SwapChain::dimension() const
{
    return m_dimension;
}

const VkSwapchainKHR& SwapChain::vkSwapChain() const
{
    return m_swapChain;
}

const VkSurfaceFormatKHR& SwapChain::vkSurfaceFormat() const
{
    return m_properties.surfaceFormat;
}

VkPresentModeKHR SwapChain::vkPresentMode() const
{
    return m_properties.presentMode;
}

VkFormat SwapChain::currentImageFormat() const
{
    return m_properties.surfaceFormat.format;
}

/* --------------------------------- Private Methods --------------------------------- */

VkSurfaceFormatKHR SwapChain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == imageFormat && availableFormat.colorSpace == colorSpace) {
            return availableFormat;
        }
    }
    return availableFormats[0];
}

VkPresentModeKHR SwapChain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentMode;
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D SwapChain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
    if (capabilities.currentExtent.width != UINT32_MAX) {
        return capabilities.currentExtent;
    }
    else {
    
        VkExtent2D actualExtent = {
            static_cast<uint32_t>(m_dimension.width),
            static_cast<uint32_t>(m_dimension.height)
        };

        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }
}