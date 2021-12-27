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
        glm::vec4 worldCamera;
        // Should be an array of vec3, but vulkan apparently sucks at indexing arrays of vector3 due to alignment rounded to 16
        // Extension GL_EXT_scalar_block_layout might get it worked out, but why ???????
        alignas(16) glm::vec4 planes[18];
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
