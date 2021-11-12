#pragma once

#include "RenderContext.h"
#include "ImageView.h"


class TextureLoader
{
public:
    TextureLoader(RenderContext* context);
    ~TextureLoader();

public:
    ImageView loadTexture(const std::string& path, const VkFormat& format, VkImageAspectFlags aspect);

private:
    void generateMipmaps(const Image& image, int32_t texWidth, int32_t texHeight);
    void transitionImageLayout(const Image& image, VkImageLayout oldLayout, VkImageLayout newLayout);
    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
    bool hasStencilComponent(VkFormat format);

private:
    RenderContext* m_renderContext;
};

