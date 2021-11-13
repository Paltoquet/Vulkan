#pragma once

#include <vulkan/vulkan.h>

#include <string>

class ShaderLoader
{
public:
    ShaderLoader();
    ~ShaderLoader();

public:
    static VkShaderModule loadShader(const std::string& filename, VkDevice device);
};

