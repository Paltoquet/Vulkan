#pragma once

#include <vulkan/vulkan.h>

class ImageView
{
public:
    ImageView() = default;
    ImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mip_levels);
    ~ImageView();

public:
    const VkImageView& view() const;

private:
    VkImageView m_imageView;
};

