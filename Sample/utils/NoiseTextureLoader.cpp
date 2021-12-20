#include "NoiseTextureLoader.h"
#include <core/VkInitializer.h>

#include <cstdint>
#include <algorithm>
#include <iostream> 

#include <noise/BrownianNoise.h>
#include <noise/BrownianNoise3D.h>

NoiseTextureLoader::NoiseTextureLoader(RenderContext* context):
    m_renderContext(context)
{
}


NoiseTextureLoader::~NoiseTextureLoader()
{
}

/* --------------------------------- Public methods --------------------------------- */

ImageView NoiseTextureLoader::loadTexture(VkImageAspectFlags aspect)
{
    Image imageInfo;
    imageInfo.Vkformat = VK_FORMAT_R8_UNORM;
    imageInfo.Vkmemory = VK_NULL_HANDLE;
    imageInfo.mipLevels = 1;
    imageInfo.aspectFlag = aspect;
    imageInfo.textureSize = VkExtent3D({ 128, 128, 128 });

    ImageView result;

    // Format support check
    // 3D texture support in Vulkan is mandatory (in contrast to OpenGL) so no need to check if it's supported
    VkFormatProperties formatProperties;
    vkGetPhysicalDeviceFormatProperties(m_renderContext->physicalDevice(), imageInfo.Vkformat, &formatProperties);
    // Check if format supports transfer
    if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_DST_BIT))
    {
        std::cout << "Error: Device does not support flag TRANSFER_DST for selected texture format!" << std::endl;
        //return;
    }

    // Create optimal tiled target image
    VkImageCreateInfo imageCreateInfo;
    imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCreateInfo.imageType = VK_IMAGE_TYPE_3D;
    imageCreateInfo.format = imageInfo.Vkformat;
    imageCreateInfo.mipLevels = imageInfo.mipLevels;
    imageCreateInfo.arrayLayers = 1;
    imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageCreateInfo.extent.width = imageInfo.textureSize.width;
    imageCreateInfo.extent.height = imageInfo.textureSize.height;
    imageCreateInfo.extent.depth = imageInfo.textureSize.depth;
    // Set initial layout of the image to undefined
    imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageCreateInfo.pNext = nullptr;
    imageCreateInfo.flags = 0;
    vkCreateImage(m_renderContext->device(), &imageCreateInfo, nullptr, &imageInfo.Vkimage);

    // Device local memory to back up image
    VkMemoryAllocateInfo memAllocInfo{};
    memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    VkMemoryRequirements memReqs = {};
    VkBool32 memoryFound;
    vkGetImageMemoryRequirements(m_renderContext->device(), imageInfo.Vkimage, &memReqs);
    memAllocInfo.allocationSize = memReqs.size;
    memAllocInfo.memoryTypeIndex = vk_initializer::findMemoryType(m_renderContext->physicalDevice(), memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vkAllocateMemory(m_renderContext->device(), &memAllocInfo, nullptr, &imageInfo.Vkmemory);
    vkBindImageMemory(m_renderContext->device(), imageInfo.Vkimage, imageInfo.Vkmemory, 0);

    // Create image view
    VkImageViewCreateInfo view{};
    view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view.image = imageInfo.Vkimage;
    view.viewType = VK_IMAGE_VIEW_TYPE_3D;
    view.format = imageInfo.Vkformat;
    view.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
    view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    view.subresourceRange.baseMipLevel = 0;
    view.subresourceRange.baseArrayLayer = 0;
    view.subresourceRange.layerCount = 1;
    view.subresourceRange.levelCount = 1;
    vkCreateImageView(m_renderContext->device(), &view, nullptr, &result.m_vkImageView);

    result.m_imageInfo = imageInfo;
    updateNoiseTexture(imageInfo, result);

    return result;
}

void NoiseTextureLoader::updateNoiseTexture(Image& imageInfo, ImageView& view)
{
    const uint32_t texMemSize = imageInfo.textureSize.width * imageInfo.textureSize.height * imageInfo.textureSize.depth;
    BrownianNoise3D noiseGenerator = BrownianNoise3D(glm::ivec3(64, 64, 64), 3);

    uint8_t *data = new uint8_t[texMemSize];
    memset(data, 0, texMemSize);

    glm::vec3 pixelPos;
    float noiseValue;
    for (int32_t z = 0; z < imageInfo.textureSize.depth; z++)
    {
        for (int32_t y = 0; y < imageInfo.textureSize.height; y++)
        {
            for (int32_t x = 0; x < imageInfo.textureSize.width; x++)
            {
                pixelPos = glm::vec3(x, y, z);
                noiseValue = noiseGenerator.evaluate(pixelPos) * 255.0f;
                data[x + y * imageInfo.textureSize.width + z * imageInfo.textureSize.width * imageInfo.textureSize.height] = noiseValue;
            }
        }
    }

    // Create a host-visible staging buffer that contains the raw image data
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingMemory;
    m_renderContext->createBuffer(texMemSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingMemory);

    // Copy texture data into staging buffer
    uint8_t *mapped;
    vkMapMemory(m_renderContext->device(), stagingMemory, 0, texMemSize, 0, (void **)&mapped);
    memcpy(mapped, data, texMemSize);
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
    setImageLayout(
        copyCmd,
        imageInfo.Vkimage,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        subresourceRange);

    // Copy 3D noise data to texture

    // Setup buffer copy regions
    VkBufferImageCopy bufferCopyRegion{};
    bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    bufferCopyRegion.imageSubresource.mipLevel = 0;
    bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
    bufferCopyRegion.imageSubresource.layerCount = 1;
    bufferCopyRegion.imageExtent.width = imageInfo.textureSize.width;
    bufferCopyRegion.imageExtent.height = imageInfo.textureSize.height;
    bufferCopyRegion.imageExtent.depth = imageInfo.textureSize.depth;

    vkCmdCopyBufferToImage(
        copyCmd,
        stagingBuffer,
        imageInfo.Vkimage,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &bufferCopyRegion);

    // Change texture image layout to shader read after all mip levels have been copied
    //m_texture.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    setImageLayout(
        copyCmd,
        imageInfo.Vkimage,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        subresourceRange);

    m_renderContext->endSingleTimeCommands(copyCmd);

    // Clean up staging resources
    delete[] data;
    vkFreeMemory(m_renderContext->device(), stagingMemory, nullptr);
    vkDestroyBuffer(m_renderContext->device(), stagingBuffer, nullptr);
}

void NoiseTextureLoader::setImageLayout(
    VkCommandBuffer cmdbuffer,
    VkImage image,
    VkImageLayout oldImageLayout,
    VkImageLayout newImageLayout,
    VkImageSubresourceRange subresourceRange,
    VkPipelineStageFlags srcStageMask,
    VkPipelineStageFlags dstStageMask)
{
    // Create an image barrier object
    VkImageMemoryBarrier imageMemoryBarrier{};
    imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageMemoryBarrier.oldLayout = oldImageLayout;
    imageMemoryBarrier.newLayout = newImageLayout;
    imageMemoryBarrier.image = image;
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
        cmdbuffer,
        srcStageMask,
        dstStageMask,
        0,
        0, nullptr,
        0, nullptr,
        1, &imageMemoryBarrier);
}
