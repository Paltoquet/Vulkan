#include "Cube.h"


Cube::Cube(const glm::mat3& transfo):
    Mesh::Mesh(),
    m_vertexTransfo(transfo),
    m_bboxMin(-0.5f, -0.5, -0.5f),
    m_bboxMax(0.5f, 0.5f, 0.5f)
{
    glm::vec3 red = glm::vec3(1.0, 0.1, 0.1);
    glm::vec3 blue = glm::vec3(0.0, 0.6, 0.8);
    glm::vec3 grey = glm::vec3(0.4, 0.4, 0.4);
    glm::vec3 orange = glm::vec3(0.8, 0.4, 0.0);
    glm::vec3 purple = glm::vec3(0.8, 0.0, 0.8);

    addFaces(glm::vec3(0.0, -0.5, 0.0), glm::vec3(0.0, 0.0, 0.5), glm::vec3(0.5, 0.0, 0.0), red);
    addFaces(glm::vec3(0.0,  0.5, 0.0), glm::vec3(0.0, 0.0, 0.5), glm::vec3(-0.5, 0.0, 0.0), orange);
    addFaces(glm::vec3(0.5,  0.0, 0.0), glm::vec3(0.0, 0.0, 0.5), glm::vec3(0.0, 0.5, 0.0), purple);
    addFaces(glm::vec3(-0.5, 0.0, 0.0), glm::vec3(0.0, 0.0, 0.5), glm::vec3(0.0, -0.5, -0.0), blue);
    addFaces(glm::vec3(0.0,  0.0, 0.5), glm::vec3(0.0, 0.5, 0.0), glm::vec3(0.5, 0.0, 0.0), grey);
    addFaces(glm::vec3(0.0, 0.0, -0.5), glm::vec3(0.0, -0.5, 0.0), glm::vec3(0.5, 0.0, 0.0), grey);

    m_bboxMin = m_vertexTransfo * m_bboxMin;
    m_bboxMax = m_vertexTransfo * m_bboxMax;
}

Cube::~Cube()
{

}

/* -------------------------- Public methods -------------------------- */

glm::vec3 Cube::bboxMin() const
{
    return m_bboxMin;
}

glm::vec3 Cube::bboxMax() const
{
    return m_bboxMax;
}

/* -------------------------- Private methods -------------------------- */

void Cube::addFaces(const glm::vec3& center, const glm::vec3& up, const glm::vec3& right, const glm::vec3& color)
{
    static std::vector<glm::vec2> uvs = { { 0.0f, 0.0f },{ 1.0f, 0.0f },{1.0f, 1.0f}, {0.0f, 1.0f} };

    uint32_t startIndex = m_vertices.size();

    glm::vec3 topLeft     = m_vertexTransfo * (center + up - right);
    glm::vec3 topRight    = m_vertexTransfo * (center + up + right);
    glm::vec3 bottomLeft  = m_vertexTransfo * (center - up - right);
    glm::vec3 bottomRight = m_vertexTransfo * (center - up + right);

    m_vertices.push_back(VertexData(topLeft, color, uvs.at(0)));
    m_vertices.push_back(VertexData(topRight, color, uvs.at(1)));
    m_vertices.push_back(VertexData(bottomRight, color, uvs.at(2)));
    m_vertices.push_back(VertexData(bottomLeft, color, uvs.at(3)));

    // Face 0
    m_indices.push_back(startIndex);
    m_indices.push_back(startIndex + 1);
    m_indices.push_back(startIndex + 2);
    // Face 1
    m_indices.push_back(startIndex + 2);
    m_indices.push_back(startIndex + 3);
    m_indices.push_back(startIndex);
}