#pragma once

#include "Mesh.h"
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <vector>
#include <array>

class Cube : public Mesh
{
public:
    Cube(const glm::mat3& transfo);
    ~Cube();

public:
    glm::vec3 bboxMin() const;
    glm::vec3 bboxMax() const;

private:
    void addFaces(const glm::vec3& center, const glm::vec3& up, const glm::vec3& right, const glm::vec3& color);

private:
    glm::mat3 m_vertexTransfo;
    glm::vec3 m_bboxMin;
    glm::vec3 m_bboxMax;
};
