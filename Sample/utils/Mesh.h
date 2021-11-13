#pragma once

#include <vulkan/vulkan.h>
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
    void loadMesh(const std::string& fileName);
    VkVertexInputBindingDescription getBindingDescription();
    std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions();
    const std::vector<VertexData>& vertices() const;
    const std::vector<uint32_t>& indices() const;

private:
    std::vector<VertexData> m_vertices;
    std::vector<uint32_t> m_indices;
};