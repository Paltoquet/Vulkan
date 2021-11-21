#pragma once

#include <core/RenderContext.h>
#include <glm/glm.hpp>
#include <vector>
#include <array>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

struct VertexData {
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 texCoord;

    bool operator==(const VertexData& other) const {
        return pos == other.pos && color == other.color && texCoord == other.texCoord;
    }
};

namespace std {
    template<> struct hash<VertexData> {
        size_t operator()(VertexData const& vertex) const {
            return ((hash<glm::vec3>()(vertex.pos) ^
                (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
                (hash<glm::vec2>()(vertex.texCoord) << 1);
        }
    };
}

class Mesh
{
public:
    Mesh();
    ~Mesh();

public:
    void createBuffers(const RenderContext& renderContext);
    virtual void cleanUp(const RenderContext& renderContext);
    virtual VkVertexInputBindingDescription getBindingDescription();
    virtual std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions();

    const std::vector<VertexData>& vertices() const;
    const std::vector<uint32_t>& indices() const;
    VkBuffer vertexBuffer() const;
    VkBuffer indexBuffer() const;

protected:
    virtual void createVertexBuffer(const RenderContext& renderContext);
    virtual void createIndexBuffer(const RenderContext& renderContext);

protected:
    std::vector<VertexData> m_vertices;
    std::vector<uint32_t> m_indices;

    VkBuffer m_vertexBuffer;
    VkDeviceMemory m_vertexBufferMemory;
    VkBuffer m_indexBuffer;
    VkDeviceMemory m_indexBufferMemory;
};