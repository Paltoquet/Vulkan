#include "TextureLoader.h"
#include <core/VkInitializer.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <cstdint>
#include <algorithm>
#include <iostream> 

#include <noise/BrownianNoise.h>
#include <noise/BrownianNoise3D.h>

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

    transitionImageLayout(imageInfo, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    copyBufferToImage(stagingBuffer, imageInfo.Vkimage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
    //transitioned to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL while generating mipmaps
    //transitionImageLayout(m_textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, m_mipLevels);

    vkDestroyBuffer(m_renderContext->device(), stagingBuffer, nullptr);
    vkFreeMemory(m_renderContext->device(), stagingBufferMemory, nullptr);

    generateMipmaps(imageInfo, texWidth, texHeight);

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

    BrownianNoise noiseGenerator = BrownianNoise(glm::ivec2(64, 64), 3);
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

    transitionImageLayout(imageInfo, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    copyBufferToImage(stagingBuffer, imageInfo.Vkimage, static_cast<uint32_t>(dimension.width), static_cast<uint32_t>(dimension.height));
    //transitioned to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL while generating mipmaps
    transitionImageLayout(imageInfo, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);


    vkDestroyBuffer(m_renderContext->device(), stagingBuffer, nullptr);
    vkFreeMemory(m_renderContext->device(), stagingBufferMemory, nullptr);

    //generateMipmaps(imageInfo, dimension.width, dimension.height);

    return ImageView(m_renderContext->device(), imageInfo, VK_IMAGE_VIEW_TYPE_2D);
}

ImageView TextureLoader::loadNoiseTexture3D(const VkExtent3D& dimension, VkImageAspectFlags aspect)
{
    Image img;
    img.mipLevels = 1;
    img.Vkformat = VK_FORMAT_R8_UNORM;
    img.aspectFlag = aspect;

    /* ----------------------- Loading datas ----------------------- */
    const uint32_t texMemSize = dimension.width * dimension.height * dimension.depth;
    uint8_t *data = new uint8_t[texMemSize];
    memset(data, 0, texMemSize);

    BrownianNoise3D noiseGenerator = BrownianNoise3D(glm::ivec3(64, 64, 64), 3);
    uint8_t *noiseDatas = new uint8_t[texMemSize];
    size_t pageSize = dimension.width * dimension.height;

    glm::vec3 pixelPos;
    float noiseValue;
    unsigned char value;
    size_t index;
    for (size_t k = 0; k < dimension.depth; k++) {
        for (size_t j = 0; j < dimension.height; j++) {
            for (size_t i = 0; i < dimension.width; i++) {
                pixelPos = glm::vec3(i, j, k);
                index = k * pageSize + j * dimension.width + i;
                //noiseValue = noiseGenerator.evaluate(pixelPos) * 255.0f;
                noiseValue = 1.0; // k / float(dimension.depth);
                noiseValue = 64;
                value = static_cast<uint8_t>(noiseValue);
                noiseDatas[index] = value;
            }
        }
        //std::cout << "noise val: " << noiseDatas[index] << std::endl;
    }

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    m_renderContext->createBuffer(texMemSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
    uint8_t* bufferData;
    vkMapMemory(m_renderContext->device(), stagingBufferMemory, 0, texMemSize, 0, (void **)&bufferData);
    memcpy(data, noiseDatas, texMemSize);
    vkUnmapMemory(m_renderContext->device(), stagingBufferMemory);

    //
    //vk_initializer::createImage(m_renderContext->device(), m_renderContext->physicalDevice(), dimension.width, dimension.height, 
    //imageInfo.mipLevels, VK_SAMPLE_COUNT_1_BIT, imageInfo.Vkformat, VK_IMAGE_TILING_OPTIMAL,
    //VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
    //    imageInfo.Vkimage, imageInfo.Vkmemory);

    //ImageCreateInfo imageInfo{};
    //ageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    //ageInfo.imageType = VK_IMAGE_TYPE_2D;
    //ageInfo.extent.width = width;
    //ageInfo.extent.height = height;
    //ageInfo.extent.depth = 1;
    //ageInfo.mipLevels = mip_levels;
    //ageInfo.arrayLayers = 1;
    //ageInfo.format = format;
    //ageInfo.tiling = tiling;
    //ageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    //ageInfo.usage = usage;
    //ageInfo.samples = nbSamples;
    //ageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    //
    // (vkCreateImage(device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
    //  throw std::runtime_error("failed to create image!");
    //
    //
    //MemoryRequirements memRequirements;
    //GetImageMemoryRequirements(device, image, &memRequirements);
    //
    //MemoryAllocateInfo allocInfo{};
    //locInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    //locInfo.allocationSize = memRequirements.size;
    //locInfo.memoryTypeIndex = findMemoryType(physicalDevice, memRequirements.memoryTypeBits, properties);
    //
    // (vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
    //  throw std::runtime_error("failed to allocate image memory!");
    //
    //
    //BindImageMemory(device, image, imageMemory, 0);


    // ----------------------- ImageInfo -----------------------
    VkImageCreateInfo imageCreateInfo{};
    imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCreateInfo.imageType = VK_IMAGE_TYPE_3D;
    imageCreateInfo.extent.width = dimension.width;
    imageCreateInfo.extent.height = dimension.height;
    imageCreateInfo.extent.depth = dimension.depth;
    imageCreateInfo.mipLevels = img.mipLevels;
    imageCreateInfo.arrayLayers = 1;
    imageCreateInfo.format = img.Vkformat;
    imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateImage(m_renderContext->device(), &imageCreateInfo, nullptr, &img.Vkimage) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image!");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(m_renderContext->device(), img.Vkimage, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = vk_initializer::findMemoryType(m_renderContext->physicalDevice(), memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vkAllocateMemory(m_renderContext->device(), &allocInfo, nullptr, &img.Vkmemory);
    vkBindImageMemory(m_renderContext->device(), img.Vkimage, img.Vkmemory, 0);

    transitionImageLayout(img, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    copyBufferToImage(stagingBuffer, img.Vkimage, static_cast<uint32_t>(dimension.width), static_cast<uint32_t>(dimension.height), static_cast<uint32_t>(dimension.depth));
    transitionImageLayout(img, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    // ----------------------- ImageView -----------------------
    /*VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = img.Vkimage;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_3D;
    viewInfo.format = img.Vkformat;
    viewInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;
    viewInfo.subresourceRange.levelCount = 1;*/

    /*VkImageView view;
    if (vkCreateImageView(m_renderContext->device(), &viewInfo, nullptr, &view) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture image view!");
    }*/

    vkDestroyBuffer(m_renderContext->device(), stagingBuffer, nullptr);
    vkFreeMemory(m_renderContext->device(), stagingBufferMemory, nullptr);

    return ImageView(m_renderContext->device(), img, VK_IMAGE_VIEW_TYPE_3D);
}

/* --------------------------------- Private methods --------------------------------- */

void TextureLoader::generateMipmaps(const Image& image, int32_t texWidth, int32_t texHeight) {

    // Check if image format supports linear blitting
    VkFormatProperties formatProperties;
    vkGetPhysicalDeviceFormatProperties(m_renderContext->physicalDevice(), image.Vkformat, &formatProperties);

    if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
        throw std::runtime_error("texture image format does not support linear blitting!");
    }

    VkCommandBuffer commandBuffer = m_renderContext->beginSingleTimeCommands();

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

    m_renderContext->endSingleTimeCommands(commandBuffer);
}

void TextureLoader::transitionImageLayout(const Image& image, VkImageLayout oldLayout, VkImageLayout newLayout) {
    VkCommandBuffer commandBuffer = m_renderContext->beginSingleTimeCommands();
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image.Vkimage;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = image.mipLevels;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    /* Color, Depth and Stencil*/
    if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

        if (hasStencilComponent(image.Vkformat)) {
            barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
    }
    else {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    }
    else {
        throw std::invalid_argument("unsupported layout transition!");
    }


    vkCmdPipelineBarrier(
        commandBuffer,
        sourceStage, destinationStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );
    m_renderContext->endSingleTimeCommands(commandBuffer);
}


void TextureLoader::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t depth) {
    VkCommandBuffer commandBuffer = m_renderContext->beginSingleTimeCommands();

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

    m_renderContext->endSingleTimeCommands(commandBuffer);
}

bool TextureLoader::hasStencilComponent(VkFormat format) {
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}