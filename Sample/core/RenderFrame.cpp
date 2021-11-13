#include "RenderFrame.h"

#include <stdexcept>       

/* --------------------------------- Constructors --------------------------------- */

RenderFrame::RenderFrame(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mip_levels):
    m_device(device),
    m_synchronizationFence(VK_NULL_HANDLE)
{
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = mip_levels;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(device, &viewInfo, nullptr, &m_imageView) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture image view!");
    }

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    if (vkCreateFence(device, &fenceInfo, nullptr, &m_synchronizationFence) != VK_SUCCESS) {
        throw std::runtime_error("failed to create semaphores for a frame!");
    }
}


RenderFrame::~RenderFrame()
{
    vkDestroyImageView(m_device, m_imageView, nullptr);
    vkDestroyFence(m_device, m_synchronizationFence, nullptr);
}

/* --------------------------------- Public Methods --------------------------------- */

VkImageView RenderFrame::getImageView() const
{
    return m_imageView;
}

VkFence RenderFrame::synchronizationFence() const
{
    return m_synchronizationFence;
}