#pragma once

#include <core/RenderContext.h>
#include <utils/ImageView.h>

class NoiseTextureLoader
{
public:
    NoiseTextureLoader(RenderContext* context);
    ~NoiseTextureLoader();

public:
    ImageView loadTexture(VkImageAspectFlags aspect);

private:
    void setImageLayout(VkCommandBuffer cmdbuffer,
        VkImage image,
        VkImageLayout oldImageLayout,
        VkImageLayout newImageLayout,
        VkImageSubresourceRange subresourceRange,
        VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
        VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

    void updateNoiseTexture(Image& imageInfo, ImageView& view);

private:
    RenderContext* m_renderContext;
};

