#include "Cube.h"


Cube::Cube()
{
    addFaces(glm::vec3(0.0, -0.5, 0.0), glm::vec3(0.0, 0.0, 0.5), glm::vec3(0.5, 0.0, 0.0));
    addFaces(glm::vec3(0.0,  0.5, 0.0), glm::vec3(0.0, 0.0, 0.5), glm::vec3(-0.5, 0.0, 0.0));
    addFaces(glm::vec3(0.5,  0.0, 0.0), glm::vec3(0.0, 0.0, 0.5), glm::vec3(0.0, 0.5, 0.0));
    addFaces(glm::vec3(-0.5, 0.0, 0.0), glm::vec3(0.0, 0.0, 0.5), glm::vec3(0.0, -0.5, -0.0));
    addFaces(glm::vec3(0.0,  0.0, 0.5), glm::vec3(0.0, 0.5, 0.0), glm::vec3(0.5, 0.0, 0.0));
    addFaces(glm::vec3(0.0, 0.0, -0.5), glm::vec3(0.0, -0.5, 0.0), glm::vec3(0.5, 0.0, 0.0));
}

Cube::~Cube()
{

}

/* -------------------------- Private methods -------------------------- */

void Cube::addFaces(const glm::vec3& center, const glm::vec3& up, const glm::vec3& right)
{
    static std::vector<glm::vec3> colors = { { 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } };
    static std::vector<glm::vec2> uvs = { { 0.0f, 0.0f },{ 1.0f, 0.0f },{1.0f, 1.0f}, {0.0f, 1.0f} };

    uint32_t startIndex = m_vertices.size();

    glm::vec3 topLeft     = center + up - right;
    glm::vec3 topRight    = center + up + right;
    glm::vec3 bottomLeft  = center - up - right;
    glm::vec3 bottomRight = center - up + right;

    m_vertices.push_back(VertexData(topLeft, colors.at(0), uvs.at(0)));
    m_vertices.push_back(VertexData(topRight, colors.at(1), uvs.at(1)));
    m_vertices.push_back(VertexData(bottomRight, colors.at(2), uvs.at(2)));
    m_vertices.push_back(VertexData(bottomLeft, colors.at(3), uvs.at(3)));

    // Face 0
    m_indices.push_back(startIndex);
    m_indices.push_back(startIndex + 1);
    m_indices.push_back(startIndex + 2);
    // Face 1
    m_indices.push_back(startIndex + 2);
    m_indices.push_back(startIndex + 3);
    m_indices.push_back(startIndex);
}