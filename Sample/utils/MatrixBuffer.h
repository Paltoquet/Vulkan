#pragma once

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

class MatrixBuffer
{
public:
    struct BufferData {
        alignas(16) glm::mat4 model;
        alignas(16) glm::mat4 view;
        alignas(16) glm::mat4 proj;
        float time;
    };

public:
    MatrixBuffer();
    ~MatrixBuffer();

public:
    static VkDescriptorSetLayoutBinding descriptorBinding();

public:
    BufferData buffer;
};

