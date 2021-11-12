#pragma once

#include "ImageView.h"

class RenderFrame
{
public:
    RenderFrame(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mip_levels);
    ~RenderFrame();

public:
    const VkImageView& getImageView() const;
    VkFence synchronizationFence() const;

private:
    VkDevice m_device;
    ImageView m_imageView;
    VkFence m_synchronizationFence;
};

