#include "RenderFrame.h"



RenderFrame::RenderFrame(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mip_levels):
    m_imageView(device, image, format, aspectFlags, mip_levels)
{

}


RenderFrame::~RenderFrame()
{
}


const VkImageView& RenderFrame::getImageView() const
{
    return m_imageView.view();
}
