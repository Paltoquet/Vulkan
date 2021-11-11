#pragma once

#include <vulkan/vulkan.h>
#include <vector>

struct SwapChainSupportInfos {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

struct SwapchainProperties
{
    uint32_t                      imageCount{ 3 };
    VkExtent2D                    extent{};
    VkSurfaceFormatKHR            surfaceFormat{};
    uint32_t                      arrayLayers;
    VkImageUsageFlags             imageUsage;
    VkSurfaceTransformFlagBitsKHR preTransform;
    VkCompositeAlphaFlagBitsKHR   compositeAlpha;
    VkPresentModeKHR              presentMode;
};


class SwapChain
{
public:
    SwapChain(const VkDevice& device,  const VkSurfaceKHR& surface, const VkExtent2D& dimension, const SwapChainSupportInfos& availableDetails);
    ~SwapChain();

public:
    void create();
    void cleanUp();

public:
    static const VkFormat imageFormat;
    static const VkColorSpaceKHR colorSpace ;
    static const VkPresentModeKHR presentMode;

public:
    const std::vector<VkImage>& images() const;
    uint32_t size() const;
    VkFormat currentImageFormat() const;
    const VkExtent2D& dimension() const;
    const VkSwapchainKHR& vkSwapChain() const;

private:
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

private:
    const VkDevice& m_device;
    const VkSurfaceKHR& m_surface;
    VkExtent2D m_dimension;
    SwapchainProperties m_properties;
    VkSwapchainKHR m_swapChain;

    std::vector<VkImage> m_swapChainImages;
};

