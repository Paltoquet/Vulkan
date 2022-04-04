#include "TextureLoader.h"
#include <core/VkInitializer.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <cstdint>
#include <algorithm>
#include <iostream> 

#include <noise/BrownianNoise.h>
#include <noise/BrownianNoise3D.h>
#include <noise/CloudGenerator.h>
#include <noise/WorleyNoise3D.h>
#include <noise/WorleyNoise2D.h>

#include <glm/gtx/string_cast.hpp>

TextureLoader::TextureLoader(RenderContext* context):
    m_renderContext(context)
{
}


TextureLoader::~TextureLoader()
{
}

/* --------------------------------- Public methods --------------------------------- */

ImageView TextureLoader::loadTexture(const std::string& path, const VkFormat& format, VkImageAspectFlags aspect)
{
    Image imageInfo;
    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    VkDeviceSize imageSize = texWidth * texHeight * 4;
    imageInfo.Vkformat = format;
    imageInfo.mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;
    imageInfo.aspectFlag = aspect;

    if (!pixels) {
        throw std::runtime_error("failed to load texture image!");
    }

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    m_renderContext->createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
    void* data;
    vkMapMemory(m_renderContext->device(), stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, pixels, static_cast<size_t>(imageSize));
    vkUnmapMemory(m_renderContext->device(), stagingBufferMemory);
    stbi_image_free(pixels);

    vk_initializer::createImage(m_renderContext->device(), m_renderContext->physicalDevice(), texWidth, texHeight, imageInfo.mipLevels, VK_SAMPLE_COUNT_1_BIT, imageInfo.Vkformat, VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        imageInfo.Vkimage, imageInfo.Vkmemory);

    VkCommandBuffer transitionCmd = m_renderContext->beginSingleTimeCommands();
    setImageLayout(transitionCmd, imageInfo, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    copyBufferToImage(transitionCmd, stagingBuffer, imageInfo.Vkimage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
    generateMipmaps(transitionCmd, imageInfo, texWidth, texHeight);
    m_renderContext->endSingleTimeCommands(transitionCmd);

    vkDestroyBuffer(m_renderContext->device(), stagingBuffer, nullptr);
    vkFreeMemory(m_renderContext->device(), stagingBufferMemory, nullptr);

    return ImageView(m_renderContext->device(), imageInfo, VK_IMAGE_VIEW_TYPE_2D);
}

ImageView TextureLoader::loadNoiseTexture(const VkExtent2D& dimension, const VkFormat& format, VkImageAspectFlags aspect)
{
    Image imageInfo;
    std::vector<unsigned char> noiseDatas;
    VkDeviceSize imageSize = dimension.width * dimension.height * 4;
    imageInfo.Vkformat = format;
    imageInfo.mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(dimension.width, dimension.height)))) + 1;
    imageInfo.aspectFlag = aspect;

    BrownianNoise noiseGenerator = BrownianNoise(glm::ivec2(64, 64), 4);
    noiseDatas.resize(imageSize);

    glm::vec2 pixelPos;
    float noiseValue;
    unsigned char value;
    size_t index;
    for (size_t j = 0; j < dimension.height; j++) {
        for (size_t i = 0; i < dimension.width; i++) {
            pixelPos = glm::vec2(i, j);
            index = (j * dimension.width + i) * 4;
            noiseValue = noiseGenerator.evaluate(pixelPos) * 255;
            value = static_cast<unsigned char>(noiseValue);
            noiseDatas[index] = value;
            noiseDatas[index + 1] = value;
            noiseDatas[index + 2] = value;
            noiseDatas[index + 3] = 255;
        }
    }

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    m_renderContext->createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
    void* data;
    vkMapMemory(m_renderContext->device(), stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, noiseDatas.data(), static_cast<size_t>(imageSize));
    vkUnmapMemory(m_renderContext->device(), stagingBufferMemory);

    vk_initializer::createImage(m_renderContext->device(), m_renderContext->physicalDevice(), dimension.width, dimension.height, imageInfo.mipLevels, VK_SAMPLE_COUNT_1_BIT, imageInfo.Vkformat, VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        imageInfo.Vkimage, imageInfo.Vkmemory);

    VkCommandBuffer copyCmd = m_renderContext->beginSingleTimeCommands();

    setImageLayout(copyCmd, imageInfo, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    copyBufferToImage(copyCmd, stagingBuffer, imageInfo.Vkimage, static_cast<uint32_t>(dimension.width), static_cast<uint32_t>(dimension.height));
    setImageLayout(copyCmd, imageInfo, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    //generateMipmaps(imageInfo, dimension.width, dimension.height);
    m_renderContext->endSingleTimeCommands(copyCmd);

    vkDestroyBuffer(m_renderContext->device(), stagingBuffer, nullptr);
    vkFreeMemory(m_renderContext->device(), stagingBufferMemory, nullptr);

    return ImageView(m_renderContext->device(), imageInfo, VK_IMAGE_VIEW_TYPE_2D);
}

ImageView TextureLoader::loadWorleyNoiseTexture(const VkExtent2D& dimension, const VkFormat& format, VkImageAspectFlags aspect)
{
    Image imageInfo;
    std::vector<unsigned char> noiseDatas;
    VkDeviceSize imageSize = dimension.width * dimension.height * 4;
    imageInfo.Vkformat = format;
    imageInfo.mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(dimension.width, dimension.height)))) + 1;
    imageInfo.aspectFlag = aspect;

    WorleyNoise2D worleyGenerator = WorleyNoise2D(glm::vec2(8.0f, 8.0f));
    noiseDatas.resize(imageSize);

    float width  = static_cast<float>(dimension.width);
    float height = static_cast<float>(dimension.height);
    glm::vec2 pixelPos;
    float noiseValue;
    unsigned char value;
    size_t index;
    for (size_t j = 0; j < dimension.height; j++) {
        for (size_t i = 0; i < dimension.width; i++) {
            pixelPos = glm::vec2(i / width, j / height);
            index = (j * dimension.width + i) * 4;
            noiseValue = worleyGenerator.evaluate(pixelPos, 6.8f) * 255.0f;
            value = static_cast<unsigned char>(noiseValue);
            noiseDatas[index] = value;
            noiseDatas[index + 1] = value;
            noiseDatas[index + 2] = value;
            noiseDatas[index + 3] = 255;
        }
    }

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    m_renderContext->createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
    void* data;
    vkMapMemory(m_renderContext->device(), stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, noiseDatas.data(), static_cast<size_t>(imageSize));
    vkUnmapMemory(m_renderContext->device(), stagingBufferMemory);

    vk_initializer::createImage(m_renderContext->device(), m_renderContext->physicalDevice(), dimension.width, dimension.height, imageInfo.mipLevels, VK_SAMPLE_COUNT_1_BIT, imageInfo.Vkformat, VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        imageInfo.Vkimage, imageInfo.Vkmemory);

    VkCommandBuffer copyCmd = m_renderContext->beginSingleTimeCommands();

    setImageLayout(copyCmd, imageInfo, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    copyBufferToImage(copyCmd, stagingBuffer, imageInfo.Vkimage, static_cast<uint32_t>(dimension.width), static_cast<uint32_t>(dimension.height));
    //setImageLayout(copyCmd, imageInfo, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    generateMipmaps(copyCmd, imageInfo, dimension.width, dimension.height);
    m_renderContext->endSingleTimeCommands(copyCmd);

    vkDestroyBuffer(m_renderContext->device(), stagingBuffer, nullptr);
    vkFreeMemory(m_renderContext->device(), stagingBufferMemory, nullptr);

    return ImageView(m_renderContext->device(), imageInfo, VK_IMAGE_VIEW_TYPE_2D);
}

