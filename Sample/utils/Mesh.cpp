#include "Mesh.h"

Mesh::Mesh():
    m_vertexBuffer(VK_NULL_HANDLE),
    m_indexBuffer(VK_NULL_HANDLE)
{

}

Mesh::~Mesh()
{

}

/* -------------------------- Public methods -------------------------- */

void Mesh::createBuffers(const RenderContext& renderContext)
{
    createVertexBuffer(renderContext);
    createIndexBuffer(renderContext);
}


VkVertexInputBindingDescription Mesh::getBindingDescription()
{
    VkVertexInputBindingDescription bindingDescription{};

    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(VertexData);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return bindingDescription;
}

std::array<VkVertexInputAttributeDescription, 3> Mesh::getAttributeDescriptions()
{
    std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};
    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
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

void Mesh::cleanUp(const RenderContext& renderContext)
{
    if (m_vertexBuffer != VK_NULL_HANDLE) {
        vkDestroyBuffer(renderContext.device(), m_indexBuffer, nullptr);
        vkFreeMemory(renderContext.device(), m_indexBufferMemory, nullptr);
        vkDestroyBuffer(renderContext.device(), m_vertexBuffer, nullptr);
        vkFreeMemory(renderContext.device(), m_vertexBufferMemory, nullptr);
    }
}

/* -------------------------- Protected methods -------------------------- */

void Mesh::createVertexBuffer(const RenderContext& renderContext)
{
    VkDeviceSize bufferSize = sizeof(m_vertices[0]) * m_vertices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    renderContext.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(renderContext.device(), stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, m_vertices.data(), (size_t)bufferSize);
    vkUnmapMemory(renderContext.device(), stagingBufferMemory);

    renderContext.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_vertexBuffer, m_vertexBufferMemory);
    renderContext.copyBuffer(stagingBuffer, m_vertexBuffer, bufferSize);

    vkDestroyBuffer(renderContext.device(), stagingBuffer, nullptr);
    vkFreeMemory(renderContext.device(), stagingBufferMemory, nullptr);
}

void Mesh::createIndexBuffer(const RenderContext& renderContext)
{
    VkDeviceSize bufferSize = sizeof(m_indices[0]) * m_indices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    renderContext.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(renderContext.device(), stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, m_indices.data(), (size_t)bufferSize);
    vkUnmapMemory(renderContext.device(), stagingBufferMemory);

    renderContext.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_indexBuffer, m_indexBufferMemory);
    renderContext.copyBuffer(stagingBuffer, m_indexBuffer, bufferSize);

    vkDestroyBuffer(renderContext.device(), stagingBuffer, nullptr);
    vkFreeMemory(renderContext.device(), stagingBufferMemory, nullptr);
}

/* -------------------------- Getter & Setters -------------------------- */

const std::vector<VertexData>& Mesh::vertices() const
{
    return m_vertices;
}

const std::vector<uint32_t>& Mesh::indices() const
{
    return m_indices;
}

VkBuffer Mesh::vertexBuffer() const
{
    return m_vertexBuffer;
}

VkBuffer Mesh::indexBuffer() const
{
    return m_indexBuffer;
}