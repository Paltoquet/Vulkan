#pragma once

#include <glm/glm.hpp>

#include <vector>

class BrownianNoise
{
public:
    BrownianNoise(const glm::ivec2& kernelSize, size_t nbLayers);
    ~BrownianNoise();

public:
    float evaluate(const glm::vec2& pos) const;

private:
    float computeNoise(const glm::vec2& pos) const;
    float smoothstep(float val) const;
    float interpolate(float min, float max, float value) const;

    void computeKernel();

private:
    glm::ivec2 m_kernelSize;
    size_t m_nbLayers;
    int m_randomSeed;
    float m_baseFrequency;
    float m_rateOffChanged;

    std::vector<float> m_kernelData;
};

