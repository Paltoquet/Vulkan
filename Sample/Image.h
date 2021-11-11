#pragma once

#include <vulkan/vulkan.h>

class Image
{
public:
    Image();
    ~Image();

public:
    VkImage Vkimage;
    VkDeviceMemory Vkmemory;
    VkFormat Vkformat;
    uint32_t mipLevels;
};

