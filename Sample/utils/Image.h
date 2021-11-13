#pragma once

#include <vulkan/vulkan.h>

class Image
{
public:
    Image();
    ~Image();

public:
    void cleanUp(VkDevice device);

public:
    VkImage Vkimage;
    VkDeviceMemory Vkmemory;
    VkImageAspectFlags aspectFlag;
    VkFormat Vkformat;
    uint32_t mipLevels;
};

