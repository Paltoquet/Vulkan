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
    ImageView loadNoiseTexture3D(const VkExtent3D& dimension, VkImageAspectFlags aspect);
    /*ImageView loadCubeMap(const std::string& front, const std::string& back, const std::string& up,
                          const std::string& down, const std::string& left, const std::string& right,
                          const VkFormat& format, VkImageAspectFlags aspect)*/

private:
    void generateMipmaps(const Image& image, int32_t texWidth, int32_t texHeight);
    void transitionImageLayout(const Image& image, VkImageLayout oldLayout, VkImageLayout newLayout);
    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t depth = 1);
    bool hasStencilComponent(VkFormat format);

private:
    RenderContext* m_renderContext;
};

