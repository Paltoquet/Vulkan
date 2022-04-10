#pragma once

#include <glm/glm.hpp>

#include <vector>

class BrownianNoise3D
{
public:
    BrownianNoise3D(const glm::ivec3& kernelSize, size_t nbLayers, float randomSeed);
    ~BrownianNoise3D();

public:
    float evaluate(const glm::vec3& pos) const;

private:
    float computeNoise(const glm::vec3& pos) const;
    float smoothstep(float val) const;
    float interpolate(float min, float max, float value) const;

    void computeKernel();

private:
    glm::ivec3 m_kernelSize;
    size_t m_nbLayers;
    int m_randomSeed;
    float m_baseFrequency;
    float m_rateOffChanged;

    std::vector<float> m_kernelData;
};

