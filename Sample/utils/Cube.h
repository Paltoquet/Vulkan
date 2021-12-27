#pragma once

#include "Mesh.h"
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <vector>
#include <array>

class Cube : public Mesh
{
public:
    Cube();
    ~Cube();

private:
    void addFaces(const glm::vec3& center, const glm::vec3& up, const glm::vec3& right, const glm::vec3& color);
};
