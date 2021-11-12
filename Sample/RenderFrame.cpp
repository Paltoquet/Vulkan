#include "RenderFrame.h"

#include <stdexcept>       

/* --------------------------------- Constructors --------------------------------- */

RenderFrame::RenderFrame(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mip_levels):
    m_device(device),
    m_imageView(device, image, format, aspectFlags, mip_levels),
    m_synchronizationFence(VK_NULL_HANDLE)
{
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
    vkDestroyFence(m_device, m_synchronizationFence, nullptr);
}

/* --------------------------------- Public Methods --------------------------------- */

const VkImageView& RenderFrame::getImageView() const
{
    return m_imageView.view();
}

VkFence RenderFrame::synchronizationFence() const
{
    return m_synchronizationFence;
}