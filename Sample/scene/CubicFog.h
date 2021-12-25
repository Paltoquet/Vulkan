#pragma once

#include <utils/Cube.h>
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <vector>
#include <array>

class CubicFog
{
public:
    CubicFog();
    ~CubicFog();

public:
    struct CloudData {
        alignas(16) glm::vec4 worldCamera;
        alignas(16) glm::vec3 planes[24];
    };

public:
    Cube& mesh();
    const VkDescriptorSetLayoutBinding& descriptorBinding() const;
    CloudData& shaderData();

private:
    Cube m_mesh;
    CloudData m_shaderData;
    VkDescriptorSetLayoutBinding m_descriptorBinding;
};
