#pragma once

#include "Mesh.h"
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <vector>
#include <array>

class SkinMesh : public Mesh
{
public:
    SkinMesh();
    ~SkinMesh();

public:
    void load(const std::string& fileName);
};