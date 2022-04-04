#pragma once

#include <core/RenderContext.h>
#include <utils/ImageView.h>

class TextureLoader
{
public:
    TextureLoader(RenderContext* context);
    ~TextureLoader();

public:
    ImageView loadTexture(const std::string& path, const VkFormat& format, VkImageAspectFlags aspect);
    ImageView loadNoiseTexture(const VkExtent2D& dimension, const VkFormat& format, VkImageAspectFlags aspect);
    ImageView loadWorleyNoiseTexture(const VkExtent2D& dimension, const VkFormat& format, VkImageAspectFlags aspect);
    ImageView load3DCloudTexture(const VkExtent3D& dimension, VkImageAspectFlags aspect, float noiseScale);

    void updateCloudTexture(ImageView& imageView, float noiseScale);
    void updateImageView(ImageView& imageView, const std::vector<unsigned char>& data);

private:
    void generateMipmaps(VkCommandBuffer commandBuffer, Image& image, int32_t texWidth, int32_t texHeight);
    void setImageLayout(VkCommandBuffer commandBuffer, Image& image, VkImageLayout oldImageLayout, VkImageLayout newImageLayout,
        VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
        VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
        // Create an image barrier object
    void copyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t depth = 1);
    bool hasStencilComponent(VkFormat format);

private:
    RenderContext* m_renderContext;
};

