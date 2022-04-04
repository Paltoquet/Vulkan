#pragma once

#include <glm/glm.hpp>

#include <vector>

class CloudGenerator
{
public:
    CloudGenerator(uint32_t width, uint32_t height, uint32_t depth);
    ~CloudGenerator() = default;

public:
    std::vector<unsigned char> compute3DTexture(float noiseScale);

private:
    uint32_t m_width;
    uint32_t m_height;
    uint32_t m_depth;
};

