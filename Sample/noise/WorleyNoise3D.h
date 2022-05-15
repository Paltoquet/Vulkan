#pragma once

#include <glm/glm.hpp>

#include <vector>

class WorleyNoise3D
{
public:
    WorleyNoise3D(const glm::ivec3& kernelSize);
    WorleyNoise3D(const glm::ivec3& kernelSize, float randomSeed);

    ~WorleyNoise3D();

public:
    float evaluate(const glm::vec3& pos, float scale) const;

private:
    float computeNoise(const glm::vec3& pos) const;
    float smoothstep(float val) const;
    float interpolate(float min, float max, float value) const;

    void computeKernel();
    glm::vec3 getKernelData(const glm::vec3& pos) const;

private:
    glm::vec3 m_kernelSize;
    glm::vec3 m_kernelOffset;
    int m_randomSeed;

    std::vector<glm::vec3> m_kernelData;
};

