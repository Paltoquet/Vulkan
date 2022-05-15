#pragma once

#include <glm/glm.hpp>

#include <vector>

class WorleyNoise2D
{
public:
    WorleyNoise2D(const glm::vec2& kernelSize);
    WorleyNoise2D(const glm::vec2& kernelSize, float randomSeed);
    ~WorleyNoise2D();

public:
    float evaluate(const glm::vec2& pos, float scale) const;

private:
    void computeKernel();
    glm::vec2 getKernelData(const glm::vec2& pos) const;

private:
    glm::vec2 m_kernelSize;
    glm::vec2 m_kernelOffset;
    int m_randomSeed;

    std::vector<glm::vec2> m_kernelData;
};

