#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <memory>

namespace vk_initializer {

    inline VkPipelineVertexInputStateCreateInfo pipeline_vertex_input_state_create_info()
    {
        VkPipelineVertexInputStateCreateInfo pipeline_vertex_input_state_create_info{};
        pipeline_vertex_input_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        return pipeline_vertex_input_state_create_info;
    }

    inline VkPipelineInputAssemblyStateCreateInfo pipeline_input_assembly_state_create_info(
        VkPrimitiveTopology                     topology,
        VkPipelineInputAssemblyStateCreateFlags flags,
        VkBool32                                primitive_restart_enable)
    {
        VkPipelineInputAssemblyStateCreateInfo pipeline_input_assembly_state_create_info{};
        pipeline_input_assembly_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        pipeline_input_assembly_state_create_info.topology = topology;
        pipeline_input_assembly_state_create_info.flags = flags;
        pipeline_input_assembly_state_create_info.primitiveRestartEnable = primitive_restart_enable;
        return pipeline_input_assembly_state_create_info;
    }

    uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);

    void createImage(VkDevice device, VkPhysicalDevice physicalDevice, uint32_t width, uint32_t height, uint32_t mip_levels,
        VkSampleCountFlagBits nbSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
        VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);

    VkImageView createImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mip_levels);
}