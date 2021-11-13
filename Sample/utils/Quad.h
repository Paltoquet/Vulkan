#pragma once

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <vector>
#include <array>

class Quad
{
public:
    struct VertexData {
        glm::vec3 pos;
        glm::vec3 color;
        glm::vec2 texCoord;
    };

public:
    Quad();
    ~Quad();

public:
    VkVertexInputBindingDescription getBindingDescription();
    std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions();
    const std::vector<VertexData>& vertices() const;
    const std::vector<uint16_t>& indices() const;

private:
    std::vector<VertexData> m_vertices;
    std::vector<uint16_t> m_indices;
};
