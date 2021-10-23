#include "Quad.h"


Quad::Quad()
{
    m_vertices = {
        {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
        {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f} , {1.0f, 0.0f}},
        {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f } , {1.0f, 1.0f}},
        {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f} , {0.0f, 1.0f}}
    };

    m_indices = {
        0, 1, 2, 2, 3, 0
    };
}

Quad::~Quad()
{

}

/* -------------------------- Public methods -------------------------- */

VkVertexInputBindingDescription Quad::getBindingDescription()
{
    VkVertexInputBindingDescription bindingDescription{};

    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(VertexData);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return bindingDescription;
}

std::array<VkVertexInputAttributeDescription, 3> Quad::getAttributeDescriptions()
{
    std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};
    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(VertexData, pos);

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(VertexData, color);

    attributeDescriptions[2].binding = 0;
    attributeDescriptions[2].location = 2;
    attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[2].offset = offsetof(VertexData, texCoord);

    return attributeDescriptions;
}

const std::vector<Quad::VertexData>& Quad::vertices() const
{
    return m_vertices;
}

const std::vector<uint16_t>& Quad::indices() const
{
    return m_indices;
}