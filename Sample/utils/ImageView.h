#pragma once

#include "Image.h"

#include <vulkan/vulkan.h>

class ImageView
{
public:
    ImageView() = default;
    ImageView(VkDevice device, const Image& image);
    ~ImageView();

public:
    void cleanUp(VkDevice device);

    const VkImageView& view() const;
    VkFormat Vkformat() const;
    uint32_t mipLevels() const;

private:
    VkImageView m_vkImageView;
    Image m_imageInfo;
};