ImageView TextureLoader::load3DCloudTexture(const VkExtent3D& dimension, VkImageAspectFlags aspect, float noiseScale)
{
    ImageView result;
    result.imageInfo.Vkformat = VK_FORMAT_R8_UNORM;
    result.imageInfo.Vkmemory = VK_NULL_HANDLE;
    result.imageInfo.mipLevels = 1;
    result.imageInfo.aspectFlag = aspect;
    result.imageInfo.textureSize = dimension;

    // Format support check
    // 3D texture support in Vulkan is mandatory (in contrast to OpenGL) so no need to check if it's supported
    VkFormatProperties formatProperties;
    vkGetPhysicalDeviceFormatProperties(m_renderContext->physicalDevice(), result.imageInfo.Vkformat, &formatProperties);
    // Check if format supports transfer
    if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_DST_BIT))
    {
        std::cout << "Error: Device does not support flag TRANSFER_DST for selected texture format!" << std::endl;
    }

    // Create optimal tiled target image
    VkImageCreateInfo imageCreateInfo;
    imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCreateInfo.imageType = VK_IMAGE_TYPE_3D;
    imageCreateInfo.format = result.imageInfo.Vkformat;
    imageCreateInfo.mipLevels = result.imageInfo.mipLevels;
    imageCreateInfo.arrayLayers = 1;
    imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageCreateInfo.extent.width = dimension.width;
    imageCreateInfo.extent.height = dimension.height;
    imageCreateInfo.extent.depth = dimension.depth;
    // Set initial layout of the image to undefined
    imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageCreateInfo.pNext = nullptr;
    imageCreateInfo.flags = 0;
    vkCreateImage(m_renderContext->device(), &imageCreateInfo, nullptr, &result.imageInfo.Vkimage);

    // Device local memory to back up image
    VkMemoryAllocateInfo memAllocInfo{};
    memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    VkMemoryRequirements memReqs = {};
    vkGetImageMemoryRequirements(m_renderContext->device(), result.imageInfo.Vkimage, &memReqs);
    memAllocInfo.allocationSize = memReqs.size;
    memAllocInfo.memoryTypeIndex = vk_initializer::findMemoryType(m_renderContext->physicalDevice(), memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vkAllocateMemory(m_renderContext->device(), &memAllocInfo, nullptr, &result.imageInfo.Vkmemory);
    vkBindImageMemory(m_renderContext->device(), result.imageInfo.Vkimage, result.imageInfo.Vkmemory, 0);

    /* --------------------------------- Load Buffers --------------------------------- */
    // Compute 3D texture data
    CloudGenerator generator(dimension.width, dimension.height, dimension.depth);
    std::vector<unsigned char> data = generator.compute3DTexture(noiseScale);
    // Load data on the GPU
    updateImageView(result, data);
    // Create image view
    VkImageViewCreateInfo view{};
    view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view.image = result.imageInfo.Vkimage;
    view.viewType = VK_IMAGE_VIEW_TYPE_3D;
    view.format = result.imageInfo.Vkformat;
    view.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
    view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    view.subresourceRange.baseMipLevel = 0;
    view.subresourceRange.baseArrayLayer = 0;
    view.subresourceRange.layerCount = 1;
    view.subresourceRange.levelCount = 1;
    vkCreateImageView(m_renderContext->device(), &view, nullptr, &result.vkImageView);

    return result;
}

