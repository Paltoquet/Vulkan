#pragma once

#include <glm/glm.hpp>

#include <vector>
#include <noise/WorleyNoise3D.h>

class CloudGenerator
{
public:
    CloudGenerator(uint32_t width, uint32_t height, uint32_t depth, float randomSeed);
    ~CloudGenerator() = default;

public:
    std::vector<unsigned char> compute3DTexture(float noiseScale);
    float computeFBM(const glm::vec3& pixelPos, float scale);

private:
    WorleyNoise3D m_worleyGenerator;
    float m_randomSeed;
    uint32_t m_fbmLevels;
    uint32_t m_width;
    uint32_t m_height;
    uint32_t m_depth;
};

