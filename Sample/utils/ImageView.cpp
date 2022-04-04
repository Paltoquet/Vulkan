#include "ImageView.h"

#include <stdexcept>

/* --------------------------------- Constructors  --------------------------------- */

ImageView::ImageView(VkDevice device, const Image& image, VkImageViewType viewType):
    imageInfo(image)
{
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image.Vkimage;
    viewInfo.viewType = viewType;
    viewInfo.format = image.Vkformat;
    viewInfo.subresourceRange.aspectMask = image.aspectFlag;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = image.mipLevels;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(device, &viewInfo, nullptr, &vkImageView) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture image view!");
    }
}

ImageView::~ImageView()
{
}

/* --------------------------------- Public Methods  --------------------------------- */

void ImageView::cleanUp(VkDevice device)
{
    vkDestroyImageView(device, vkImageView, nullptr);
    imageInfo.cleanUp(device);
}

const VkImageView& ImageView::view() const
{
    return vkImageView;
}

VkFormat ImageView::Vkformat() const
{
    return imageInfo.Vkformat;
}

uint32_t ImageView::mipLevels() const
{
    return imageInfo.mipLevels;
}