void TextureLoader::updateCloudTexture(ImageView& imageView, float noiseScale)
{
    CloudGenerator generator(imageView.imageInfo.textureSize.width, imageView.imageInfo.textureSize.height, imageView.imageInfo.textureSize.depth);
    std::vector<unsigned char> data = generator.compute3DTexture(noiseScale);
    // Load data on the GPU
    updateImageView(imageView, data);
}

void TextureLoader::updateImageView(ImageView& imageView, const std::vector<unsigned char>& data)
{
    // Create a host-visible staging buffer that contains the raw image data
    std::size_t texMemSize = data.size();
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingMemory;
    m_renderContext->createBuffer(texMemSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingMemory);

    // Copy texture data into staging buffer
    uint8_t *mapped;
    vkMapMemory(m_renderContext->device(), stagingMemory, 0, texMemSize, 0, (void **)&mapped);
    memcpy(mapped, data.data(), texMemSize);
    vkUnmapMemory(m_renderContext->device(), stagingMemory);

    VkCommandBuffer copyCmd = m_renderContext->beginSingleTimeCommands();
    // The sub resource range describes the regions of the image we will be transitioned
    VkImageSubresourceRange subresourceRange = {};
    subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subresourceRange.baseMipLevel = 0;
    subresourceRange.levelCount = 1;
    subresourceRange.layerCount = 1;

    // Optimal image will be used as destination for the copy, so we must transfer from our
    // initial undefined image layout to the transfer destination layout
    setImageLayout(copyCmd, imageView.imageInfo, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    // Setup buffer copy regions
    VkBufferImageCopy bufferCopyRegion{};
    bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    bufferCopyRegion.imageSubresource.mipLevel = 0;
    bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
    bufferCopyRegion.imageSubresource.layerCount = 1;
    bufferCopyRegion.imageExtent.width = imageView.imageInfo.textureSize.width;
    bufferCopyRegion.imageExtent.height = imageView.imageInfo.textureSize.height;
    bufferCopyRegion.imageExtent.depth = imageView.imageInfo.textureSize.depth;

    vkCmdCopyBufferToImage(
        copyCmd,
        stagingBuffer,
        imageView.imageInfo.Vkimage,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &bufferCopyRegion);

    // Change texture image layout to shader read after all mip levels have been copied
    setImageLayout(copyCmd, imageView.imageInfo, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    m_renderContext->endSingleTimeCommands(copyCmd);
    // Clean up staging resources
    vkFreeMemory(m_renderContext->device(), stagingMemory, nullptr);
    vkDestroyBuffer(m_renderContext->device(), stagingBuffer, nullptr);
}

/* --------------------------------- Private methods --------------------------------- */

void TextureLoader::generateMipmaps(VkCommandBuffer commandBuffer, Image& image, int32_t texWidth, int32_t texHeight) {

    // Check if image format supports linear blitting
    VkFormatProperties formatProperties;
    vkGetPhysicalDeviceFormatProperties(m_renderContext->physicalDevice(), image.Vkformat, &formatProperties);

    if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
        throw std::runtime_error("texture image format does not support linear blitting!");
    }

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.image = image.Vkimage;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.subresourceRange.levelCount = 1;

    int32_t mipWidth = texWidth;
    int32_t mipHeight = texHeight;

    for (uint32_t i = 1; i < image.mipLevels; i++) {
        barrier.subresourceRange.baseMipLevel = i - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

        vkCmdPipelineBarrier(commandBuffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
            0, nullptr,
            0, nullptr,
            1, &barrier);

        VkImageBlit blit{};
        blit.srcOffsets[0] = { 0, 0, 0 };
        blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
        blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.srcSubresource.mipLevel = i - 1;
        blit.srcSubresource.baseArrayLayer = 0;
        blit.srcSubresource.layerCount = 1;
        blit.dstOffsets[0] = { 0, 0, 0 };
        blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
        blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.dstSubresource.mipLevel = i;
        blit.dstSubresource.baseArrayLayer = 0;
        blit.dstSubresource.layerCount = 1;

        vkCmdBlitImage(commandBuffer,
            image.Vkimage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            image.Vkimage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1, &blit,
            VK_FILTER_LINEAR);

        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(commandBuffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
            0, nullptr,
            0, nullptr,
            1, &barrier);

        if (mipWidth > 1) mipWidth /= 2;
        if (mipHeight > 1) mipHeight /= 2;
    }

    barrier.subresourceRange.baseMipLevel = image.mipLevels - 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(commandBuffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
        0, nullptr,
        0, nullptr,
        1, &barrier);
}

void TextureLoader::setImageLayout(VkCommandBuffer commandBuffer, Image& image, VkImageLayout oldImageLayout, VkImageLayout newImageLayout,
    VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask)
{
    // The sub resource range describes the regions of the image we will be transitioned
    VkImageSubresourceRange subresourceRange = {};
    subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subresourceRange.baseMipLevel = 0;
    subresourceRange.levelCount = image.mipLevels;
    subresourceRange.layerCount = 1;

    // Create an image barrier object
    VkImageMemoryBarrier imageMemoryBarrier{};
    imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageMemoryBarrier.oldLayout = oldImageLayout;
    imageMemoryBarrier.newLayout = newImageLayout;
    imageMemoryBarrier.image = image.Vkimage;
    imageMemoryBarrier.subresourceRange = subresourceRange;

    // Source layouts (old)
    // Source access mask controls actions that have to be finished on the old layout
    // before it will be transitioned to the new layout
    switch (oldImageLayout)
    {
    case VK_IMAGE_LAYOUT_UNDEFINED:
        // Image layout is undefined (or does not matter)
        // Only valid as initial layout
        // No flags required, listed only for completeness
        imageMemoryBarrier.srcAccessMask = 0;
        break;

    case VK_IMAGE_LAYOUT_PREINITIALIZED:
        // Image is preinitialized
        // Only valid as initial layout for linear images, preserves memory contents
        // Make sure host writes have been finished
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
        // Image is a color attachment
        // Make sure any writes to the color buffer have been finished
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
        // Image is a depth/stencil attachment
        // Make sure any writes to the depth/stencil buffer have been finished
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
        // Image is a transfer source
        // Make sure any reads from the image have been finished
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        break;

    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
        // Image is a transfer destination
        // Make sure any writes to the image have been finished
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
        // Image is read by a shader
        // Make sure any shader reads from the image have been finished
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
        break;
    default:
        // Other source layouts aren't handled (yet)
        break;
    }

    // Target layouts (new)
    // Destination access mask controls the dependency for the new image layout
    switch (newImageLayout)
    {
    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
        // Image will be used as a transfer destination
        // Make sure any writes to the image have been finished
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
        // Image will be used as a transfer source
        // Make sure any reads from the image have been finished
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        break;

    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
        // Image will be used as a color attachment
        // Make sure any writes to the color buffer have been finished
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
        // Image layout will be used as a depth/stencil attachment
        // Make sure any writes to depth/stencil buffer have been finished
        imageMemoryBarrier.dstAccessMask = imageMemoryBarrier.dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
        // Image will be read in a shader (sampler, input attachment)
        // Make sure any writes to the image have been finished
        if (imageMemoryBarrier.srcAccessMask == 0)
        {
            imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
        }
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        break;
    default:
        // Other source layouts aren't handled (yet)
        break;
    }

    // Put barrier inside setup command buffer
    vkCmdPipelineBarrier(
        commandBuffer,
        srcStageMask,
        dstStageMask,
        0,
        0, nullptr,
        0, nullptr,
        1, &imageMemoryBarrier);
}

void TextureLoader::copyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t depth) 
{
    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = { 0, 0, 0 };
    region.imageExtent = {
        width,
        height,
        depth
    };

    vkCmdCopyBufferToImage(
        commandBuffer,
        buffer,
        image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region
    );
}

bool TextureLoader::hasStencilComponent(VkFormat format) {
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}